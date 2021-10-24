// new tracker code
#ifndef _TRACKER_H
#define _TRACKER_H
#include <stdio.h>
#include <math.h>
#include <errno.h>

#if defined(ANDROID) || defined(_WIN32)
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
#include <arpa/inet.h>
#endif
#ifdef __APPLE__
extern "C" {
  #include "nsstub.h"
}
#endif
#include <stdint.h>
#include <string.h>
#include <string>
#include <vector>
#include <mutex>

#include "fextra.h"
#include "soundchip.h"

#define TRACKER_VER 153

#define minval(a,b) (((a)<(b))?(a):(b)) // for Linux compatibility
#define maxval(a,b) (((a)>(b))?(a):(b)) // for Linux compatibility

typedef std::string string;
typedef std::wstring wstring;
#define S(x) string(x)

size_t utf8len(const char* s);
size_t utf8clen(const char* s);
size_t utf8pos(const char* s, size_t inpos);
size_t utf8cpos(const char* s, size_t inpos);
size_t utf8findcpos(const char* s, float inpos);
char utf8csize(const unsigned char* c);

wstring utf8To16(const char* in);
string utf16To8(const wchar_t* in);

#ifdef _WIN32
unsigned short ntohs(unsigned short orig);
#endif

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

enum SwipeStatus {
  swNone=0,
  swHolding,
  swDragging
};

class Swiper {
  float* out;
  float speed;
  float frict;
  float lMin;
  float lMax;
  float dpos;
  float curpos;
  float oldpos;
  float iout;
  bool drag;
  bool actuallyDrag;
  bool dir;
  public:
    bool start(float pos) {
      if (out==NULL) return false;
      if (!drag) {
        drag=true;
        actuallyDrag=false;
        dpos=pos;
        iout=*out;
        curpos=pos;
        oldpos=pos;
        return true;
      }
      return false;
    }
    bool update(float pos) {
      if (out==NULL) return false;
      if (drag) {
        if (actuallyDrag) {
          *out=iout-(pos-dpos);
        } else {
          if (fabs(pos-dpos)>3) {
            actuallyDrag=true;
            dpos=pos;
          }
        }
        oldpos=curpos;
        curpos=pos;
      } else {
        if (dir) {
          *out-=speed;
        } else {
          *out+=speed;
        }
        speed-=frict;
        if (speed<0) speed=0;
      }
      if (*out>lMax) {
        *out=lMax;
        speed=0;
      }
      if (*out<lMin) {
        *out=lMin;
        speed=0;
      }
      return (drag && actuallyDrag);
    }
    bool end(float pos) {
      if (out==NULL) return false;
      if (drag) {
        drag=false;
        dir=(oldpos-curpos)<0;
        speed=fabs(oldpos-curpos);
        return true;
      }
      return false;
    }
    SwipeStatus getStatus() {
      if (drag) {
        if (actuallyDrag) return swDragging;
        return swHolding;
      }
      return swNone;
    }
    void setOut(float* o) {
      out=o;
    }
    void setFrict(float f) {
      frict=f;
    }
    void setRange(float rMin, float rMax) {
      lMin=rMin;
      lMax=rMax;
    }
    Swiper():
      out(NULL),
      speed(0),
      frict(0.2),
      lMin(0),
      lMax(500),
      dpos(0),
      iout(0),
      drag(false) {}
};

class Graphics {
  Point textPos;
  Point scrSize;
  Color textCol;
  char putBuf[4096];
  float nlPos;
  float align;
  float dpiScale;
  
  // texture pages are 512x512 each and contain 1024 characters
  bool loadedPage[64];
  
  SDL_Window* sdlWin;
  SDL_Renderer* sdlRend;
  //TTF_Font* sdlFont;
  SDL_Surface* fontCache;
  SDL_Texture* sdlText;
  
  SDL_Surface* uCache;
  SDL_Texture* uText[64];
  
