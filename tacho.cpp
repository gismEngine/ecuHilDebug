#include "Arduino.h"
#include "tacho.h"

uint32_t rpm_setpoint = 0;
uint32_t rpm_pot_sample_time = 0;

uint32_t tooth_time = 0;
uint32_t crank_sample_time = 0;
uint8_t crank_teeth_i = 0;
bool crank_out = LOW;


hw_timer_t * timer = NULL;

// This is hardcore interrupt code.
// But this is the only way I found to avoid RPM oscilations
void IRAM_ATTR onTimer() {
  if (crank_teeth_i < (CRANK_TEETH - CRANK_MISSING_TEETH) * 2){
    crank_out = !crank_out;
    digitalWrite(CRANK_P_PIN, crank_out);
  }
  
  crank_teeth_i = crank_teeth_i + 1;
  if (crank_teeth_i > CRANK_TEETH * 2) crank_teeth_i = 0;
}


void tacho_begin(){
  pinMode(CRANK_N_PIN, OUTPUT);
  pinMode(CRANK_P_PIN, OUTPUT);

  digitalWrite(CRANK_N_PIN, N_PIN_STATUS);
  digitalWrite(CRANK_P_PIN, LOW);

  ledcSetup(PWM1_CHANNEL, PWM1_FREQ, PWM1_RES);
  ledcAttachPin(PWM1_PIN, PWM1_CHANNEL);
  
  ledcWrite(PWM1_CHANNEL, PWM1_INIT_VAL);

  // Create task to manage crank signal (instead of timer interrupt)
//  xTaskCreatePinnedToCore(crank_manage, "crankTask", 10000, NULL, 9, NULL,  0); 
//  delay(500);
//  crank_sample_time = micros();

  // Create timer with 80 prescaler with auto-reload (trigger each microsecond)
  timer = timerBegin(0, 80, true);

  //refreshRpmPot();
  setRpmSetpoint(rpm_setpoint);
}


void tacho_manage(){
  
  // crank_manage();   // No longer needed. Managed as a timer interrupt. 
                       // This is for polling configuration.
                       // Also this task can be set as a task by RTOS (check setup comments)


// Interface to set RPM setpoint by potentiometer (ADC sampling)
// No longer used (websocket interface instead)
//  if (RPM_POT_REFRESH > 0){
//    if (millis() > rpm_pot_sample_time){
//      refreshRpmPot();
//      rpm_pot_sample_time = rpm_pot_sample_time + RPM_POT_REFRESH;
//    }
//  }

}

uint32_t calcToothTime(uint32_t rpm){
  // Each engine revolution has CRANK_TEETH
  // Change rpm (minutes) to us
  return (60.0 * 1000.0 * 1000.0) / (rpm * CRANK_TEETH * 2);
}

uint32_t getRpmSetpoint(){
  return rpm_setpoint;
}

uint32_t getToothTime(){
  return tooth_time;
}

void setRpmSetpoint(uint32_t rpm){
  rpm_setpoint = rpm;
  tooth_time = calcToothTime(rpm_setpoint);

  timerAlarmDisable(timer);
  timerDetachInterrupt(timer);

  if (rpm_setpoint != 0){                           // There is no need for the IF. Somehow it is working anyway
    timerAttachInterrupt(timer, &onTimer, true);
    timerAlarmWrite(timer, tooth_time, true);
    timerAlarmEnable(timer);
  }

//  Serial.print(rpm_setpoint);
//  Serial.print("\t");
//  Serial.println(tooth_time);
}


void setPwmSetpoint(uint32_t pwm){
  if (pwm <= 0x2000) {  // ONLY 13 bits. 0x2000 is max value
    ledcWrite(PWM1_CHANNEL, pwm);
  }else{
    ledcWrite(PWM1_CHANNEL, 0x2000);
  }
}





















// If task is used instead of timer interrupt
// Now is dead code and can be removed
void crank_manage(void * pvParameters){

  for(;;){
    
    TIMERG0.wdt_wprotect=TIMG_WDT_WKEY_VALUE;
    TIMERG0.wdt_feed=1;
    TIMERG0.wdt_wprotect=0;
  
    if(micros() > crank_sample_time){
      if (tooth_time != 0xFFFFFFFF) {
        refreshCrank();
        crank_sample_time = crank_sample_time + tooth_time;
      }
//    Serial.print("ct: ");
//    Serial.print(crank_sample_time);
//    Serial.print("\ttt: ");
//    Serial.print(tooth_time);
//    Serial.print("\trpm: ");
//    Serial.println(rpm_setpoint);
    }
  }
}

// If task is used instead of timer interrupt
// Now is dead code and can be removed
void refreshCrank(){

  if(tooth_time == 0) return;

  if (crank_teeth_i < (CRANK_TEETH - CRANK_MISSING_TEETH) * 2){
    crank_out = !crank_out;
    digitalWrite(CRANK_P_PIN, crank_out);
  }

  crank_teeth_i = crank_teeth_i + 1;
  if (crank_teeth_i > CRANK_TEETH * 2) crank_teeth_i = 0;
}

// Function to set RPM from potentiometer instead of current websocket interface
// Not used anymore
void refreshRpmPot(){
  
  uint16_t raw_adc = analogRead(RPM_POT_PIN);
  rpm_setpoint = map(raw_adc, 0, MAX_ADC, 0, MAX_RPM);    // map(value, fromLow, fromHigh, toLow, toHigh)

  // Each engine revolution has CRANK_TEETH
  // Change rpm (minutes) to us
  tooth_time = calcToothTime(rpm_setpoint);

//  Serial.print(raw_adc);
//  Serial.print("\t");
//  Serial.print(rpm_setpoint);
//  Serial.print("\t");
//  Serial.println(tooth_time);

//  broadcastJsonById("adc", raw_adc);
//  broadcastJsonById("rpm_sp", rpm_setpoint);
//  broadcastJsonById("tooth_time", tooth_time);
}
