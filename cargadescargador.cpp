

#include "cargadescargador.h"
/*void MAX17260::begin(uint8_t _fuelgaugeadd){
	deviceAddress=_fuelgaugeadd;
}*/
uint16_t MAX17260::read(uint8_t _address){
	uint16_t temp;
	uint8_t buffarray[2];
	Wire.beginTransmission(deviceAddress);
	Wire.write(_address);  //set the memory address and leave the communication open 
	Wire.endTransmission();
	Wire.requestFrom(deviceAddress,2,1); //request 2 bytes to the fuel gauge and send a stop signal
	buffarray[0]=Wire.read(); //lsb
	buffarray[1]=Wire.read(); //msb
	temp=(buffarray[1]<<8)|(buffarray[0]); //stick them together
	return temp;
}

void MAX17260::write(int8_t _memory_add,uint16_t _data){
	Wire.beginTransmission(deviceAddress);
	Wire.write(_memory_add);
	Wire.write((uint8_t)(_data)); //send lsb
	Wire.write(_data>>8); //send msb
	Wire.endTransmission(1); //send a stop signal
}



void MAX17260::setup(uint16_t _designcap, uint16_t _terminationcurrent, float _vempty, float _vrecovery){
	
	//calculate parameters from values given
	CutOffCurrent=_terminationcurrent;
	designCapacity=_designcap;
	rawDesignCapacity=_designcap*2;
	rawCutOffCurrent=_terminationcurrent*6400;
	rawVempty=(((uint8_t)_vempty*100)<<7)|((uint8_t)_vrecovery*400);
	emptyVoltage=_vempty;
	recoveryVoltage=_vrecovery;
	uint16_t HibCFG;
	uint16_t Status;
	//Step 1. Delay until FSTAT.DNR bit == 0 
	while(this->read(0x3D)&0x0001){
		delay(100);
	}

	//Step 2: Initialize Configuration
	HibCFG=this->read(0xBA);
	this->write(0x60,0x90);
	this->write(0xBA,0x00);
	this->write(0x60,0x00);

	//2.1 OPTION 1 EZ Config (No INI file is needed):
	this->write(0x18,rawDesignCapacity);
	this->write(0x1E,rawCutOffCurrent);
	this->write(0x3A , rawVempty); //both configuration bits are on the same register, and yes, verecovery is 7 bits long 
	this->write(0xDB , 0x8000) ; //where 0x8000 means 4.2v max voltage


	this->write (0xBA , HibCFG) ; // Restore Original HibCFG value

	//Step 3: Initialization Complete
	Status=this->read(0x00);
	this->write(0x00,(Status&0xFFFD));
}

bool MAX17260::isAlive(){
	Wire.beginTransmission(deviceAddress);
	if(Wire.endTransmission(1)==0){//success
		return 1;
	}
	else{
		return 0;
	}
}


bool MAX17260::checkPOR(){    //0 hasnt been reset, 1 has been reset and not cleared
	return (this->read(0x00)& 0x0002);
}

void MAX17260::clearPOR(){
	uint16_t _temp=this->read(0x00);
	_temp&= ~(1UL << 2);
	this->write(0x00,_temp);
}

bool MAX17260::quickstart(uint16_t _designcap, uint16_t _terminationcurrent, float _vempty, float _vrecovery){ //only for verification of device, DO NOT USE UNDER NORMAL OPERATION
	uint16_t data;
	this->setup(_designcap,_terminationcurrent,_vempty,_vrecovery);
	data=this->read(0x2B);
	data|=0x0400;
	this->write(0x2B,data);

	while(this->read(0xDB)&0x8000 && this->read(0x3D)&0x0001){
		delay(10); //wait for the fuel gauge to be ready
	}
	rawRemainingCapacity=this->read(0x05);
	rawReportedSOC=this->read(0x06);

}

void MAX17260::reset(){
	this->write(0x60,0x0F);		//uknown command
	this->write(0xBB,0x01);  //affects the config2 register
}

void MAX17260::changeBattery(uint16_t _designcap, uint16_t _terminationcurrent, float _vempty, float _vrecovery){
	this->reset();
	this->setup(_designcap,_terminationcurrent,_vempty,_vrecovery);
}

void MAX17260::poll(){ //update dynamic variables, must be called periodically
	rawReportedSOC=this->read(RepSOC_Reg);
	rawRemainingCapacity=this->read(RepCap_Reg);
	rawcellVoltage=this->read(VCell_Reg);
	rawCurrent=this->read(Current_reg);
	rawFullRepCap=this->read(FullCapRep_Reg);

	FullRepCap=rawFullRepCap*Capacitystep;
	Current=rawCurrent*Istep;
	cellVoltage=rawcellVoltage*Vstep;
	remainingCapacity=rawRemainingCapacity*Capacitystep;
	reportedSOC=rawReportedSOC*Percentagestep;
}