  void loadPage(int num);
  public:
    Point getTPos();
    Point getWSize();
    // HACK BEGIN //
    SDL_Renderer* _getDisplay();
    float _getScale();
    // HACK END //
    
    // HACK: Allegro methods wrapped for eventual SDL transition
    Color _WRAP_map_rgb(unsigned char r, unsigned char g, unsigned char b) {
      return Color(r,g,b);
    }
    Color _WRAP_map_rgba(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
      return Color(r,g,b,a);
    }
    Color _WRAP_map_rgb_f(float r, float g, float b) {
      return Color(r,g,b);
    }
    Color _WRAP_map_rgba_f(float r, float g, float b, float a) {
      return Color(r,g,b,a);
    }
    double _WRAP_get_time() {
      return 0;//al_get_time();
    }
    void _WRAP_flip_display() {
      SDL_RenderPresent(sdlRend);
    }
    void _WRAP_draw_filled_rectangle(float x1, float y1, float x2, float y2, Color color) {
      SDL_FRect re;
      re.x=x1;
      re.y=y1;
      re.w=x2-x1;
      re.h=y2-y1;
      if (re.w<0) {
        re.x+=re.w;
        re.w=-re.w;
      }
      if (re.h<0) {
        re.y+=re.h;
        re.h=-re.h;
      }
      SDL_SetRenderDrawColor(sdlRend,color.r*255,color.g*255,color.b*255,color.a*255);
      SDL_RenderFillRectF(sdlRend,&re);
    }
    void _WRAP_clear_to_color(Color color) {
      SDL_SetRenderDrawColor(sdlRend,color.r*255,color.g*255,color.b*255,color.a*255);
      SDL_RenderClear(sdlRend);
    }
    Texture _WRAP_create_bitmap(int w, int h) {
      Texture ret;
      ret.actual=SDL_CreateTexture(sdlRend,SDL_PIXELFORMAT_RGBA8888,SDL_TEXTUREACCESS_TARGET,w,h);
      SDL_SetTextureBlendMode(ret.actual,SDL_BLENDMODE_BLEND);
      ret.w=w;
      ret.h=h;
      return ret;
    }
    void _WRAP_destroy_bitmap(Texture bitmap) {
      SDL_DestroyTexture(bitmap.actual);
      bitmap.w=0;
      bitmap.h=0;
    }
    bool _WRAP_get_next_event(SDL_Event* e) {
      return SDL_PollEvent(e);
    }
    void _WRAP_rest(double t) {
#ifdef _WIN32
      Sleep(t*1000);
#else
      usleep(t*1000000);
#endif
    }
    void _WRAP_draw_pixel(float x, float y, Color color) {
      SDL_SetRenderDrawColor(sdlRend,color.r*255,color.g*255,color.b*255,color.a*255);
      SDL_RenderDrawPointF(sdlRend,x,y);
    }
    void _WRAP_draw_bitmap(Texture bitmap, float x, float y, int flags) {
      SDL_Rect dr;
      dr.x=x;
      dr.y=y;
      dr.w=bitmap.w;
      dr.h=bitmap.h;
      SDL_RenderCopy(sdlRend,bitmap.actual,NULL,&dr);
    }
    void _WRAP_set_blender(SDL_BlendMode op) {
      SDL_SetRenderDrawBlendMode(sdlRend,op);
    }
    void _WRAP_draw_line(float x1, float y1, float x2, float y2, Color color, float thick) {
      SDL_SetRenderDrawColor(sdlRend,color.r*255,color.g*255,color.b*255,color.a*255);
      SDL_RenderDrawLineF(sdlRend,x1,y1,x2,y2);
    }
    void _WRAP_draw_rectangle(float x1, float y1, float x2, float y2, Color color, float thick) {
      SDL_FRect re;
      re.x=x1;
      re.y=y1;
      re.w=x2-x1;
      re.h=y2-y1;
      if (re.w<0) {
        ::printf("w\n");
      }
      if (re.h<0) {
        ::printf("h\n");
      }
      SDL_SetRenderDrawColor(sdlRend,color.r*255,color.g*255,color.b*255,color.a*255);
      SDL_RenderDrawRectF(sdlRend,&re);
    }
    void _WRAP_reset_clipping_rectangle() {
      SDL_RenderSetClipRect(sdlRend,NULL);
    }
    void _WRAP_set_clipping_rectangle(float x, float y, float w, float h) {
      SDL_Rect r;
      r.x=x; r.y=y; r.w=w; r.h=h;
      if (r.w<1 || r.h<1) {
        r.x=-10;
        r.y=-10;
        r.w=1;
        r.h=1;
      }
      SDL_RenderSetClipRect(sdlRend,&r);
    }
    int _WRAP_get_bitmap_width(Texture bitmap) {
      return bitmap.w;
    }
    void _WRAP_draw_bitmap_region(Texture bitmap, float x, float y, float w, float h, float dx, float dy, int flags) {
      SDL_Rect sr, dr, tr;
      sr.x=x;
      sr.y=y;
      sr.w=w;
      sr.h=h;
      dr.x=dx;
      dr.y=dy;
      dr.w=w;
      dr.h=h;
      tr.x=0;
      tr.y=0;
      tr.w=bitmap.w;
      tr.h=bitmap.h;
      if ((sr.h+sr.y)>tr.h) {
        sr.h-=(sr.h+sr.y)-tr.h;
        dr.h=sr.h;
      }
      SDL_RenderCopy(sdlRend,bitmap.actual,&sr,&dr);
    }
    void _WRAP_disregard_scale_draw(Texture bitmap, float x, float y, float w, float h, float dx, float dy, float cscale, int flags) {
      SDL_Rect sr, dr, tr;
      sr.x=x;
      sr.y=y;
      sr.w=w;
      sr.h=h;
      dr.x=dx;
      dr.y=dy;
      dr.w=w;
      dr.h=h;
      tr.x=0;
      tr.y=0;
      tr.w=bitmap.w;
      tr.h=bitmap.h;
      sr.w=tr.w;
      sr.h=tr.h;
      dr.w=tr.w;
      dr.h=tr.h;
      /*if ((sr.h+sr.y)>tr.h) {
        sr.h-=(sr.h+sr.y)-tr.h;
        dr.h=sr.h;
      }*/
      dr.w*=cscale;
      dr.h*=cscale;
      SDL_RenderSetScale(sdlRend,1,1);
      SDL_RenderCopy(sdlRend,bitmap.actual,&sr,&dr);
      SDL_RenderSetScale(sdlRend,dpiScale,dpiScale);
    }
    void _WRAP_draw_scaled_bitmap(Texture bitmap, float sx, float sy, float sw, float sh, float dx, float dy, float dw, float dh, int flags) {
      SDL_Rect sr, dr;
      sr.x=sx;
      sr.y=sy;
      sr.w=sw;
      sr.h=sh;
      dr.x=dx;
      dr.y=dy;
      dr.w=dw;
      dr.h=dh;
      SDL_RenderCopy(sdlRend,bitmap.actual,&sr,&dr);
    }
    void _WRAP_draw_rotated_bitmap(Texture bitmap, float cx, float cy, float x, float y, float r, int flags) {
      // TODO
      ::printf("Draw Rotated Bitmap\n");
      //al_draw_rotated_bitmap(bitmap,cx,cy,x,y,r,flags);
    }
    void _WRAP_draw_circle(float x, float y, float r, Color color, float thick) {
      ::printf("Draw Circle\n");
      //al_draw_circle(x,y,r,color,thick);
    }
    Point maxTexSize() {
      SDL_RendererInfo si;
      Point ret;
      SDL_GetRendererInfo(sdlRend,&si);
      ret.x=si.max_texture_width;
      ret.y=si.max_texture_height;
      return ret;
    }
    
