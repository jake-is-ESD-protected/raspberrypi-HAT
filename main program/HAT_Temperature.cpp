/*
	auth:			   Jakob Tschavoll, Stefan Dünser
	brief:			temperature-obj constructor, sensor communication, mini-UI, hardware-error-checks
	date: 			May 21st, 2021
	modified by: 	Jakob T.
	notes:			
	guide:		   use isClean() both as safety check and for debugging, it prints the stored errors
*/

#include "HAT_Temperature.h"

/*construct HAT_thermo object.
   -all hardware peripherals are started
   -sets error flag in case of mishaps during init
*/
HAT_thermo::HAT_thermo(){

   //wiringPi setup:
   int mySPI = wiringPiSPISetupMode(CE_CHANNEL, SPI_CLOCK, SPI_MODE);
	
   HAT_error = noError;
   t_flag = standby;

   //wiringPi error check
   if(mySPI < 0){
      printf("wiringPi-setup failed: SPI-fd: %d\n", mySPI);
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
bool HAT_thermo::printTempSamples(int n){

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
bool HAT_thermo::pokeSensor(uint8_t read_command){

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
bool HAT_thermo::resetSensor(void){

   if(isClean() != 1)return false;

   unsigned char reset[4] = {RESET_BYTE, RESET_BYTE, RESET_BYTE, RESET_BYTE};
   wiringPiSPIDataRW(CE_CHANNEL, reset, 4);
   delay(500);
   return true;
}

/*receive single value temp as double
   -returns -999 if init was not clean
*/
double HAT_thermo::getTemp(void){

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

/*polling thread method, gets called via pthread_create
   -enters one of 3 states and creates threads accordingly
   -see github-documentation for infos on types of states
*/
void* pollForButton_thermo(void* arg){
   HAT_thermo* pObj = (HAT_thermo*) arg;
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
               pthread_create(&t_passiveSend[1], NULL, passiveSend_state_thermo, arg);
               break;

            case botSend:
               setColor(purple);
               //enter chat-mode
               t_flag = botSend;
               printf("chat-mode\n");
               pthread_t t_botSend[1];
               pthread_create(&t_botSend[1], NULL, botSend_state_thermo, arg);
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

/*stream samples thread method, gets called via pthread_create
   -prints thermo-samples every 500ms to console
*/
void* passiveSend_state_thermo(void* arg){
   HAT_thermo* pObj = (HAT_thermo*) arg;
   while(t_flag == passiveSend){
      pObj->printTempSamples(1);
   }
   pthread_exit(NULL);
}

/*create telegram-bot thread method, gets called via pthread_create
   -starts a bot to chat with
   -write /start in chat to trigger the bot
   -write /temp in chat to obtain current temperature
   -bot gets terminated after switching the state
*/
void* botSend_state_thermo(void* arg){
   printf("enter botmode\n");
   HAT_thermo* pObj = (HAT_thermo*) arg;
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