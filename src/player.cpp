#include "tracker.h"

int getNote(unsigned char note) {
  return (12*(note>>4)+(note&15));
}

int isNote(unsigned char note) {
  return (note!=0 && (note&15)<13);
}

// formula to calculate 6203.34:
// - 65536*440/(chipClock/64)
// chipClock is 297500 (PAL)
// or 309000 (NTSC)
unsigned int Player::getNoteFreq(float note) {
  return (int)((6203.34-(song->detune*2))*(pow(2.0f,(float)(((float)note-58)/12.0f))));
}

unsigned int Player::getNotePeriod(float note) {
  return ((297500+(song->detune*100))/(440*(pow(2.0f,(float)(((float)note-58)/12)))));
}

float Player::offsetNote(float note, unsigned char off) {
  if (off>=128) return off-128;
  if (off>=64) return note-(off-64);
  return note+off;
}

void Player::noteOn(int channel, int note) {
  ChannelStatus& status=chan[channel];
  soundchip::channel& c=chip[channel>>3].chan[channel&7];

  Instrument* ins=song->ins[status.instr];

  status.active=true;

  if (ins->filterMode&8) {
    c.flags.pcm=1;
    c.pcmpos=ins->pcmPos;
    c.pcmbnd=ins->pcmPos+ins->pcmLen;
    if (ins->pcmMult&128) {
      c.pcmrst=ins->pcmPos+ins->pcmLoop;
      c.flags.pcmloop=1;
    } else {
      c.pcmrst=0;
      c.flags.pcmloop=0;
    }
  } else {
    c.flags.pcm=0;
  }
  status.note=note;
  status.finePitch=0;

  /// FILTER MODE
  c.flags.fmode=ins->filterMode&7;
  //printf("filter mode is now %d\n",c.flags.fmode);
  c.reson=48;

  /// MACROS
  if (ins->volMacro>=0) {
    status.macroVol.load(song->macros[ins->volMacro]);
  } else {
    status.macroVol.load(NULL);
    status.envVol=255;
    status.volChanged=true;
  }
  if (ins->cutMacro>=0) {
    status.macroCut.load(song->macros[ins->cutMacro]);
  } else {
    status.macroCut.load(NULL);
  }
  if (ins->resMacro>=0) {
    status.macroRes.load(song->macros[ins->resMacro]);
  } else {
    status.macroRes.load(NULL);
  }
  if (ins->dutyMacro>=0) {
    status.macroDuty.load(song->macros[ins->dutyMacro]);
  } else {
    status.macroDuty.load(NULL);
    c.duty=63;
  }
  if (ins->shapeMacro>=0) {
    status.macroShape.load(song->macros[ins->shapeMacro]);
  } else {
    status.macroShape.load(NULL);
    c.flags.shape=0;
  }
  if (ins->pitchMacro>=0) {
    status.macroPitch.load(song->macros[ins->pitchMacro]);
  } else {
    status.macroPitch.load(NULL);
  }
  if (ins->finePitchMacro>=0) {
    status.macroFinePitch.load(song->macros[ins->finePitchMacro]);
  } else {
    status.macroFinePitch.load(NULL);
  }
  if (ins->panMacro>=0) {
    status.macroPan.load(song->macros[ins->panMacro]);
  } else {
    status.macroPan.load(NULL);
    c.pan=status.channelPan;
  }

  status.freqChanged=true;

  if (ins->flags&32) {
    c.flags.restim=true;
    c.restimer=getNotePeriod(offsetNote(note,ins->LFO));
  } else {
    c.flags.restim=false;
  }
}

void Player::noteOff(int channel) {
  ChannelStatus& status=chan[channel];

  status.macroVol.release();
  status.macroCut.release();
  status.macroRes.release();
  status.macroDuty.release();
  status.macroShape.release();
  status.macroPitch.release();
  status.macroFinePitch.release();
  status.macroPan.release();
}

void Player::noteCut(int channel) {
  ChannelStatus& status=chan[channel];
  soundchip::channel& c=chip[channel>>3].chan[channel&7];
  status.vol=0;
  c.vol=0;
  status.active=false;
}

void Player::noteAftertouch(int channel, int val) {
  ChannelStatus& status=chan[channel];
  soundchip::channel& c=chip[channel>>3].chan[channel&7];
  status.vol=val;
  c.vol=status.vol;
  status.volChanged=true;
}

void Player::noteProgramChange(int channel, int val) {
  ChannelStatus& status=chan[channel];
  soundchip::channel& c=chip[channel>>3].chan[channel&7];

  status.instr=val;
  noteAftertouch(channel,minval(127,song->ins[status.instr]->vol*2));
}

void Player::notePanChange(int channel, signed char val) {
  ChannelStatus& status=chan[channel];
  soundchip::channel& c=chip[channel>>3].chan[channel&7];

  status.channelPan=val;
  c.pan=val;
}