    void clearScreen() {
      SDL_SetRenderDrawColor(sdlRend,0,0,0,255);
      SDL_RenderClear(sdlRend);
    }
    void tPos(float x, float y);
    void tPos(float y);
    void tNLPos(float x);
    void tAlign(float x);
    void tColor(unsigned char color);
    void setTarget(Texture where);
    void resetTarget();
    void setTitle(string t);
    void trigResize(int tx, int ty);
    int printf(const char* format, ...);
    bool preinit();
    bool init(int width, int height);
    bool quit();
    Graphics(): nlPos(0), align(0) {}
};

extern Graphics g;
extern int scrW, scrH;

class Button {
  string lab;
  
  void (*callback)(void*);
  void (*pCallback)(void*);
  
  void* cData;
  void* cpData;
  
  public:
    float x, y, w, h;
    void _pMove(float px, float py) {
      
    }
    void _pPress(float px, float py, int b) {
      if (b==0 && PIR(x,y,x+w,y+h,px,py)) {
        if (pCallback!=NULL) {
          pCallback(cpData);
        }
      }
    }
    void _pRelease(float px, float py, int b) {
      if (b==0 && PIR(x,y,x+w,y+h,px,py)) {
        if (callback!=NULL) {
          callback(cData);
        }
      }
    }
    
