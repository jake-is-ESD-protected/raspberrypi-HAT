/*
	auth:			Jakob Tschavoll, Stefan Dünser
	brief:			alpha version of full HAT program
	date: 			May 28th, 2021
	modified by: 	
	notes:			set_flag_mutex should be defined as global variable
	guide:		    for more info, check out https://github.com/jake-is-ESD-protected/raspberrypi-HAT
*/

//g++ -o main main.cpp --std=c++14 -I/usr/local/include -lTgBot -lboost_system -lssl -lcrypto -lpthread -lwiringPi -lasound -lfftw3 -lm
//./main

#include "HAT_Temperature.h"
#include "HAT_Audio.h"
#include "HAT.h"


pthread_mutex_t set_flag_mutex = PTHREAD_MUTEX_INITIALIZER;
flag t_flag;
sel_hat hat_type;

int main(void){



    wiringPiSetup();

    if(digitalRead(THERMO_BUTTON_PIN) == HIGH){

        printf("Thermo-HAT registered. Starting thermo-branch.\n");
        hat_type = thermo;

    }
    if(digitalRead(AUDIO_BUTTON_PIN) == HIGH){

        printf("Audio-HAT registered. Starting audio-branch.\n");
        hat_type = audio;
    }

    pthread_t threads[1];

    if(hat_type == thermo){
        HAT_thermo* pMainHAT_thermo = new HAT_thermo();
        if(pMainHAT_thermo->isClean() != 1){
            printf("Warning: init not clean, check above error.\n");
        }
        pthread_create(&threads[1], NULL, pollForButton_thermo, pMainHAT_thermo);
        pthread_join(threads[1], NULL);
    }
    else{
        HAT_audio* pMainHAT_audio = new HAT_audio(SAMPLE_RATE, BIT_DEPTH, BUF_LEN, DEV_NAME);
        if(pMainHAT_audio->isClean() != 1){
            printf("Warning: init not clean, check above error.\n");
        }
    }
    pthread_exit(NULL);
    return 0;
}