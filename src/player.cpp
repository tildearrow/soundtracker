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
  status.vibPos=0;
  status.vibValue=0;
  status.tremPos=0;
  status.tremValue=0;

  /// FILTER MODE
  c.flags.fmode=ins->filterMode&7;
  if (ins->filterMode&7) {
    c.cutoff=ins->filterH;
  }
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
  if (ins->filterModeMacro>=0) {
    status.macroFilterMode.load(song->macros[ins->filterModeMacro]);
  } else {
    status.macroFilterMode.load(NULL);
  }
  if (ins->volSweepMacro>=0) {
    status.macroVolSweep.load(song->macros[ins->volSweepMacro]);
  } else {
    status.macroVolSweep.load(NULL);
  }
  if (ins->freqSweepMacro>=0) {
    status.macroFreqSweep.load(song->macros[ins->freqSweepMacro]);
  } else {
    status.macroFreqSweep.load(NULL);
  }
  if (ins->cutSweepMacro>=0) {
    status.macroCutSweep.load(song->macros[ins->cutSweepMacro]);
  } else {
    status.macroCutSweep.load(NULL);
  }
  if (ins->pcmPosMacro>=0) {
    status.macroPCM.load(song->macros[ins->pcmPosMacro]);
  } else {
    status.macroPCM.load(NULL);
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
  status.macroFilterMode.release();
  status.macroVolSweep.release();
  status.macroFreqSweep.release();
  status.macroCutSweep.release();
  status.macroPCM.release();
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
  //soundchip::channel& c=chip[channel>>3].chan[channel&7];

  status.instr=val;
  status.vibPos=0;
  status.vibValue=0;
  status.tremPos=0;
  status.tremValue=0;
  noteAftertouch(channel,minval(127,song->ins[status.instr]->vol*2));
}

void Player::notePanChange(int channel, signed char val) {
  ChannelStatus& status=chan[channel];
  soundchip::channel& c=chip[channel>>3].chan[channel&7];

  status.channelPan=val;
  c.pan=val;
}

void Player::testNoteOn(int channel, int ins, int note) {
  scheduledNotes.push(ScheduledNote(channel,ins,note));
}

void Player::processChanRow(Pattern* p, int i) {
  ChannelStatus& status=chan[i];
  // instrument
  if (p->data[step][i][1]!=0) {
    noteProgramChange(i,p->data[step][i][1]);
  }
  // note
  if ((p->data[step][i][0]&15)!=0) {
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
      if (nextJump==-1) nextJump=pat+1;
      nextJumpStep=status.fxVal;
      break;
    case 'D': case 'K': case 'L': { // volume slide
      if (status.fxVal!=0) {
        status.volSlide=status.fxVal;
      }
      int volAdd=status.volSlide>>4;
      int volSub=status.volSlide&15;
      if (volAdd==15 && volSub!=0) {
        status.vol-=volSub*2;
        if (status.vol<0) status.vol=0;
        status.volChanged=true;
      } else if (volSub==15 && volAdd!=0) {
        status.vol+=volAdd*2;
        if (status.vol>127) status.vol=127;
        status.volChanged=true;
      }
      break;
    }
    case 'E': // slide down
      if (status.fxVal>0 && status.fxVal<0xe0) {
        status.slideSpeed=(float)status.fxVal/16.0f;
      } else if (status.fxVal<0xf0) {
        status.slideSpeed=0;
        status.note-=(float)(status.fxVal&15)/64.0f;
        status.freqChanged=true;
      } else {
        status.slideSpeed=0;
        status.note-=(float)(status.fxVal&15)/16.0f;
        status.freqChanged=true;
      }
      break;
    case 'F': // slide up
      if (status.fxVal>0 && status.fxVal<0xe0) {
        status.slideSpeed=(float)status.fxVal/16.0f;
      } else if (status.fxVal<0xf0) {
        status.slideSpeed=0;
        status.note+=(float)(status.fxVal&15)/64.0f;
        status.freqChanged=true;
      } else {
        status.slideSpeed=0;
        status.note+=(float)(status.fxVal&15)/16.0f;
        status.freqChanged=true;
      }
      break;
    case 'G': // porta
      if (status.fxVal!=0) status.portaSpeed=(float)status.fxVal/16.0f;
      if (isNote(p->data[step][i][0])) status.portaTarget=getNote(p->data[step][i][0]);
      break;
    case 'H': // vibrato
      if ((status.fxVal>>4)!=0) {
        status.vibSpeed=4*(status.fxVal>>4);
      }
      if ((status.fxVal&15)!=0) {
        status.vibDepth=(status.fxVal&15)*4;
      }
      break;
    case 'J': // arpeggio
      if (status.fxVal!=0) {
        status.arp=status.fxVal;
      }
      break;
    case 'M': // channel volume
      status.channelVol=status.fxVal*2;
      if (status.channelVol>0x80) status.channelVol=0x80;
      status.volChanged=true;
      break;
    case 'R': // tremolo
      if ((status.fxVal>>4)!=0) {
        status.tremSpeed=4*(status.fxVal>>4);
      }
      if ((status.fxVal&15)!=0) {
        status.tremDepth=(status.fxVal&15)*4;
      }
      break;
    case 'S': { // special effects
      unsigned char subEffect=status.fxVal>>4;
      unsigned char subVal=status.fxVal&15;
      switch (subEffect) {
        case 0xB: // pattern loop
          if (subVal==0) {
            patLoopPos=step;
          } else {
            if (patLoopCount==0) {
              patLoopCount=subVal;
              nextJump=pat;
              nextJumpStep=patLoopPos;
            } else if (--patLoopCount>0) {
              nextJump=pat;
              nextJumpStep=patLoopPos;
            }
          }
          break;
        case 0xC: // delayed cut
          status.cutTimer=subVal+1;
          if (status.cutTimer<2) status.cutTimer=2;
          break;
      }
      break;
    }
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
      notePanChange(i,status.fxVal^0x80);
      break;
  }
}

