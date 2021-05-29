all:
	gcc main.c MyGPIO.c i2c.c uart.c NBIOT.c UserIO.c -o Testprogramm -Wall