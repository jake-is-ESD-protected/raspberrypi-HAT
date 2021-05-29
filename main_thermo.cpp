/*
	auth:			Jakob Tschavoll
	brief:			alpha version of full thermo-HAT program
	date: 			May 28th, 2021
	modified by: 	
	notes:			
	guide:		    
*/

//g++ -o main_thermo main_thermo.cpp --std=c++14 -I/usr/local/include -lTgBot -lboost_system -lssl -lcrypto -lpthread -lwiringPi
//./main_thermo

#include "/home/pi/workspace/HATlib/raspberrypi-HAT/HAT_Temperature.h"
#include "/home/pi/workspace/HATlib/raspberrypi-HAT/HAT_Temperature.cpp"

pthread_mutex_t set_flag_mutex = PTHREAD_MUTEX_INITIALIZER;

int main(void){

    printf("boot...\nPress the hardware-button on the HAT:\n");
    wiringPiSetup();
    pinMode(THERMO_BUTTON_PIN, INPUT);

    while(1){

        if(digitalRead(THERMO_BUTTON_PIN) == LOW){

            printf("Thermo-HAT registered. Starting thermo-branch.\n");
            break;

        }
    }

    HAT_temp* pMainHAT = new HAT_temp();
    if(pMainHAT->isClean() != 1){
        printf("Warning: init not clean, check above error.\n");
    }

    

    pthread_t threads[1];
    
    pthread_create(&threads[1], NULL, pollForButton, pMainHAT);
    pthread_join(threads[1], NULL);
    pthread_exit(NULL);

    return 0;
}