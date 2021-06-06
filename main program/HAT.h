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
#include <mosquittopp.h>

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

#define MQTT_PORT           1883
#define MQTT_HOST           "192.168.2.141"
#define MQTT_ID_THERMO      "T_HAT"
#define MQTT_ID_AUDIO       "A_HAT"
#define MQTT_TOPIC_THERMO   "thermo"
#define MQTT_TOPIC_AUDIO    "audio"

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

class mqtt_publisher : public mosqpp::mosquittopp
{
    private:
        const char* host;
        const char* id;
        const char* topic;
        int port;
        int keepalive;

        void on_connect(int rc);
        void on_disconnect(int rc);
        void on_publish(int mid);
        
    public:
        mqtt_publisher(const char *id, const char * _topic, const char *host, int port);
        ~mqtt_publisher();
        bool send_message(const char * _message);
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
    mqttPublish,
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