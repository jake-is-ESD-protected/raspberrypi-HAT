#ifndef SARA_H
#define SARA_H

#include <pigpio.h>
#include <wiringPi.h>
#include <stdio.h>
#include <wiringSerial.h>

#define PORT			    "/dev/ttyAMA0"
#define BAUDRATE		    115200
#define PWRON_PIN 		    27
#define RESET_N_PIN 	    22

bool pokeSARA(const char* message);
void saraSetupScript(void);

#endif