/*
	auth:			Jakob Tschavoll
	brief:			test hardware
	date: 			May 24th, 2021
	modified by: 	
	notes:			
	guide:		    
*/

//g++ -o test_hardware test_hardware.cpp -lwiringPi
//./test_hardware

#include "/home/pi/workspace/TemperatureHAT/raspberrypi-temperatureHAT/HAT_Temperature.h"
#include "/home/pi/workspace/TemperatureHAT/raspberrypi-temperatureHAT/HAT_Temperature.cpp"

#include <stdio.h>

int main(void){

    HAT_temp myHAT;


    myHAT.printTempSamples(20);

    

    return 0;
}