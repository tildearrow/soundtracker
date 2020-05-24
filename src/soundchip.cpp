#include "soundchip.h"
#include <string.h>

void soundchip::NextSample(float* l, float* r) {
  for (int i=0; i<8; i++) {
    if (chan[i].vol==0 && !chan[i].flags.swvol) {fns[i]=0; continue;}
    if (chan[i].flags.pcm) {
      ns[i]=pcm[chan[i].pcmpos];
    } else switch (chan[i].flags.shape) {
      case 0:
        ns[i]=(((cycle[i]>>15)&127)>chan[i].duty)*127;
        break;
      case 1: case 2: case 3:
        ns[i]=(short)ShapeFunctions[(chan[i].flags.shape)][(cycle[i]>>14)&255];
        break;
      case 4: case 5:
        ns[i]=(lfsr[i]&1)*127;
        break;
      case 6:
        ns[i]=randmem[i][(cycle[i]>>15)&127]*127;
        break;
      default:
        ns[i]=(short)ShapeFunctions[(chan[i].flags.shape)][(cycle[i]>>14)&255];
        break;
    }
    
    if (chan[i].flags.pcm) {
      pcmdec[i]+=chan[i].freq;
      if (pcmdec[i]>32767) {
        pcmdec[i]-=32767;
        if (chan[i].pcmpos<chan[i].pcmbnd) {
          chan[i].pcmpos++;
          chan[i].wc++;
          if (chan[i].pcmpos==chan[i].pcmbnd) {
            if (chan[i].flags.pcmloop) {
              chan[i].pcmpos=chan[i].pcmrst;
            }
          }
        } else if (chan[i].flags.pcmloop) {
          chan[i].pcmpos=chan[i].pcmrst;
        }
      }
    } else {
      ocycle[i]=cycle[i];
      if (chan[i].flags.shape==5) {
        switch ((chan[i].duty>>4)&3) {
          case 0:
            cycle[i]+=chan[i].freq*1-(chan[i].freq>>3);
            break;
          case 1:
            cycle[i]+=chan[i].freq*2-(chan[i].freq>>3);
            break;
          case 2:
            cycle[i]+=chan[i].freq*4-(chan[i].freq>>3);
            break;
          case 3:
            cycle[i]+=chan[i].freq*8-(chan[i].freq>>3);
            break;
        }
      } else {
        cycle[i]+=chan[i].freq;
      }
      if ((cycle[i]&0xf80000)!=(ocycle[i]&0xf80000)) {
        if (chan[i].flags.shape==4) {
          lfsr[i]=(lfsr[i]>>1|(((lfsr[i]) ^ (lfsr[i] >> 2) ^ (lfsr[i] >> 3) ^ (lfsr[i] >> 5) ) & 1)<<31);
        } else {
          switch ((chan[i].duty>>4)&3) {
            case 0:
              lfsr[i]=(lfsr[i]>>1|(((lfsr[i] >> 3) ^ (lfsr[i] >> 4) ) & 1)<<5);
              break;
            case 1:
              lfsr[i]=(lfsr[i]>>1|(((lfsr[i] >> 2) ^ (lfsr[i] >> 3) ) & 1)<<5);
              break;
            case 2:
              lfsr[i]=(lfsr[i]>>1|(((lfsr[i]) ^ (lfsr[i] >> 2) ^ (lfsr[i] >> 3) ) & 1)<<5);
              break;
            case 3:
              lfsr[i]=(lfsr[i]>>1|(((lfsr[i]) ^ (lfsr[i] >> 2) ^ (lfsr[i] >> 3) ^ (lfsr[i] >> 5) ) & 1)<<5);
              break;
          }
          if ((lfsr[i]&63)==0) {
            lfsr[i]=0xaaaa;
          }
        }
      }
      if (chan[i].flags.restim) {
        if (--rcycle[i]<=0) {
          cycle[i]=0;
          rcycle[i]=chan[i].restimer;
          lfsr[i]=0xaaaa;
        }
      }
    }
    //ns[i]=(char)((short)ns[i]*(short)vol[i]/256);
    fns[i]=ns[i]*chan[i].vol*2;
    if (chan[i].flags.fmode!=0) {
      float f=2*sin(3.141592653589*(((float)chan[i].cutoff)/2.5)/297500);
      int ff=f*65536;
      nslow[i]=nslow[i]+(((ff)*nsband[i])>>16);
      nshigh[i]=fns[i]-nslow[i]-(((256-chan[i].reson)*nsband[i])>>8);
      nsband[i]=(((ff)*nshigh[i])>>16)+nsband[i];
      fns[i]=(((chan[i].flags.fmode&1)?(nslow[i]):(0))+((chan[i].flags.fmode&2)?(nshigh[i]):(0))+((chan[i].flags.fmode&4)?(nsband[i]):(0)));
    }
    nsL[i]=(fns[i]*SCpantabL[(unsigned char)chan[i].pan])>>8;
    nsR[i]=(fns[i]*SCpantabR[(unsigned char)chan[i].pan])>>8;
    /*
    if ((chan[i].freq>>8)!=(oldfreq[i]>>8) || oldflags[i]!=chan[i].flags.flags) {
      bool feed=((lfsr[i]) ^ (lfsr[i] >> 2) ^ (lfsr[i] >> 3) ^ (lfsr[i] >> 5) ) & 1;
      for (int j=0; j<127; j++) {
        randmem[i][j]=randmem[i][j+1];
      }
      randmem[i][127]=lfsr[i]&1;
      lfsr[i]=(lfsr[i]>>1|feed<<31);
    }*/
    oldfreq[i]=chan[i].freq;
    oldflags[i]=chan[i].flags.flags;
    if (chan[i].flags.swvol) {
      if (--swvolt[i]<=0) {
        swvolt[i]=chan[i].swvol.speed;
        if (chan[i].swvol.dir) {
          chan[i].vol+=chan[i].swvol.amt;
          if (chan[i].vol>chan[i].swvol.bound && !chan[i].swvol.loop) {
            chan[i].vol=chan[i].swvol.bound;
          }
          if (chan[i].vol&0x80) {
            if (chan[i].swvol.loop) {
              if (chan[i].swvol.loopi) {
                chan[i].swvol.dir=!chan[i].swvol.dir;
                chan[i].vol=0xff-chan[i].vol;
              } else {
                chan[i].vol&=~0x80;
              }
            } else {
              chan[i].vol=0x7f;
            }
          }
        } else {
          chan[i].vol-=chan[i].swvol.amt;
          if (chan[i].vol&0x80) {
            if (chan[i].swvol.loop) {
              if (chan[i].swvol.loopi) {
                chan[i].swvol.dir=!chan[i].swvol.dir;
                chan[i].vol=-chan[i].vol;
              } else {
                chan[i].vol&=~0x80;
              }
            } else {
              chan[i].vol=0x0;
            }
          }
          if (chan[i].vol<chan[i].swvol.bound && !chan[i].swvol.loop) {
            chan[i].vol=chan[i].swvol.bound;
          }
        }
      }
    }
    if (chan[i].flags.swfreq) {
      if (--swfreqt[i]<=0) {
        swfreqt[i]=chan[i].swfreq.speed;
        if (chan[i].swfreq.dir) {
          if (chan[i].freq>(0xffff-chan[i].swfreq.amt)) {
            chan[i].freq=0xffff;
          } else {
            chan[i].freq=(chan[i].freq*(0x80+chan[i].swfreq.amt))>>7;
            if ((chan[i].freq>>8)>chan[i].swfreq.bound) {
              chan[i].freq=chan[i].swfreq.bound<<8;
            }
          }
        } else {
          if (chan[i].freq<chan[i].swfreq.amt) {
            chan[i].freq=0;
          } else {
            chan[i].freq=(chan[i].freq*(0xff-chan[i].swfreq.amt))>>8;
            if ((chan[i].freq>>8)<chan[i].swfreq.bound) {
              chan[i].freq=chan[i].swfreq.bound<<8;
            }
          }
        }
      }
    }
    if (chan[i].flags.swcut) {
      if (--swcutt[i]<=0) {
        swcutt[i]=chan[i].swcut.speed;
        if (chan[i].swcut.dir) {
          if (chan[i].cutoff>(0xffff-chan[i].swcut.amt)) {
            chan[i].cutoff=0xffff;
          } else {
            chan[i].cutoff+=chan[i].swcut.amt;
            if ((chan[i].cutoff>>8)>chan[i].swcut.bound) {
              chan[i].cutoff=chan[i].swcut.bound<<8;
            }
          }
        } else {
          if (chan[i].cutoff<chan[i].swcut.amt) {
            chan[i].cutoff=0;
          } else {
            chan[i].cutoff=((2048-(unsigned int)chan[i].swcut.amt)*(unsigned int)chan[i].cutoff)>>11;
            if ((chan[i].cutoff>>8)<chan[i].swcut.bound) {
              chan[i].cutoff=chan[i].swcut.bound<<8;
            }
          }
        }
      }
    }
    if (chan[i].flags.resosc) {
      cycle[i]=0;
      rcycle[i]=chan[i].restimer;
      ocycle[i]=0;
      chan[i].flags.resosc=0;
    }
  }
  tnsL=((nsL[0]+nsL[1]+nsL[2]+nsL[3]+nsL[4]+nsL[5]+nsL[6]+nsL[7]));///256;
  tnsR=((nsR[0]+nsR[1]+nsR[2]+nsR[3]+nsR[4]+nsR[5]+nsR[6]+nsR[7]));///256;
  tnsL/=32768;
  tnsR/=32768;
  *l=0.9997*(pnsL+tnsL-ppsL);
  *r=0.9997*(pnsR+tnsR-ppsR);
  pnsL=*l;
  pnsR=*r;
  ppsL=tnsL;
  ppsR=tnsR;
}

