#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306_STM32.h>

/////////////////////////
#define OLED_RESET -1
Adafruit_SSD1306 display(OLED_RESET);

////////PIN DEFINITIONS//////////////
#define POT_CS PB12
#define POT_CLK PB13
#define POT_MOSI PB15
#define POT_RESET PA8

#define CHARGER_FAULT PA9
#define CHARGER_ALERT PA10
#define CHARGER_ENABLE PB3
#define CHARGER_OUT PB1
#define CHARGER_IN PA15

#define SD_MISO PA6
#define SD_MOSI PA7
#define SD_CLK PA5
#define SD_CS PA4

#define LOAD_ACTIVATE PB11
#define MAX_CURRENT 2
#define fuelGauge 0x36

float idetCurrent=0.1;
float chargeCurrent=1;

uint16_t Reported_capacity;
uint16_t Reported_stateofcharge;
uint16_t Status_POR;
//for samsung inr18650-25r
uint16_t Design_capacity=2500; //2500mah/0.5mah (step)
uint16_t Ichargeterm=800; //125mA/156.25ua (step)
uint16_t Vempty=250; //*10mv=2.5V
uint8_t Vrecovery=63; //*40mv=2.52V
uint16_t Vcell=0;



SPIClass POT(2);

void setup() {
  pinMode(LOAD_ACTIVATE, OUTPUT);

  pinMode(CHARGER_OUT, OUTPUT);
  pinMode(CHARGER_IN, INPUT);
  pinMode(CHARGER_ENABLE, OUTPUT);
  pinMode(CHARGER_FAULT, INPUT);

  pinMode(POT_CS,OUTPUT);
  pinMode(POT_RESET,OUTPUT);
  pinMode(POT_CLK, OUTPUT);
  pinMode(POT_MOSI, OUTPUT);
  pinMode(CHARGER_ALERT, INPUT);
  pinMode(PC13,OUTPUT);

  digitalWrite(LOAD_ACTIVATE,LOW);//turn off mosfet
  digitalWrite(CHARGER_ENABLE,HIGH);//Shutdown charger
  digitalWrite(POT_RESET,HIGH); //NO RESET

  //group alll the begins
  Serial.begin(9600);
  POT.begin();//Initialize the SPI_2 port.
  Wire.begin();
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  
  display.setTextSize(1);
  display.setTextColor(WHITE); 
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("display ok");
  display.display();
   
  POT.setBitOrder(MSBFIRST); // Set the SPI_2 bit order
  POT.setDataMode(SPI_MODE0); //Set the  SPI_2 data mode 0

  if(setIdet(idetCurrent)){ //initialize the digital pot with default values
  	display.println("error idet set");
  	display.display();
  	while(1); //error handler
  }

  if(setChargeCurrent(chargeCurrent)){ //initialize the digital pot with default values
  	display.println("error idet set");
  	display.display();
  	while(1); //error handler
  }


  fuelGaugeSetup();

  display.println("All ok");
  display.display();
  
  Serial.println("good");
}

void loop() {

  display.clearDisplay();
  display.setCursor(0,0);

 
  Reported_capacity=readfuelgauge(0x05)*0.5;
  Reported_stateofcharge=readfuelgauge(0x06)/256;
  if(Reported_stateofcharge<=50){
  	digitalWrite(CHARGER_ENABLE,LOW);
  	display.println("charging...");
  }
  else{
  	digitalWrite(CHARGER_ENABLE,HIGH);
  	display.println("not charging");
  }
  display.print("cap: ");
  display.println(readfuelgauge(0x18),BIN);
  display.println(Reported_capacity,DEC); 
  display.println(Reported_stateofcharge,DEC);
  display.println(readfuelgauge(0x09)*0.000078125);//read voltage
  display.display();
  while(Serial.available()){

  }

}

