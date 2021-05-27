//g++ -o i2s_main -lasound -lfftw3 -lm i2s_main.cpp
//./i2s_main

#include "/home/pi/workspace/HATlib/raspberrypi-HAT/HAT_i2s.h"
#include "/home/pi/workspace/HATlib/raspberrypi-HAT/HAT_i2s.cpp"


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

    //uint8_t i2s_fd = pMainSampler->readI2S();

    fprintf(stderr, "int-buffer size: %d\n", pMainSampler->bufSize);
    fprintf(stderr, "int-buffer size in samples: %d\n", pMainSampler->bufSizeSamples);

     int fft_fd = pMainSampler->calc_fft();
    // if(fft_fd != 0){
    //     fprintf(stderr, "fft calc failed: %d\n", fft_fd);
    // }

    // for(int i = 0; i < (pMainSampler->bufSizeSamples / 2); i++){
    //     fprintf(stderr, "%lf\n", pMainSampler->fftBuffer[i]);
    // }
    
    // delete pMainSampler;

    return 0;

}