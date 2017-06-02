#include <ESP8266WiFi.h>
#include <SPI.h>
#include <TFT_ILI9163C.h>
#include <Adafruit_GFX.h>
#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>
#include <IRremoteESP8266.h>
extern "C" {
#include "user_interface.h"
}
#include <FS.h>
#include "rboot.h"
#include "rboot-api.h"

#include <Fonts/FreeSans12pt7b.h>

#define BNO055_SAMPLERATE_DELAY_MS (10)

#define VERSION 2

//#define USEWIFI
#define USEIR

#define GPIO_LCD_DC 0
#define GPIO_TX     1
#define GPIO_WS2813 4
#define GPIO_RX     3
#define GPIO_DN     2
#define GPIO_DP     5

#define GPIO_BOOT   16
#define GPIO_MOSI   13
#define GPIO_CLK    14
#define GPIO_LCD_CS 15
#define GPIO_BNO    12

#define MUX_JOY 0
#define MUX_BAT 1
#define MUX_LDR 2
#define MUX_ALK 4
#define MUX_IN1 5

#define VIBRATOR 3
#define MQ3_EN   4
#define LCD_LED  5
#define IR_EN    6
#define OUT1     7

#define UP      711
#define DOWN    567
#define RIGHT   469
#define LEFT    950
#define OFFSET  20

#define I2C_PCA 0x25

#define NUM_LEDS    4

#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF


TFT_ILI9163C tft = TFT_ILI9163C(GPIO_LCD_CS, GPIO_LCD_DC);
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_LEDS, GPIO_WS2813, NEO_GRB + NEO_KHZ800);
Adafruit_BNO055 bno = Adafruit_BNO055(BNO055_ID, BNO055_ADDRESS_B);

IRsend irsend(GPIO_DP);
IRrecv irrecv(GPIO_DN);
decode_results results;

byte portExpanderConfig = 0; //stores the 74HC595 config


String sym = "";
String text = "";

int count = 0;
int thresh = 3000;

boolean IRon = false;
boolean IRcheck = false;

#define COLORMAPSIZE 32
int joystick = 0; //Jostick register
boolean check = false;
boolean leercheck = false;
boolean loeschcheck = false;
String rev = "";
boolean start = true;



void setup() {
  initBadge();
  rboot_config rboot_config = rboot_get_config();
  SPIFFS.begin();
  File f = SPIFFS.open("/rom"+String(rboot_config.current_rom),"w");
  f.println("MateMorsen\n");
  
  //irrecv.enableIRIn();
  
  Serial.print("start MateMorsen");
 
}

void loop() {

  if(start){
    start = false;
     tft.fillScreen(BLACK);
      tft.writeFramebuffer();
            
      tft.setTextSize(2);
      tft.setCursor(0,10);
      tft.println("MateMorsen");
    
      tft.setTextSize(1);
      tft.setCursor(0,30);
      tft.println("hoch: senden/empf");
      tft.setCursor(0,50);
      tft.println("enter: morsen");
      tft.setCursor(0,70);
      tft.println("unten: bestaetigen");
      tft.setCursor(0,90);
      tft.println("links: loeschen");
      tft.setCursor(0,110);
      tft.println("rechts: Leerzeichen");
      tft.writeFramebuffer();
      delay(4000);
      tft.fillScreen(BLACK);
      tft.writeFramebuffer();
  }

  joystick = getJoystick();

  if(joystick == 1){//hoch
      IRcheck = true;
  }else if(IRcheck){
      if(IRon){
        IRon = false;
        tft.fillScreen(BLACK);
        tft.writeFramebuffer();
      }else{
        IRon = true;
        //tft.fillScreen(GREEN);
        //tft.writeFramebuffer();
      }
      IRcheck = false;
    }

  if(IRon){
    if(text.length()<1){//recieve
       tft.setCursor(0,80);
       tft.println("recieve");
       tft.writeFramebuffer();
       
       decode_results  results; 
       if (irrecv.decode(&results)) {
        Serial.println(results.value, HEX);
        rev = rev+char(results.value);
        tft.writeFramebuffer();
        irrecv.resume(); // Receive the next value
      }
      tft.println(rev);
      delay(100);
    }else{//send
      Serial.println("NEC");
      tft.setCursor(0,80);
      tft.println("sending...");
      tft.writeFramebuffer();
      
      for(int i = 0; i<text.length(); i++){
        irsend.sendNEC(text.charAt(i), 32);
        delay(1000);
      }
      
      tft.fillScreen(BLACK);
      tft.writeFramebuffer();
      IRon = false;
      text = "";
    }
  }else{

    
    
    if(joystick == 2 && !sym.equals("")){ //runter
      //setGPIO(VIBRATOR, 1);
      text = text + getLetter(sym);
      sym = "";
      tft.fillScreen(BLACK);
      tft.setCursor(0,20);
      tft.println(text);
      tft.writeFramebuffer();
    }
    if (joystick == 4) {
      loeschcheck = true;
    }else if(loeschcheck){//links
      //setGPIO(VIBRATOR, 1);
      loeschcheck = false;
      text.remove(text.length()-1);
      tft.fillScreen(BLACK);
      tft.setCursor(0,20);
      tft.println(text);
      tft.writeFramebuffer();
      
    }else if (joystick == 5) {
      //setGPIO(VIBRATOR, 1);
      IRon = false;
      check = true;
      rev = "";
      count++;
      
    }else if(check){
      check = false;
      sym = sym + checkSym(count);
      tft.setCursor(0,0);
      tft.println(sym);
      count = 0;
      tft.writeFramebuffer();
    }
    
    if(joystick == 3){//rechts
      leercheck = true;
    }else if(leercheck){
      leercheck = false;
      text = text + "_";
      tft.fillScreen(BLACK);
      tft.setCursor(0,20);
      tft.println(text);
      tft.writeFramebuffer();
    }
  }

}

