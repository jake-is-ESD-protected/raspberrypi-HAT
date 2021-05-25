/*
auth:			Jakob Tschavoll, Stefan Dünser
brief:			header for all wireless service settings and macros
date: 			May 21st, 2021
modified by: 	Jakob T.
notes:			
guide:														  
*/

#ifndef HAT_WIRELESS_H
#define HAT_WIRELESS_H

#include <inttypes.h>
#include <wiringPi.h>
#include <wiringSerial.h>
#include <stdio.h>
#include <tgbot/tgbot.h>
#include <string>

#define BOT_TOKEN       "1806971019:AAHZ7TsycZH6Z402hfbYwqqDHuiEPaEGvhA"
#define PORT			"/dev/ttyAMA0"
#define BAUDRATE		115200


class wirelessService{

private:

protected:
    enum error{
        noInit,
        noError,
        error_uart,
        sara_error_unreachable

    } wireless_error;

public:
    uint8_t isClean(void);
};

class sara : public wirelessService{

private:
    int serial;

public:
    sara(void);
    ~sara();
    bool pokeSARA(const char* command);
};

class telegramBot : public wirelessService{

private:

public:
    telegramBot(std::string token);
    bool chatTemp(double temp);
};


#endif