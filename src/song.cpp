#include "tracker.h"

static Pattern emptyPat;

Pattern* Song::getPattern(unsigned char num, bool create) {
  if (pat[num]==NULL) {
    if (!create) return &emptyPat;
    //printf("\x1b[1;32mCreating pattern %d.\x1b[m\n",num);
    pat[num]=new Pattern;
  }
  return pat[num];
}

Song::Song():
  version(TRACKER_VER),
  insC(0),
  patC(0),
  orders(0),
  speed(6),
  flags(0),
  tempo(0),
  DFM(0),
  channels(8),
  macrosC(0),
  globalVol(128),
  globalPan(128),
  detune(0),
  len(0) {
  memcpy(header,"TRACK8BT",8);
  memset(name,0,32);
  pcmPtr[0]=0; pcmPtr[1]=0;
  commentPtr[0]=0; commentPtr[1]=0;
  for (int i=0; i<32; i++) {
    defaultVol[i]=128;
    defaultPan[i]=((i+1)&2)?96:-96;
  }
  memset(order,0,256);
  for (int i=0; i<256; i++) {
    ins[i]=new Instrument;
    pat[i]=NULL;
  }
}

Song::~Song() {
  for (int i=0; i<256; i++) {
    if (ins[i]!=NULL) {
      delete ins[i];
      ins[i]=NULL;
    }
    if (pat[i]!=NULL) {
      //printf("\x1b[1;31mDeleting pattern %d.\x1b[m\n",i);
      delete pat[i];
      pat[i]=NULL;
    }
  }
  for (Macro* i: macros) {
    delete i;
  }
  macros.clear();
}