String checkSym(int c){
    if(c==0){
      return "";
    }else if(c<thresh){
      return ".";
    }else{
      return "-";
    }
}

String getLetter(String code){
    if(code.equals(".-")){
      return "A";
    }else if(code.equals("-...")){
      return "B";
    }else if(code.equals("-.-.")){
      return "C";
    }else if(code.equals("-..")){
      return "D";
    }else if(code.equals(".")){
      return "E";
    }else if(code.equals("..-.")){
      return "F";
    }else if(code.equals("--.")){
      return "G";
    }else if(code.equals("....")){
      return "H";
    }else if(code.equals("..")){
      return "I";
    }else if(code.equals(".---")){
      return "J";
    }else if(code.equals("-.-")){
      return "K";
    }else if(code.equals(".-..")){
      return "L";
    }else if(code.equals("--")){
      return "M";
    }else if(code.equals("-.")){
      return "N";
    }else if(code.equals("---")){
      return "O";
    }else if(code.equals(".--.")){
      return "P";
    }else if(code.equals("--.-")){
      return "Q";
    }else if(code.equals(".-.")){
      return "R";
    }else if(code.equals("...")){
      return "S";
    }else if(code.equals("-")){
      return "T";
    }else if(code.equals("..-")){
      return "U";
    }else if(code.equals("...-")){
      return "V";
    }else if(code.equals(".--")){
      return "W";
    }else if(code.equals("-..-")){
      return "X";
    }else if(code.equals("-.--")){
      return "Y";
    }else if(code.equals("--..")){
      return "Z";
    }
    return "";
}

















int getJoystick() {
  uint16_t adc = analogRead(A0);
  /*tft.fillScreen(BLACK);
  tft.setCursor(0,80);
    tft.println(adc);
    tft.writeFramebuffer();*/

  if (adc < UP + OFFSET && adc > UP - OFFSET)             return 1;
  else if (adc < DOWN + OFFSET && adc > DOWN - OFFSET)    return 2;
  else if (adc < RIGHT + OFFSET && adc > RIGHT - OFFSET)  return 3;
  else if (adc < LEFT + OFFSET && adc > LEFT - OFFSET)    return 4;
  if (digitalRead(GPIO_BOOT) == 1) return 5;
  return 0;
}

void setGPIO(byte channel, boolean level) {
  bitWrite(portExpanderConfig, channel, level);
  Wire.beginTransmission(I2C_PCA);
  Wire.write(portExpanderConfig);
  Wire.endTransmission();
}

void setAnalogMUX(byte channel) {
  portExpanderConfig = portExpanderConfig & 0b11111000;
  portExpanderConfig = portExpanderConfig | channel;
  Wire.beginTransmission(I2C_PCA);
  Wire.write(portExpanderConfig);
  Wire.endTransmission();
}







void initBadge() { //initialize the badge

#ifdef USEIR
  Serial.begin(115200, SERIAL_8N1, SERIAL_TX_ONLY);
  setGPIO(IR_EN, HIGH);
  irrecv.enableIRIn(); // Start the receiver
  irsend.begin();
#else
  Serial.begin(115200);
#endif

#ifdef USEWIFI
  // Next 2 line seem to be needed to connect to wifi after Wake up
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  delay(20);
#endif

  pinMode(GPIO_BOOT, INPUT_PULLDOWN_16);  // settings for the leds
  pinMode(GPIO_WS2813, OUTPUT);

  pixels.begin(); //initialize the WS2813
  pixels.clear();
  pixels.show();

  Wire.begin(9, 10); // Initalize i2c bus
  Wire.beginTransmission(I2C_PCA);
  Wire.write(0b00000000); //...clear the I2C extender to switch off vibrator and backlight
  Wire.endTransmission();

  delay(100);

  tft.begin(); //initialize the tft. This also sets up SPI to 80MHz Mode 0
  tft.setRotation(2); //turn screen
  tft.scroll(32); //move down by 32 pixels (needed)
  tft.fillScreen(BLACK);  //make screen black
  tft.writeFramebuffer();
  setGPIO(LCD_LED, HIGH);

  pixels.clear(); //clear the WS2813 another time, in case they catched up some noise
  pixels.show();
}
