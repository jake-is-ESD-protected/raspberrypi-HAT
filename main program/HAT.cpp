/*
	auth:			   Jakob Tschavoll, Stefan Dünser
	brief:			General HAT-class which only touches features BOTH HATs have:
                  SARA-module, RGB-LED
	date: 			May 30st, 2021
	modified by: 	Jakob T.
	notes:			
	guide:		   use isClean() both as safety check and for debugging, it prints the stored errors
*/

#include "HAT.h"

/*construct HAT-object
   -opens serial
   -GPIO init
   -boots SARA
*/
HAT::HAT(void){
   HAT_error = noError;

   int chk = wiringPiSetup();
   if(chk < 0){
      printf("WiringPi init problems...\n");
      HAT_error = wiringPi_error;
      return;
   }

   //GPIO-Setup:
   pinMode(LED_BLUE_PIN, OUTPUT);
   pinMode(LED_GREEN_PIN, OUTPUT);
  	pinMode(LED_RED_PIN, OUTPUT);
	pinMode(PWRON_PIN, OUTPUT);
	pinMode(RESET_N_PIN, OUTPUT);
   pinMode(THERMO_BUTTON_PIN, INPUT);

   //wiringPi-setup
   serial = serialOpen(PORT, BAUDRATE);
   if(serial < 0){
      printf("UART-setup failed in SARA-init\n");
      HAT_error = error_uart;
      return;
   }

   digitalWrite (LED_RED_PIN, GPIO_HIGH);
   digitalWrite (LED_GREEN_PIN, GPIO_LOW);
   digitalWrite (LED_BLUE_PIN, GPIO_LOW);
   printf("Starting HAT...\n\n");
	digitalWrite (PWRON_PIN, GPIO_LOW);
   delay(2000);
   digitalWrite(PWRON_PIN, GPIO_HIGH);
   delay(2000);
   digitalWrite (PWRON_PIN, GPIO_LOW);
   printf("SARA booting (5s)...\n");
   delay(5000);
}

/*destruct sara-object
   -closes serial
*/
HAT::~HAT(){
   serialClose(serial);
   HAT_error = noInit;
}

/*send command to SARA and get printed answers
   -opens serial
*/
bool HAT::pokeSARA(const char* message){
   if(isClean() != 1)return false;

   serialPrintf(serial, message);
   printf("command sent: %s\n", message);

   //wait, otherwise read call is too fast
   delay(2000);
   
   //RX:
	unsigned char bufReceive[500];
	int receiveLen = serialDataAvail(serial);
   if (receiveLen == 0) {
		printf("Error: Empty string!\n");
      HAT_error = sara_error_unreachable;
      return false;
	}
	else if(receiveLen < 0){
		printf("Error: Unable to receive!\n");
      HAT_error = error_uart;
      return false;
	}
   else{
      for(int i = 0; i < receiveLen; i++){
         bufReceive[i] = serialGetchar(serial);
         printf("%c", bufReceive[i]);
      }
      printf("\n");
      //serialFlush(serial);

      while(1){
         int size = serialDataAvail(serial);
         if(size > 0){
            printf("DEBUG\n");
            for(int i = 0; i < receiveLen; i++){
               bufReceive[i] = serialGetchar(serial);
               printf("%c", bufReceive[i]);
            }
            break;
         }
      }
   }


   return true;
}

/*checks for occured errors and returns/prints it as enum.
   -returns 1 in case of clean init
   -prints errors in detail
*/
uint8_t HAT::isClean(void){

   switch (HAT_error){

      case noInit :
         printf("warning: init nit occured yet\n");
         break;

      case noError :
         break;

      case error_uart :
         printf("warning: serial-init problems\n");
         break;

      case sara_error_unreachable :
         printf("warning: SARA not responding\n");
         break;

      case wiringPi_error :
         printf("warning: wiringPi init problems\n");
         break;

      case sensor_error_unreachable :
         printf("warning: sensor not responding\n");
         break;

      case NA :
         printf("warning: illegal value\n");
         break;

      default :
         printf("warning: undefined error\n");
   }
   return HAT_error;   
}

/*simplify LED-driving and set a color specified in the color-enum
   -possible colors: white, red, green, blue, yellow, cyan, purple
*/
void setColor(uint8_t color){
   switch(color){
      case white:
         digitalWrite(LED_RED_PIN, HIGH);
         digitalWrite(LED_GREEN_PIN, HIGH);
         digitalWrite(LED_BLUE_PIN, HIGH);
         break;
      case red:
         digitalWrite(LED_RED_PIN, HIGH);
         digitalWrite(LED_GREEN_PIN, LOW);
         digitalWrite(LED_BLUE_PIN, LOW);
         break;
      case green:   
         digitalWrite(LED_RED_PIN, LOW);
         digitalWrite(LED_GREEN_PIN, HIGH);
         digitalWrite(LED_BLUE_PIN, LOW);
         break;
      case blue:
         digitalWrite(LED_RED_PIN, LOW);
         digitalWrite(LED_GREEN_PIN, LOW);
         digitalWrite(LED_BLUE_PIN, HIGH);
         break;
      case yellow:
         digitalWrite(LED_RED_PIN, HIGH);
         digitalWrite(LED_GREEN_PIN, HIGH);
         digitalWrite(LED_BLUE_PIN, LOW);
         break;
      case cyan:
         digitalWrite(LED_RED_PIN, LOW);
         digitalWrite(LED_GREEN_PIN, HIGH);
         digitalWrite(LED_BLUE_PIN, HIGH);
         break;
      case purple:
         digitalWrite(LED_RED_PIN, HIGH);
         digitalWrite(LED_GREEN_PIN, LOW);
         digitalWrite(LED_BLUE_PIN, HIGH);
         break;
      case dark:
         digitalWrite(LED_RED_PIN, LOW);
         digitalWrite(LED_GREEN_PIN, LOW);
         digitalWrite(LED_BLUE_PIN, LOW);
      default:
         printf("color not implemented.\n");                                                   
   }
}