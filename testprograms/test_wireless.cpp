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

#include "/home/pi/workspace/HATlib/raspberrypi-HAT/HAT_Wireless.h"
#include "/home/pi/workspace/HATlib/raspberrypi-HAT/HAT_Wireless.cpp"
#include "/home/pi/workspace/HATlib/raspberrypi-HAT/HAT_Temperature.h"
#include "/home/pi/workspace/HATlib/raspberrypi-HAT/HAT_Temperature.cpp"

pthread_mutex_t set_flag_mutex = PTHREAD_MUTEX_INITIALIZER;

int main(void){

	
	HAT_temp hardware;
	sara mysara;
	mysara.pokeSARA("AT+CGMI\r");
	delay(1000);
	mysara.pokeSARA("AT+CGMI\r");
	delay(1000);	
	mysara.pokeSARA("AT+CFUN=4\r");
	delay(1000);
	mysara.pokeSARA("AT+URAT=4,3\r");
	delay(1000);
	// mysara.pokeSARA("AT+CPWROFF\r");
	// delay(1000);

		

    return 0;
}