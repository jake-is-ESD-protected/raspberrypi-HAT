/*
	auth:			   Jakob Tschavoll, Stefan Dünser
	brief:			establish communication from RPi4 to ADT7310 temperature sensor via SPI and to SARA-module via UART
	date: 			06.03.21
	modified by: 	Jakob Tschavoll
	notes:			reading temperature consists of 3 bytes: command byte, 16bit value
	guide:		   Make sure to enable SPI in Raspi-config
                  To build use: g++ -o demohat demohat.cpp -lwiringPi -lpigpio
                  To run use: sudo ./demohat
*/
#include "demohat.h"


bool hat_callSARA(const char* msg){

   //init serial (fd is needed here)
   int mySerial = serialOpen(PORT, BAUDRATE);
   //printf("UART-File descriptor: %d\n\n", mySerial);
   if(mySerial < 0){
      printf("UART-setup failed\n");
      return false;
   }

   //printf("UART-communication:\n\n");
   serialPrintf(mySerial, msg);
   printf("sent: %s\n", msg);

   //wait, otherwise read call is too fast
   delay(1000);
   
   //RX:
	unsigned char bufReceive[1000];
	int receiveLen = serialDataAvail(mySerial);
   if (receiveLen == 0) {
		printf("Error: Empty string!\n");
      return false;
	}
	else if(receiveLen < 0){
		printf("Error: Unable to receive!\n");
	}
   else{
      for(int i = 0; i < receiveLen; i++){
         bufReceive[i] = serialGetchar(mySerial);
         printf("%c", bufReceive[i]);
      }
      printf("\n");
      serialFlush(mySerial);
   }
   //delay(1000);
   serialClose(mySerial);

   return true;
}


int main(void){

   //peripheral setup
   gpioInitialise();
  	gpioSetMode(LED_PIN, OUTPUT);
	gpioSetMode(PWRON_PIN, OUTPUT);
	gpioSetMode(RESET_N_PIN, OUTPUT);

   gpioWrite(LED_PIN, LOW);
   printf("Starting HAT...\n\n");
	gpioWrite (PWRON_PIN, LOW);
   delay(2000);
   gpioWrite(PWRON_PIN, HIGH);
   delay(2000);
   gpioWrite (PWRON_PIN, LOW);
   delay(2000);

   hat_callSARA("AT+COPS?\r");
	delay(1000);
   hat_callSARA("AT+CGMI\r");
	delay(1000);
   hat_callSARA("AT+CPSMS=0\r");
	delay(1000);
   hat_callSARA("AT+UMNOPROF=0,1\r");
	delay(1000);	
	hat_callSARA("AT+CEREG=1\r");
	delay(1000);
	hat_callSARA("AT+CSCON=1\r");
	delay(1000);	
	hat_callSARA("AT+COPS=1,2,\"23203\",9\r");
	delay(200000);
	hat_callSARA("AT+CGPADDR=1\r");
	delay(1000);
   hat_callSARA("AT+CIND?\r");
	delay(1000);
   return 0;
}



