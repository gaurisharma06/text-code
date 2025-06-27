#include <Wire.h>
#include <Arduino.h>
#include <WiFi.h>
#include "address.h"
const byte rxPin = 16;  // rx2
const byte txPin = 17;  // tx2
int TH_Drops = 0;
int TA_Drops = 0;

int TH_B_C = 0;
int TA_C_D = 0;

HardwareSerial dwinSerial(1);
void setup() {
  Serial.begin(115200);
  dwinSerial.begin(115200, SERIAL_8N1, rxPin, txPin);
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:
   readScreen();

}

void readScreen(){
    unsigned char Buffer[9];
  if (dwinSerial.available()) {

    for (int i = 0; i <= 8; i++)  // this loop will store whole frame in buffer array.
    {
      Buffer[i] = dwinSerial.read();
    }

    if (Buffer[4] == 0x01 && Buffer[5] == 0x10) {
      dwinSerial.write(test_page_change, 10);
    } 
    else if (Buffer[4] == turbidity_test_add_first && Buffer[5] == turbidity_test_add_second) {
      int intValue = 4*100;
      Serial.println("true");
      turbidity_result_reading[6] = highByte(intValue);
      turbidity_result_reading[7] = lowByte(intValue);
      delay(3000);
      for(int i=0;i<8;i++){
      Serial.println(turbidity_result_reading[i],HEX);
      }
      dwinSerial.write(turbidity_page_change, 10);
      dwinSerial.write(turbidity_result_reading, 8);
    } else if (Buffer[4] == frc_test_add_first && Buffer[5] == frc_test_add_second) {
      int intValue = 1;
      frc_result_reading[6] = highByte(intValue);
      frc_result_reading[7] = lowByte(intValue);
      dwinSerial.write(frc_result_reading, 8);
      dwinSerial.write(frc_page_change, 10);
    } else if (Buffer[4] == tds_test_add_first && Buffer[5] == tds_test_add_second) {
      float intValue = 200;
      setTextDwin(0x5020, String(intValue) + "         ");
      dwinSerial.write(tds_page_change, 10);
    } else if (Buffer[4] == iron_test_add_first && Buffer[5] == iron_test_add_second) {
      int intValue = 1;
      iron_result_reading[6] = highByte(intValue);
      iron_result_reading[7] = lowByte(intValue);
      dwinSerial.write(iron_page_change, 10);
      dwinSerial.write(iron_result_reading, 8);
    } else if (Buffer[4] == ph_test_add_first && Buffer[5] == ph_test_add_second) {
      int intValue = 12*100;
      Serial.println("true");
      ph_result_reading[6] = highByte(intValue);
      ph_result_reading[7] = lowByte(intValue);
      delay(3000);
      dwinSerial.write(ph_page_change, 10);
      dwinSerial.write(ph_result_reading, 8);
    } else if (Buffer[4] == th_test_add_first && Buffer[5] == th_test_add_second) {
      float intValue = 500;
      setTextDwin(th_result_add, String(intValue));
    } else if (Buffer[4] == ta_test_add_first && Buffer[5] == ta_test_add_second) {
      float intValue = 400;
      setTextDwin(ta_result_add, String(intValue));
    } else if (Buffer[4] == nitrate_test_add_first && Buffer[5] == nitrate_test_add_second) {
      int intValue = 40;
      nitrate_result_reading[6] = highByte(intValue);
      nitrate_result_reading[7] = lowByte(intValue);
      dwinSerial.write(nitrate_page_change, 10);
      dwinSerial.write(nitrate_result_reading, 8);
    } else if (Buffer[4] == flouride_test_add_first && Buffer[5] == flouride_test_add_second) {
      int intValue = 1;
      flouride_result_reading[6] = highByte(intValue);
      flouride_result_reading[7] = lowByte(intValue);
      dwinSerial.write(flouride_page_change, 10);
      dwinSerial.write(flouride_result_reading, 8);
    } else if (Buffer[4] == copper_test_add_first && Buffer[5] == copper_test_add_second) {
      int intValue = 1;
      copper_result_reading[6] = highByte(intValue);
      copper_result_reading[7] = lowByte(intValue);
      dwinSerial.write(copper_page_change, 10);
      dwinSerial.write(copper_result_reading, 8);
    } else if (Buffer[4] == ec_test_add_first && Buffer[5] == ec_test_add_second) {
      float intValue = 350;
      setTextDwin(0x6000, String(intValue) + "       ");
      dwinSerial.write(ec_page_change, 10);
    }


    // TH and TA add drops
    else if (Buffer[4] == TA_incremental_decremental_add_first && Buffer[5] == TA_incremental_decremental_add_second) {
        TA_Drops = Buffer[8];
    } else if (Buffer[4] == TH_incremental_decremental_add_first && Buffer[5] == TH_incremental_decremental_add_second) {
        TH_Drops = Buffer[8];
    }
    // TH and TA select B C D
    else if (Buffer[4] == 0x00 && Buffer[5] == TA_C_add) {
      TA_C_D = 0;
    } else if (Buffer[4] == 0x00 && Buffer[5] == TA_D_add) {
      TA_C_D = 1;
    } else if (Buffer[4] == 0x00 && Buffer[5] == TH_B_add) {
      TH_B_C = 0;
    } else if (Buffer[4] == 0x00 && Buffer[5] == TH_C_add ) {
      TH_B_C = 1;
    }
}
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

