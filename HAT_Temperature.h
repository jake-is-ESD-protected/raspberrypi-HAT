/*
auth:			Jakob Tschavoll, Stefan Dünser
brief:			header for all local hardware settings and macros
date: 			May 21st, 2021
modified by: 	Jakob T.
notes:			
guide:			command-byte instructions:
                |   C7   |   C6   |   C5   |   C4   |   C3   |   C2   |   C1   |   C0   |
                |   0    |R=1, W=0|~~~~~~~~~REG_ADDR~~~~~~~~~|cont R=1|   0    |   0    |													  
*/

#ifndef HAT_TEMPERATURE_H
#define HAT_TEMPERATURE_H

//libs
#include <stdio.h>
#include <wiringPiSPI.h>
#include <wiringSerial.h>
#include <wiringPi.h>
#include <inttypes.h>
#include <pthread.h>

//register adress of ADT7310
#define STAT_REG        0x00
#define CONFIG_REG      0x01
#define TEMP_VAL_REG    0x02    //(R.O.)
#define ID_REG          0x03	//(R.O.)
#define TEMP_CRIT_REG   0x04
#define TEMP_HYST_REG   0x05
#define TEMP_HIGH_REG   0x06
#define TEMP_LOW_REG    0x07

//specific command-bytes for ADT7310
#define STAT_REG_R_COMMAND    	0b01000000
#define CONFIG_REG_R_COMMAND	0b01001000
#define VAL_REG_R_COMMAND     	0b01010000
#define ID_REG_R_COMMAND	  	0b01011000
#define CONFIG_REG_W_COMMAND	0b00001000
#define CONFIG_REG_SET_RES_16	0b10000000
#define RESET_BYTE            	0b11111111

//peripheral defines
#define PWRON_PIN 		    2
#define RESET_N_PIN 	    3
#define SPI_CLOCK           50000
#define SPI_MODE            3
#define CE_CHANNEL          0
#define LED_RED_PIN         5
#define LED_GREEN_PIN       6
#define LED_BLUE_PIN        27
#define THERMO_BUTTON_PIN   4
#define ROUGH_TEMP_OFFS     0

//#define USE_PROTOBOARD

//#define SKIP_INIT

#ifdef USE_PROTOBOARD
    #define GPIO_HIGH       LOW
    #define GPIO_LOW        HIGH
#else
    #define GPIO_HIGH       HIGH
    #define GPIO_LOW        LOW
#endif   

class HAT_temp{

private:
    enum error{
        noInit,
        noError,
        wiringPi_error,
        sensor_error_unreachable,
        NA,
        uart_error, 
    } HAT_error;  

public:
    HAT_temp(void);
    bool pokeSensor(uint8_t command);
    bool resetSensor(void);
    bool printTempSamples(int n);
    double getTemp(void);
    uint8_t isClean(void);


};

void* pollForButton(void* arg);
void* passiveSend_state(void* arg);
void* botSend_state(void* arg);
void setColor(uint8_t color);

enum color{
    white,
    red,
    green,
    blue,
    yellow,
    cyan,
    purple,
};

enum flag{
    standby,
    passiveSend,
    botSend,
} t_flag;  

extern pthread_mutex_t set_flag_mutex;
#endif