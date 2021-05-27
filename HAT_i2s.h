#ifndef I2S_H
#define I2S_H

#include <alsa/asoundlib.h>
#include <inttypes.h>
#include <math.h>
#include <fftw3.h>
#include <complex.h>

#define ALSA_PCM_NEW_HW_PARAMS_API
#define SAMPLE_RATE     44100
#define BIT_DEPTH       32
#define BUF_LEN         16384
#define DBSPL_OFFS      94
#define DEV_NAME        "plughw:1"

class sampler {

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

    //private vars for fft
    double* transform_buf_in;
    fftw_complex* transform_buf_out;
    fftw_plan p;
    double bin;

public:
    int bufSize;
    int bufSizeSamples;
    int32_t* rdyBuffer;
    const char* devName;
    int sampler_err;
    double* fftBuffer;

    sampler(uint16_t x_sampleRate, uint8_t x_bitDepth, uint32_t x_buflen, const char *x_devName);
    ~sampler();

    uint8_t readI2S();

    int32_t calc_dB_SPL_Z();

    uint8_t calc_fft();
};

#endif