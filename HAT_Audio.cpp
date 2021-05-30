/*
	auth:			Jakob Tschavoll
	brief:			MEMS-mic sampling functions
	date: 			May 20st, 2021
	modified by: 	Jakob T.
	notes:			
	guide:		    
*/

#include "HAT_Audio.h"

#define ALSA_PCM_NEW_HW_PARAMS_API

/*construct sampler object.
   -pcm device is started
   -sets error flag in case of mishaps during init
*/
HAT_audio::HAT_audio(uint16_t x_sampleRate, uint8_t x_bitDepth, uint32_t x_buflen, const char* x_devName){

    //set instances:
    sampleRate = x_sampleRate;
    bitDepth = x_bitDepth;
    bufSize = x_buflen;
    devName = x_devName;
    sampler_err = 0;
    frames = (bufSize / (bitDepth / 8)) / 2;
    bufSizeSamples = frames * 2;

    //open device
    fd = snd_pcm_open(&handle, devName, SND_PCM_STREAM_CAPTURE, 0);
    if (fd < 0) {
        fprintf(stderr, "unable to open pcm device: %s\n", snd_strerror(fd));
        sampler_err = 1;
        ///TODO: impelemnt auto-selection of device
    }

    //set bit-depth
    if(bitDepth == 32){
        bitDepthFlag = SND_PCM_FORMAT_S32_LE;
    }
    else if(bitDepth == 16){
        bitDepthFlag = SND_PCM_FORMAT_S16_LE;
    }
    else{
        fprintf(stderr, "illegal bit depth: %d. Please select 16 or 32 bit\n", bitDepth);
        sampler_err = 1;
    }

    //configure driver
    snd_pcm_hw_params_alloca(&params);                                              //Allocate a hardware parameters object.
    snd_pcm_hw_params_any(handle, params);                                          //Fill it in with default values.
    snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);    //Interleaved mode
    snd_pcm_hw_params_set_format(handle, params, bitDepthFlag);                     //Signed 32-bit little-endian format                                 
    snd_pcm_hw_params_set_channels(handle, params, 2);                              //Two channels (stereo)                                                                    
    snd_pcm_hw_params_set_rate_near(handle, params, &sampleRate, &dir);             //44100 bits/second sampling rate (CD quality)
    snd_pcm_hw_params_set_period_size_near(handle, params, &frames, &dir);          //Set period size to 32 frames.  

    fd = snd_pcm_hw_params(handle, params);                                         //Write the parameters to the driver
    if (fd < 0) {
        fprintf(stderr, "unable to set hw parameters: %s\n", snd_strerror(fd));
        sampler_err = 1;
    }

    //snd_pcm_hw_params_get_period_size(params, &frames, &dir);                       //Use a buffer large enough to hold one period

    //alloc memory
    buffer = (char *) malloc(bufSize);
    if(buffer == NULL){
        fprintf(stderr, "unable to allocate memory for internal buffer\n");
        sampler_err = 1;
    }

    //FFTW3-allocation:
    transform_buf_in = (double*) fftw_malloc(sizeof(double) * bufSizeSamples);
    transform_buf_out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * bufSizeSamples);
    fftBuffer = (double*) malloc(sizeof(double) * (bufSizeSamples/2));

    if(transform_buf_in == NULL || transform_buf_out == NULL || fftBuffer == NULL){
        fprintf(stderr, "unable to allocate memory for fft buffers\n");
        sampler_err = 1;
    }

    init_rdy = true;
}

/*reads single buffer from audio stream and stores it
   -get access to buffer via char* getBuf()
*/
uint8_t HAT_audio::readI2S(){

    if(sampler_err != 0){
        return sampler_err;
    }
    
    if(init_rdy == false){
        sampler_err = 1;
        return sampler_err;
    }  

    fd = snd_pcm_readi(handle, buffer, frames);
    if (fd == -EPIPE) {
        /* EPIPE means overrun */
        fprintf(stderr, "overrun occurred\n");
        snd_pcm_prepare(handle);
        return 1;

    } else if (fd < 0) {
        fprintf(stderr, "error from read: %s\n", snd_strerror(fd));
        return 1;

    } else if (fd != (int)frames) {
        fprintf(stderr, "short read, read %d frames\n", fd);
        return 1;
    }
    rdyBuffer = (int32_t*)(buffer);

    return 0;
}

/*calc rough Z-weighted SPL from single buffer
*/
int32_t HAT_audio::calc_dB_SPL_Z(){

    if(rdyBuffer == NULL){
        sampler_err = 1;
        return sampler_err;
    }
    if(sampler_err != 0){
        return sampler_err;
    }

    double sum = 0;

    for(int i = 0; i < bufSize; i++){
        double sample = ((double)(rdyBuffer[i])) / ((double)INT32_MAX);
        sum += (sample * sample);
    }
    
    double ms = sum / bufSize;                                  //mean
    
    int32_t decibel = ((int32_t)(10 * log10(ms))) + DBSPL_OFFS; //log
    return decibel;     
}

/*calc spectrum of single buffer
   -quality of spectrum depends on buffer-size
*/
uint8_t HAT_audio::calc_fft(){

    if(rdyBuffer == NULL){
        sampler_err = 2;
        return sampler_err;
    }
    if(sampler_err != 0){
        return sampler_err;
    }
    
    for(int i = 0; i < bufSizeSamples; i++){
        
        transform_buf_in[i] = (double)(0.5 * (1 - cos(2*M_PI*i/2047)))*(rdyBuffer[i]);
    }
    
    p = fftw_plan_dft_r2c_1d(bufSizeSamples, transform_buf_in, transform_buf_out, FFTW_MEASURE);
    fftw_execute(p);
    for(int i = 0; i < (bufSizeSamples/2); i++){
        
        fftw_complex n = {transform_buf_out[i][0], transform_buf_out[i][1]};
        
        bin = n[i, 0] * n[i, 0] + n[i, 1] * n[i, 1];

        fftBuffer[i] = bin;
    }

    return 0;
}

/*destruct sampler object.
   -buffer memory is freed
*/
HAT_audio::~HAT_audio(){

    snd_pcm_close(handle);
    free(buffer);
    buffer = NULL;
    fftw_destroy_plan(p);
    fftw_free(transform_buf_in); fftw_free(transform_buf_out);
    free(fftBuffer);
    fftBuffer = NULL;
    delete this;
}

/*get access to sample buffer
   -samples are of type int32_t
*/
int32_t* HAT_audio::getBuf(){

    return (int32_t*)buffer;
}

/*get buffer size in samples
*/
uint32_t HAT_audio::getBufSize(){

    return (uint32_t)bufSize;
}