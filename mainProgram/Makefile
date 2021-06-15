#target: dependencies
#	action

# -o for executable
# -c for object file

all: main

main: main.cpp
	g++ main.cpp HAT.cpp HAT_Audio.cpp HAT_Temperature.cpp -o main --std=c++14 -I/usr/local/include -lTgBot -lboost_system -lssl -lcrypto -lpthread -lwiringPi -lasound -lfftw3 -lm -lmosquittopp -lpigpio

clean:
	rm -f *.o main	