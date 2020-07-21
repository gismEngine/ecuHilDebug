#include "Arduino.h"
#include "ecuMng.h"

Frame acc_frame;
Frame rpm_frame;
Frame turbo_frame;
Frame v_bat_frame;
Frame switch_frame;

Frame *last_frame;
bool tx_blocked = false;

uint32_t acc_req_t = 0;
uint32_t vbat_req_t = 0;
uint32_t rpm_req_t = 0;
uint32_t switch_req_t = 0;
uint32_t turbo_req_t = 0;

uint32_t tankSwitchReqCount = 0;

void ecu_begin(){
  ECU.begin(9600);    // ECU serial port
  initFrames();
}

void ecu_manage(){
  
  // Pull from Serial Rx buffer from the ECU
  checkRxFrames();

  if (millis() >= vbat_req_t + VBAT_SAMPLE_T){
    if (!tx_blocked){
      sendFrame(&v_bat_frame);
      vbat_req_t = millis();
    }
  }

  if (millis() >= rpm_req_t + RPM_SAMPLE_T){
    if (!tx_blocked){
      sendFrame(&rpm_frame);
      rpm_req_t = millis();
    }
  }

  if (tankSwitchReqCount){
    if (!tx_blocked){
      Serial.println("send tank sw cmd");
      sendFrame(&switch_frame);
      tankSwitchReqCount = tankSwitchReqCount - 1;
    }
  }

  if (DEBUG.available()){
     char c = DEBUG.read();
     if (c == 'a'){
        DEBUG.print("ACC >>");
        sendFrame(&acc_frame);
     }else if (c == 'r'){
        DEBUG.print("RPM >> ");
        sendFrame(&rpm_frame);
      }else if (c == 'R'){
          DEBUG.print(F("RPM setpoint: "));
          DEBUG.println(getRpmSetpoint());

//          DEBUG.print(F("Tooth time: "));
//          DEBUG.println(getToothTime());          
          
     }else if (c == 't'){
        DEBUG.print("TURBO >>");
        sendFrame(&turbo_frame);
     }else if (c == 'v'){
        DEBUG.print("V_BAT >>");
        sendFrame(&v_bat_frame);
     }else if (c == 's'){
        DEBUG.print("SWITCH >>");
        sendFrame(&switch_frame);
     }else if (c == 'l'){
        DEBUG.print("LAST >>");
        sendFrame(last_frame);
     }
  }
}

// Check if there is any ECU frame (Serial).
// If there is new byte add to buffer
// If buffer size is same as expected
// Then process frame information

void checkRxFrames(){
  if (ECU.available()) {    
    uint32_t inTxByte = ECU.read();
    //if (inTxByte < 0xF) Serial.print('0');
    //Serial.println(inTxByte, HEX);

    if(last_frame != nullptr){                                //  Check not null pointer
      if(last_frame->response_received < RX_BUFF_SIZE){       //  Check there is no buffer overflow

        last_frame->response[last_frame->response_received] = inTxByte;       // Save received byte
        last_frame->response_received = last_frame->response_received + 1;    // Increase Rx buffer

        if(last_frame->response_received == last_frame->response_size){        // If received buffer is same size as expected:
          processFrame(last_frame);                                           // Process frame
        }
        
      }
    }
  }
}

void initFrames(){

  last_frame = nullptr;

  // 0xAA2D552C
  acc_frame.request[0] = 0xAA;
  acc_frame.request[1] = 0x2D;
  acc_frame.request[2] = 0x55;
  acc_frame.request[3] = 0x2C;
  acc_frame.id = ACC_ID;
  acc_frame.response_size = 3;

  // 0xAA815580
  rpm_frame.request[0] = 0xAA;
  rpm_frame.request[1] = 0x81;
  rpm_frame.request[2] = 0x55;
  rpm_frame.request[3] = 0x80;
  rpm_frame.id = RPM_ID;
  rpm_frame.response_size = 4;

  // 0xAA2B552A
  turbo_frame.request[0] = 0xAA;
  turbo_frame.request[1] = 0x2B;
  turbo_frame.request[2] = 0x55;
  turbo_frame.request[3] = 0x2A;
  turbo_frame.id = TURBO_ID;
  turbo_frame.response_size = 3;
  
  // 0xAA255524
  v_bat_frame.request[0] = 0xAA;
  v_bat_frame.request[1] = 0x25;
  v_bat_frame.request[2] = 0x55;
  v_bat_frame.request[3] = 0x24;
  v_bat_frame.id = VBAT_ID;
  v_bat_frame.response_size = 18;
  
  // 0xAA855584
  switch_frame.request[0] = 0xAA;
  switch_frame.request[1] = 0x85;
  switch_frame.request[2] = 0x55;
  switch_frame.request[3] = 0x84;
  switch_frame.id = SWITCH_ID;
  switch_frame.response_size = 1;
}


