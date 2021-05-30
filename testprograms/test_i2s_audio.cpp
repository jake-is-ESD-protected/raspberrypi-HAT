/*
	auth:			Jakob Tschavoll
	brief:			test sampling and fft
	date: 			May 30th, 2021
	modified by: 	
	notes:			
	guide:		    
*/

//g++ -o test_i2s_audio -lasound -lfftw3 -lm test_i2s_audio.cpp
//./test_i2s_audio

#include "/home/pi/workspace/HATlib/raspberrypi-HAT/HAT_Audio.h"
#include "/home/pi/workspace/HATlib/raspberrypi-HAT/HAT_Audio.cpp"


int main(){

    sampler* pMainSampler = new sampler(SAMPLE_RATE, BIT_DEPTH, BUF_LEN, DEV_NAME);

    for(int i = 0; i < 100; i++){
        uint8_t fd = pMainSampler->readI2S();
        if(fd != 0){
            fprintf(stderr, "error occured on wrapper layer!\n");
            return 0;
        }
        fprintf(stderr, "calc dB: %d\n", pMainSampler->calc_dB_SPL_Z());
    }

    fprintf(stderr, "int-buffer size: %d\n", pMainSampler->getBufSize());
    fprintf(stderr, "int-buffer size in samples: %d\n", (pMainSampler->getBufSize() * (BIT_DEPTH / 8)));

    return 0;

}