/*
auth:			Jakob Tschavoll, Stefan Dünser
brief:			establish communication from RPi4 to ADT7310 temperature sensor via SPI
date: 			06.03.21
modified by: 	Jakob Tschavoll
notes:			reading temperature consists of 3 bytes: command byte, 16bit value.
                calling hat_callSensor is necessary before gainig access to samples
guide:		    Make sure to enable SPI in Raspi-config
                To build use: g++ -o TsensorHAT TsensorHAT.cpp -lwiringPi
                To run use: sudo ./TsensorHAT
*/
#include "demohat.h"

bool hat_init(void){

    //peripheral setup
    wiringPiSetup();
    int mySPI = wiringPiSPISetupMode(CE_CHANNEL, SPI_CLOCK, SPI_MODE);
    pinMode(LED_PIN, OUTPUT);
    pinMode(PWRON_PIN, OUTPUT);
    pinMode(RESET_N_PIN, OUTPUT);

    digitalWrite (LED_PIN, HIGH);
    digitalWrite (PWRON_PIN, HIGH);
    digitalWrite (RESET_N_PIN, HIGH);

    //error check
    printf("\nInitializing...\n\n");
    printf("SPI-File descriptor: %d\n\n", mySPI);
    if(mySPI < 0){
        printf("SPI-setup failed\n");
        return false;
    }

    //happy to be ready!
    for(int i = 0; i < 5; i++){
        digitalWrite(LED_PIN, LOW);
        delay(100);
        digitalWrite(LED_PIN, HIGH);
        delay(100);
    }
    return true;
}

bool getTempSamples(int n){
    unsigned char dbuf[3] = {0, 0};

    for(int i = 0; i < n; i++){

        dbuf[0] = VAL_REG_R_COMMAND;
        dbuf[1] = 0;
        dbuf[2] = 0;
        wiringPiSPIDataRW(CE_CHANNEL, dbuf, 3);

        uint16_t rawtemp = ((uint16_t)(dbuf[1]) << 8) | dbuf[2];
        double temp = ((double)(rawtemp)) / 128;
        printf("\ncalculated temperature %d: %lf\n\n", i, temp);
        delay(500);
    }
    return true;
}

bool hat_callSensor(void){

    unsigned char reset[4] = {RESET_BYTE, RESET_BYTE, RESET_BYTE, RESET_BYTE};
    unsigned char dbuf[2] = {0, 0};
    printf("SPI-communication:\n\n");
    
    //factory reset
    wiringPiSPIDataRW(CE_CHANNEL, reset, 4);

    //request status register
    dbuf[0] = STAT_REG_R_COMMAND;
    dbuf[1] = 0;
    wiringPiSPIDataRW(CE_CHANNEL, dbuf, 2);
    printf("status register 1st byte: %d\n", dbuf[0]);
    printf("status register 2nd byte: %d\n", dbuf[1]);

    delay(500);

    //request ID register
    dbuf[0] = ID_REG_R_COMMAND;
    dbuf[1] = 0;
    wiringPiSPIDataRW(CE_CHANNEL, dbuf, 2);
    printf("ID register 1st byte: %d\n", dbuf[0]);
    printf("ID register 2nd byte: %d\n", dbuf[1]);

    delay(500);

    //request config register
    dbuf[0] = CONFIG_REG_R_COMMAND;
    dbuf[1] = 0;
    wiringPiSPIDataRW(CE_CHANNEL, dbuf, 2);
    printf("config register 1st byte: %d\n", dbuf[0]);
    printf("config register 2nd byte: %d\n", dbuf[1]);

    delay(500);

    //request temperature
    getTempSamples(1);

    return true;
}

//test program
int main(){

    hat_init();
    delay(100);
    hat_callSensor();
    delay(100);
    getTempSamples(20);

    return 0;
}
