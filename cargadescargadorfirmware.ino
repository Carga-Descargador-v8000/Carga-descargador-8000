
#include "cargadescargador.h"
/////////////////////////
struct DPOT{
	float idetCurrent=0.250;
	float chargeCurrent=1.0;
	int progresistance;
	int idetresistance;
}digitalpot;
	
	Adafruit_SSD1306 display(OLED_RESET);
	MAX17260 fuelgauge;
	SPIClass POT(2);
	
bool battery_present=0;
uint8_t mode=0;





void setup() {
  pinsetup();

  digitalWrite(LOAD_ACTIVATE,LOW);//turn off mosfet
  digitalWrite(CHARGER_ENABLE,HIGH);//Shutdown charger
  digitalWrite(POT_RESET,HIGH); //NO RESET

  //group alll the begins
  
  POT.begin();//Initialize the SPI_2 port.
  Wire.begin();
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  configdisplay();
  POT.setBitOrder(MSBFIRST); // Set the SPI_2 bit order
  POT.setDataMode(SPI_MODE0); //Set the  SPI_2 data mode 0
  Serial.begin(9600);
  setMode(mode);
   
  

  if(setIdet(digitalpot.idetCurrent)){ //initialize the digital pot with default values
  	display.println("error idet set");
  	display.display();
  	while(1); //error handler
  }

  if(setChargeCurrent(digitalpot.chargeCurrent)){ //initialize the digital pot with default values
  	display.println("error idet set");
  	display.display();
  	while(1); //error handler
  }

  if(fuelgauge.isAlive()){  //check if battery is present
  	battery_present=1;
  	if(fuelgauge.checkPOR()){
  		fuelgauge.setup(1500,100,3.0,3.1);
  	}
  }else{
  	battery_present=0;
  	display.println("battery detached");
  }
  

  display.println("All ok");
  display.display();
 
}

void loop() {
	display.clearDisplay();
	display.setCursor(0,0);
	if(fuelgauge.isAlive()){  //check if battery is present
		while (Serial.available()){
		setMode(Serial.read());
		}
  	battery_present=1;
	  	if(fuelgauge.checkPOR()){
	  		fuelgauge.setup(1500,100,3.0,3.1);//reinitialize if the fuelgauge has been reset
	  	}

	  	fuelgauge.poll();  //update values
	  	display.println("Battery present");
	  	display.print(fuelgauge.remainingCapacity);
	  	display.println(" mah");
	  	display.print(fuelgauge.reportedSOC);
	  	display.println("%");
	  	display.print(fuelgauge.cellVoltage);
	  	display.println(" V");
	  	display.print(fuelgauge.Current);
	  	display.println(" A");
	}
	else{
	  	battery_present=0;
		display.println("battery detached");
	}

	display.display();
	delay(100);

}



bool setMode(uint8_t _mode){
		mode=_mode;
	  	switch(_mode){
	  	case'0':{
  			display.print("Standby");
  			digitalWrite(LOAD_ACTIVATE, LOW);
  			digitalWrite(CHARGER_ENABLE,CHARGER_OFF);
  			break;
  		}

  		case'1':{
  			display.print("charging");
  			digitalWrite(LOAD_ACTIVATE, LOW);
	  		delay(1);
	  		digitalWrite(CHARGER_ENABLE,CHARGER_ON);
	  		break;		
  		}
  		
  		case'2':{
  			display.print("discharging");
	  		digitalWrite(CHARGER_ENABLE,CHARGER_OFF);
	  		delay(1);
	  		digitalWrite(LOAD_ACTIVATE, HIGH);
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

	  pinMode(POT_CS,OUTPUT);
	  pinMode(POT_RESET,OUTPUT);
	  pinMode(POT_CLK, OUTPUT);
	  pinMode(POT_MOSI, OUTPUT);
	  pinMode(CHARGER_ALERT, INPUT);
	  pinMode(PC13,OUTPUT);
	}

	bool setChargeCurrent(float current){
		digitalpot.progresistance=1109.895/current;//from datasheet
		if (current>MAX_CURRENT || digitalpot.progresistance<200){
			return 1;  //charge current cannot be larger than the max current
			//also pot cannot go lower than 200
		}
		else{
			digitalpot.chargeCurrent=current;
			
			//uint8_t data=map(resistance,200,1200,255,0);
			uint8_t data=map(digitalpot.progresistance,200,10000,0,255);
			digitalWrite(POT_CS, LOW);
			POT.transfer(0x01);
			POT.transfer(data);
			digitalWrite(POT_CS, HIGH);
			return 0;
		}
	}

	bool setIdet(float current){
		digitalpot.idetresistance=110.9895f/current; //from datasheet
		if (current>MAX_CURRENT || current>=digitalpot.chargeCurrent || digitalpot.idetresistance<200){
			return 1;  //idet cannot be larger than charge current or even larger than the max current
			//also pot cannot go lower than 200
		}
		else{
			digitalpot.idetCurrent=current;
			
			uint8_t data=map(digitalpot.idetresistance,200,10000,255,0);
			digitalWrite(POT_CS, LOW);
			POT.transfer(0x00);
			POT.transfer(data);
			digitalWrite(POT_CS, HIGH);
			return 0;
		}
		
	}



