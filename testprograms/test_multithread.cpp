/*
	auth:			Jakob Tschavoll
	brief:			test pthreads via console and LEDs
	date: 			May 27th, 2021
	modified by: 	
	notes:			
	guide:		    
*/

//g++ -o test_multithread test_multithread.cpp -lwiringPi -lpthread
//./test_multithread

#include "/home/pi/workspace/HATlib/raspberrypi-HAT/HAT_Temperature.h"
#include "/home/pi/workspace/HATlib/raspberrypi-HAT/HAT_Temperature.cpp"
#include <pthread.h>

void* poller(void* foo){

    pinMode(THERMO_BUTTON_PIN, INPUT);
    pinMode(LED_GREEN_PIN, OUTPUT);

    while(1){
        int state = digitalRead(THERMO_BUTTON_PIN);
        if(state == LOW){
            digitalWrite(LED_GREEN_PIN, HIGH);
            delay(1000);
        }
        digitalWrite(LED_GREEN_PIN, LOW);
    }

    pthread_exit(NULL);
}

void* printer(void* foo){

    for(int i = 0; i < 10; i++){
        printf("blink ");
        delay(1000);
        printf("LED\n");
        delay(1000);
    }
    pthread_exit(NULL);
}

int main(void){

    pthread_t threads[2];
    wiringPiSetup();

    pthread_create(&threads[1], NULL, poller, NULL);
    pthread_create(&threads[2], NULL, printer, NULL);

    pthread_exit(NULL);
}