void Player::nextRow() {
  if (nextJump>=0) {
    pat=nextJump; step=nextJumpStep-1;
    nextJump=-1;
    nextJumpStep=0;
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
    // check for row delay
    if ((0x40+p->data[step][i][3]=='S') && (p->data[step][i][4]>>4)==0xD) {
      ChannelStatus& status=chan[i];
      status.rowDelay=1+(p->data[step][i][4]&15);
      if (status.rowDelay<2) status.rowDelay=2;
      continue;
    }
    processChanRow(p,i);
  }
  
  tick=speed;
}

void Player::update() {
  while (!scheduledNotes.empty()) {
    if (playMode==0) playMode=2;
    ScheduledNote s=scheduledNotes.front();
    noteProgramChange(s.chan,s.ins);
    noteOn(s.chan,s.note);
    scheduledNotes.pop();
  }

  if (playMode==0) return;
  if (song==NULL) return;

  // next tick
  if (playMode==1) if (--tick<=0) {
    nextRow();
  }
  
  for (int i=0; i<song->channels; i++) {
    ChannelStatus& status=chan[i];
    if (!status.active) continue;

    if (status.rowDelay) {
      if (--status.rowDelay<1) {
        processChanRow(song->getPattern(song->order[pat],false),i);
      }
    }

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
      c.pan=status.macroPan.value^0x80;
    }

    status.macroFilterMode.next();
    if (status.macroFilterMode.hasChanged) {
      c.flags.fmode=status.macroFilterMode.value;
    }

    status.macroVolSweep.next();
    if (status.macroVolSweep.hasChanged) {
      *(unsigned int*)(&c.swvol)=status.macroVolSweep.value;
      c.flags.swvol=(status.macroVolSweep.value!=0);
    }

    status.macroFreqSweep.next();
    if (status.macroFreqSweep.hasChanged) {
      *(unsigned int*)(&c.swfreq)=status.macroFreqSweep.value;
      c.flags.swfreq=(status.macroFreqSweep.value!=0);
    }

    status.macroCutSweep.next();
    if (status.macroCutSweep.hasChanged) {
      *(unsigned int*)(&c.swcut)=status.macroCutSweep.value;
      c.flags.swcut=(status.macroCutSweep.value!=0);
    }

    status.macroPCM.next();
    if (status.macroPCM.hasChanged) {
      c.pcmpos=ins->pcmPos+status.macroPCM.value;
    }

    status.arpValue=0;

#define VOL_SLIDE { \
  int volAdd=status.volSlide>>4; \
  int volSub=status.volSlide&15; \
  if (!((volAdd==15 && volSub!=0) || (volSub==15 && volAdd!=0))) { \
    status.vol+=(volAdd-volSub)*2; \
    if (status.vol>127) status.vol=127; \
    if (status.vol<0) status.vol=0; \
    status.volChanged=true; \
  } \
}

