// new tracker code
#ifndef _TRACKER_H
#define _TRACKER_H
#include <stdio.h>
#include <math.h>
#include <errno.h>

#ifdef HAVE_GUI
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif
#ifdef ANDROID
#define ANDRO
#endif
#ifdef _WIN32
#include <windows.h>
#include <wchar.h>
#else
#if !defined(__APPLE__) && !defined(ANDROID)
#include <X11/Xlib.h>
#endif
#include <dirent.h>
#include <unistd.h>
#endif
#ifdef __APPLE__
extern "C" {
  #include "nsstub.h"
}
#endif
#include <stdint.h>
#include <string.h>
#include <string>
#include <queue>
#include <vector>
#include <mutex>

#include "fextra.h"
#include "soundchip.h"

#define TRACKER_VER 153

#define minval(a,b) (((a)<(b))?(a):(b)) // for Linux compatibility
#define maxval(a,b) (((a)>(b))?(a):(b)) // for Linux compatibility

#include "utfutils.h"

bool PIR(float x1, float y1, float x2, float y2, float checkx, float checky);

enum SupportedFormats {
  FormatUnknown=-1,
  
  FormatTRACK=0,
  FormatTRACKINS,
  FormatMOD,
  FormatS3M,
  FormatIT,
  FormatXM,
  FormatAudio,
};

struct Point {
  float x, y;
};

struct Color {
  float r, g, b, a;
  Color():
    r(0),
    g(0),
    b(0),
    a(1) {}
  explicit Color(unsigned char re, unsigned char gr, unsigned char bl):
    r((float)re/255.0f),
    g((float)gr/255.0f),
    b((float)bl/255.0f),
    a(1.0f) {}
  explicit Color(unsigned char re, unsigned char gr, unsigned char bl, unsigned char al):
    r((float)re/255.0f),
    g((float)gr/255.0f),
    b((float)bl/255.0f),
    a((float)al/255.0f) {}
  explicit Color(float re, float gr, float bl):
    r(re),
    g(gr),
    b(bl),
    a(1.0f) {}
  explicit Color(float re, float gr, float bl, float al):
    r(re),
    g(gr),
    b(bl),
    a(al) {}
};

struct Texture {
  SDL_Texture* actual;
  int w, h;
  Texture(): actual(NULL), w(0), h(0) {}
};

enum UIType {
  UIClassic=0,
  UIModern,
  UIMobile
};

extern int scrW, scrH;

class PopupBox {
  string title;
  string content;
  
  bool show;
  
  public:
    void hide() {
      show=false;
    }
    
    bool isVisible() {
      return show;
    }
    
    void draw() {
    }
    
    //content("Lorem ipsum, quia dolor sit, amet, consectetur, adipisci velit, sed quia non numquam eius modi tempora incidunt, ut labore et dolore magnam aliquam quaerat voluptatem.")
    
    PopupBox(string t, string c):
      title(t),
      content(c),
      show(true) {}
      
    PopupBox(bool s):
      title("Message"),
      content("Lorem ipsum, quia dolor sit, amet."),
      show(s) {}
      
    
    PopupBox():
      title("Message"),
      content("Lorem ipsum, quia dolor sit, amet."),
      show(true) {}
};

// NEW STUFF BEGIN //
struct LegacyInstrument {
  char name[32];
  unsigned char id, pcmMult, activeEnv;
  unsigned char env[8];
  unsigned char noteOffset;
  unsigned char FPt, FPR, DFM, LFO;
  unsigned char vol;
  signed char pitch;
  unsigned short pcmLen, filterH;
  unsigned char res;
  unsigned char pcmPos[2]; // alignment
  unsigned char pcmLoop[2];
  unsigned char FTm;
  unsigned short ver;
  unsigned char flags, RMf;
};

struct Instrument {
  char name[32];
  unsigned char id, pcmMult;
  short volMacro, cutMacro, resMacro, pitchMacro;
  unsigned char unused1, noteOffset;
  unsigned char FPt, FPR, filterMode, LFO;
  unsigned char vol;
  signed char pitch;
  unsigned short pcmLen, filterH;
  unsigned char res, FTm;
  unsigned short pcmPos;
  unsigned short pcmLoop;
  unsigned short ver;
  unsigned char flags, RMf;
  short finePitchMacro, shapeMacro, dutyMacro, panMacro, filterModeMacro, volSweepMacro, freqSweepMacro, cutSweepMacro;
  short pcmPosMacro;
  unsigned short unused2;
  unsigned int unused3[3];
  Instrument():
    id(0),
    pcmMult(0),
    volMacro(-1), cutMacro(-1), resMacro(-1), pitchMacro(-1),
    unused1(0),
    noteOffset(48),
    FPt(0),
    FPR(0),
    filterMode(0),
    LFO(0),
    vol(64),
    pitch(0),
    pcmLen(0),
    filterH(0),
    res(0),
    FTm(0),
    pcmPos(0),
    pcmLoop(0),
    ver(TRACKER_VER),
    flags(0),
    RMf(0),
    finePitchMacro(-1), shapeMacro(-1), dutyMacro(-1), panMacro(-1),
    filterModeMacro(-1), volSweepMacro(-1), freqSweepMacro(-1), cutSweepMacro(-1),
    pcmPosMacro(-1), unused2(0) {
      memset(name,0,32);
      unused3[0]=0;
      unused3[1]=0;
      unused3[2]=0;
    }
};

