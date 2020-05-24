// specs2 soundchip sequence interpreter...
#include "soundchip.h"
#include "ssinter.h"

// maximum permissible notes
unsigned short noteFreqs[12]={
  0x7344,
  0x7a1e,
  0x8161,
  0x8913,
  0x913a,
  0x99dc,
  0xa302,
  0xacb4,
  0xb6f9,
  0xc1da,
  0xcd61,
  0xd998
};

int decHex(int ch) {
  switch (ch) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      return ch-'0';
      break;
    case 'A':
    case 'B':
    case 'C':
    case 'D':
    case 'E':
    case 'F':
      return 10+ch-'A';
      break;
    case 'a':
    case 'b':
    case 'c':
    case 'd':
    case 'e':
    case 'f':
      return 10+ch-'a';
      break;
  }
  return 0;
}

int bufchar(const char* buf, size_t tell, size_t bound) {
  if (tell>=bound) return EOF;
  return buf[tell];
}

#define _NEXT_ bufchar(buf,set++,size)

// returns false if end of stream
bool SSInter::next(const char* buf, int& set, size_t size) {
  char temp;
  int c;
  if ((unsigned)set>=size) {
    return false;
  }
  while ((unsigned)set<size) {
    c=_NEXT_;
    switch (c) {
      case '!': // reset channel
        memset(&out->chan[curChan],0,32);
        out->chan[curChan].duty=0x3f;
        break;
      case '$':
        temp=_NEXT_;
        if (temp=='x') {
          curChan=prefChan;
        } else {
          curChan=(temp-'0')&7;
        }
        break;
      case 'V':
        out->chan[curChan].vol=(decHex(_NEXT_)<<4);
        out->chan[curChan].vol+=decHex(_NEXT_);
        break;
      case 'Y':
        out->chan[curChan].duty=(decHex(_NEXT_)<<4);
        out->chan[curChan].duty+=decHex(_NEXT_);
        break;
      case 'f':
        out->chan[curChan].freq=(decHex(_NEXT_)<<12);
        out->chan[curChan].freq+=(decHex(_NEXT_)<<8);
        out->chan[curChan].freq+=(decHex(_NEXT_)<<4);
        out->chan[curChan].freq+=decHex(_NEXT_);
        break;
      case 'S':
        out->chan[curChan].flags.shape=_NEXT_-'0';
        break;
      case 'I':
        out->chan[curChan].flags.fmode=_NEXT_-'0';
        break;
      case 'c':
        out->chan[curChan].cutoff=(decHex(_NEXT_)<<12);
        out->chan[curChan].cutoff+=(decHex(_NEXT_)<<8);
        out->chan[curChan].cutoff+=(decHex(_NEXT_)<<4);
        out->chan[curChan].cutoff+=decHex(_NEXT_);
        break;
      case 'r':
        out->chan[curChan].reson=(decHex(_NEXT_)<<4);
        out->chan[curChan].reson+=decHex(_NEXT_);
        break;
      case 'M':
        temp=(_NEXT_-'0')&7;
        out->chan[curChan].flags.swvol=!!(temp&1);
        out->chan[curChan].flags.swfreq=!!(temp&2);
        out->chan[curChan].flags.swcut=!!(temp&4);
        break;
      case 'v':
        out->chan[curChan].swvol.speed=(decHex(_NEXT_)<<12);
        out->chan[curChan].swvol.speed+=(decHex(_NEXT_)<<8);
        out->chan[curChan].swvol.speed+=(decHex(_NEXT_)<<4);
        out->chan[curChan].swvol.speed+=decHex(_NEXT_);
        temp=(decHex(_NEXT_)<<4);
        temp+=decHex(_NEXT_);
        out->chan[curChan].swvol.amt=temp&0x1f;
        out->chan[curChan].swvol.dir=!!(temp&0x20);
        out->chan[curChan].swvol.loop=!!(temp&0x40);
        out->chan[curChan].swvol.loopi=!!(temp&0x80);
        out->chan[curChan].swvol.bound=(decHex(_NEXT_)<<4);
        out->chan[curChan].swvol.bound+=decHex(_NEXT_);
        break;
      case 'k':
        out->chan[curChan].swfreq.speed=(decHex(_NEXT_)<<12);
        out->chan[curChan].swfreq.speed+=(decHex(_NEXT_)<<8);
        out->chan[curChan].swfreq.speed+=(decHex(_NEXT_)<<4);
        out->chan[curChan].swfreq.speed+=decHex(_NEXT_);
        temp=(decHex(_NEXT_)<<4);
        temp+=decHex(_NEXT_);
        out->chan[curChan].swfreq.amt=temp&0x7f;
        out->chan[curChan].swfreq.dir=!!(temp&0x80);
        out->chan[curChan].swfreq.bound=(decHex(_NEXT_)<<4);
        out->chan[curChan].swfreq.bound+=decHex(_NEXT_);
        break;
      case 'l':
        out->chan[curChan].swcut.speed=(decHex(_NEXT_)<<12);
        out->chan[curChan].swcut.speed+=(decHex(_NEXT_)<<8);
        out->chan[curChan].swcut.speed+=(decHex(_NEXT_)<<4);
        out->chan[curChan].swcut.speed+=decHex(_NEXT_);
        temp=(decHex(_NEXT_)<<4);
        temp+=decHex(_NEXT_);
        out->chan[curChan].swcut.amt=temp&0x7f;
        out->chan[curChan].swcut.dir=!!(temp&0x80);
        out->chan[curChan].swcut.bound=(decHex(_NEXT_)<<4);
        out->chan[curChan].swcut.bound+=decHex(_NEXT_);
        break;
      case 'O':
        octave=_NEXT_-'0';
        if (octave<0) octave=0;
        if (octave>7) octave=7;
        break;
      case 'C':
        out->chan[curChan].freq=noteFreqs[0]>>(7-octave);
        break;
      case 'D':
        out->chan[curChan].freq=noteFreqs[2]>>(7-octave);
        break;
      case 'E':
        out->chan[curChan].freq=noteFreqs[4]>>(7-octave);
        break;
      case 'F':
        out->chan[curChan].freq=noteFreqs[5]>>(7-octave);
        break;
      case 'G':
        out->chan[curChan].freq=noteFreqs[7]>>(7-octave);
        break;
      case 'A':
        out->chan[curChan].freq=noteFreqs[9]>>(7-octave);
        break;
      case 'B':
        out->chan[curChan].freq=noteFreqs[11]>>(7-octave);
        break;
      case '#':
        c=_NEXT_;
        switch (c) {
          case 'C':
            out->chan[curChan].freq=noteFreqs[1]>>(7-octave);
            break;
          case 'D':
            out->chan[curChan].freq=noteFreqs[3]>>(7-octave);
            break;
          case 'F':
            out->chan[curChan].freq=noteFreqs[6]>>(7-octave);
            break;
          case 'G':
            out->chan[curChan].freq=noteFreqs[8]>>(7-octave);
            break;
          case 'A':
            out->chan[curChan].freq=noteFreqs[10]>>(7-octave);
            break;
        }
        break;
    }
    if (c=='R') break;
  }
  return true;
};

void SSInter::setChan(int ch) {
  prefChan=ch;
}

void SSInter::init(soundchip* where) {
  out=where;
}
