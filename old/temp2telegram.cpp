/*
auth:			Jakob Tschavoll, Stefan Dünser, Telegram API sample
brief:			write temperature from sensor to a Telegram-bot
date: 			11.05.21
modified by: 	
notes:			bot name is "raspberryHAT_DSJT_FHV_bot", high level API provided by Telegram BotFather
                bot repeats phrases and provides temperature after /temp call
guide:	        To build use: g++ temp2telegram.cpp -o temp2telegram --std=c++14 -I/usr/local/include -lTgBot -lboost_system -lssl -lcrypto -lpthread -lwiringPi
                To run use: ./temp2telegram
*/


#include <stdio.h>
#include <tgbot/tgbot.h>
#include <string>
#include "/home/pi/workspace/adt7310demo/HAT_Temperature.h"

#define BOT_TOKEN "1806971019:AAHZ7TsycZH6Z402hfbYwqqDHuiEPaEGvhA"



int main() {

    //hardware-init
    hat_init();
    delay(100);
    hat_callSensor();
    delay(100);

    //init message
    TgBot::Bot bot(BOT_TOKEN);
    bot.getEvents().onCommand("start", [&bot](TgBot::Message::Ptr message) {    
        bot.getApi().sendMessage(message->chat->id, "Hi, I'm currently wearing my temperature-HAT!");
    });

    //handle arrival of any message
    bot.getEvents().onAnyMessage([&bot](TgBot::Message::Ptr message) {
        printf("User wrote %s\n", message->text.c_str());
        if (StringTools::startsWith(message->text, "/start")) {
            return;
        }
        if(StringTools::startsWith(message->text, "/temp")){
            double temp = getTemp();
            std::string t = std::to_string(temp);
            bot.getApi().sendMessage(message->chat->id, "Current temperature on my HAT is: " + t);
        }
        else{
            bot.getApi().sendMessage(message->chat->id, "Sorry, i don't know '" + message->text + "'.");
        }
    });

    try {
        printf("Bot username: %s\n", bot.getApi().getMe()->username.c_str());
        TgBot::TgLongPoll longPoll(bot);
        while (true) {
            printf("Long poll started\n");
            longPoll.start();
        }
    } catch (TgBot::TgException& e) {
        printf("error: %s\n", e.what());
    }
    return 0;
}