    void setCallback(void(*c)(void*), void* data) {
      callback=c;
      cData=data;
    }
    void setPressCallback(void(*c)(void*), void* data) {
      pCallback=c;
      cpData=data;
    }
    
    void label(string l) {
      lab=l;
    }
    
    void pos(float px, float py) {
      x=px; y=py;
    }
    void size(float sx, float sy) {
      w=sx; h=sy;
    }
    
    void draw() {
      g._WRAP_draw_rectangle(x,y,x+w,y+h,Color(1.0f,1.0f,1.0f),0);
      g.tPos((x+(w-lab.size()*8)/2)/8,(y+(h/2)-8)/12);
      g.printf("%s",lab.c_str());
    }
    
    Button(float px, float py, float pw, float ph, string l, void(*c)(void*), void* d):
      lab(l),
      callback(c),
      pCallback(NULL),
      cData(d),
      cpData(NULL),
      x(px), y(py), w(pw), h(ph) {}
    
    Button():
      lab("Button"),
      callback(NULL),
      pCallback(NULL),
      cData(NULL),
      cpData(NULL),
      x(16), y(80), w(32), h(16) {}
};

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
      int sizeX=content.size();
      int sizeY=4;
      
      g._WRAP_draw_filled_rectangle((scrW-(sizeX+2)*8)/2,(scrH-sizeY*12)/2,(scrW+(sizeX+2)*8)/2,(scrH+sizeY*12)/2,Color(0.0f,0.0f,0.0f));
      g._WRAP_draw_rectangle((scrW-(sizeX+2)*8)/2,(scrH-sizeY*12)/2,(scrW+(sizeX+2)*8)/2,(scrH+sizeY*12)/2,Color(1.0f,1.0f,1.0f),0);
      
      g.tPos((((float)scrW/2)/8)-((float)title.size())/2,((float)(scrH-sizeY*12+2)/2)/12);
      g.tColor(9);
      g.printf(title.c_str());
      
      g.tPos((((float)scrW/2)/8)-((float)sizeX)/2,((float)(scrH-10)/2)/12);
      g.tColor(15);
      g.printf(content.c_str());
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

class NumberPad {
  //float x, y;
  Button bNum[16];
  Button bDel;
  Button bOK;
  Button bUp;
  Button bDown;
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
  cmdSub
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
  std::vector<MacroCommand> cmds;
  Macro():
    jumpRelease(-1) {}
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

class Player {
  Song* song;
  soundchip* chip;

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