#define VIBRATO \
  status.vibPos+=status.vibSpeed; \
  status.vibValue=-sin(M_PI*(float)status.vibPos/128.0f)*((float)status.vibDepth/32.0f); \
  status.freqChanged=true;

#define TREMOLO \
  status.tremPos+=status.tremSpeed; \
  status.tremValue=-sin(M_PI*(float)status.tremPos/128.0f)*(status.tremDepth); \
  status.volChanged=true;

#define PORTAMENTO \
  if (status.note<status.portaTarget) { \
    status.note+=status.portaSpeed; \
    if (status.note>=status.portaTarget) status.note=status.portaTarget; \
    status.freqChanged=true; \
  } else { \
    status.note-=status.portaSpeed; \
    if (status.note<=status.portaTarget) status.note=status.portaTarget; \
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
        status.arpValue=(status.arp>>(8-((speed-tick)%3)*4))&15;
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
      case 'R': // tremolo
        TREMOLO
        break;
    }

    // changes
    if (status.volChanged) {
      if (channelMask[i]) {
        c.vol=0;
      } else {
        c.vol=minval(127,(status.vol*status.envVol*status.channelVol*(int)(128.5+status.tremValue))>>22);
      }
      status.volChanged=false;
    }
    if (status.freqChanged) {
      unsigned int freq=getNoteFreq(offsetNote((status.note-48)+((float)ins->pitch/128.0f)+ins->noteOffset+status.vibValue+status.arpValue+((float)status.finePitch/64.0f),status.macroPitch.value));
      if (song->flags&4 && c.flags.shape==4) {
        if (freq>0x3ffff) freq=0x3ffff;
        c.freq=freq>>2;
      } else {
        if (freq>0xffff) freq=0xffff;
        c.freq=freq;
      }

      if (ins->flags&32) c.restimer=getNotePeriod(offsetNote(status.note,ins->LFO));

      status.freqChanged=false;
    }

    // note cut
    if (status.cutTimer) {
      if (--status.cutTimer<1) {
        noteCut(i);
      }
    }
  }
}

void Player::reset() {
  pat=0;
  step=-1;
  tick=0;
  playMode=0;
  nextJump=-1;
  nextJumpStep=0;

  patLoopPos=0;
  patLoopCount=0;

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
  for (int i=0; i<32; i++) {
    chan[i]=ChannelStatus();
  }
}

void Player::panic() {
  if (chip!=NULL) {
    memset(chip[0].chan,0,256);
    memset(chip[1].chan,0,256);
    memset(chip[2].chan,0,256);
    memset(chip[3].chan,0,256);
  }
  for (int i=0; i<32; i++) {
    chan[i]=ChannelStatus();
    chan[i].channelPan=song->defaultPan[i];
    chan[i].channelVol=song->defaultVol[i];
    chip[i>>3].chan[i&7].pan=chan[i].channelPan;
  }
}

void Player::play() {
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
    chan[i].channelVol=song->defaultVol[i];
    chip[i>>3].chan[i&7].pan=chan[i].channelPan;
  }
  playMode=1;
}

void Player::stop() {
  playMode=0;
  for (int i=0; i<song->channels; i++) {
    chip[i>>3].chan[i&7].vol=0;
    chip[i>>3].chan[i&7].flags.swvol=0;
  }
}

void Player::maskChannel(int channel, bool mask) {
  channelMask[channel]=mask;
  if (channelMask[channel]) {
    chip[channel>>3].chan[channel&7].vol=0;
  }
  chan[channel].volChanged=true;
}

void Player::toggleChannel(int channel) {
  maskChannel(channel,!channelMask[channel]);
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
