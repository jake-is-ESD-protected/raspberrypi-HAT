/*
	auth:			   Jakob Tschavoll, Stefan Dünser
	brief:			General HAT-class which only touches features BOTH HATs have:
                  RGB-LED, button
	date: 			May 30st, 2021
	modified by: 	Jakob T.
	notes:			
	guide:		   use isClean() both as safety check and for debugging, it prints the stored errors
*/

#include "HAT.h"

/*construct HAT-object
   -opens serial
   -GPIO init
*/
HAT::HAT(void){
   HAT_error = noError;

   //GPIO-Setup:
   gpioSetMode(LED_BLUE_PIN, PI_OUTPUT);
   gpioSetMode(LED_GREEN_PIN, PI_OUTPUT);
  	gpioSetMode(LED_RED_PIN, PI_OUTPUT);

   gpioWrite (LED_RED_PIN, GPIO_HIGH);
   gpioWrite (LED_GREEN_PIN, GPIO_LOW);
   gpioWrite (LED_BLUE_PIN, GPIO_LOW);
   printf("Starting HAT...\n\n");
}

/*destruct sara-object
*/
HAT::~HAT(){
   HAT_error = noInit;
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
         gpioWrite(LED_RED_PIN, HIGH);
         gpioWrite(LED_GREEN_PIN, HIGH);
         gpioWrite(LED_BLUE_PIN, HIGH);
         break;
      case red:
         gpioWrite(LED_RED_PIN, HIGH);
         gpioWrite(LED_GREEN_PIN, LOW);
         gpioWrite(LED_BLUE_PIN, LOW);
         break;
      case green:   
         gpioWrite(LED_RED_PIN, LOW);
         gpioWrite(LED_GREEN_PIN, HIGH);
         gpioWrite(LED_BLUE_PIN, LOW);
         break;
      case blue:
         gpioWrite(LED_RED_PIN, LOW);
         gpioWrite(LED_GREEN_PIN, LOW);
         gpioWrite(LED_BLUE_PIN, HIGH);
         break;
      case yellow:
         gpioWrite(LED_RED_PIN, HIGH);
         gpioWrite(LED_GREEN_PIN, HIGH);
         gpioWrite(LED_BLUE_PIN, LOW);
         break;
      case cyan:
         gpioWrite(LED_RED_PIN, LOW);
         gpioWrite(LED_GREEN_PIN, HIGH);
         gpioWrite(LED_BLUE_PIN, HIGH);
         break;
      case purple:
         gpioWrite(LED_RED_PIN, HIGH);
         gpioWrite(LED_GREEN_PIN, LOW);
         gpioWrite(LED_BLUE_PIN, HIGH);
         break;
      case dark:
         gpioWrite(LED_RED_PIN, LOW);
         gpioWrite(LED_GREEN_PIN, LOW);
         gpioWrite(LED_BLUE_PIN, LOW);
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
