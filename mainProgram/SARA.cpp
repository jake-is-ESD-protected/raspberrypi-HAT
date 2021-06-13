/*
	auth:			Jakob Tschavoll, Stefan Dünser
	brief:			SARA-communication code. Removed from main program because it's unusable
	date: 			June 13th, 2021
	modified by: 	Jakob T.
	notes:			
	guide:		    THIS IS DEPRICATED AND NO LONGER PART OF THE MAIN PROGRAM.
*/

//g++ -o SARA SARA.cpp -lwiringPi
//./SARA

/*send command to SARA and get printed answers
   -opens serial
*/

#include "SARA.h"
#include "wiringPi.h"
#include "stdio.h"
#include <wiringSerial.h>

#define PORT			    "/dev/ttyAMA0"
#define BAUDRATE		    115200
#define PWRON_PIN 		    2
#define RESET_N_PIN 	    3
#define LED_RED_PIN         5

bool pokeSARA(const char* message){

    int uBloxSerial = serialOpen(PORT, BAUDRATE);
    if(uBloxSerial < 0){
        printf("UART-setup failed in SARA-init\n");
        return false;
    }

   serialPrintf(uBloxSerial, message);
   printf("sent: %s\n", message);

   //wait, otherwise read call is too fast
   delay(1000);
   
   //RX:
	unsigned char bufReceive[1000];
	int receiveLen = serialDataAvail(uBloxSerial);
    if (receiveLen == 0) {
		printf("Error: Empty string!\n");
      return false;
	}
	else if(receiveLen < 0){
		printf("Error: Unable to receive!\n");
	}
    else{
      for(int i = 0; i < receiveLen; i++){
         bufReceive[i] = serialGetchar(uBloxSerial);
         printf("%c", bufReceive[i]);
      }
      printf("\n");
      serialFlush(uBloxSerial);
   }

   serialClose(uBloxSerial);

   return true;
}

void saraSetupScript(void){

    pokeSARA("AT+CGMM\r");
    delay(2000);
    printf("DEBUG\n");
    pokeSARA("AT+UMNOPROF?\r");
    delay(1000);
    pokeSARA("AT+GMR\r");
    delay(1000);
    pokeSARA("AT+CIND?\r");
    delay(1000);
    pokeSARA("AT+CFUN?\r");
    delay(1000);	
    pokeSARA("AT+COPS?\r");
    delay(1000);
    pokeSARA("AT+UGPIOC=16,10\r");
    delay(1000);
    pokeSARA("AT+UMNOPROF?\r");
    delay(1000);
    pokeSARA("AT+COPS=2\r");
    delay(1000);	
    pokeSARA("AT+COPS?\r");
    delay(1000);
    pokeSARA("AT+CIND?\r");
    delay(1000);
    pokeSARA("AT+UBANDMASK=0,524293\r");
    delay(1000);
    pokeSARA("AT+CIND?\r");
    delay(1000);    
}

//this is a replacement for larger algorithm in the main program
int main(void){

    wiringPiSetup();

    pinMode(PWRON_PIN, OUTPUT);
	pinMode(RESET_N_PIN, OUTPUT);

	digitalWrite (PWRON_PIN, LOW);
    delay(2000);
    digitalWrite(PWRON_PIN, HIGH);
    delay(2000);
    digitalWrite (PWRON_PIN, LOW);
    printf("SARA booting (5s)...\n");
    delay(5000);

    saraSetupScript();

}