void Player::nextRow() {
  if (nextJump>=0) {
    pat=nextJump; step=-1;
    nextJump=-1;
    if (pat>song->orders) pat=0;
  }

  step++;

  Pattern* p=song->getPattern(song->order[pat],false);
  if (step>=p->length) {
    pat++; step=0;
    if (pat>song->orders) pat=0;
    p=song->getPattern(song->order[pat],false);
  }

  for (int i=0; i<song->channels; i++) {
    ChannelStatus& status=chan[i];
    // instrument
    if (p->data[step][i][1]!=0) {
      noteProgramChange(i,p->data[step][i][1]);
    }
    // note
    if (p->data[step][i][0]!=0) {
      switch (p->data[step][i][0]&15) {
        case 13: // OFF
          noteOff(i);
          break;
        case 14: // FADE
          break;
        case 15: // CUT
          noteCut(i);
          break;
        default:
          // NOTE ON - check for portamento
          if (0x40+p->data[step][i][3]!='G') noteOn(i,getNote(p->data[step][i][0]));
          break;
      }
    }
    // volume
    if (p->data[step][i][2]>=64 && p->data[step][i][2]<=127) {
      noteAftertouch(i,minval(127,(p->data[step][i][2]-64)*2));
    }
    // cancel previous pitch-changing effect
    switch (status.fx+0x40) {
      case 'H': case 'J':
        status.freqChanged=true;
        break;
    }
    // effect
    status.fx=p->data[step][i][3];
    status.fxVal=p->data[step][i][4];
    switch (status.fx+0x40) {
      case 'A': // speed
        speed=status.fxVal;
        break;
      case 'B': // jump
        nextJump=status.fxVal;
        break;
      case 'C': // jump to next
        nextJump=pat+1;
        break;
      case 'D': case 'K': case 'L': // volume slide
        if (status.fxVal!=0) {
          int volAdd=status.fxVal>>4;
          int volSub=status.fxVal&15;
          if (volAdd==15 && volSub!=0) {
            status.vol-=volSub;
            if (status.vol<0) status.vol=0;
            status.volChanged=true;
            status.volSlide=0;
          } else if (volSub==15 && volAdd!=0) {
            status.vol+=volAdd;
            if (status.vol>127) status.vol=127;
            status.volChanged=true;
            status.volSlide=0;
          } else {
            status.volSlide=(volAdd-volSub)*2;
          }
        }
        break;
      case 'E': // slide down
        if (status.fxVal>0 && status.fxVal<0xe0) {
          status.slideSpeed=(float)status.fxVal/16.0f;
        } else if (status.fxVal<0xf0) {
          status.note-=(float)(status.fxVal&15)/64.0f;
        } else {
          status.note-=(float)(status.fxVal&15)/16.0f;
        }
        break;
      case 'F': // slide up
        if (status.fxVal>0 && status.fxVal<0xe0) {
          status.slideSpeed=(float)status.fxVal/16.0f;
        } else if (status.fxVal<0xf0) {
          status.note+=(float)(status.fxVal&15)/64.0f;
        } else {
          status.note+=(float)(status.fxVal&15)/16.0f;
        }
        break;
      case 'G': // porta
        if (status.fxVal!=0) status.slideSpeed=(float)status.fxVal/16.0f;
        if (isNote(p->data[step][i][0])) status.slideTarget=getNote(p->data[step][i][0]);
        break;
      case 'H': // vibrato
        if ((status.fxVal>>4)!=0) {
          status.vibSpeed=4*(status.fxVal>>4);
        }
        if ((status.fxVal&15)!=0) {
          status.vibDepth=(status.fxVal&15)*4;
        }
        break;
      case 'T': // tempo
        if (status.fxVal>0x1f) {
          tempo=status.fxVal;
        }
        break;
      case 'U': // fine vibrato
        if ((status.fxVal>>4)!=0) {
          status.vibSpeed=4*(status.fxVal>>4);
        }
        if ((status.fxVal&15)!=0) {
          status.vibDepth=(status.fxVal&15);
        }
        break;
      case 'X': // channel pan
        notePanChange(i,status.fxVal);
        break;
    }
  }
  
  tick=speed;
}

