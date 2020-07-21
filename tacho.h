#ifndef tacho_h
#define tacho_h

  #include <Arduino.h>                            // if not included --> PAIN. No idea
  #include "sockMng.h"

  // Includes to manage WDT reset for crank task
  #include "soc/timer_group_struct.h"
  #include "soc/timer_group_reg.h"  
  
  #define RPM_POT_PIN 34

  #define CRANK_N_PIN 25
  #define CRANK_P_PIN 33
  
  #define MAX_RPM 8000                // in rpm

  #define MAX_ADC 4095               // ESP32
  //#define MAX_ADC 1024             // ARDUINO MEGA
  
  #define RPM_POT_REFRESH 0           // in ms
  
  #define CRANK_TEETH 60
  #define CRANK_MISSING_TEETH 2
            
  #define N_PIN_STATUS HIGH

  #define PWM1_FREQ 5000                // uint32_t freq_hz
  #define PWM1_RES 13 
  #define PWM1_PIN 26
  #define PWM1_CHANNEL 0
  
  #define PWM1_INIT_VAL 0

  void tacho_begin();
  void tacho_manage();
  void setRpmSetpoint(uint32_t rpm);
  void setPwmSetpoint(uint32_t pwm);

  uint32_t calcToothTime(uint32_t rpm);
  uint32_t getRpmSetpoint();
  uint32_t getToothTime();

  // DEPRECATED
  void crank_manage(void * pvParameters);
  void refreshCrank();
  void refreshRpmPot();

#endif
