/*
auth:			Jakob Tschavoll
brief:			header for all sound-related hardware settings and macros
date: 			May 30st, 2021
modified by: 	Jakob T.
notes:			
guide:			
												  
*/

#ifndef HAT_AUDIO_H
#define HAT_AUDIO_H

//libs
#include <alsa/asoundlib.h>
#include <math.h>
#include <fftw3.h>
#include <complex.h>
#include "HAT.h"

//pcm-settings
#define ALSA_PCM_NEW_HW_PARAMS_API
#define SAMPLE_RATE         44100
#define BIT_DEPTH           32
#define BUF_LEN             16384
#define DBSPL_OFFS          94
#define DEV_NAME            "plughw:1"

//GPIOs
#define ROTARY_A            9
#define ROTARY_B            8

class HAT_audio : public HAT{

private:
    //private vars for sampler
    int fd;
    snd_pcm_t *handle;
    snd_pcm_hw_params_t *params;
    unsigned int sampleRate;
    uint8_t bitDepth;
    snd_pcm_uframes_t frames;   
    snd_pcm_format_t bitDepthFlag;
    int dir;
    bool init_rdy = false;
    char *buffer;
    int bufSize;
    int bufSizeSamples;
    int32_t* rdyBuffer;
    const char* devName;
    int sampler_err;
    double* fftBuffer;    

    //private vars for fft
    double* transform_buf_in;
    fftw_complex* transform_buf_out;
    fftw_plan p;
    double bin;

public:
    HAT_audio(uint16_t x_sampleRate, uint8_t x_bitDepth, uint32_t x_buflen, const char *x_devName);
    ~HAT_audio();
    uint8_t readI2S();
    int32_t calc_dB_SPL_Z();
    uint8_t calc_fft();
    int32_t* getBuf();
    uint32_t getBufSize();
};

void CreateWavHeader(uint8_t* header, int waveDataSize, int samplingRate, int bitDepth);
void* pollForButton_audio(void* arg);
void* passiveSend_state_audio(void* arg);
void* botSend_state_audio(void* arg);
void* mqtt_state_audio(void* arg);


#endif