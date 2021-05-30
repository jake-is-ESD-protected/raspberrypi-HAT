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

#include "/home/pi/workspace/HATlib/raspberrypi-HAT/HAT.h"
#include "/home/pi/workspace/HATlib/raspberrypi-HAT/HAT.cpp"

pthread_mutex_t set_flag_mutex = PTHREAD_MUTEX_INITIALIZER;

int main(void){

	
	HAT hardware;

	hardware.pokeSARA("AT+COPS?\r");
	delay(1000);
	hardware.pokeSARA("AT+URAT?\r");
	delay(1000);	
	hardware.pokeSARA("AT+UMNOPROF=1\r");
	delay(1000);
	hardware.pokeSARA("AT+CREG?\r");
	delay(1000);	
	hardware.pokeSARA("AT+COPS?\r");
	delay(1000);
	// hardware.pokeSARA("AT+CEREG=2\r");
	// delay(1000);
	// hardware.pokeSARA("AT+CPWROFF\r");
	// delay(1000);

		

    return 0;
}