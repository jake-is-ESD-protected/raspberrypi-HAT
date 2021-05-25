/*
	auth:			   Jakob Tschavoll, Stefan Dünser
	brief:			SARA-obj and  constructor, sensor communication, mini-UI, hardware-error-checks
	date: 			May 21st, 2021
	modified by: 	Jakob T.
	notes:			
	guide:		   use isClean() both as safety check and for debugging, it prints the stored errors
*/

#include "HAT_Wireless.h"
#include "HAT_Temperature.h"

/*construct sara-object
   -opens serial
*/
sara::sara(void){
   wireless_error = noError;
    serial = serialOpen(PORT, BAUDRATE);

   //uart error check
   if(serial < 0){
      printf("UART-setup failed in SARA-init\n");
      wireless_error = error_uart;
      return;
   }
   printf("clean sara-init\n");
}

/*destruct sara-object
   -closes serial
*/
sara::~sara(){
   serialClose(serial);
   wireless_error = noInit;
}

/*send command to SARA and get printed answers
   -opens serial
*/
bool sara::pokeSARA(const char* message){
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
      wireless_error = sara_error_unreachable;
      return false;
	}
	else if(receiveLen < 0){
		printf("Error: Unable to receive!\n");
      wireless_error = error_uart;
      return false;
	}
   else{
      for(int i = 0; i < receiveLen; i++){
         bufReceive[i] = serialGetchar(serial);
         printf("%c", bufReceive[i]);
      }
      printf("\n");
      serialFlush(serial);
   }
   return true;
}

/*checks for occured errors and returns/prints it as enum.
   -returns 1 in case of clean init
   -prints errors in detail
*/
uint8_t wirelessService::isClean(void){

   switch (wireless_error){

      case noError :
         break;

      case error_uart :
         printf("warning: serial-init problems\n");
         break;

      case noInit :
         printf("warning: init nit occured yet\n");
         break;

      case sara_error_unreachable :
         printf("warning: SARA not responding\n");
         break;

      default :
         printf("warning: undefined error\n");
   }
   return wireless_error;   
}

/*init bot-communication

*/
telegramBot::telegramBot(std::string token){
   
   TgBot::Bot bot(token);
   bot.getEvents().onCommand("start", [&bot](TgBot::Message::Ptr message) {    
      bot.getApi().sendMessage(message->chat->id, "Hi, I'm currently wearing my temperature-HAT!");
   });
}

/*send temperature to telegram chat with bot (TX only, no prompt)

*/
bool telegramBot::chatTemp(double temp){

   return true;
}