/*
auth:			Jakob Tschavoll, Stefan Dünser
brief:			header for hat demo
date: 			06.03.21
modified by: 	Jakob Tschavoll
notes:			
guide:			command-byte instructions:
                |   C7   |   C6   |   C5   |   C4   |   C3   |   C2   |   C1   |   C0   |
                |   0    |R=1, W=0|~~~~~~~~~REG_ADDR~~~~~~~~~|cont R=1|   0    |   0    |													  
*/

#ifndef DEMOHAT_H
#define DEMOHAT_H

//libs
#include <stdio.h>
#include <wiringPiSPI.h>
#include <wiringSerial.h>
#include <wiringPi.h>
#include <pigpio.h>
#include <inttypes.h>

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
#define PORT			"/dev/ttyAMA0"
#define BAUDRATE		115200
#define PWRON_PIN 		27
#define RESET_N_PIN 	22
#define SPI_CLOCK       50000
#define SPI_MODE        3
#define CE_CHANNEL      0
#define LED_PIN         24
#define ROUGH_TEMP_OFFS 10

bool hat_init(void);

bool hat_callSARA(const char* msg);

bool hat_callSensor(void);

bool printTempSamples(int n);

double getTemp(void);

#endif