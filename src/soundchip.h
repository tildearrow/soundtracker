#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
const float pi=3.1415926535;

class soundchip {
  signed char SCsaw[257];
  signed char SCsine[257];
  signed char SCtriangle[257];
  signed char SCpantabL[257];
  signed char SCpantabR[257];
  unsigned int ocycle[8];
  unsigned int cycle[8];
  int rcycle[8];
  unsigned int lfsr[8];
  signed char ns[8];
  bool randmem[8][128];
  char randpos[8];
  int fns[8];
  int nsL[8];
  int nsR[8];
  int nslow[8];
  int nshigh[8];
  int nsband[8];
  float pnsL, pnsR, ppsL, ppsR, tnsL, tnsR;
  //char (*ShapeFunctions[8])(int theduty, float value);
  signed char* ShapeFunctions[8];
  unsigned short oldfreq[8];
  unsigned short oldflags[8];
  public:
    unsigned short resetfreq[8];
    //char pan[8];
    //unsigned char res[8];
    //char postvol[8];
    unsigned short voldcycles[8];
    unsigned short volicycles[8];
    unsigned short fscycles[8];
    unsigned char sweep[8];
    unsigned short swvolt[8];
    unsigned short swfreqt[8];
    unsigned short swcutt[8];
    //int pcmpos[8];
    int pcmdec[8];
    //int pcmend[8];
    //int pcmreset[8];
    //unsigned char pcmmult[8];
    struct channel {
      unsigned short freq;
      signed char vol;
      signed char pan;
      union {
        unsigned short flags;
        struct {
          unsigned char shape: 3;
          unsigned char pcm: 1;
          unsigned char ring: 1;
          unsigned char fmode: 3;
          unsigned char resosc: 1;
          unsigned char resfilt: 1;
          unsigned char pcmloop: 1;
          unsigned char restim: 1;
          unsigned char swfreq: 1;
          unsigned char swvol: 1;
          unsigned char swcut: 1;
          unsigned char padding: 1;
        };
      } flags;
      unsigned short cutoff;
      unsigned char duty;
      unsigned char reson;
      unsigned short pcmpos;
      unsigned short pcmbnd;
      unsigned short pcmrst;
      struct {
        unsigned short speed;
        unsigned char amt: 7;
        unsigned char dir: 1;
        unsigned char bound;
      } swfreq;
      struct {
        unsigned short speed;
        unsigned char amt: 5;
        unsigned char dir: 1;
        unsigned char loop: 1;
        unsigned char loopi: 1;
        unsigned char bound;
      } swvol;
      struct {
        unsigned short speed;
        unsigned char amt: 7;
        unsigned char dir: 1;
        unsigned char bound;
      } swcut;
      unsigned short wc;
      unsigned short restimer;
    } chan[8];
    signed char pcm[65280];
    void NextSample(float* l, float* r);
    void Init();
    void Reset();
};
