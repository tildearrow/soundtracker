#include "tracker.h"

int getNote(unsigned char note) {
  return (12*(note>>4)+(note&15));
}

// formula to calculate 6203.34:
// - 65536*440/(chipClock/64)
// chipClock is 297500 (PAL)
// or 309000 (NTSC)
unsigned int Player::getNoteFreq(float note) {
  return (int)((6203.34-(song->detune*2))*(pow(2.0f,(float)(((float)note-58)/12.0f))));
}

void Player::noteOn(int channel, int note) {
  ChannelStatus& status=chan[channel];
  soundchip::channel& c=chip[channel>>3].chan[channel&7];

  Instrument* ins=song->ins[status.instr];

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
  }
  status.note=(note-48)+ins->noteOffset;

  /// FILTER MODE
  c.flags.fmode=ins->filterMode&7;
  //printf("filter mode is now %d\n",c.flags.fmode);
  c.reson=127;

  /// MACROS
  if (ins->volMacro>=0) {
    status.macroVol.load(song->macros[ins->volMacro]);
  } else {
    status.macroVol.load(NULL);
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
  }
  if (ins->shapeMacro>=0) {
    status.macroShape.load(song->macros[ins->shapeMacro]);
  } else {
    status.macroShape.load(NULL);
  }

  c.freq=getNoteFreq(status.note);
  c.duty=63;
}

void Player::noteOff(int channel) {

}

void Player::noteCut(int channel) {
  ChannelStatus& status=chan[channel];
  soundchip::channel& c=chip[channel>>3].chan[channel&7];
  status.vol=0;
  c.vol=0;
}

void Player::noteAftertouch(int channel, int val) {
  ChannelStatus& status=chan[channel];
  soundchip::channel& c=chip[channel>>3].chan[channel&7];
  status.vol=val;
  c.vol=status.vol;
}

void Player::noteProgramChange(int channel, int val) {
  ChannelStatus& status=chan[channel];
  soundchip::channel& c=chip[channel>>3].chan[channel&7];

  status.instr=val;
  noteAftertouch(channel,minval(127,song->ins[status.instr]->vol*2));
}


void Player::nextRow() {
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
          noteOn(i,getNote(p->data[step][i][0]));
          break;
      }
    }
    // volume
    if (p->data[step][i][2]>=64 && p->data[step][i][2]<=127) {
      noteAftertouch(i,minval(127,(p->data[step][i][2]-64)*2));
    }
    // effect
    status.fx=p->data[step][i][3];
    status.fxVal=p->data[step][i][4];
    switch (status.fx) {
      case 1: // Axx: speed
        speed=status.fxVal;
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
    soundchip::channel& c=chip[i>>3].chan[i&7];
    Instrument* ins=song->ins[status.instr];

    // run macros!
    status.macroVol.next();
    if (status.macroVol.hasChanged) {
      c.vol=(status.vol*status.macroVol.value)>>8;
    }

    status.macroCut.next();
    if (status.macroCut.hasChanged) {
      c.cutoff=(ins->filterH*status.macroCut.value)>>8;
      //printf("cutoff is now %d\n",c.cutoff);
    }

    status.macroRes.next();
    if (status.macroRes.hasChanged) {
      c.reson=status.macroRes.value;
      //printf("resonance is now %d\n",c.reson);
    }

    status.macroDuty.next();
    if (status.macroDuty.hasChanged) {
      c.duty=status.macroDuty.value>>1;
    }

    status.macroShape.next();
    if (status.macroShape.hasChanged) {
      c.flags.shape=status.macroShape.value>>5;
    }
  }
}

void Player::reset() {
  pat=0;
  step=-1;
  tick=0;
  playMode=0;
  speed=6;
  tempo=ntsc?150:125;
}

void Player::play() {
  playMode=1;
  step=-1;
  tick=0;
}

void Player::stop() {
  playMode=0;
}

void Player::setSong(Song* s) {
  song=s;
}

void Player::bindChips(soundchip* s) {
  chip=s;
}

Player::Player():
  ntsc(false) {
  reset();
}