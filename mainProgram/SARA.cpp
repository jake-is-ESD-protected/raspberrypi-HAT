/*
	auth:			Jakob Tschavoll, Stefan Dünser
	brief:			SARA-communication code. Removed from main program because it's unusable
	date: 			June 13th, 2021
	modified by: 	Jakob T.
	notes:			
	guide:		    THIS IS DEPRICATED AND NO LONGER PART OF THE MAIN PROGRAM.
*/

//g++ -o SARA SARA.cpp -lwiringPi -lpigpio
//sudo ./SARA

#include "SARA.h"

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

    pokeSARA("AT+COPS?\r");
	delay(1000);
    pokeSARA("AT+CGMI\r");
	delay(1000);
    pokeSARA("AT+CPSMS=0\r");
	delay(1000);
    pokeSARA("AT+UMNOPROF=0,1\r");
	delay(1000);	
	pokeSARA("AT+CEREG=1\r");
	delay(1000);
	pokeSARA("AT+CSCON=1\r");
	delay(1000);	
	pokeSARA("AT+COPS=1,2,\"23203\",9\r");
    printf("reboot to log into magenta, takes 3 minutes...\n");
	delay(200000);
	pokeSARA("AT+CGPADDR=1\r");
	delay(1000);
    pokeSARA("AT+CIND?\r");
	delay(1000);
}

//this is a replacement for larger algorithm in the main program
int main(void){

    gpioInitialise();
	gpioSetMode(PWRON_PIN, PI_OUTPUT);
	gpioSetMode(RESET_N_PIN, PI_OUTPUT);

    printf("booting SARA...\n\n");
	gpioWrite (PWRON_PIN, LOW);
    delay(2000);
    gpioWrite(PWRON_PIN, HIGH);
    delay(2000);
    gpioWrite (PWRON_PIN, LOW);
    delay(2000);

    saraSetupScript();

}