#ifndef WAV_H
#define WAV_H

#include <inttypes.h>

void CreateWavHeader(uint8_t* header, int waveDataSize, int samplingRate, int bitDepth);

#endif