void Player::update() {
  if (playMode==0) return;
  if (song==NULL) return;

  // next tick
  if (--tick<=0) {
    nextRow();
  }
  
  for (int i=0; i<song->channels; i++) {
    ChannelStatus& status=chan[i];
    if (!status.active) continue;

    soundchip::channel& c=chip[i>>3].chan[i&7];
    Instrument* ins=song->ins[status.instr];

    // run macros!
    status.macroVol.next();
    if (status.macroVol.hasChanged) {
      status.envVol=status.macroVol.value;
      status.volChanged=true;
    }

    status.macroCut.next();
    if (status.macroCut.hasChanged) {
      c.cutoff=(ins->filterH*status.macroCut.value)>>9;
    }

    status.macroRes.next();
    if (status.macroRes.hasChanged) {
      c.reson=status.macroRes.value;
    }

    status.macroDuty.next();
    if (status.macroDuty.hasChanged) {
      c.duty=status.macroDuty.value>>1;
    }

    status.macroShape.next();
    if (status.macroShape.hasChanged) {
      c.flags.shape=status.macroShape.value;
      if (song->flags&4) status.freqChanged=true;
    }

    status.macroPitch.next();
    if (status.macroPitch.hasChanged) {
      status.freqChanged=true;
    }

    status.macroFinePitch.next();
    if (status.macroFinePitch.hasChanged) {
      status.finePitch+=((signed char)status.macroFinePitch.value);
      status.freqChanged=true;
    }

    status.macroPan.next();
    if (status.macroPan.hasChanged) {
      c.pan=status.macroPan.value;
    }

    status.arpValue=0;
    status.vibValue=0;

#define VOL_SLIDE \
  status.vol+=status.volSlide; \
  if (status.vol>127) status.vol=127; \
  if (status.vol<0) status.vol=0; \
  status.volChanged=true;

#define VIBRATO \
  status.vibPos+=status.vibSpeed; \
  status.vibValue=sin(M_PI*(float)status.vibPos/128.0f)*((float)status.vibDepth/32.0f); \
  status.freqChanged=true;

#define PORTAMENTO \
  if (status.note<status.slideTarget) { \
    status.note+=status.slideSpeed; \
    if (status.note>=status.slideTarget) status.note=status.slideTarget; \
    status.freqChanged=true; \
  } else { \
    status.note-=status.slideSpeed; \
    if (status.note<=status.slideTarget) status.note=status.slideTarget; \
    status.freqChanged=true; \
  }

    // effects
    switch (status.fx+0x40) {
      case 'D': // volume slide
        VOL_SLIDE
        break;
      case 'E': // slide down
        if (status.fxVal<0xe0) {
          status.note-=status.slideSpeed;
          status.freqChanged=true;
        }
        break;
      case 'F': // slide up
        if (status.fxVal<0xe0) {
          status.note+=status.slideSpeed;
          status.freqChanged=true;
        }
        break;
      case 'G': // portamento
        PORTAMENTO
        break;
      case 'H': // vibrato
        VIBRATO
        break;
      case 'J': // arpeggio
        status.arpValue=(status.fxVal>>(8-((speed-tick)%3)*4))&15;
        status.freqChanged=true;
        break;
      case 'K': // volume slide and vibrato
        VOL_SLIDE
        VIBRATO
        break;
      case 'L': // volume slide and portamento
        VOL_SLIDE
        PORTAMENTO
        break;
    }

    // changes
    if (status.volChanged) {
      c.vol=minval(127,(status.vol*status.envVol)>>8);
      status.volChanged=false;
    }
    if (status.freqChanged) {
      unsigned int freq=getNoteFreq(offsetNote((status.note-48)+((float)ins->pitch/128.0f)+ins->noteOffset+status.vibValue+status.arpValue+((float)status.finePitch/64.0f),status.macroPitch.value));
      if (song->flags&4 && c.flags.shape==4) {
        c.freq=freq>>2;
      } else {
        c.freq=freq;
      }

      if (ins->flags&32) c.restimer=getNotePeriod(offsetNote(status.note,ins->LFO));

      status.freqChanged=false;
    }
  }
}

void Player::reset() {
  pat=0;
  step=-1;
  tick=0;
  playMode=0;
  nextJump=-1;

  if (song!=NULL) {
    speed=song->speed;
    if (song->tempo!=0) {
      tempo=song->tempo;
    } else {
      tempo=ntsc?150:125;
    }
  } else {
    speed=6;
    tempo=ntsc?150:125;
  }

  for (int i=0; i<32; i++) {
    chan[i]=ChannelStatus();
    if (chip!=NULL) {
      memset(chip[0].chan,0,256);
      memset(chip[1].chan,0,256);
      memset(chip[2].chan,0,256);
      memset(chip[3].chan,0,256);
    }
  }
}

void Player::play() {
  playMode=1;
  step=-1;
  nextJump=-1;
  tick=0;
  if (song!=NULL) {
    speed=song->speed;
    if (song->tempo!=0) {
      tempo=song->tempo;
    } else {
      tempo=ntsc?150:125;
    }
  } else {
    speed=6;
    tempo=ntsc?150:125;
  }
  if (chip!=NULL) {
    memset(chip[0].chan,0,256);
    memset(chip[1].chan,0,256);
    memset(chip[2].chan,0,256);
    memset(chip[3].chan,0,256);
  }
  for (int i=0; i<song->channels; i++) {
    chan[i]=ChannelStatus();
    chan[i].channelPan=song->defaultPan[i];
    chip[i>>3].chan[i&7].pan=chan[i].channelPan;
  }
}

void Player::stop() {
  playMode=0;
  for (int i=0; i<song->channels; i++) {
    chip[i>>3].chan[i&7].vol=0;
  }
}

void Player::setSong(Song* s) {
  song=s;
}

void Player::bindChips(soundchip* s) {
  chip=s;
}

Player::Player():
  song(NULL),
  ntsc(false) {
  reset();
}
