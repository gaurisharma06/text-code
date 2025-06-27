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

}
