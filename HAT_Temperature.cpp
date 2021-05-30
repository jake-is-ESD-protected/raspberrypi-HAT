/*
	auth:			   Jakob Tschavoll, Stefan Dünser
	brief:			temperature-obj constructor, sensor communication, mini-UI, hardware-error-checks
	date: 			May 21st, 2021
	modified by: 	Jakob T.
	notes:			
	guide:		   use isClean() both as safety check and for debugging, it prints the stored errors
*/

#include "HAT_Temperature.h"
#include "HAT_Wireless.h"

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
   pinMode(THERMO_BUTTON_PIN, INPUT);

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
   t_flag = standby;

   #ifndef SKIP_INIT

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

   #endif
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
      printf("calculated temperature: %lf\n", temp);
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

void* pollForButton(void* arg){
   HAT_temp* pObj = (HAT_temp*) arg;
   int i = 0;
   while(1){
      
      if(digitalRead(THERMO_BUTTON_PIN) == LOW){
         pthread_mutex_lock(&(set_flag_mutex));
         switch(i){
            case standby:
               setColor(yellow);
               //do nothing
               t_flag = standby;
               printf("On standby\n");
               break;

            case passiveSend:
               setColor(cyan);
               //enter send-mode
               t_flag = passiveSend;
               printf("print-mode\n");
               pthread_t t_passiveSend[1];
               pthread_create(&t_passiveSend[1], NULL, passiveSend_state, arg);
               break;

            case botSend:
               setColor(purple);
               //enter chat-mode
               t_flag = botSend;
               printf("chat-mode\n");
               pthread_t t_botSend[1];
               pthread_create(&t_botSend[1], NULL, botSend_state, arg);
               break;

            default:
               break;           
         }
         pthread_mutex_unlock(&set_flag_mutex);
         while(digitalRead(THERMO_BUTTON_PIN) == LOW);
         i++;         
         if(i > botSend){
            i = standby;
         }
      }
   }
}

void* passiveSend_state(void* arg){
   HAT_temp* pObj = (HAT_temp*) arg;
   while(t_flag == passiveSend){
      pObj->printTempSamples(1);
   }
   pthread_exit(NULL);
}

void* botSend_state(void* arg){
   printf("enter botmode\n");
   HAT_temp* pObj = (HAT_temp*) arg;
   TgBot::Bot* bot = new TgBot::Bot(BOT_TOKEN);
   while(t_flag == botSend){

      bot->getEvents().onCommand("start", [bot](TgBot::Message::Ptr message) {    
         bot->getApi().sendMessage(message->chat->id, "Hi, I'm currently wearing my temperature-HAT!");
      });

      //handle arrival of any message
      bot->getEvents().onAnyMessage([bot, pObj](TgBot::Message::Ptr message) {
         printf("User wrote %s\n", message->text.c_str());
         if (StringTools::startsWith(message->text, "/start")) {
            return;
         }
         if(StringTools::startsWith(message->text, "/temp")){
            double temp = pObj->getTemp();
            std::string t = std::to_string(temp);
            bot->getApi().sendMessage(message->chat->id, "Current temperature on my HAT is: " + t + "°C");
         }
         else{
            bot->getApi().sendMessage(message->chat->id, "Sorry, i don't know '" + message->text + "'.");
         }
      });

      try {
         printf("Bot username: %s\n", bot->getApi().getMe()->username.c_str());
         TgBot::TgLongPoll longPoll(*bot);
         while (t_flag == botSend) {
               printf("Long poll started\n");
               longPoll.start();
         }
      } catch (TgBot::TgException& e) {
         printf("error: %s\n", e.what());
      }      
   }
   delete bot;
   pthread_exit(NULL);
}

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
      default:
         printf("color not implemented.\n");                                                   
   }
}



