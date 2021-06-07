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
         break;
      default:
         printf("color not implemented.\n");                                                   
   }
}

/*LED-showcase state

*/
void* color_state(void* arg){

   uint8_t thisColor = white;
   while(t_flag == LEDdemo){
      while(thisColor <= dark){
         setColor(thisColor);
         thisColor++;
         delay(200);
      }  
   }
   pthread_exit(NULL);
}

/*constructs a mqtt client specialized on publishing data
   -inits mqtt-lib
   -starts in async mode
   -starts an external thread
*/
mqtt_publisher::mqtt_publisher(const char * _id, const char * _topic, const char* _host, int _port) : mosquittopp(_id){

   mosqpp::lib_init();
   this->keepalive = 60;
   this->id = _id;
   this->port = _port;
   this->host = _host;
   this->topic = _topic;
   connect_async(host, port, keepalive);
   loop_start();
}

/*destructs a mqtt client specialized on publishing data
   -stops loop
   -kills thread
   -deallocs recources from lib init
*/
mqtt_publisher::~mqtt_publisher(){
    
   loop_stop(true);
   mosqpp::lib_cleanup();
}

/*publish content
   -returns false in case of an error
*/
bool mqtt_publisher::send_message(const char* _message){

   int ret = publish(NULL, this->topic, strlen(_message), _message, 1, false);
   return (ret == MOSQ_ERR_SUCCESS);
}

/*eventhandler for disconnection
   -TODO: EXPAND
*/
void mqtt_publisher::on_disconnect(int rc) {

   printf(">> mqtt_publisher - disconnection(%d)\n", rc);
}

/*eventhandler for connection
   -TODO: EXPAND
*/
void mqtt_publisher::on_connect(int rc){
   
   if ( rc == 0 ) {
      printf(">> mqtt_publisher - connected with server\n");
   } 
   else {
      printf(">> mqtt_publisher - Impossible to connect with server(%d)\n", rc);
   }
}

/*eventhandler for publishing
   -TODO: EXPAND
*/
void mqtt_publisher::on_publish(int mid){

   printf(">> mqtt_publisher - Message (%d) succeed to be published\n", mid);
}
