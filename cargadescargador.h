#ifndef _Cargadescargadorlib
	#define _Cargadescargadorlib
	#include <Arduino.h>
	#include <SPI.h>
	#include <Wire.h>
	#include <Adafruit_GFX.h>
	#include <Adafruit_SSD1306_STM32.h>
	//Pin definitions
	#define OLED_RESET -1
	#define POT_CS PB12
	#define POT_CLK PB13
	#define POT_MOSI PB15
	#define POT_RESET PA8

	#define CHARGER_FAULT PA9
	#define CHARGER_ALERT PA10
	#define CHARGER_ENABLE PB3

	#ifdef  LTC4001
		#define CHARGER_ON LOW
		#define CHARGER_OFF HIGH
	#endif  //LTC
	
	#ifdef  TP4056
		#define CHARGER_ON HIGH
		#define CHARGER_OFF LOW
	#endif  //LTC

	#define CHARGER_OUT PB1
	#define CHARGER_IN PA15

	#define SD_MISO PA6
	#define SD_MOSI PA7
	#define SD_CLK PA5
	#define SD_CS PA4

	#define LOAD_ACTIVATE PB11
	#define MAX_CURRENT 2
	#define FUEL_GAUGE_ADD 0x36
	
	#define Vstep 0.000078125 //returns result in V
    #define Istep 0.00015625 //returns results in amps
	#define Capacitystep 0.5 //returns result in mah
	#define Percentagestep 1/256

	#define RepSOC_Reg 			0x06
	#define RepCap_Reg			0x05
	#define FullCapRep_Reg		0x10
	#define VCell_Reg			0x09
	#define Current_reg			0x09

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
		int16_t  rawFullRepCap;

		uint8_t  deviceAddress=FUEL_GAUGE_ADD;	
	};




	



#endif //_Cargadescargadorlib