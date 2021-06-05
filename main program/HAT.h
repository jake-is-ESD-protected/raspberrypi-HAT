/*
auth:			Jakob Tschavoll, Stefan Dünser
brief:			header for all general HAT-class
date: 			May 21st, 2021
modified by: 	Jakob T.
notes:			
guide:														  
*/

#ifndef HAT_H
#define HAT_H

#include <inttypes.h>
#include <wiringPi.h>
#include <wiringSerial.h>
#include <stdio.h>
#include <tgbot/tgbot.h>
#include <string>

#define BOT_TOKEN           "1806971019:AAHZ7TsycZH6Z402hfbYwqqDHuiEPaEGvhA"
#define PORT			    "/dev/ttyAMA0"
#define BAUDRATE		    115200

#define PWRON_PIN 		    2
#define RESET_N_PIN 	    3
#define LED_RED_PIN         5
#define LED_GREEN_PIN       6
#define LED_BLUE_PIN        27
#define THERMO_BUTTON_PIN   4
#define AUDIO_BUTTON_PIN    7

#define GPIO_HIGH           HIGH
#define GPIO_LOW            LOW

class HAT{

private:
    int serial;

public:
    HAT(void);
    ~HAT();
    bool pokeSARA(const char* command);
    uint8_t isClean(void);

    enum error{
        noInit,
        noError,
        error_uart,
        sara_error_unreachable,
        wiringPi_error,
        sensor_error_unreachable,
        NA,
    }HAT_error;    
};

void setColor(uint8_t color);

enum color{
    white,
    red,
    green,
    blue,
    yellow,
    cyan,
    purple,
    dark,
};

enum flag{
    standby,
    passiveSend,
    botSend,
    off,
};
extern flag t_flag;

enum sel_hat{
    thermo,
    audio,
};
extern sel_hat hat_type;

extern pthread_mutex_t set_flag_mutex;

#endif