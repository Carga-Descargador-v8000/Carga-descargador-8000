#include <SPI.h>
#include "cargadescargador.h"

/////////////////////////

	
	Adafruit_SSD1306 display(OLED_RESET);
	MAX17260 fuelgauge;
	SPIClass POT(2);
	DPOT digitalpot;


bool battery_present=0;
uint8_t mode='0';
//uint8_t test=0;




void setup() {
  pinsetup();

  digitalWrite(LOAD_ACTIVATE,LOW);//turn off mosfet
  digitalWrite(CHARGER_ENABLE,CHARGER_OFF);//Shutdown charger
  digitalWrite(POT_RESET,HIGH); //NO RESET

  //group alll the begins
  
  
  Wire.begin();
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  configdisplay();
  POT.begin();//Initialize the SPI_2 port.
  POT.setBitOrder(MSBFIRST); // Set the SPI_2 bit order
  POT.setDataMode(SPI_MODE0); //Set the  SPI_2 data mode 1
  POT.setClockDivider(SPI_CLOCK_DIV16);
  
  Serial.begin(9600);
  setMode(mode);
   
   if(setChargeCurrent(digitalpot.chargeCurrent)){ //initialize the digital pot with default values
  	display.println("error current set");
  	display.display();
  	while(1); //error handler
  }

  if(setIdet(digitalpot.idetCurrent)){ //initialize the digital pot with default values
  	display.println("error idet set");
  	display.display();
  	while(1); //error handler
  }



  if(fuelgauge.isAlive()){  //check if battery is present
  	battery_present=1;
  	if(fuelgauge.checkPOR()){
  		fuelgauge.setup(1500,75,3.0,3.1);
  	}
  }else{
  	battery_present=0;
  	display.println("battery detached");
  }
  

  display.println("All ok");
  display.display();
  delay(1000);
 
}

void loop() {

	if(fuelgauge.isAlive()){  //check if battery is present
		while (Serial.available()){
		setMode(Serial.read());
		}
  		battery_present=1;
	  	if(fuelgauge.checkPOR()){
	  		fuelgauge.setup(1500,75,3.0,3.1);//reinitialize if the fuelgauge has been reset
	  	}
	  	fuelgauge.poll();  //update values
	  	/*display and clear display are strategically separated by the time we use doing things
	  	like polling the fuel gauge and doing other checks that dont involve manipulating the 
	  	frame buffer*/
		
	    display.clearDisplay(); 
		display.setCursor(0,0);

	  	
	  	display.println("Battery present");
	  	printMode(mode);
	  	display.print(fuelgauge.remainingCapacity);
	  	display.println(" mah");
	  	display.print(fuelgauge.reportedSOC);
	  	display.println("%");
	  	display.print(fuelgauge.cellVoltage,5);
	  	display.println(" V");
	  	display.print(fuelgauge.Current,5);
	  	display.println(" A");
	  	display.println(millis());
	  	if((millis()%5000)<=100){		
	  		Serial.print(fuelgauge.Current,5);
	  		Serial.print(",");
	  		Serial.print(fuelgauge.cellVoltage,5);
	  		Serial.print(",");
	  		Serial.println(4.2);

	  	}

	}
	else{
		display.clearDisplay(); 
		display.setCursor(0,0);
	  	battery_present=0;
		display.println("battery detached");
	}

	//////////////////this is a resistor test/////////////////////////
	
	/*if(Serial.available()>0){
		uint8_t temp_test=Serial.parseInt();
		if(temp_test!=0){
			test=temp_test;
		}
	}
	digitalWrite(POT_CS, LOW);
	POT.transfer(0x00);
	POT.transfer(test);
	digitalWrite(POT_CS, HIGH);

	digitalWrite(POT_CS, LOW);
	POT.transfer(0x01);
	POT.transfer(test);
	digitalWrite(POT_CS, HIGH);

	display.clearDisplay();
	display.setCursor(0,0);
	display.print(test);*/
	display.display();

}



void setMode(uint8_t _mode){
		mode=_mode;
	  	switch(_mode){
	  	case'0':{
  			digitalWrite(LOAD_ACTIVATE, LOW);
  			digitalWrite(CHARGER_ENABLE,CHARGER_OFF);//standby
  			break;
  		}

  		case'1':{
  			digitalWrite(LOAD_ACTIVATE, LOW);
	  		delay(1);
	  		digitalWrite(CHARGER_ENABLE,CHARGER_ON);//charging
	  		break;		
  		}
  		
  		case'2':{
	  		digitalWrite(CHARGER_ENABLE,CHARGER_OFF);//discharging
	  		delay(1);
	  		digitalWrite(LOAD_ACTIVATE, HIGH);
  			break;
  		}

  	}

}

void printMode(uint8_t _mode){
	switch(_mode){
	  	case'0':{
  			display.println("Standby");
  			break;
  		}

  		case'1':{
  			display.println("charging");
	  		break;		
  		}
  		
  		case'2':{
  			display.println("discharging");
  			break;
  		}

  	}

}

	void configdisplay(){
	  display.setTextSize(1);
	  display.setTextColor(WHITE); 
	  display.clearDisplay();
	  display.setCursor(0,0);
	  display.println("display ok");
	  display.display();
	}
	void pinsetup(){
	  pinMode(LOAD_ACTIVATE, OUTPUT);

	  pinMode(CHARGER_OUT, OUTPUT);
	  pinMode(CHARGER_IN, INPUT);
	  pinMode(CHARGER_ENABLE, OUTPUT);
	  pinMode(CHARGER_FAULT, INPUT);

	  
	  pinMode(POT_RESET,OUTPUT);
	  pinMode(POT_CS,OUTPUT);
	  digitalWrite(POT_CS, HIGH);
	 // pinMode(POT_CLK, OUTPUT);
	 // pinMode(POT_MOSI, OUTPUT);
	  pinMode(CHARGER_ALERT, INPUT);
	  pinMode(PC13,OUTPUT);
	}

	bool setChargeCurrent(float current){
		int temp=1109.895f/current;
		//from datasheet
		if (current>MAX_CURRENT || temp<89){
			return 1;  //charge current cannot be larger than the max current
			//also pot cannot go lower than 89
		}
		else{
			digitalpot.progresistance=temp;
			digitalpot.chargeCurrent=current;
			uint8_t data=-0.0272f*digitalpot.progresistance + 256.09f;;
			digitalWrite(POT_CS, LOW);
			POT.transfer(0x01);
			POT.transfer(data);
			digitalWrite(POT_CS, HIGH);
			return 0;
		}
	}

	bool setIdet(float current){
		int temp=110.9895f/current; //from datasheet
		if (current>MAX_CURRENT || current>=digitalpot.chargeCurrent || temp<89){
			return 1;  //idet cannot be larger than charge current or even larger than the max current
			//also pot cannot go lower than 200
		}
		else{
			digitalpot.idetresistance=temp;
			digitalpot.idetCurrent=current;
			
			uint8_t data= -0.0272f*digitalpot.idetresistance + 256.09f;//calculated from table in datasheet
			digitalWrite(POT_CS, LOW);
			POT.transfer(0x00);
			POT.transfer(data);
			digitalWrite(POT_CS, HIGH);
			return 0;
		}
		
	}



