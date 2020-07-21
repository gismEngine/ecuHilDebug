#include "tacho.h"
#include "webMng.h"
#include "sockMng.h"
#include "ecuMng.h"
#include <Esp.h>

const char* PROGMEM FIRMWARE_NAME = "DGID_DIAG";
const char* PROGMEM FIRMWARE_VER = "0.1";

#define DEBUG Serial

void setup() {
  DEBUG.begin(115200);   // USB
  
  DEBUG.print(F("\n\n\n"));                         //  Print firmware name and version const string
  DEBUG.print(FIRMWARE_NAME);
  DEBUG.print(F(" - "));
  DEBUG.println(FIRMWARE_VER);

  DEBUG.print(F("CPU@ "));      
  DEBUG.print(getCpuFrequencyMhz());    // In MHz
  DEBUG.println(F(" MHz"));

  DEBUG.print(F("Xtal@ "));      
  DEBUG.print(getXtalFrequencyMhz());    // In MHz
  DEBUG.println(F(" MHz"));

  DEBUG.print(F("APB@ "));      
  DEBUG.print(getApbFrequency());        // In Hz
  DEBUG.println(F(" Hz"));

  EspClass e;
  
  DEBUG.print(F("SDK v: "));
  DEBUG.println(e.getSdkVersion());

  DEBUG.print(F("Flash size: "));
  DEBUG.println(e.getFlashChipSize());
 
  DEBUG.print(F("Flash Speed: "));
  DEBUG.println(e.getFlashChipSpeed());
 
  DEBUG.print(F("Chip Mode: "));
  DEBUG.println(e.getFlashChipMode());

  DEBUG.print(F("Sketch size: "));
  DEBUG.println(e.getSketchSize());

  DEBUG.print(F("Sketch MD5: "));
  DEBUG.println(e.getSketchMD5());

  ecu_begin();
  tacho_begin();

  webMng.begin(FIRMWARE_NAME, FIRMWARE_VER);        // Init Wifi-AP + Webserver + WebSocket + DNSserver
  websocket_begin();
}

uint32_t alive = 0;
uint32_t c_alive = 0;

void loop() {

//  if (millis() >= alive + 1000){
//    broadcastJsonById("alive!", c_alive);
//    c_alive = c_alive + 1;
//    alive = millis();
//  }
  
  webMng.manage();         // Do wifi stuff
  websocket_manage();      // Do websocket stuff
  
  tacho_manage();          // Do RPM stuff
  ecu_manage();            // Do ECU-communication stuff

  // Check if new RPM is set on web UI/API (by websocket)
  // Set new engine speed setpoint
  if(isNewRpmSetpoint()){
    uint32_t rpm_sp = getWebRpmSetpoint();
    setRpmSetpoint(rpm_sp);
    DEBUG.print("NEW RPM! ");
    DEBUG.println(rpm_sp);
  }

  // Check if new PWM1 is set on web UI/API (by websocket)
  // Set new PWM1 setpoint
  if(isNewPwm1Setpoint()){
    uint32_t pwm1_sp = getPwm1Setpoint();
    setPwmSetpoint(pwm1_sp);
  }

  // Check if new Tank SW request set on web UI/API (by websocket)
  // If number of request is odd send command to change status
  if(isNewTankSw()){
    uint32_t reqCount = getTankSwReqCount();
    if (reqCount % 2 != 0){
      sendTankSwCmd();
    }
  }
   
}
