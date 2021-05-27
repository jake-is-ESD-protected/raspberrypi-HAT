/*
	auth:			   Jakob Tschavoll, Stefan Dünser
	brief:			temperature-obj constructor, sensor communication, mini-UI, hardware-error-checks
	date: 			May 21st, 2021
	modified by: 	Jakob T.
	notes:			
	guide:		   use isClean() both as safety check and for debugging, it prints the stored errors
*/

#include "HAT_Temperature.h"

/*construct central HAT object.
   -all hardware peripherals are started
   -sets error flag in case of mishaps during init
*/
HAT_temp::HAT_temp(){

   //wiringPi setup:
   int myWPi = wiringPiSetup();
   int mySPI = wiringPiSPISetupMode(CE_CHANNEL, SPI_CLOCK, SPI_MODE);

   //GPIO-Setup:
   pinMode(LED_BLUE_PIN, OUTPUT);
   pinMode(LED_GREEN_PIN, OUTPUT);
  	pinMode(LED_RED_PIN, OUTPUT);
	pinMode(PWRON_PIN, OUTPUT);
	pinMode(RESET_N_PIN, OUTPUT);
   pinMode(BUTTON_PIN, INPUT);

	digitalWrite (LED_RED_PIN, GPIO_HIGH);
   digitalWrite (LED_GREEN_PIN, GPIO_LOW);
   digitalWrite (LED_BLUE_PIN, GPIO_LOW);
   printf("Starting THERMO-HAT...\n\n");
	digitalWrite (PWRON_PIN, GPIO_LOW);
   delay(2000);
   digitalWrite(PWRON_PIN, GPIO_HIGH);
   delay(2000);
   digitalWrite (PWRON_PIN, GPIO_LOW);
	

   HAT_error = noError;
   
   //wiringPi error check
   if(mySPI < 0){
      printf("wiringPi-setup failed: wiringPi-fd: %d, SPI-fd: %d\n", myWPi, mySPI);
      HAT_error = wiringPi_error;
      return;
   }

   //reset sensor
   resetSensor();

   //sensor error check
   if(!pokeSensor(CONFIG_REG_R_COMMAND)){
      printf("warning: Can't reach sensor\n");
      HAT_error = sensor_error_unreachable;
      return;
   }

   #ifdef USE_PROTOBOARD
      printf("\nInfo: config set to PROTOBOARD\n");
   #endif

   //happy to be ready!
   printf("\n\n***************RaspberryPi temperature-HAT by Stefan & Jakob is ready!***************\n\n");

   for(int i = 0; i < 3; i++){
      digitalWrite(LED_RED_PIN, GPIO_HIGH);
      delay(200);
      digitalWrite(LED_RED_PIN, GPIO_LOW);
      digitalWrite(LED_BLUE_PIN, GPIO_HIGH);      
      delay(200);
      digitalWrite(LED_BLUE_PIN, GPIO_LOW);
      digitalWrite(LED_GREEN_PIN, GPIO_HIGH);
      delay(200);
      digitalWrite(LED_GREEN_PIN, GPIO_LOW);
   }
}

/*print n samples to console
   -returns false if init was not clean
   -contains 500ms delay between samples
*/
bool HAT_temp::printTempSamples(int n){

   if(isClean() != 1)return false;

   unsigned char dbuf[3] = {0, 0};

   for(int i = 0; i < n; i++){

      dbuf[0] = VAL_REG_R_COMMAND;
      dbuf[1] = 0;
      dbuf[2] = 0;
      wiringPiSPIDataRW(CE_CHANNEL, dbuf, 3);

      uint16_t rawtemp = ((uint16_t)(dbuf[1]) << 8) | dbuf[2];
      double temp = ((double)(rawtemp)) / 128 - ROUGH_TEMP_OFFS;
      printf("calculated temperature %d: %lf\n", i, temp);
      delay(500);
   }
   return true;
}

/*send a single read-command to sensor and get answer on terminal.
   -returns false if received buffer is empty
*/
bool HAT_temp::pokeSensor(uint8_t read_command){

   if(isClean() != 1)return false;

   printf("Sensor stats:\n\n");
   unsigned char dbuf[2] = {0, 0};

   //request status register
   dbuf[0] = STAT_REG_R_COMMAND;
   dbuf[1] = 0;
   wiringPiSPIDataRW(CE_CHANNEL, dbuf, 2);
   printf("\tstatus register 1st byte: %d\n", dbuf[0]);
   printf("\tstatus register 2nd byte: %d\n", dbuf[1]);

   delay(500);

   //request ID register
   dbuf[0] = ID_REG_R_COMMAND;
   dbuf[1] = 0;
   wiringPiSPIDataRW(CE_CHANNEL, dbuf, 2);
   printf("\tID register 1st byte: %d\n", dbuf[0]);
   printf("\tID register 2nd byte: %d\n", dbuf[1]);

   delay(500);

   //request config register
   dbuf[0] = CONFIG_REG_R_COMMAND;
   dbuf[1] = 0;
   wiringPiSPIDataRW(CE_CHANNEL, dbuf, 2);
   printf("\tconfig register 1st byte: %d\n", dbuf[0]);
   printf("\tconfig register 2nd byte: %d\n", dbuf[1]);

   delay(500);

   return true;

}

/*factory-reset sensor
   -returns false if init was not clean
*/
bool HAT_temp::resetSensor(void){

   if(isClean() != 1)return false;

   unsigned char reset[4] = {RESET_BYTE, RESET_BYTE, RESET_BYTE, RESET_BYTE};
   wiringPiSPIDataRW(CE_CHANNEL, reset, 4);
   delay(500);
   return true;
}

/*receive single value temp as double
   -returns -999 if init was not clean
*/
double HAT_temp::getTemp(void){

   if(isClean() != 1)return -999;

   unsigned char dbuf[3] = {0, 0};
   dbuf[0] = VAL_REG_R_COMMAND;
   dbuf[1] = 0;
   dbuf[2] = 0;
   wiringPiSPIDataRW(CE_CHANNEL, dbuf, 3);

   uint16_t rawtemp = ((uint16_t)(dbuf[1]) << 8) | dbuf[2];
   double temp = ((double)(rawtemp)) / 128 - ROUGH_TEMP_OFFS;
   return temp;
}

/*checks for occured errors and returns/prints it as enum. 
   -returns 1 in case of clean init
   -prints errors in detail
*/
uint8_t HAT_temp::isClean(void){

   switch (HAT_error){
      case wiringPi_error :
         printf("warning: wiringPi-init problems\n");
         break;

      case noError :
         break;

      case sensor_error_unreachable :
         printf("warning: sensor not responding\n");
         break;

      default :
         printf("warning: undefined error\n");
   }
   return HAT_error;
}





