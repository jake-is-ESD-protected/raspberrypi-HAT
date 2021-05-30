//g++ -o audio-sampling -lasound -lm audio-sampling.cpp
//./audio-sampling
//based on: https://stackoverflow.com/questions/38615396/sound-recording-play-on-raspberry-pi-using-alsa-in-c

#include <alsa/asoundlib.h>
#include <math.h>
#include <inttypes.h>

#define ALSA_PCM_NEW_HW_PARAMS_API
#define SAMPLE_RATE 44100
#define BIT_DEPTH 32
#define BIG_BUF_LEN 4096
#define SAMPLE_TIME_IN_SEC_TESTPHASE 5
#define DBSPL_OFFS 94

//Contains size-adjustable wav-header with padding
void CreateWavHeader(uint8_t* header, int waveDataSize, int samplingRate, int bitDepth){
  header[0] = 'R';
  header[1] = 'I';
  header[2] = 'F';
  header[3] = 'F';
  unsigned int fileSizeMinus8 = waveDataSize + 512 - 8;
  header[4] = (uint8_t)(fileSizeMinus8 & 0xFF);
  header[5] = (uint8_t)((fileSizeMinus8 >> 8) & 0xFF);
  header[6] = (uint8_t)((fileSizeMinus8 >> 16) & 0xFF);
  header[7] = (uint8_t)((fileSizeMinus8 >> 24) & 0xFF);
  header[8] = 'W';
  header[9] = 'A';
  header[10] = 'V';
  header[11] = 'E';
  header[12] = 'f';
  header[13] = 'm';
  header[14] = 't';
  header[15] = ' ';
  header[16] = 0x10;  // fmt lenght = 16byte
  header[17] = 0x00;  // 
  header[18] = 0x00;  //
  header[19] = 0x00;  //
  header[20] = 0x01;  // format tag = 1 = PCM
  header[21] = 0x00;  //
  header[22] = 0x02;  // channels = 2 = stereo
  header[23] = 0x00;  //
  header[24] = (uint8_t)(samplingRate & 0xFF);           //sampling rate
  header[25] = (uint8_t)((samplingRate >> 8) & 0xFF);    //
  header[26] = (uint8_t)((samplingRate >> 16) & 0xFF);   //
  header[27] = (uint8_t)((samplingRate >> 24) & 0xFF);   //
  unsigned int bytePerSec = samplingRate * 2 * (bitDepth/8);
  header[28] = (uint8_t)(bytePerSec & 0xFF);             //
  header[29] = (uint8_t)((bytePerSec >> 8) & 0xFF);      //
  header[30] = (uint8_t)((bytePerSec >> 16) & 0xFF);     //
  header[31] = (uint8_t)((bytePerSec >> 24) & 0xFF);     //
  uint8_t blockAlign = 2 * ((bitDepth + 7) / 8);
  header[32] = (uint8_t)(blockAlign & 0xFF);   // 
  header[33] = (uint8_t)((blockAlign >> 8) & 0xFF);  //
  header[34] = (uint8_t)(bitDepth & 0xFF);  // bits/sample
  header[35] = (uint8_t)((bitDepth >> 8) & 0xFF);  //
  header[36] = 'p';   //new padding sub-chunk. Increases SD-performance since it writes in 512 byte sized blocks naturally
  header[37] = 'a';   //
  header[38] = 'd';   //
  header[39] = 'd';   //
  header[40] = 0xCC;  //padlen is rest of header - 512 = 460
  header[41] = 0x01;  //
  header[42] = 0x00;  //
  header[43] = 0x00;  //
  for(int i = 44; i < 504; i++){
    header[i] = 0x00;
  }
  header[504] = 'd';  //data sub-chunk
  header[505] = 'a';  //
  header[506] = 't';  // 
  header[507] = 'a';  //
  header[508] = (uint8_t)(waveDataSize & 0xFF);
  header[509] = (uint8_t)((waveDataSize >> 8) & 0xFF);
  header[510] = (uint8_t)((waveDataSize >> 16) & 0xFF);
  header[511] = (uint8_t)((waveDataSize >> 24) & 0xFF);
  //real samples get appended here
}

