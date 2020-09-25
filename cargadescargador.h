#ifndef _Cargadescargadorlib
	#define _Cargadescargadorlib
	#include <Arduino.h>
	#include <SPI.h>
	#include <Wire.h>
	#include <Adafruit_GFX.h>
	#include <Adafruit_SH1106.h>
 
	
	//#define TP4056

	//Pin definitions
	#define OLED_RESET -1
	#define POT_CS PB12
	#define POT_CLK PB13
	#define POT_MOSI PB15
	#define POT_RESET PA8
	#define SD_CS PA4
	#define SD_DETECT PC13


	#define CHARGER_ALERT PA10



	#ifndef TP4056
		#define CHARGER_ON LOW
		#define CHARGER_OFF HIGH
		#define CHARGER_FAULT PA9
		#define CHARGER_ENABLE PB3
		#define CHARGER_OUT PB1
		#define CHARGER_IN PA15
		#define MAX_CURRENT 2
	#else
		#define CHARGER_ON HIGH
		#define CHARGER_OFF LOW
		#define MAX_CURRENT 1
		#define STDBY PA9
		#define CHRG PA15
		#define CHARGER_ENABLE PB3
	#endif



	#define SD_MISO PA6
	#define SD_MOSI PA7
	#define SD_CLK PA5
	#define SD_CS PA4

	#define LOAD_ACTIVATE PB11
	
	#define FUEL_GAUGE_ADD 0x36
	
	#define Vstep 0.000078125 //returns result in V
    #define Istep 0.00015625 //returns results in amps
	#define Capacitystep 0.5 //returns result in mah
	#define Percentagestep 1/256 //returns %
    #define Tempstep	1/256   //returns degrees celsius

	#define MIN_CELL_V 3.2

	#define RepSOC_Reg 			0x06
	#define RepCap_Reg			0x05
	#define FullCapRep_Reg		0x10
	#define VCell_Reg			0x09
	#define Current_Reg			0x0A
	#define Temp_Reg			0x08
	#define avgVcell_Reg        0x19
	#define avgCurrent_Reg		0x0B
	#define avgTemp_Reg			0x16
	#define coulombCounter_Reg  0x4D
	class MAX17260{
	
	public:

		uint16_t 	designCapacity;       //in mAh
		uint16_t 	CutOffCurrent;         //in mA
		float 		emptyVoltage;			//in volts
		float	 	recoveryVoltage;       //above empty voltage
		//dynamic values
		int16_t     FullRepCap;				//in mAh
		uint16_t 	remainingCapacity;   //in mAh
		uint8_t 	reportedSOC; // %
		float 		cellVoltage;			//in V
		float 		Current;           //in A
		float       avgVoltage;
		float       avgCurrent;
		float		Temp;
		float		avgTemp;
		unsigned long coulombCounter;

		//void 		begin(uint8_t _fuelgaugeadd);
		uint16_t    read(uint8_t _address);
		void 		write(int8_t _memory_add,uint16_t _data);
		void 		setup(uint16_t _designcap, uint16_t _terminationcurrent, float _vempty, float _vrecovery);
		bool 		isAlive();
		bool	    checkPOR();
		void 		clearPOR();
		bool	    quickstart(uint16_t _designcap, uint16_t _terminationcurrent, float _vempty, float _vrecovery);
		void 		reset();
		void 		changeBattery(uint16_t _designcap, uint16_t _terminationcurrent, float _vempty, float _vrecovery);
		void 		poll();


	private:

		uint16_t rawDesignCapacity;		//weird voltage value
		uint16_t rawCutOffCurrent;
		uint16_t rawVempty;             //includes Vempty and Vrecovery
		//dynamic values
		uint16_t rawReportedSOC;		//
		uint16_t rawRemainingCapacity;   //weird voltage value
		uint16_t rawcellVoltage;
		int16_t  rawCurrent;          //signed 2' complement
		uint16_t rawFullRepCap;
		uint16_t rawavgVoltage;
		int16_t  rawavgCurrent;
		uint16_t rawcoulombCounter;
		int16_t  rawTemp;
		int16_t  rawavgTemp;

		uint8_t  deviceAddress=FUEL_GAUGE_ADD;	
	};

	class Cargadescargador{
	public:
		bool connection_state=0; // is a connection active, presupose there is none
		uint8_t chargestate=0;   //0=charging stopped 1=charging(CC) 2=charge termination (CV)
		uint16_t cycles=1;  // number of charge-discharge cycles left
		float loadresistor=2.8;// value of attached load in ohm
		bool SD_present=0;
		bool SD_init=0;
		uint8_t mode=0;       //0=standby 1=charging 2=discharging
		bool battery_present=0;
		bool battery_prepared=0;
		bool test_running=0;
		bool serialconnected=0;
		bool chitchat=0;     //am i transmitting to software?
		unsigned long chitchat_time=60;


		void setMode(uint8_t _mode);
		void adjustchargestate();



	private:

	};


	class DPOT{
	public:
		float idetCurrent=0.05;
		float chargeCurrent=0.5;
		int progresistance;
		int idetresistance;
	};




	



#endif //_Cargadescargadorlib