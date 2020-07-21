#include "Arduino.h"
#include "sockMng.h"


WebSocketsServer webSocket = WebSocketsServer(81);

bool newRpmSetpoint = false;
uint32_t rpmSetpoint = 0;

bool newPwm1Setpoint = false;
uint32_t pwm1Setpoint = 0;

bool newTankSwReq = false;
uint32_t tankSwReqCount = 0;

void websocket_begin(){
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
}

void websocket_manage() {
    webSocket.loop();
}

void broadcastJsonById(char* id, uint32_t v){ 
    DynamicJsonDocument doc(1024);
    String pl;
    doc["ID"] = id;
    doc["data"] = v;
    serializeJson(doc, pl);
    webSocket.broadcastTXT(pl);
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

    switch(type) {
        case WStype_DISCONNECTED:
            Serial.printf("[%u] Disconnected!\n", num);
            break;
        case WStype_CONNECTED:
            {
                IPAddress ip = webSocket.remoteIP(num);
                Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

        // send message to client
        webSocket.sendTXT(num, "Connected");
            }
            break;
        case WStype_TEXT:
            {
              Serial.printf("[%u] get Text: %s\n", num, payload);
  
              DynamicJsonDocument received_doc(1024);
              auto error1 = deserializeJson(received_doc, payload);
              // serializeJson(received_doc, Serial);
              // Serial.println();
  
              if (error1) {
                Serial.print(F("Error1: deserializeJson() failed with code "));
                Serial.println(error1.c_str());
                return;
              }
  
              String r_id = received_doc["ID"];
              String r_data = received_doc["data"];
  
              if (r_id.equalsIgnoreCase("set_rpm")){
                rpmSetpoint = atol(received_doc["data"]);
                newRpmSetpoint = true;
                // Serial.print("NEW RPM SETPOINT: ");
                // Serial.println(rpmSetpoint);
              }
              if (r_id.equalsIgnoreCase("tank_sw")){
                newTankSwReq = true;
                tankSwReqCount = tankSwReqCount + 1;
                // Serial.println("New Tank SW");
              }
              if (r_id.equalsIgnoreCase("set_pwm1")){
                newPwm1Setpoint = true;
                pwm1Setpoint = atol(received_doc["data"]);
                // Serial.print("New PWM request: ");
                // Serial.println(pwm1Setpoint);
              }
            }
            break;
        case WStype_BIN:
            Serial.printf("[%u] get binary length: %u\n", num, length);
            hexdump(payload, length);

            // send message to client
            // webSocket.sendBIN(num, payload, length);
            break;
    case WStype_ERROR:      
    case WStype_FRAGMENT_TEXT_START:
    case WStype_FRAGMENT_BIN_START:
    case WStype_FRAGMENT:
    case WStype_FRAGMENT_FIN:
      break;
    }
}

void hexdump(const void *mem, uint32_t len, uint8_t cols) {
  const uint8_t* src = (const uint8_t*) mem;
  Serial.printf("\n[HEXDUMP] Address: 0x%08X len: 0x%X (%d)", (ptrdiff_t)src, len, len);
  for(uint32_t i = 0; i < len; i++) {
    if(i % cols == 0) {
      Serial.printf("\n[0x%08X] 0x%08X: ", (ptrdiff_t)src, i);
    }
    Serial.printf("%02X ", *src);
    src++;
  }
  Serial.printf("\n");
}


bool isNewRpmSetpoint(){
  return newRpmSetpoint;
}

uint32_t getWebRpmSetpoint(){
  newRpmSetpoint = false;
  return rpmSetpoint;
}

bool isNewTankSw(){
  return newTankSwReq;
}

uint32_t getTankSwReqCount(){
  uint32_t aux = tankSwReqCount;
  newTankSwReq = false;
  tankSwReqCount = 0;
  return aux;;
}

bool isNewPwm1Setpoint(){
  return newPwm1Setpoint;
}

uint32_t getPwm1Setpoint(){
  newPwm1Setpoint = false;
  return pwm1Setpoint;
}