int main() {

    long loops;                                 //for finite recording time
    int rc;                                     //file descriptor
    int size;                                   //buffer size in samples
    snd_pcm_t *handle;                          //pcm struct handle
    snd_pcm_hw_params_t *params;                //default params handle
    unsigned int val = SAMPLE_RATE;             //sample rate
    int dir;                                    //default directory handle                                   
    snd_pcm_uframes_t frames = 2048;              //buffer size in frames
    char *buffer;                               //pointer to storage
    int32_t* buffer_big;                        //large storage
    
    int wav_fd;                                 //fd for wav file (for recording only)
    uint8_t wav_header[512];                    //header for wav-file
    int waveDataSize = 2 * SAMPLE_RATE * 4 * SAMPLE_TIME_IN_SEC_TESTPHASE; //size of wav file

    CreateWavHeader(wav_header, waveDataSize, SAMPLE_RATE, BIT_DEPTH);              //create fitting wav-header for recording
    wav_fd = open("sample.wav", O_WRONLY);
    write(wav_fd, wav_header, 512);

    rc = snd_pcm_open(&handle, "plughw:1", SND_PCM_STREAM_CAPTURE, 0);              //Open PCM device for recording (capture).
    if (rc < 0) {
        fprintf(stderr, "unable to open pcm device: %s\n", snd_strerror(rc));
        exit(1);
    }

    snd_pcm_hw_params_alloca(&params);                                              //Allocate a hardware parameters object.
    snd_pcm_hw_params_any(handle, params);                                          //Fill it in with default values.
    snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);    //Interleaved mode
    snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S32_LE);            //Signed 32-bit little-endian format                                 
    snd_pcm_hw_params_set_channels(handle, params, 2);                              //Two channels (stereo)                                                                    
    snd_pcm_hw_params_set_rate_near(handle, params, &val, &dir);                    //44100 bits/second sampling rate (CD quality)
    snd_pcm_hw_params_set_period_size_near(handle, params, &frames, &dir);          //Set period size to 32 frames.

    rc = snd_pcm_hw_params(handle, params);                                         //Write the parameters to the driver
    if (rc < 0) {
        fprintf(stderr, "unable to set hw parameters: %s\n", snd_strerror(rc));
        exit(1);
    }

    snd_pcm_hw_params_get_period_size(params, &frames, &dir);                       //Use a buffer large enough to hold one period
    size = frames * 8; /* 4 bytes/, 2 channels */
    buffer = (char *) malloc(size);
    buffer_big = (int32_t*)malloc(BIG_BUF_LEN * sizeof(int32_t));
    fprintf(stderr, "DEBUGGER: size = %d\n", size);

    snd_pcm_hw_params_get_period_time(params, &val, &dir);                          //We want to loop for 5 seconds
    loops = (SAMPLE_TIME_IN_SEC_TESTPHASE * 1000000) / val;

    double sum = 0;
    int j = 0;

    while (loops > 0) {
        loops--;

        rc = snd_pcm_readi(handle, buffer, frames);                                 //central read call
        if (rc == -EPIPE) {
            /* EPIPE means overrun */
            fprintf(stderr, "overrun occurred\n");
            snd_pcm_prepare(handle);
        } else if (rc < 0) {
            fprintf(stderr, "error from read: %s\n", snd_strerror(rc));
        } else if (rc != (int)frames) {
            fprintf(stderr, "short read, read %d frames\n", rc);
        }
        
        int32_t* samples = (int32_t*)buffer;             
        for(int i = 0; i < (size / (BIT_DEPTH / 8)); i++){
            buffer_big[j] = samples[i];
            double sample = ((double)(buffer_big[j])) / ((double)INT32_MAX);
            sum += (sample * sample);
            j++;
        }
        
        if(j == BIG_BUF_LEN){

            double ms = sum / BIG_BUF_LEN;                                   //mean
            int32_t decibel = ((int32_t)(10 * log10(ms))) + DBSPL_OFFS;      //log
            fprintf(stderr, "%" PRId32 " dB SPL Z\n", decibel);
            j = 0;
            sum = 0;
        }

        rc = write(wav_fd, buffer, size);
        if (rc != size){
            fprintf(stderr, "short write: wrote %d bytes\n", rc);
        }
    }
    fprintf(stderr, "ALSA-buffersize in byte: %d. In samples: %d. In frames: %d.\n", size, (size / (BIT_DEPTH / 8)), (size / (BIT_DEPTH / 4)));
    fprintf(stderr, "custom buffersize in byte: %d. In samples: %d. In frames: %d.\n", BIG_BUF_LEN * 4, BIG_BUF_LEN, BIG_BUF_LEN / 2);

    //snd_pcm_drain(handle);    //unknown purpose, stalls program
    snd_pcm_close(handle);
    free(buffer);
    free(buffer_big);
    close(wav_fd);

    return 0;
}