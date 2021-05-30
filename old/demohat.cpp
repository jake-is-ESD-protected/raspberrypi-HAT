/*
	auth:			   Jakob Tschavoll, Stefan Dünser
	brief:			establish communication from RPi4 to ADT7310 temperature sensor via SPI and to SARA-module via UART
	date: 			06.03.21
	modified by: 	Jakob Tschavoll
	notes:			reading temperature consists of 3 bytes: command byte, 16bit value
	guide:		   Make sure to enable SPI in Raspi-config
                  To build use: g++ -o demohat demohat.cpp -lwiringPi
                  To run use: sudo ./demohat
*/
#include "demohat.h"


bool hat_callSARA(const char* msg){

   //init serial (fd is needed here)
   int mySerial = serialOpen(PORT, BAUDRATE);
   printf("UART-File descriptor: %d\n\n", mySerial);
   if(mySerial < 0){
      printf("UART-setup failed\n");
      return false;
   }

   printf("UART-communication:\n\n");
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
   wiringPiSetup();
  	pinMode(LED_PIN, OUTPUT);
	pinMode(PWRON_PIN, OUTPUT);
	pinMode(RESET_N_PIN, OUTPUT);

   digitalWrite (LED_PIN, HIGH);
   printf("Starting HAT...\n\n");
	digitalWrite (PWRON_PIN, LOW);
   delay(2000);
   digitalWrite(PWRON_PIN, HIGH);
   delay(2000);
   digitalWrite (PWRON_PIN, LOW);
   printf("SARA booting (5s)...\n");
   delay(5000);

   //error check
   printf("\nInitializing...\n\n");

   //happy to be ready!
   for(int i = 0; i < 5; i++){
      digitalWrite(LED_PIN, LOW);
      delay(100);
      digitalWrite(LED_PIN, HIGH);
      delay(100);
   }

   hat_callSARA("AT+COPS?\r");
	delay(1000);
	hat_callSARA("AT+URAT?\r");
	delay(1000);	
	hat_callSARA("AT+UMNOPROF=100\r");
	delay(1000);
	hat_callSARA("AT+CREG?\r");
	delay(1000);	
	hat_callSARA("AT+COPS?\r");
	delay(1000);

   return 0;
}



