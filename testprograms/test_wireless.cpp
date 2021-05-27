/*
	auth:			Jakob Tschavoll
	brief:			test wireless peripherals
	date: 			May 24th, 2021
	modified by: 	
	notes:			
	guide:		    
*/

//g++ test_wireless.cpp -o test_wireless --std=c++14 -I/usr/local/include -lTgBot -lboost_system -lssl -lcrypto -lpthread -lwiringPi
//./test_wireless

#include "/home/pi/workspace/TemperatureHAT/raspberrypi-temperatureHAT/HAT_Wireless.h"
#include "/home/pi/workspace/TemperatureHAT/raspberrypi-temperatureHAT/HAT_Wireless.cpp"
#include "/home/pi/workspace/TemperatureHAT/raspberrypi-temperatureHAT/HAT_Temperature.h"
#include "/home/pi/workspace/TemperatureHAT/raspberrypi-temperatureHAT/HAT_Temperature.cpp"

int main(void){

	HAT_temp hardware;
    //telegramBot bot(BOT_TOKEN);
	sara mysara;
	mysara.pokeSARA("AT&V\r");

    return 0;
}