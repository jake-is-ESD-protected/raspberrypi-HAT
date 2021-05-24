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
   ///TODO: add two LEDs when new HAT is ready
  	pinMode(LED_RED_PIN, OUTPUT);
	pinMode(PWRON_PIN, OUTPUT);
	pinMode(RESET_N_PIN, OUTPUT);
   pinMode(BUTTON_PIN, INPUT);
	digitalWrite (LED_RED_PIN, GPIO_LOW);
	digitalWrite (PWRON_PIN, GPIO_HIGH);
	digitalWrite (RESET_N_PIN, GPIO_HIGH);

   //wiringPi error check
   if(mySPI < 0){
      printf("wiringPi-setup failed: wiringPi-fd: %d, SPI-fd: %d\n", myWPi, mySPI);
      HAT_error = wiringPi_error;
      return;
   }

   //sensor error check
   if(!pokeSensor(CONFIG_REG_R_COMMAND)){
      printf("warning: Can't reach sensor\n");
      HAT_error = sensor_error_unreachable;
      return;
   }

   //happy to be ready!
   printf("***************RaspberryPi temperature-HAT by Stefan & Jakob is ready!***************\n");
   for(int i = 0; i < 5; i++){
      digitalWrite(LED_RED_PIN, LOW);
      delay(100);
      digitalWrite(LED_RED_PIN, HIGH);
      delay(100);
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
      printf("\ncalculated temperature %d: %lf\n\n", i, temp);
      delay(500);
   }
   return true;
}

/*send a single read-command to sensor and get answer on terminal.
   -returns false if received buffer is empty
*/
bool HAT_temp::pokeSensor(uint8_t read_command){

   if(isClean() != 1)return false;

   uint8_t dbuf[2] = {0, 0};
   dbuf[0] = read_command;

   delay(500);

   wiringPiSPIDataRW(CE_CHANNEL, dbuf, 2);
   if((dbuf[0] == 0) && (dbuf[1] == 0)){
      printf("warning: SPI-return empty!\n");
      return false;
   }
   printf("status register 1st byte: %d\n", dbuf[0]);
   printf("status register 2nd byte: %d\n", dbuf[1]);
   
   return true;
}

/*factory-reset sensor
   -returns false if init was not clean
*/
bool HAT_temp::resetSensor(void){

   if(isClean() != 1)return false;

   unsigned char reset[4] = {RESET_BYTE, RESET_BYTE, RESET_BYTE, RESET_BYTE};
   wiringPiSPIDataRW(CE_CHANNEL, reset, 4);
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



