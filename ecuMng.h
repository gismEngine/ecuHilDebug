#ifndef ecuMng_h
#define ecuMng_h

  #include <Arduino.h>
  #include "sockMng.h"
  #include "tacho.h"

  #define DEBUG Serial      // Serial to USB
  #define ECU Serial2       // On ESP32 Serial1 pins are used for flash memori. Use Serial2 instead :)

  #define RX_BUFF_SIZE 20

  #define ACC_SAMPLE_T    500
  #define VBAT_SAMPLE_T   1000
  #define RPM_SAMPLE_T    250
  #define SWITCH_SAMPLE_T 1000
  #define TURBO_SAMPLE_T  1000
  #define TEXA1_SAMPLE_T  1000

  #define ACC_ID 0x61
  #define VBAT_ID 0x62
  #define RPM_ID 0x72
  #define SWITCH_ID 0x73
  #define TURBO_ID 0x74
  #define TEXA1_ID 0x75
  

  // GENERIC FRAME:
  struct Frame {
      uint8_t request[4];
      uint8_t request_size = 4;
      uint8_t response[RX_BUFF_SIZE];
      uint8_t response_received = 0;
      uint8_t response_size = 0;
      uint8_t id = 0;
  };

  void ecu_begin();
  void ecu_manage();

  void sendTankSwCmd();

  // Intended private
  void initFrames();
  void checkRxFrames();
  void processFrame(Frame *f);
  void sendFrame(Frame *f);
  void printHuman(Frame *f);
  
#endif
