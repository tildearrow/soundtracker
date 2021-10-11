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
  soundchip::channel& c=chip[channel>>3].chan[channel&7];

  c.vol=127;
  c.freq=getNoteFreq(note);
  c.duty=63;
  c.swvol.amt=1;
  c.swvol.dir=false;
  c.swvol.speed=3000;
  c.flags.swvol=true;
}

void Player::nextRow() {
  step++;
  tick=speed;

  Pattern* p=song->getPattern(song->order[pat],false);
  if (step>=p->length) {
    pat++; step=0;
    if (pat>=song->orders) pat=0;
    p=song->getPattern(song->order[pat],false);
  }

  for (int i=0; i<song->channels; i++) {
    if (p->data[step][i][0]!=0) {
      noteOn(i,getNote(p->data[step][i][0]));
    }
  }
}

void Player::update() {
  if (playMode==0) return;
  if (song==NULL) return;

  // next tick
  if (--tick<=0) {
    nextRow();
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