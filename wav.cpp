#include "wav.h"

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