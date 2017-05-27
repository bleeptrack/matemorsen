#include<GPNBadge.hpp>
#include <FS.h>
#include "rboot.h"
#include "rboot-api.h"
#include <SPI.h>
#include <Wire.h>

#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>

#define BNO055_SAMPLERATE_DELAY_MS (100)

Adafruit_BNO055 bno = Adafruit_BNO055(BNO055_ID, BNO055_ADDRESS_B);

Badge badge;

#define STORAGEDEPTH 112
#include "sensor.h"

Sensor sens;
String code = "";
String text = "";
int count = 0;
int wait = 0;

int thresh = 13;


void setup() {
  badge.init();
  badge.setBacklight(true);
  bno.begin();
  delay(300);
  badge.setGPIO(MQ3_EN,1);
  
  count = 0;
  rboot_config rboot_config = rboot_get_config();
  SPIFFS.begin();
  File f = SPIFFS.open("/rom"+String(rboot_config.current_rom),"w");
  f.println("Morsen\n");
}



void loop() {
  String sym;
  if (badge.getJoystickState() == JoystickState::BTN_ENTER){
    while(badge.getJoystickState() == JoystickState::BTN_ENTER){ 
      count++;
    }
    
    if(count==0){
      sym = "";
    }else if(count<thresh){
      sym = ".";
      wait=0;
    }else{
      sym = "-";
      wait=0;
    }
    code = code + sym;
   
    count = 0;
    sens.title(text+code);
    tft.writeFramebuffer();
  }

  else if (badge.getJoystickState() == JoystickState::BTN_DOWN){
     text = "jo";
  }
  

  if(wait>17){
  
    if(code.equals(".-")){
      text = text + "A";
    }else if(code.equals("-...")){
      text = text + "B";
    }else if(code.equals("-.-.")){
      text = text + "C";
    }else if(code.equals("-..")){
      text = text + "D";
    }else if(code.equals(".")){
      text = text + "E";
    }else if(code.equals("..-.")){
      text = text + "F";
    }else if(code.equals("--.")){
      text = text + "G";
    }else if(code.equals("....")){
      text = text + "H";
    }else if(code.equals("..")){
      text = text + "I";
    }else if(code.equals(".---")){
      text = text + "J";
    }else if(code.equals("-.-")){
      text = text + "K";
    }else if(code.equals(".-..")){
      text = text + "L";
    }else if(code.equals("--")){
      text = text + "M";
    }else if(code.equals("-.")){
      text = text + "N";
    }else if(code.equals("---")){
      text = text + "O";
    }else if(code.equals(".--.")){
      text = text + "P";
    }else if(code.equals("--.-")){
      text = text + "Q";
    }else if(code.equals(".-.")){
      text = text + "R";
    }else if(code.equals("...")){
      text = text + "S";
    }else if(code.equals("-")){
      text = text + "T";
    }else if(code.equals("..-")){
      text = text + "U";
    }else if(code.equals("...-")){
      text = text + "V";
    }else if(code.equals(".--")){
      text = text + "W";
    }else if(code.equals("-..-")){
      text = text + "X";
    }else if(code.equals("-.--")){
      text = text + "Y";
    }else if(code.equals("--..")){
      text = text + "Z";
    }
    
    
      code = "";
      wait = 0;
      sens.title(text+code);
      tft.writeFramebuffer();
    
  }
  delay(50);

  wait++;
  
}