void processFrame(Frame *f){

  if (f->response_received != f->response_size){
    DEBUG.println(F("ERROR: Rx buffer not correct size"));
    f->response_received = 0;
    return;
  }
  
//  for (byte i = 0; i < f->response_received; i = i + 1) {
//    uint8_t b = f->response[i];
//    if (b < 0x10) DEBUG.print('0');
//    DEBUG.print(b, HEX);
//    DEBUG.print(' ');
//  }
//  DEBUG.println();

  printHuman(f);

  // RESET response buffer
  f->response_received = 0x0;
  tx_blocked = false;
}

void sendFrame(Frame *f){
  
  tx_blocked = true;
  
  for (byte i = 0; i < f->request_size; i = i + 1) {
    uint8_t b = f->request[i];
   
//    if (b < 0x10) DEBUG.print('0');
//    DEBUG.print(b, HEX);
//    DEBUG.print(' ');
    
    ECU.write(b);
  }
//  DEBUG.println();

  // RESET response buffer
  f->response_received = 0x0;

  // Save latest sent frame (to manage response)
  last_frame = f;
}

void printHuman(Frame *f){

  if(f->id == ACC_ID){
    DEBUG.print(f->response[1], DEC);
    DEBUG.println(" %");
    
  }else if(f->id == VBAT_ID){
    
    float v_bat = 0.1441 * f->response[2]  - 0.1794;   // incorrect formula. To be confirmed and understood
    float v_key = 0.1441 * f->response[1]  - 0.1794;
    
    if (v_bat < 0) v_bat = 0;
    if (v_key < 0) v_key = 0;
    
//    DEBUG.print("Vbatt: ");
//    DEBUG.print(v_bat, 1);
//    DEBUG.print(" V");

    uint32_t v_bat_uint = v_bat * 10;
    broadcastJsonById("v_bat", v_bat_uint);

//    DEBUG.print(" | ");
//    
//    DEBUG.print("Vkey: ");
//    DEBUG.print(v_key, 1);
//    DEBUG.print(" V");

    uint32_t v_key_uint = v_key * 10;
    broadcastJsonById("v_key", v_key_uint);

//    DEBUG.print(" | ");
//    
//    DEBUG.print("P_MANI: ");
    float p_mani = (float) (f->response[5]) / 100;
//    DEBUG.print(p_mani, 2);
//    DEBUG.print(" bar");

    uint32_t p_mani_uint = p_mani * 100;
    broadcastJsonById("p_mani", p_mani_uint);
    
//    DEBUG.print(" | ");
//    
//    DEBUG.print("P_GAS: ");
    //float p_gas = (float) (f->response[12] * 2) / 100;
    float p_gas = (float) f->response[12];
//    DEBUG.print(p_gas, 2);
//    DEBUG.print(" bar");

    uint32_t p_gas_uint = p_gas * 100;
    broadcastJsonById("p_gas", p_gas_uint);
    
//    DEBUG.print(" | ");
//    
//    DEBUG.print("T_RAIL_GAS: ");
    uint32_t t_rail_gas = (f->response[14] * 5) - 40;
//    DEBUG.print(t_rail_gas, DEC);
//    DEBUG.print(" ºC");

    broadcastJsonById("t_rail_gas", t_rail_gas);

//    DEBUG.print(" | ");
//
//    DEBUG.print("LEVEL: ");
    uint32_t gas_level = f->response[15];
//    DEBUG.print(gas_level, DEC);
//    DEBUG.print(" bar");

    broadcastJsonById("gas_level", gas_level);

//    DEBUG.print(" | ");
//    
//    DEBUG.print("SWITCH: ");
    uint32_t gas_sw = f->response[16];
//    if (gas_sw == 0){
//      DEBUG.print("OFF");
//    }else if (gas_sw == 1){
//      DEBUG.print("ON");
//    }else{
//      DEBUG.print(gas_sw, DEC);
//    }

    broadcastJsonById("gas_sw", gas_sw);

//    DEBUG.print(" | CRC: ");
//    DEBUG.print(f->response[17], DEC);
//    DEBUG.println();
    
  }else if(f->id == RPM_ID){

    uint8_t r_msb = f->response[1];
    uint8_t r_lsb = f->response[2];
    
    uint16_t rpm = ((uint16_t)r_msb << 8) | r_lsb;
//    DEBUG.print(rpm, DEC);
//    DEBUG.println(" rpm");

    broadcastJsonById("rpm", rpm);
  
  }else if(f->id == SWITCH_ID){
    DEBUG.print("\nSWITCH!\n\n");
    
  }else if(f->id == TURBO_ID){
   
    float turbo = (float) f->response[1] / 100;
    DEBUG.print(turbo, 2);
    DEBUG.println(" bar");
    
  }else{
    DEBUG.println(F("ERROR: Unknown frame ID"));
  }

}

void sendTankSwCmd(){
  tankSwitchReqCount = tankSwitchReqCount + 1;
  Serial.print("tankSwitchReqCount: ");
  Serial.println(tankSwitchReqCount);
}