bool setIdet(float current){
	float resistance=110.9895/current; //from datasheet
	if (current>MAX_CURRENT || current>=chargeCurrent || resistance<200){
		return 1;  //idet cannot be larger than charge current or even larger than the max current
		//also pot cannot go lower than 200
	}
	else{
		idetCurrent=current;
		
		uint8_t data=(resistance - 200) * (0 - 255) / (1200 - 200) + 255;
		digitalWrite(POT_CS, LOW);
		POT.transfer(0x00);
		POT.transfer(data);
		digitalWrite(POT_CS, HIGH);
		return 0;
	}
	
}

bool setChargeCurrent(float current){
	float resistance=1109.895/current;//from datasheet
	if (current>MAX_CURRENT || resistance<200){
		return 1;  //charge current cannot be larger than the max current
		//also pot cannot go lower than 200
	}
	else{
		chargeCurrent=current;
		
		//uint8_t data=map(resistance,200,1200,255,0);
		uint8_t data=(resistance - 200) * (0 - 255) / (1200 - 200) + 255;
		digitalWrite(POT_CS, LOW);
		POT.transfer(0x01);
		POT.transfer(data);
		digitalWrite(POT_CS, HIGH);
		return 0;
	}


	
}

void quick_start(){//only to be used to check if the device is good

	uint16_t data;
	fuelGaugeSetup();
	data=readfuelgauge(0x2B);
	data|=0x0400;
	writefuelgauge(0x2B,data);

	while(readfuelgauge(0xDB)&0x8000 && readfuelgauge(0x3D)&0x0001){
		delay(10); //wait for the fuel gauge to be ready
	}
	Reported_capacity=readfuelgauge(0x05);
	Reported_stateofcharge=readfuelgauge(0x06);
}



bool fuelGaugeSetup(){
	uint16_t HibCFG;
	uint16_t Status;
	uint16_t temp;
	//Step 0: Check for POR 
	//not doing this cause we are initializing anyway
	//Status_POR=readfuelgauge(0x00) & 0x0002;
	//Step 1. Delay until FSTAT.DNR bit == 0 
	while(readfuelgauge(0x3D)&0x0001){
		delay(100);
	}

	//Step 2: Initialize Configuration
	HibCFG=readfuelgauge(0xBA);
	writefuelgauge(0x60,0x90);
	writefuelgauge(0xBA,0x00);
	writefuelgauge(0x60,0x00);

	//2.1 OPTION 1 EZ Config (No INI file is needed):
	writefuelgauge(0x18,Design_capacity);
	writefuelgauge(0x1E,Ichargeterm);
	writefuelgauge(0x3A , (Vempty<<7)|Vrecovery); //both configuration bits are on the same register, and yes, verecovery is 7 bits long 
	writefuelgauge(0xDB , 0x8000) ; //where 0x8000 means 4.2v max voltage


	writefuelgauge (0xBA , HibCFG) ; // Restore Original HibCFG value

	//Step 3: Initialization Complete
	Status=readfuelgauge(0x00);
	writefuelgauge(0x00,(Status&0xFFFD));

}


uint16_t readfuelgauge(uint8_t memory_add){
	uint16_t temp;
	uint8_t buffarray[2];
	Wire.beginTransmission(fuelGauge);
	Wire.write(memory_add);  //set the memory address and leave the communication open 
	Wire.endTransmission();
	Wire.requestFrom(fuelGauge,2,1); //request 2 bytes to the fuel gauge and send a stop signal
	buffarray[0]=Wire.read(); //lsb
	buffarray[1]=Wire.read(); //msb
	temp=(buffarray[1]<<8)|(buffarray[0]); //stick them together
	return temp;
}

void writefuelgauge(uint8_t memory_add,uint16_t data){
	Wire.beginTransmission(fuelGauge);
	Wire.write(memory_add);
	Wire.write((uint8_t)(data)); //send lsb
	Wire.write(data>>8); //send msb
	Wire.endTransmission(1); //send a stop signal
}

bool isFuelGaugeAlive(){ //check if the fuel gauge responds to its address
	Wire.beginTransmission(fuelGauge);
	if(Wire.endTransmission(1)==0){//success
		return 1;
	}
	else{
		return 0;
	}

}