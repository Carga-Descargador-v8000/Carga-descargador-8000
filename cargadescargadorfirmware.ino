#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306_STM32.h>

#define OLED_RESET -1
Adafruit_SSD1306 display(OLED_RESET);

////////PIN DEFINITIONS//////////////
#define POT_CS PB12
#define POT_CLK PB13
#define POT_MOSI PB15
#define POT_RESET PA8

#define CHARGER_FAULT PA9
#define CHARGER_ALERT PA10
#define CHARGER_ENABLE PA11
#define CHARGER_OUT PA12
#define CHARGER_IN PA15

#define SD_MISO PA6
#define SD_MOSI PA7
#define SD_CLK PA5
#define SD_CS PA4

#define LOAD_ACTIVATE PB11
#define MAX_CURRENT 2

float idetCurrent=1;
float chargecurrent=0.5;

SPIClass POT(2);
void setup() {
  pinMode(LOAD_ACTIVATE, OUTPUT);

  pinMode(CHARGER_OUT, OUTPUT);
  pinMode(CHARGER_IN, INPUT);
  pinMode(CHARGER_ENABLE, OUTPUT);
  pinMode(CHARGER_ALERT, INPUT);
  pinMode(CHARGER_FAULT, INPUT);

  pinMode(POT_CS,OUTPUT);
  pinMode(POT_RESET,OUTPUT);
  pinMode(POT_CLK, OUTPUT);
  pinMode(POT_MOSI, OUTPUT);

  digitalWrite(CHARGER_ENABLE,HIGH);
  digitalWrite(POT_RESET,HIGH); //NO RESET


  POT.begin(); //Initialize the SPI_2 port.
  POT.setBitOrder(MSBFIRST); // Set the SPI_2 bit order
  POT.setDataMode(SPI_MODE0); //Set the  SPI_2 data mode 0



  //display setup
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); 
  display.clearDisplay();
  display.setCursor(0,0);
  display.write("All ok");
  display.display();
}

void loop() {
  // put your main code here, to run repeatedly:

}

bool setIdet(float current){
	if (current>MAX_CURRENT || current>=chargecurrent){
		return 1;  //idet cannot be larger than charge current or even larger than the max current
	}
	else{
		idetCurrent=current;
		float resistance=1109.895/current;
		uint8_t data=mapfloat(resistance,200,1200,255,0);
		digitalWrite(POT_CS, LOW);
		POT.transfer(0x00);
		POT.transfer(data);
		digitalWrite(POT_CS, HIGH);
		return 0;
	}
	
}
float mapfloat(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}