#include "soundchip.h"
#include <string.h>

#ifdef _WIN32
#define M_PI 3.141592653589793238
#endif

void HLESoundchip::NextSample(short* l, short* r) {
  *l=0;
  *r=0;
}

void HLESoundchip::Init() {
  Reset();
  for (int i=0; i<65280; i++) {
    pcm[i]=0;
  }
}

void HLESoundchip::Reset() {
  for (int i=0; i<8; i++) {
  }
  memset(chan,0,sizeof(channel)*8);
}
