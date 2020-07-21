#ifndef sockMng_h
#define sockMng_h

  #include <Arduino.h>
  #include <WebSocketsServer.h>

  #include <ArduinoJson.h>          // https://github.com/bblanchon/ArduinoJson

  void websocket_begin();
  void websocket_manage();
  void broadcastJsonById(char* id, uint32_t v);

  bool isNewRpmSetpoint();
  uint32_t getWebRpmSetpoint();

  bool isNewPwm1Setpoint();
  uint32_t getPwm1Setpoint();

  bool isNewTankSw();
  uint32_t getTankSwReqCount();
  
  void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length);
  void hexdump(const void *mem, uint32_t len, uint8_t cols = 16);
  
#endif
