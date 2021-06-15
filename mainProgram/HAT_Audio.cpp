/*
	auth:			Jakob Tschavoll
	brief:			MEMS-mic sampling functions
	date: 			May 20st, 2021
	modified by: 	Jakob Tschavoll
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

       //happy to be ready!
    printf("\n\n***************RaspberryPi audio-HAT by Stefan & Jakob is ready!***************\n\n");

    for(int i = 0; i < 3; i++){
        gpioWrite(LED_RED_PIN, GPIO_HIGH);
        delay(200);
        gpioWrite(LED_RED_PIN, GPIO_LOW);
        gpioWrite(LED_BLUE_PIN, GPIO_HIGH);      
        delay(200);
        gpioWrite(LED_BLUE_PIN, GPIO_LOW);
        gpioWrite(LED_GREEN_PIN, GPIO_HIGH);
        delay(200);
        gpioWrite(LED_GREEN_PIN, GPIO_LOW);
    }    
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

/*creates a 512 byte padded wav-header for audio recordings
*/
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
    header[32] = (uint8_t)(blockAlign & 0xFF);             // 
    header[33] = (uint8_t)((blockAlign >> 8) & 0xFF);      //
    header[34] = (uint8_t)(bitDepth & 0xFF);               // bits/sample
    header[35] = (uint8_t)((bitDepth >> 8) & 0xFF);        //
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

/*polling thread method, gets called via pthread_create
   -enters one of 5 states and creates threads accordingly
   -see github-documentation for infos on types of states
*/
void* pollForButton_audio(void* arg){
    HAT_audio* pObj = (HAT_audio*) arg;
    int i = 0;

    while(1){
        if(digitalRead(AUDIO_BUTTON_PIN)==LOW){
            printf("in loop\n");
            delay(1000);
        }
        
    }
    while(1){
      
        if(digitalRead(AUDIO_BUTTON_PIN) == LOW){

            pthread_mutex_lock(&(set_flag_mutex));
            switch(i){
                case standby:
                setColor(yellow);
                //do nothing
                t_flag = standby;
                printf("On standby\n");
                break;

                case LEDdemo:
                //show LEDs
                t_flag = LEDdemo;
                printf("blinking\n");
                pthread_t t_LEDdemo[1];
                pthread_create(&t_LEDdemo[1], NULL, color_state, NULL);
                break;
                
                case passiveSend:
                setColor(cyan);
                //enter send-mode
                t_flag = passiveSend;
                printf("print-mode\n");
                pthread_t t_passiveSend[1];
                pthread_create(&t_passiveSend[1], NULL, passiveSend_state_audio, arg);
                break;

                case botSend:
                setColor(purple);
                //enter chat-mode
                t_flag = botSend;
                printf("chat-mode\n");
                pthread_t t_botSend[1];
                pthread_create(&t_botSend[1], NULL, botSend_state_audio, arg);
                break;

                case mqttPublish:
                setColor(white);
                //publish data to mqtt server
                t_flag = mqttPublish;
                printf("mqtt-mode\n");
                pthread_t t_mqtt[1];
                pthread_create(&t_mqtt[1], NULL, mqtt_state_audio, arg);

                default:
                break;           
            }
            pthread_mutex_unlock(&set_flag_mutex);
            while(digitalRead(AUDIO_BUTTON_PIN) == LOW);
            delay(200);
            setColor(dark);
            i++;         
            if(i > mqttPublish){
                i = standby;
            }
        }
    }
}

/*stream samples thread method, gets called via pthread_create
   -prints dBSPL-Z values to console
*/
void* passiveSend_state_audio(void* arg){
    HAT_audio* pObj = (HAT_audio*) arg;
    while(t_flag == passiveSend){
        int32_t db = pObj->calc_dB_SPL_Z();
        printf("sound pressure level (Z): %d\n", db);
   }
   pthread_exit(NULL);
}

/*create telegram-bot thread method, gets called via pthread_create
   -starts a bot to chat with
   -write /start in chat to trigger the bot
   -write /temp in chat to obtain current temperature
   -bot gets terminated after switching the state
*/
void* botSend_state_audio(void* arg){
    HAT_audio* pObj = (HAT_audio*) arg;
    TgBot::Bot* bot = new TgBot::Bot(BOT_TOKEN);
    while(t_flag == botSend){

        bot->getEvents().onCommand("start", [bot](TgBot::Message::Ptr message) {    
            bot->getApi().sendMessage(message->chat->id, "Hi, I'm currently wearing my audio-HAT!");
        });

        //handle arrival of any message
        bot->getEvents().onAnyMessage([bot, pObj](TgBot::Message::Ptr message) {
            printf("User wrote %s\n", message->text.c_str());
            if (StringTools::startsWith(message->text, "/start")) {
                return;
            }
            if(StringTools::startsWith(message->text, "/sound")){
                int32_t db = pObj->calc_dB_SPL_Z();
                std::string z = std::to_string(db);
                bot->getApi().sendMessage(message->chat->id, "Current SPL around my HAT is: " + z + "dB SPL Z");
            }
            else{
                bot->getApi().sendMessage(message->chat->id, "Sorry, i don't know '" + message->text + "'.");
            }
        });

        try {
            printf("Bot username: %s\n", bot->getApi().getMe()->username.c_str());
            TgBot::TgLongPoll longPoll(*bot);
            while (t_flag == botSend) {
                printf("Long poll started\n");
                longPoll.start();
            }
        } catch (TgBot::TgException& e) {
            printf("error: %s\n", e.what());
        }      
    }
    delete bot;
    pthread_exit(NULL);
}


void* mqtt_state_audio(void* arg){
    HAT_audio* pObj = (HAT_audio*) arg;


    mqtt_publisher* myPub = new mqtt_publisher(CHANNEL_AUDIO_ID, MQTT_AUDIO_TOPIC, MQTT_HOST, MQTT_PORT);

    while(t_flag == mqttPublish){
      
        int32_t db = pObj->calc_dB_SPL_Z();
        std::string s = std::to_string(db);
        char* pc = &s[0];
        myPub->send_message(pc);
        delay(15000);
    }
    delete myPub;
    pthread_exit(NULL);
}