enum MacroCommandType {
  cmdEnd=0,
  cmdSet,
  cmdWait,
  cmdWaitRel,
  cmdLoop,
  cmdLoopRel,
  cmdAdd,
  cmdSub,

  cmdMax
};

enum MacroIntendedUse {
  iuGeneric,
  iuShape,
  iuPitch,
  iuPan,
  iuVolSweep,
  iuOtherSweep,
  
  iuMax
};

struct MacroCommand {
  unsigned char type;
  unsigned int value;
  MacroCommand(unsigned char t, int v, bool endTick):
    type(t|(endTick<<7)),
    value(v) {}
};

struct Macro {
  int jumpRelease;
  unsigned char intendedUse;
  std::vector<MacroCommand> cmds;
  Macro():
    jumpRelease(-1),
    intendedUse(iuGeneric) {}
};

struct Pattern {
  unsigned short length;
  unsigned char data[256][32][8];
  
  Pattern(): length(64) {
    memset(data,0,256*32*8);
  }
};

struct Song {
  char header[8];
  unsigned short version;
  unsigned char insC, patC, orders, speed, flags, tempo;
  char name[32];
  unsigned char DFM, channels;
  unsigned short macrosC;
  unsigned char globalVol, globalPan;
  unsigned short pcmPtr[2];
  unsigned short commentPtr[2];
  signed char detune;
  unsigned char len;
  unsigned char defaultVol[32];
  signed char defaultPan[32];
  unsigned char order[256];
  Instrument* ins[256];
  Pattern* pat[256];
  std::vector<Macro*> macros;

  Pattern* getPattern(unsigned char num, bool create);

  Song();
  ~Song();
};

class Player;

class MacroStatus {
  Macro* macro;
  int pos, waitTime;
  bool released;
  public:
    bool hasChanged;
    unsigned int value;

    void next();
    void release();
    void load(Macro* m);
    MacroStatus();
};

struct ChannelStatus {
  bool active, noteOn;
  float note;
  short instr;
  short vol, channelVol;
  short envVol;
  short finePitch;
  unsigned char fx, fxVal;
  unsigned char arp, arpValue;
  unsigned char volSlide;
  signed char channelPan;
  unsigned char cutTimer, rowDelay;

  bool volChanged, freqChanged, panChanged;

  float slideSpeed;
  float portaSpeed, portaTarget;

  unsigned char vibPos, vibSpeed, vibDepth;
  float vibValue;

  unsigned char tremPos, tremSpeed, tremDepth;
  float tremValue;

  unsigned char panbPos, panbSpeed, panbDepth;
  float panbValue;

  MacroStatus macroVol;
  MacroStatus macroCut;
  MacroStatus macroRes;
  MacroStatus macroDuty;
  MacroStatus macroShape;
  MacroStatus macroPitch;
  MacroStatus macroFinePitch;
  MacroStatus macroPan;
  MacroStatus macroFilterMode;
  MacroStatus macroVolSweep;
  MacroStatus macroFreqSweep;
  MacroStatus macroCutSweep;
  MacroStatus macroPCM;

  ChannelStatus():
    active(false),
    noteOn(false),
    note(0),
    instr(0),
    vol(0),
    channelVol(128),
    envVol(255),
    finePitch(0),
    fx(0),
    fxVal(0),
    arpValue(0),
    volSlide(0),
    channelPan(0),
    cutTimer(0),
    rowDelay(0),
    volChanged(false),
    freqChanged(false),
    panChanged(false),
    slideSpeed(0),
    portaSpeed(0),
    portaTarget(0),
    vibPos(0),
    vibSpeed(0),
    vibDepth(0),
    vibValue(0),
    tremPos(0),
    tremSpeed(0),
    tremDepth(0),
    tremValue(0),
    panbPos(0),
    panbSpeed(0),
    panbDepth(0),
    panbValue(0) {}
};

struct ScheduledNote {
  int chan, ins, note;
  ScheduledNote(int c, int i, int n):
    chan(c),
    ins(i),
    note(n) {}
};

class Player {
  Song* song;
  soundchip* chip;

  std::queue<ScheduledNote> scheduledNotes;

  public:
    int pat, step, tick, playMode;
    int speed, tempo, nextJump, nextJumpStep;
    int patLoopPos, patLoopCount;
    bool ntsc;
    ChannelStatus chan[32];
    bool channelMask[32];

    unsigned int getNoteFreq(float note);
    unsigned int getNotePeriod(float note);

    float offsetNote(float note, unsigned char off);

    void testNoteOn(int channel, int ins, int note);
    void testNoteOff(int channel);
    void noteOn(int channel, int note);
    void noteOff(int channel);
    void noteCut(int channel);
    void noteAftertouch(int channel, int val);
    void noteProgramChange(int channel, int val);
    void notePanChange(int channel, signed char val);

    void processChanRow(Pattern* pat, int channel);
    void nextRow();

    void maskChannel(int channel, bool mask);
    void toggleChannel(int channel);

    void update();
    void reset();
    void panic();
    void play();
    void stop();

    void setSong(Song* s);
    void bindChips(soundchip* s);

    Player();
};

// NEW STUFF END //
#endif