void soundchip::Init() {
  Reset();
  for (int i=0; i<65280; i++) {
    pcm[i]=0;
  }
  ShapeFunctions[0]=SCsaw;
  ShapeFunctions[1]=SCsaw;
  ShapeFunctions[2]=SCsine;
  ShapeFunctions[3]=SCtriangle;
  ShapeFunctions[4]=SCsaw;
  ShapeFunctions[5]=SCsaw;
  ShapeFunctions[6]=SCsaw;
  ShapeFunctions[7]=SCsaw;
  for (int i=0; i<256; i++) {
    SCsaw[i]=i;
    SCsine[i]=sin((i/128.0f)*M_PI)*127;
    SCtriangle[i]=(i>127)?(255-i):(i);
    SCpantabL[i]=127;
    SCpantabR[i]=127;
  }
  for (int i=0; i<128; i++) {
    SCpantabL[i]=127-i;
    SCpantabR[128+i]=i-1;
  }
  SCpantabR[128]=0;
  for (int i=0; i<1024; i++) {
    randmem[i>>7][i&127]=rand()&1;
  }
}

void soundchip::Reset() {
  for (int i=0; i<8; i++) {
    cycle[i]=0;
    resetfreq[i]=0;
    //duty[i]=64;
    //pan[i]=0;
    //cut[i]=0;
    //res[i]=0;
    //flags[i]=0;
    //postvol[i]=0;
    voldcycles[i]=0;
    volicycles[i]=0;
    fscycles[i]=0;
    sweep[i]=0;
    ns[i]=0;
    swvolt[i]=1;
    swfreqt[i]=1;
    swcutt[i]=1;
    lfsr[i]=0xaaaa;
  }
  memset(chan,0,sizeof(channel)*8);
}
