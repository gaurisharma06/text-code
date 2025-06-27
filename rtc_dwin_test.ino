#include "RTClib.h"
RTC_PCF8563 rtc;

const byte rx_pin=16;
const byte tx_pin=17;

HardwareSerial dwinSerial(1);

#define MAX_DWIN_MSG_SIZE 32

uint8_t* getDWINMessage(uint8_t &msgLen) {
  static uint8_t buffer[MAX_DWIN_MSG_SIZE];
  msgLen = 0;
  
  // Check if at least 3 bytes are available (header and length)
  if (dwinSerial.available() < 3) {
    return NULL;
  }
  
  // Synchronize: search for the header 0x5A followed by 0xA5
  while (dwinSerial.available() >= 2) {
    if (dwinSerial.peek() == 0x5A) {
      // Read first header byte
      uint8_t byte1 = dwinSerial.read();
      // Check if the next byte is 0xA5
      if (dwinSerial.available() < 1) {
        return NULL; // Wait for next byte if not available yet
      }
      if (dwinSerial.peek() == 0xA5) {
        uint8_t byte2 = dwinSerial.read();
        // Now read the length byte (which tells how many bytes follow)
        while (dwinSerial.available() < 1) {
          ; // Wait until length byte is available
        }
        uint8_t len = dwinSerial.read();
        uint8_t totalSize = 3 + len;  // header (2) + length (1) + payload
        
        // Ensure the message size does not exceed our buffer
        if (totalSize > MAX_DWIN_MSG_SIZE) {
          // Flush the available data if message is too large
          while (dwinSerial.available()) {
            dwinSerial.read();
          }
          return NULL;
        }
        
        // Wait until the full payload is available
        while (dwinSerial.available() < len) {
          ; // Optionally add a timeout here in production code.
        }
        
        // Store the header, length, and payload in the buffer
        buffer[0] = 0x5A;
        buffer[1] = 0xA5;
        buffer[2] = len;
        for (uint8_t i = 0; i < len; i++) {
          buffer[3 + i] = dwinSerial.read();
        }
        msgLen = totalSize;
        return buffer;
      } else {
        // Not a valid header sequence, discard the 0x5A and continue
        dwinSerial.read();
      }
    } else {
      // Discard bytes until header is found
      dwinSerial.read();
    }
  }
  
  return NULL;
}

void setup() {
  Serial.begin(115200);
  dwinSerial.begin(115200, SERIAL_8N1, rx_pin, tx_pin);

    if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
  }

  if (rtc.lostPower()) {
    // Serial.println("RTC lost power, setting the time!");
    // Set the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(2025,3,25,1,0,1));
  }

  if (! rtc.isrunning()) {
  Serial.println("RTC is NOT running, let's set the time!");
  // When time needs to be set on a new device, or after a power loss, the
  // following line sets the RTC to the date & time this sketch was compiled
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  // This line sets the RTC with an explicit date & time, for example to set
  // January 21, 2014 at 3am you would call:
  // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }
}

void loop() {
  uint8_t msgLen = 0;
  uint8_t* msg = getDWINMessage(msgLen);
  delay(10);

   if (msg != NULL) {
    Serial.print("Received: ");
    for (uint8_t i = 0; i < msgLen; i++) {
      Serial.print(msg[i], HEX); // Printing the buffer received
      Serial.print(" ");
    }
    Serial.println();
}

 DateTime now = rtc.now();

  sendTextToDWIN(0x7600, String(now.hour()) + ":" + (now.minute() < 10 ? "0" + String(now.minute()) : String(now.minute())));
  unsigned char turbidity_result_reading[8] = { 0x5a, 0xa5, 0x05, 0x82, 0x78, 0x00, 0x00, 0x05};
  dwinSerial.write(turbidity_result_reading,8);

}
void setTextDwin(uint16_t vpAddress, String text) {
  uint8_t buffer[100];  // Buffer to hold the command
  int len = text.length();

  // Construct the command frame
  buffer[0] = 0x5A;                     // Start byte 1
  buffer[1] = 0xA5;                     // Start byte 2
  buffer[2] = len + 3;                  // Data length (text length + 3 for the address and control bytes)
  buffer[3] = 0x82;                     // Command for writing to a VP address
  buffer[4] = (vpAddress >> 8) & 0xFF;  // VP address high byte
  buffer[5] = vpAddress & 0xFF;         // VP address low byte

  // Copy the text data into the buffer
  for (int i = 0; i < len; i++) {
    buffer[6 + i] = text[i];
  }

  // Send the buffer to the DWIN screen
  dwinSerial.write(buffer, len + 6);
}

void stringToUtf16BE(String text, uint8_t* outBytes, size_t& outLen) {
    outLen = 0;
    for (size_t i = 0; i < text.length(); i++) {
        outBytes[outLen++] = 0x00;          // High byte
        outBytes[outLen++] = text[i];       // Low byte
    }
    // Optional: Add a UTF-16 null terminator (0x00, 0x00)
    outBytes[outLen++] = 0x00;
    outBytes[outLen++] = 0x00;
}

// Function to send a String to the DWIN display
void sendTextToDWIN(uint16_t vpAddr, String text) {
    // 1) Convert String to UTF-16 BE
    static uint8_t utf16Buf[128];  // Buffer to hold converted text
    size_t utfLen = 0;
    stringToUtf16BE(text, utf16Buf, utfLen);

    // 2) Build command frame
    uint8_t cmd[256];
    cmd[0] = 0x5A;  // DWIN Header
    cmd[1] = 0xA5;
    uint16_t length = 1 + 2 + utfLen;  // Command + Addr (2 bytes) + UTF-16 data
    cmd[2] = (uint8_t)length;
    cmd[3] = 0x82; // Write command
    cmd[4] = (vpAddr >> 8) & 0xFF;
    cmd[5] = vpAddr & 0xFF;

    // Copy UTF-16 big-endian text to command frame
    memcpy(&cmd[6], utf16Buf, utfLen);

    // 3) Send data to DWIN display
    size_t totalLen = 6 + utfLen;
    dwinSerial.write(cmd, totalLen);
}