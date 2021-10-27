//>[[[[[[[[[[[[[[[[<~>]]]]]]]]]]]]]]]]<//
//+++++++++++++++++++++++++++++++++++++//
//     sound tracker for chiptunes     //
//+++++++++++++++++++++++++++++++++++++//
// written by tildearrow in C++ during //
//             2014-2015.              //
//>++++++++++-+-++-*.*-++-+-++++++++++<//

// add 2016, 2017, 2018, 2019 and 2020 to the list.
// and 2021~

#define PROGRAM_NAME "soundtracker"

//// DEFINITIONS ////
//#define sign(a) ((a>0)?(1):((a<0)?(-1):(0)))
//#define MOUSE_GRID
//#define NTSC // define for NTSC mode
#define SOUNDS
int dpiScale;

#ifdef _WIN32
#define DIR_SEPARATOR '\\'
#define SDIR_SEPARATOR "\\"
#else
#define DIR_SEPARATOR '/'
#define SDIR_SEPARATOR "/"
#endif

bool ntsc=false;

//// INCLUDES AND STUFF ////
#include "tracker.h"

#ifdef HAVE_GUI
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_sdlrenderer.h"
#endif

#include <fmt/printf.h>

#include "ssinter.h"
#ifdef JACK
#include <jack/jack.h>
#include <jack/midiport.h>
jack_port_t* ao[2];
jack_port_t *midi;
jack_client_t *jclient;
jack_status_t jstatus;
jack_nframes_t jacksr;
#else
SDL_AudioDeviceID audioID;
SDL_AudioSpec ac;
SDL_AudioSpec ar;
uint32_t jacksr;
#endif

#include "blip_buf.h"
soundchip chip[4]; // up to 4 soundchips

blip_buffer_t* bb[2]={NULL,NULL};
int prevSample[2]={0,0};

short bbOut[2][32768];

int doframe;
unsigned char colorof[6]={0x00, 0x5f, 0x87, 0xaf, 0xd7, 0xff};
// init sound stuff
bool AlreadySkipped=false;
enum filters {
  fNone, fLowPass, fHighPass, fNotch, fBandPass, fLowBand, fHighBand, fAllPass
};
int scrollpos=0;
int valuewidth=4;
int oldpat=-1;
unsigned char CurrentIns=1;
unsigned char CurrentEnv=0;
// init tracker stuff
string tempInsName;
Instrument blankIns;
int instruments=0;
int patterns=0;
int seqs=255;
bool leftpress=false;
bool rightpress=false;
bool leftrelease=false;
bool rightrelease=false;
bool hexmode=false;
bool quit=false;
bool clockInfo=false;

string name; // song name

int curins=1; // selected instrument
int curoctave=2;
int curedchan=0; // cureditingchannel
int curedmode=0; // current editing mode, 0=note, 1=instrument number 2=volume 3=effect name 4=effect value
int curedpage=0; // current page, 0-3
int curselchan=0;
int curselmode=0;
int curzoom=1;
int selStart=0;
int selEnd=0;
bool follow=true;
int patRow=0;
int chanstodisplay=8;
int maxCTD=8;
double patseek=0;
int screen=0; // 0=patterns, 1=instruments 2=diskop 3=song 4=mixer 5=config 6=help 7=about
float diskopscrollpos=0;
bool tickstart=false;
bool linearslides=true;
bool playermode=false;
bool fileswitch=false;
bool reversemode=false;
int sfxpos=-1; // sound effect position
const char* sfxdata[32]={
  // pause
  "$x!O6V7fEM1v03e00100RRRRRRV7fCRRRRRRV7fERRRRRRV7fCRRRRRRRRRRRRRRRRRRRRR!",
  // error
  "$x!O6V7fAM1v02000100RRRRV7fO5ARRRRRRRRRRR!",
  // shot
  "$x!S4f4000V7fM3v04000100k01000000RRRRRRRRRRRRRRRRRRR!",
  // jump
  "$x!S0Y1ff0c00V7fM3v01800100k030081ffRRRRRRRRRRRRR!",
  // success
  "$x!O5V7fGM1v02000100RRRV7fO5BRRRV7fO6DRRRV7fO6GRRRRRRRRRRR!",
  NULL
}; // sound effect data
int cursfx=0; // current effect
const char HEXVALS[17]="0123456789ABCDEF"; // 17 for the null

SSInter sfxInst;

bool sfxplaying=false;

namespace ASC {
  int interval=119000;
  int currentclock=0;
}

enum envTypes {
  envVolume=0,
  envCutoff,
  envReson,
  envDuty,
  envShape,
  envPitch,
  envHiPitch,
  envPan
};

Texture logo;
bool leftclick=false;
bool leftclickprev=false;
bool rightclick=false;  
bool rightclickprev=false;
int prevZ=0;
int hover[16]={}; // hover time per button
int16_t ver=TRACKER_VER; // version number
string comments;
int inputcurpos=0;
int chantoplayfx=0;
string* inputvar=NULL;
string candInput;
string origin;
int inputwhere=0; // 0=none, 1=songname, 2=insname, 3=filepath, 4=comments, 5=filename
size_t maxinputsize=string::npos;
bool imeActive=false;
SDL_Rect inputRefRect;
char* curdir;
string curfname, loadedfname;
int pcmeditscale=0;
int pcmeditseek=0;
int pcmeditoffset=0;
bool pcmeditenable=false;
int UPDATEPATTERNS=false;
bool speedlock=false;
bool tempolock=false;
unsigned long int framecounter=0;
unsigned long int audioframecounter=0;
// init the audio
int sr;
double targetSR;
double noProc;
double procPos;

int selectedfileindex=-1;
struct FileInList {
  string name;
  bool isdir;
};
std::vector<FileInList> filenames;
std::vector<FileInList> filessorted;
int scrW, scrH;
Texture patternbitmap;
Texture pianoroll;
Texture pianoroll_temp;
Texture piano;
Texture mixer;
Texture osc;
bool firstframe=true;
float oscbuf[65536]={}; // safe oscilloscope buffer
float oscbuf2[65536]={}; // safe oscilloscope buffer
unsigned short oscbufRPos=0;
unsigned short oscbufWPos=0;
// settings
namespace settings {
  // audio settings
  bool distortion=false;
  bool threading=false;
  bool cubicspline=false;
  // importer settings
  bool names=true;
  bool split=false;
  bool samples=true;
  // filter settings
  bool nofilters=false;
}

// NEW VARIABLES BEGIN //
SDL_Window* sdlWin;
SDL_Renderer* sdlRend;

const char* pageNames[]={
  "pattern",
  "instr",
  "soundfx",
  "song",
  "file",
  "memory",
  "mixer",
  "config",
  "visual",
  "about"
};

UIType iface;
bool mobAltView;
float mobScroll;
float topScroll;
bool pageSelectShow;
bool noStoragePerm;

Swiper swX;
Swiper swY;

PopupBox popbox;

Point maxTSize;

// new file dialog
Button bdNew;
Button bdOpen;
Button bdSave;
Swiper diskopSwiper;
float doScroll;

// new things
Song* song=NULL;
Player player;
std::mutex canUseSong;

bool insEditOpen=false;

// NEW VARIABLES END //

void Playback();
int playfx(const char* fxdata,int fxpos,int achan);
void triggerfx(int num);
#define interpolatee(aa,bb,cc) (aa+((bb-aa)*cc))

#define resaf 0.33631372025095791864295318996109

#ifdef JACK
int nothing(jack_nframes_t nframes, void* arg) {
#else
static void nothing(void* userdata, Uint8* stream, int len) {
#endif
  float* buf[2];
#ifndef JACK
  signed char* buf8[2];
  short* buf16[2];
  int* buf32[2];
#endif
  short stemp[2];
  int temp[2];

#ifdef JACK
  for (int i=0; i<2; i++) {
    buf[i]=(float*)jack_port_get_buffer(ao[i],nframes);
  }
#else
  size_t nframes;
  switch (ar.format) {
    case AUDIO_U8: case AUDIO_S8:
      nframes=len/(ar.channels);
      buf8[0]=(signed char*)stream;
      break;
    case AUDIO_U16: case AUDIO_S16:
      nframes=len/(2*ar.channels);
      buf16[0]=(short*)stream;
      break;
    case AUDIO_S32: case AUDIO_F32:
      nframes=len/(4*ar.channels);
      buf[0]=(float*)stream;
      buf32[0]=(int*)stream;
      break;
  }
#endif
  if (ntsc) {
    targetSR=309000;
    noProc=sr/targetSR;
  } else {
    targetSR=297500;
    noProc=sr/targetSR;
  }
  blip_set_rates(bb[0],targetSR,jacksr);
  blip_set_rates(bb[1],targetSR,jacksr);
  
  // high quality rewrite
  int runtotal=blip_clocks_needed(bb[0],nframes);
  
  bool canPlay=canUseSong.try_lock();
  if (canPlay) for (int i=0; i<runtotal; i++) {
    ASC::currentclock-=20; // 20 CPU cycles per sound output cycle
    if (ASC::currentclock<=0) {

      player.update();

      if (sfxplaying) {
        sfxpos=playfx(sfxdata[cursfx],sfxpos,chantoplayfx);
        if (sfxpos==-1) {
          sfxplaying=false;
        }
      }
      if (ntsc) {
        // 6.18MHz * 2.5
        ASC::interval=(int)(15450000/player.tempo);
        if (player.tempo==150) ASC::interval=103103;
      } else {
        // 5.95MHz * 2.5
        ASC::interval=(int)(14875000/player.tempo);
      }
      if (leftclick && iface!=UIMobile) {
        ASC::interval=16384;
      }
      ASC::currentclock+=ASC::interval;
    }
    temp[0]=0; temp[1]=0;
    for (int j=0; j<(1+((song->channels-1)>>3)); j++) {
      chip[j].NextSample(&stemp[0],&stemp[1]);
      temp[0]+=stemp[0]; temp[1]+=stemp[1];
    }
    if (temp[0]!=prevSample[0]) blip_add_delta(bb[0],i,(short)(temp[0]-prevSample[0]));
    if (temp[1]!=prevSample[1]) blip_add_delta(bb[1],i,(short)(temp[1]-prevSample[1]));
    prevSample[0]=temp[0];
    prevSample[1]=temp[1];
  }
  if (canPlay) canUseSong.unlock();
  
  blip_end_frame(bb[0],runtotal);
  blip_end_frame(bb[1],runtotal);
  
  blip_read_samples(bb[0],bbOut[0],nframes,0);
  blip_read_samples(bb[1],bbOut[1],nframes,0);
  
  for (size_t i=0; i<nframes; i++) {
#ifdef JACK
    buf[0][i]=float(bbOut[0][i])/16384;
    buf[1][i]=float(bbOut[1][i])/16384;
#else
    switch (ar.format) {
        case AUDIO_F32:
          buf[0][i*ar.channels]=float(bbOut[0][i])/16384;
          buf[0][1+(i*ar.channels)]=float(bbOut[1][i])/16384;
          break;
        case AUDIO_S16:
          buf16[0][i*ar.channels]=bbOut[0][i];
          buf16[0][1+(i*ar.channels)]=bbOut[1][i];
          break;
        case AUDIO_S32:
          buf32[0][i*ar.channels]=bbOut[0][i]<<16;
          buf32[0][1+(i*ar.channels)]=bbOut[1][i]<<16;
          break;
        case AUDIO_S8:
          buf8[0][i*ar.channels]=bbOut[0][i]>>8;
          buf8[0][1+(i*ar.channels)]=bbOut[1][i]>>8;
          break;
        case AUDIO_U8:
        case AUDIO_U16:
          break;
    }
#endif
    oscbuf[oscbufWPos]=float(bbOut[0][i])/16384;
    oscbuf2[oscbufWPos]=float(bbOut[1][i])/16384;
    oscbufWPos++;
  }
#ifdef JACK
  return 0;
#endif
}

void initaudio() {
  //cout << "\npreparing audio system... ";
  printf("initializing audio...\n");
  
  bb[0]=blip_new(32768);
  bb[1]=blip_new(32768);
  
  procPos=0;
#ifdef JACK
  const char** jports;
  jclient=jack_client_open("soundtracker",JackNullOption,&jstatus);
  if (jclient == NULL) {
    fprintf (stderr, "jack_client_open() failed, "
       "status = 0x%2.0x\n", jstatus);
    if (jstatus & JackServerFailed) {
      fprintf (stderr, "Unable to connect to JACK server\n");
    }
    //exit (1);
  }
  if (jstatus & JackServerStarted) {
    fprintf (stderr, "JACK server started\n");
  }
  if (jstatus & JackNameNotUnique) {
    fprintf (stderr, "boom\n");
  }
  jack_set_process_callback(jclient,nothing,0);
  printf ("engine sample rate: %" PRIu32 "\n",
  jack_get_sample_rate(jclient));
  jacksr=jack_get_sample_rate(jclient);
  sr=jacksr;
  ao[0] = jack_port_register (jclient, "outL",
            JACK_DEFAULT_AUDIO_TYPE,
            JackPortIsOutput, 0);
  if (ao[0] == NULL) {
    fprintf(stderr, "no more JACK ports available\n");
  }
  ao[1] = jack_port_register (jclient, "outR",
            JACK_DEFAULT_AUDIO_TYPE,
            JackPortIsOutput, 0);
  if (ao[1] == NULL) {
    fprintf(stderr, "no more JACK ports available\n");
  }

  /*
  midi = jack_port_register (jclient, "midi",
            JACK_DEFAULT_MIDI_TYPE,
            JackPortIsOutput, 0);
  if ((midi == NULL)) {
    fprintf(stderr, "no more JACK ports available\n");
  }
  */
#else
      //////////////// SDL CODE HERE ////////////////
      SDL_InitSubSystem(SDL_INIT_AUDIO);
      ac.freq=44100;
      ac.format=AUDIO_F32;
      ac.channels=2;
      ac.samples=1024;
      ac.callback=nothing;
      ac.userdata=NULL;
      audioID=SDL_OpenAudioDevice(SDL_GetAudioDeviceName(0,0),0,&ac,&ar,SDL_AUDIO_ALLOW_ANY_CHANGE);
      jacksr=ar.freq;
      sr=jacksr;
#endif
#ifdef JACK
      if (jack_activate (jclient)) {
    fprintf (stderr, "cannot activate client");
    //exit (1);
  }
  jports = jack_get_ports (jclient, NULL, NULL,
        JackPortIsPhysical|JackPortIsInput);
  if (jports == NULL) {
    fprintf(stderr, "no physical playback ports\n");
    //exit (1);
  }
  if (jack_connect (jclient, jack_port_name (ao[0]), jports[0])) {
    printf("cannot connect output port l\n");
  }
  if (jack_connect (jclient, jack_port_name (ao[1]), jports[1])) {
    printf("cannot connect output port r\n");
  }
  free (jports);
  
#else
  //////////////// SDL CODE HERE ////////////////
#endif
  
  blip_set_rates(bb[0],297500,jacksr);
  blip_set_rates(bb[1],297500,jacksr);
  
  printf("done\n");
}

FILE* ps_fopen(const char* fn, const char* mo) {
#ifdef _WIN32
  return _wfopen(utf8To16(fn).c_str(),utf8To16(mo).c_str());
#else
  return fopen(fn,mo);
#endif
}

unsigned short bswapu16(unsigned short x) {
  return ((x&0xff)<<8)|((x&0xff00)>>8);
}

float interpolate(float p1, float p2, float amt) {
  return p1+((p2-p1)*amt);
}
float lengthdir_x(float len,float dir) {
  return len*cos(dir*(M_PI/180));
}
float lengthdir_y(float len,float dir) {
  return len*sin(dir*(M_PI/180));
}
char gethnibble(int nval) {
  if (nval<256) {
    return HEXVALS[nval>>4];
  }
  return '?';
}
char getlnibble(int nval) {
  return HEXVALS[nval&15];
}
const char* getnote(int nval) {
  switch(nval&15) {
    case 0: return ".."; break;
    case 1: return "C-"; break;
    case 2: return "C#"; break;
    case 3: return "D-"; break;
    case 4: return "D#"; break;
    case 5: return "E-"; break;
    case 6: return "F-"; break;
    case 7: return "F#"; break;
    case 8: return "G-"; break;
    case 9: return "G#"; break;
    case 10: return "A-"; break;
    case 11: return "A#"; break;
    case 12: return "B-"; break;
    case 13: return "=="; break;
    case 14: return "~~"; break;
    case 15: return "^^"; break;
  }
  return "?";
}
const char* getnotetransp(int nval) {
  switch(nval%12) {
    case 0: return "C-"; break;
    case 1: return "C#"; break;
    case 2: return "D-"; break;
    case 3: return "D#"; break;
    case 4: return "E-"; break;
    case 5: return "F-"; break;
    case 6: return "F#"; break;
    case 7: return "G-"; break;
    case 8: return "G#"; break;
    case 9: return "A-"; break;
    case 10: return "A#"; break;
    case 11: return "B-"; break;
  }
  return "?-";
}
const char* getoctavetransp(int nval) {
  switch(nval/12) {
    case 0: return "0"; break;
    case 1: return "1"; break;
    case 2: return "2"; break;
    case 3: return "3"; break;
    case 4: return "4"; break;
    case 5: return "5"; break;
    case 6: return "6"; break;
    case 7: return "7"; break;
    case 8: return "8"; break;
    case 9: return "9"; break;
    case 10: return "I"; break;
    case 11: return "I"; break;
    case 12: return "I"; break;
    case 13: return "I"; break;
    case 14: return "I"; break;
    case 15: return "I"; break;
  }
  return "?";
}
const char* getoctave(int nval) {
  if ((nval&15)==0) {return ".";}
  if (nval>12) {
    switch (nval&15) {
      case 13: return "="; break;
      case 14: return "~"; break;
      case 15: return "^"; break;
    }
  }
  switch (nval>>4) {
    case 0: return "0"; break;
    case 1: return "1"; break;
    case 2: return "2"; break;
    case 3: return "3"; break;
    case 4: return "4"; break;
    case 5: return "5"; break;
    case 6: return "6"; break;
    case 7: return "7"; break;
    case 8: return "8"; break;
    case 9: return "9"; break;
    case 10: return "I"; break;
    case 11: return "I"; break;
    case 12: return "I"; break;
    case 13: return "I"; break;
    case 14: return "I"; break;
    case 15: return "I"; break;
  }
  return "?";
}
char getinsL(int nval) {
  if (nval==0) {return '.';}
  else{return getlnibble(nval);}
}
char getinsH(int nval) {
  if (nval==0) {return '.';}
  else{return gethnibble(nval);}
}
const char* getVFX(int fxval) {
  if (fxval==0) {return ".";} // 0
  switch (fxval>>4) {
    case 0: return "a"; break;
    case 1: return "b"; break;
    case 2: return "c"; break;
    case 3: return "d"; break;
    case 4: case 5: case 6: case 7:
      return "v"; break;
    case 8: case 9: case 10: case 11:
      return "p"; break;
    case 12: return "e"; break;
    case 13: return "f"; break;
    case 14: return "g"; break;
    case 15: return "h"; break;

    case 28: return "l"; break;
    case 29: return "r"; break;
    case 30: return "o"; break;
    case 31: return "u"; break;
  }
  return "?";
}
unsigned char getVFXVal(int nval) {
  if (nval==0) {return 255;} // 0
  if (nval<128 && nval>63) {return nval-64;} // 64-127
  if (nval<193 && nval>127) {return nval-128;} // 128-192
  return nval&0x0f;
}
unsigned char getVFXColor(int fxval) {
  if (fxval==0) {return 81;} // 0

  switch (fxval>>4) {
    case 0: return 2; break;
    case 1: return 2; break;
    case 2: return 2; break;
    case 3: return 2; break;
    case 4: case 5: case 6: case 7:
      return 27; break;
    case 8: case 9: case 10: case 11:
      return 6; break;
    case 12: return 3; break;
    case 13: return 3; break;
    case 14: return 3; break;
    case 15: return 3; break;

    case 28: return 117; break;
    case 29: return 117; break;
    case 30: return 56; break;
    case 31: return 3; break;
  }
  switch((fxval-193)/10) {
  case 0: return 3; break; // 193-202
  case 1: return 3; break; // 203-212
  case 2: return 56; break; // 213-222
  }
  return 15;
}
const char* getFX(int fxval) {
  switch(fxval) {
  case 0: return "."; break;
  case 1: return "A"; break;
  case 2: return "B"; break;
  case 3: return "C"; break;
  case 4: return "D"; break;
  case 5: return "E"; break;
  case 6: return "F"; break;
  case 7: return "G"; break;
  case 8: return "H"; break;
  case 9: return "I"; break;
  case 10: return "J"; break;
  case 11: return "K"; break;
  case 12: return "L"; break;
  case 13: return "M"; break;
  case 14: return "N"; break;
  case 15: return "O"; break;
  case 16: return "P"; break;
  case 17: return "Q"; break;
  case 18: return "R"; break;
  case 19: return "S"; break;
  case 20: return "T"; break;
  case 21: return "U"; break;
  case 22: return "V"; break;
  case 23: return "W"; break;
  case 24: return "X"; break;
  case 25: return "Y"; break;
  case 26: return "Z"; break;
  case 27: return "v"; break;
  }
  return "?";
}
const char* getFX_PT(int fxval) {
  switch(fxval) {
  case 0: return "."; break;
  case 1: return "F"; break;
  case 2: return "B"; break;
  case 3: return "D"; break;
  case 4: return "A"; break;
  case 5: return "2"; break;
  case 6: return "1"; break;
  case 7: return "3"; break;
  case 8: return "4"; break;
  case 9: return "T"; break;
  case 10: return "0"; break;
  case 11: return "6"; break;
  case 12: return "5"; break;
  case 13: return "M"; break;
  case 14: return "N"; break;
  case 15: return "9"; break;
  case 16: return "P"; break;
  case 17: return "R"; break;
  case 18: return "7"; break;
  case 19: return "E"; break;
  case 20: return "f"; break;
  case 21: return "U"; break;
  case 22: return "G"; break;
  case 23: return "H"; break;
  case 24: return "8"; break;
  case 25: return "Y"; break;
  case 26: return "Z"; break;
  case 27: return "C"; break;
  }
  return "?";
}
int hscale(int nval) {
  // playback to storage note converter
  return ((nval/12)*16)+(nval%12)+1;
}
int mscale(int nval) {
  // storage to playback note converter
  return ((nval/16)*12)+((nval-1)%16);
}

string strFormat(const char* format, ...) {
  va_list va;
  char str[4096];
  string ret;
  va_start(va,format);
  if (vsnprintf(str,4095,format,va)<0) {
    va_end(va);
    return string("");
  }
  va_end(va);
  ret=str;
  return ret;
}

string getVisPat(unsigned char p) {
  if (p==254) return "--";
  if (p==255) return "==";
  return strFormat("%.2X",p);
}

// formula to calculate 6203.34:
// - 65536*440/(chipClock/64)
// chipClock is 297500 (PAL)
// or 309000 (NTSC)
unsigned int mnoteperiod(float note, int chan) {
  return (int)((6203.34-(song->detune*2))*(pow(2.0f,(float)(((float)note-58)/12.0f))));
}

int msnoteperiod(float note, int chan) {
  return ((297500+(song->detune*100))/(440*(pow(2.0f,(float)(((float)note-58)/12)))));
}

void Zxx(unsigned char value) {
  // process Zxx effect
  if (value>0x7f) {
  switch(value) {
    //case 
  }
  }
}
int FreeChannel() {
  // TODO
  return 0;
}

void Playback() {
  // TODO
}

void CleanupPatterns() {
  canUseSong.lock();
  player.reset();
  if (song!=NULL) {
    delete song;
    song=NULL;
    player.setSong(NULL);
  }
  song=new Song;
  player.setSong(song);
  origin="Unknown";
  canUseSong.unlock();
}

void ParentDir(char* thedir) {
  // set thedir to parent directory
#ifdef ANDROID
  // don't let the user escape device storage
  if (strcmp(thedir,"/storage/emulated/0")==0) {
    triggerfx(1);
    return;
  }
#endif
#ifdef _WIN32
  if (strcmp(thedir+1,":\\")==0 && thedir[0]>='A' && thedir[0]<='Z') {
    strcpy(thedir,"");
    return;
  }
#endif
  if (strrchr(thedir,DIR_SEPARATOR)!=NULL) {
    memset(strrchr(thedir,DIR_SEPARATOR),0,1);
    if (strchr(thedir,DIR_SEPARATOR)==NULL) {
      size_t littlestr=strlen(thedir);
      memset(thedir+littlestr,DIR_SEPARATOR,1);
      memset(thedir+littlestr+1,0,1);
    }
  }
}
int NumberLetter(char cval) {
  switch (cval) {
    case '0': return 0; break;
    case '1': return 1; break;
    case '2': return 2; break;
    case '3': return 3; break;
    case '4': return 4; break;
    case '5': return 5; break;
    case '6': return 6; break;
    case '7': return 7; break;
    case '8': return 8; break;
    case '9': return 9; break;
  }
  fprintf(stderr,"invalid number value entered");
  return 0;
}

char shapeSym(int sh) {
  switch (sh) {
    case 0: return 's'; break;
    case 1: return 'R'; break;
    case 2: return 'S'; break;
    case 3: return 't'; break;
    case 4: return 'N'; break;
    case 5: return 'n'; break;
  }
  return '?';
}

// CONCEPT FOR NEW CONFIG BEGIN //

// 1. General
// - UI type (auto, modern, mobile or classic)
// - import samples
// - keyboard layout

// 2. Audio
// - audio system (JACK or SDL, if available)
// - JACK config
//   - server name
//   - client name
//   - mono/stereo
//   - auto-connect (and where)
// - SDL config
//   - audio driver
//   - audio device
//   - sample rate
//   - audio format
//   - mono/stereo
//   - latency

// 3. Video
// - scale factor
// - default zoom

// CONCEPT FOR NEW CONFIG END //
float getLKeyOff(int tone) {
  switch (tone) {
    case 0: case 3:
      return 1;
    default: return 4;
  }
}

float getRKeyOff(int tone) {
  switch (tone) {
    case 2: case 6:
      return 10;
    default: return 7;
  }
}

void Play() {
  // reset cursor position
  tickstart=true;
  player.play();
}
unsigned char ITVolumeConverter(unsigned char itvol) {
  if (itvol<65) {return minval(itvol+64,127);} // 64-127
  if (itvol<193 && itvol>127) {return minval(itvol,191);} // 128-192
  if (itvol>192) {
    switch ((itvol-193)/10) {
      case 0: return 0xe0+(itvol-193); break; // 1-10
      case 1: return 0xf0+(itvol-203); break; // 11-20
    }
  }
  switch ((itvol-65)/10) {
    case 0: return itvol-65; break; // 1-10
    case 1: return 0x10+itvol-75; break; // 11-20
    case 2: return 0x20+itvol-85; break; // 21-30
    case 3: return 0x30+itvol-95; break; // 31-40
    case 4: return 0xc0+(itvol-105); break; // 41-50
    case 5: return 0xd0+(itvol-115); break; // 51-60
  }
  return itvol;
}
int ImportIT(FILE* it) {
  // import IT file, after YEARS I wasn't able to do this.
  // check out http://schismtracker.org/wiki/ITTECH.TXT for specs in IT format
  size_t size;
  char * memblock;
  int sk;
  int samples;
  //string fn;
  int NextByte;
  int NextChannel;
  int CurrentRow;
  int NextMask[32];
  int LastNote[32];
  int LastIns[32];
  int LastVol[32];
  int LastFX[32];
  int LastFXVal[32];
  //abort();
  char rfn[4096];
  strcpy(rfn,".");
  //int insparas[256];
  int patparas[256];
  //gets(rfn);
  if (it!=NULL) { // read the file
  printf("loading IT file, ");
    size=fsize(it);
    printf("%zu bytes\n",size);
    memblock=new char[size];
    fseek(it,0,SEEK_SET);
    fread(memblock,1,size,it);
    fclose(it);
  if (memblock[0]=='I' && memblock[1]=='M' && memblock[2]=='P' && memblock[3]=='M') {
    printf("IT module detected\n");
    CleanupPatterns();
    // name
    name="";
    for (sk=4;sk<30;sk++) {
      if (memblock[sk]==0) break;
      name+=memblock[sk];
    }
    printf("module name is %s\n",name.c_str());
    origin="Impulse Tracker";
    // orders, instruments, samples and patterns
    printf("%d orders, %d instruments, %d samples, %d patterns\n",(int)memblock[0x20],(int)memblock[0x22],(int)memblock[0x24],(int)memblock[0x26]);
    song->orders=(unsigned char)memblock[0x20];
    instruments=(unsigned char)memblock[0x22];
    patterns=(unsigned char)memblock[0x26];
    samples=(unsigned char)memblock[0x24];
    //cout << (int)memblock[0x29] << "." << (int)memblock[0x28];
    printf("\n");
    //cout << "volumes: global " << (int)(unsigned char)memblock[0x30] << ", mixing " << (int)(unsigned char)memblock[0x31] << "\n";
    //cout << "speeds: " << (int)memblock[0x32] << ":" << (int)(unsigned char)memblock[0x33] << "\n";
    song->speed=memblock[0x32];
    printf("---pans---\n");
    for (sk=0x40;sk<0x60;sk++) {
      song->defaultPan[sk-64]=memblock[sk];
      printf("%d ",(int)memblock[sk]);
    }
    printf("\n");
    printf("---volumes---\n");
    for (sk=0x80;sk<0xa0;sk++) {
      song->defaultVol[sk-128]=memblock[sk]*2;
      printf("%d ",(int)memblock[sk]);
    }
    printf("\n");
    printf("---ORDER LIST---\n");
    for (sk=0xc0;sk<(0xc0+song->orders);sk++) {
      song->order[sk-0xc0]=memblock[sk];
      switch(memblock[sk]) {
      case -2: printf("+++ "); break;
      case -1: printf("--- "); break;
      default: printf("%d ",(int)(unsigned char)memblock[sk]); break;
      }
    }
    printf("\n");
    // pointers
    printf("\n---POINTERS---\n");
    for (sk=0;sk<patterns;sk++) {
    patparas[sk]=((unsigned char)memblock[0xc0+song->orders+(instruments*4)+(samples*4)+(sk*4)])+(((unsigned char)memblock[0xc0+song->orders+(instruments*4)+(samples*4)+(sk*4)+1])*256)+(((unsigned char)memblock[0xc0+song->orders+(instruments*4)+(samples*4)+(sk*4)+2])*65536)+(((unsigned char)memblock[0xc0+song->orders+(instruments*4)+(samples*4)+(sk*4)+3])*16777216);
    printf("pattern %d offset: ",sk);
    printf("%d\n",patparas[sk]);
    }
    // load/unpack patterns
    song->channels=1;
    for (int pointer=0;pointer<patterns;pointer++) {
    printf("-unpacking pattern %d-\n",pointer);
    CurrentRow=0;
    sk=patparas[pointer];
    int patsize=(unsigned char)memblock[sk]+((unsigned char)memblock[sk+1]*256);
    int plength=(unsigned char)memblock[sk+2]+((unsigned char)memblock[sk+3]*256);
    Pattern* p=song->getPattern(pointer,true);
    printf("%d bytes in pattern\n",patsize);
    p->length=plength; // set length
    sk=patparas[pointer]+8;
    for (int a=0;a<patsize;a++) {
    NextByte=(unsigned char)memblock[sk+a];
    if (NextByte==0) {
      CurrentRow++;
      if (CurrentRow==plength) {break;}
      continue;
    }
    NextChannel=(NextByte-1)&31;
    if (NextChannel>=song->channels) {
      song->channels=NextChannel+1;
      if (song->channels>32) song->channels=32;
    }
    if ((NextByte&128)==128) {
      a++;
      NextMask[NextChannel]=(unsigned char)memblock[sk+a];
    }
    if (NextMask[NextChannel]&1) {
      a++;
      // decode melodical byte into raw byte
      LastNote[NextChannel]=(unsigned char)memblock[sk+a];
      switch (LastNote[NextChannel]) {
        case 255: p->data[CurrentRow][NextChannel][0]=13; break;
        case 254: p->data[CurrentRow][NextChannel][0]=15; break;
        default: if (LastNote[NextChannel]<120) {p->data[CurrentRow][NextChannel][0]=((LastNote[NextChannel]/12)*16)+(LastNote[NextChannel]%12)+1;/*p->data[CurrentRow][NextChannel][2]=0x7f;*/} else {p->data[CurrentRow][NextChannel][0]=14;}; break;
      }
    }
    if (NextMask[NextChannel]&2) {
      a++;
      LastIns[NextChannel]=(unsigned char)memblock[sk+a];
      p->data[CurrentRow][NextChannel][1]=LastIns[NextChannel];
    }
    if (NextMask[NextChannel]&4) {
      a++;
      LastVol[NextChannel]=(unsigned char)memblock[sk+a];
      p->data[CurrentRow][NextChannel][2]=ITVolumeConverter(LastVol[NextChannel]);
    }
    if (NextMask[NextChannel]&8) {
      a++;
      LastFX[NextChannel]=(unsigned char)memblock[sk+a];
      p->data[CurrentRow][NextChannel][3]=LastFX[NextChannel];
      a++;
      LastFXVal[NextChannel]=(unsigned char)memblock[sk+a];
      p->data[CurrentRow][NextChannel][4]=LastFXVal[NextChannel];
    }
    if (NextMask[NextChannel]&16) {
      switch (LastNote[NextChannel]) {
        case 255: p->data[CurrentRow][NextChannel][0]=13; break;
        case 254: p->data[CurrentRow][NextChannel][0]=15; break;
        default: if (LastNote[NextChannel]<120) {p->data[CurrentRow][NextChannel][0]=((LastNote[NextChannel]/12)*16)+(LastNote[NextChannel]%12)+1;/*p->data[CurrentRow][NextChannel][2]=0x7f;*/} else {p->data[CurrentRow][NextChannel][0]=14;}; break;
      }
    }
    if (NextMask[NextChannel]&32) {
      p->data[CurrentRow][NextChannel][1]=LastIns[NextChannel];
    }
    if (NextMask[NextChannel]&64) {
      p->data[CurrentRow][NextChannel][2]=ITVolumeConverter(LastVol[NextChannel]);
    }
    if (NextMask[NextChannel]&128) {
      p->data[CurrentRow][NextChannel][3]=LastFX[NextChannel];
      p->data[CurrentRow][NextChannel][4]=LastFXVal[NextChannel];
    }
    }
  }

  } else {printf("error while importing file! not an IT module file\n"); delete[] memblock; return 1;}
    delete[] memblock;
  }
  else {printf("error while importing file! file doesn't exist\n"); return 1;}
  if (!playermode && !fileswitch) {player.pat=0;}
  if (name=="") {
    SDL_SetWindowTitle(sdlWin,PROGRAM_NAME);
  } else {
    SDL_SetWindowTitle(sdlWin,(name+S(" - ")+S(PROGRAM_NAME)).c_str());
  }
  song->orders--;
  if (song->order[song->orders]==0xff) song->orders--;
  return 0;
}

struct MODHeader {
  char name[20];
  struct {
    char name[22];
    unsigned short len;
    char pitch;
    char vol;
    unsigned short loopStart;
    unsigned short loopLen;
  } ins[31];
  char len;
  char loop;
  char ord[128];
  int ident;
};

struct ClassicMODHeader {
  char name[20];
  struct {
    char name[22];
    unsigned short len;
    char pitch;
    char vol;
    unsigned short loopStart;
    unsigned short loopLen;
  } ins[15];
  char len;
  char loop;
  char ord[128];
};

int ImportMOD(FILE* mod) {
  // import MOD file
  // check out http://www.fileformat.info/format/mod/corion.htm for specs in MOD format
  MODHeader h;
  int sk;
  //string fn;
  int chans=0;
  int CurrentSampleSeek=0;
  bool karsten=false;
  int insMax=31;
  unsigned char noteVal[4];
  if (mod!=NULL) { // read the file
    printf("loading MOD file, ");
    CleanupPatterns();
    fseek(mod,0,SEEK_SET);
    fread(&h,1,sizeof(MODHeader),mod);
    printf("success, now importing file\n");
    switch (h.ident) {
      case 0x2e4b2e4d: // M.K.
        printf("4-channel ProTracker/NoiseTracker module detected\n");
        origin="ProTracker/NoiseTracker";
        chans=4;
        break;
      case 0x214b214d: // M!K!
        printf("4-channel ProTracker module detected\n");
        origin="ProTracker";
        chans=4;
        break;
      case 0x31384443: // CD81
      case 0x41544b4f: // OKTA
      case 0x4154434f: // OCTA
        printf("8-channel Oktalyzer/Octalyzer/OctaMED module detected\n");
        origin="Oktalyzer/Octalyzer/OctaMED";
        chans=8;
        break;
      case 0x214b264d: // M&K!
        printf("echobea3.mod detected\n");
        origin="congratulations. you are a master.";
        chans=4;
        break;
    }
    if ((h.ident&0xffffff00)==0x4e484300) { // .CHN
      printf("multi-channel module detected\n");
      origin="FastTracker or similar";
      chans=(h.ident&0xff)-'0';
    } else if ((h.ident&0xffffff)==0x544c46) { // FLT.
      printf("multi-channel Fairlight module detected\n");
      origin="StarTrekker";
      chans=((h.ident&0xff000000)>>24)-'0';
    } else if ((h.ident&0xffffff)==0x5a4454) { // TDZ.
      printf("multi-channel TakeTracker module detected\n");
      origin="TakeTracker";
      chans=((h.ident&0xff000000)>>24)-'0';
    } else if ((h.ident&0xffff0000)==0x48430000) { // ..CH
      printf("multi-channel module detected\n");
      origin="FastTracker";
      chans=10*((h.ident&0xff)-'0')+((((h.ident&0xff00)>>8)-'0'));
    } else if ((h.ident&0xffff0000)==0x4e430000) { // ..CN
      printf("multi-channel TakeTracker module detected\n");
      origin="TakeTracker";
      chans=(h.ident&0xff)-'0'+((((h.ident&0xff00)>>8)-'0')*10);
    }
    if (chans==0) {
      printf("Amiga Soundtracker module detected\n");
      origin="Amiga Soundtracker";
      chans=4;
      karsten=true;
      insMax=15;
    }
    for (int ii=0;ii<insMax;ii++) {
      h.ins[ii].len=ntohs(h.ins[ii].len);
      h.ins[ii].loopStart=ntohs(h.ins[ii].loopStart);
      h.ins[ii].loopLen=ntohs(h.ins[ii].loopLen);
      for (int jj=0;jj<22;jj++) {
        song->ins[ii+1]->name[jj]=h.ins[ii].name[jj];
      }
      song->ins[ii+1]->vol=h.ins[ii].vol;
      song->ins[ii+1]->pcmPos=CurrentSampleSeek;
      song->ins[ii+1]->pitch=h.ins[ii].pitch<<4;
      int tempsize;
      tempsize=h.ins[ii].len*2;
      int repeatpos;
      if (karsten) {
        repeatpos=h.ins[ii].loopStart;
      } else {
        repeatpos=h.ins[ii].loopStart*2;
      }
      int repeatlen;
      repeatlen=h.ins[ii].loopLen*2;
      //printf("sample %d size: %.5x repeat: %.4x replen: %.4x\n",ii,tempsize,repeatpos,repeatlen);
      //printf("finetune %d vol %d\n",h.ins[ii].pitch,h.ins[ii].vol);

      if (karsten) {
        song->ins[ii+1]->pcmPos=(CurrentSampleSeek+repeatpos);
        song->ins[ii+1]->pcmLen=(repeatpos>0 || repeatlen>2)?(minval(tempsize,repeatlen)):(tempsize);
        song->ins[ii+1]->pcmLoop=0;
      } else {
        song->ins[ii+1]->pcmLen=(repeatpos>0 || repeatlen>2)?(minval(tempsize,repeatpos+repeatlen)):(tempsize);
        song->ins[ii+1]->pcmLoop=repeatpos;
      }
      
      song->ins[ii+1]->pcmMult|=(repeatpos>0 || repeatlen>2)?(128):(0);
      CurrentSampleSeek+=tempsize;
      song->ins[ii+1]->noteOffset=12;
      song->ins[ii+1]->filterMode|=8;
    }
    // name
    name="";
    for (sk=0;sk<20;sk++) {
      if (h.name[sk]==0) break;
      name+=h.name[sk];
    }
    printf("module name is %s\n",name.c_str());
    if (karsten) {
      for (sk=0;sk<128;sk++) {
        song->order[sk]=(*((ClassicMODHeader*)&h)).ord[sk];
      }
      patterns=0;
      for (sk=0;sk<128;sk++) {
        if (song->order[sk]>patterns) {patterns=song->order[sk];}
      }
      song->orders=(*((ClassicMODHeader*)&h)).len;
      // BPM if it is set
      song->tempo=(5*716*1024)/((240-(*((ClassicMODHeader*)&h)).loop)*122*2);
    } else {
      for (sk=0;sk<128;sk++) {
        song->order[sk]=h.ord[sk];
      }
      patterns=0;
      for (sk=0;sk<128;sk++) {
        if (song->order[sk]>patterns) {patterns=song->order[sk];}
      }
      song->orders=h.len;
    }
    printf("putting samples to PCM memory if possible\n");
    if (karsten) {
      fseek(mod,sizeof(ClassicMODHeader)+((patterns+1)*chans*64*4),SEEK_SET);
    } else {
      fseek(mod,sizeof(MODHeader)+((patterns+1)*chans*64*4),SEEK_SET);
    }
    if (fread(chip[0].pcm,1,SOUNDCHIP_PCM_SIZE,mod)==SOUNDCHIP_PCM_SIZE) {
      popbox=PopupBox("Warning","out of PCM memory to load all samples!");
    }
    memcpy(chip[1].pcm,chip[0].pcm,SOUNDCHIP_PCM_SIZE);
    memcpy(chip[2].pcm,chip[0].pcm,SOUNDCHIP_PCM_SIZE);
    memcpy(chip[3].pcm,chip[0].pcm,SOUNDCHIP_PCM_SIZE);
    //printf("---PATTERNS---\n");
    if (karsten) {
      fseek(mod,sizeof(ClassicMODHeader),SEEK_SET);
    } else {
      fseek(mod,sizeof(MODHeader),SEEK_SET);
    }
    for (int importid=0;importid<patterns+1;importid++) {
      //printf("-PATTERN %d-\n",importid);
      Pattern* p=song->getPattern(importid,true);
      for (int indxr=0;indxr<64;indxr++) {
        int NPERIOD;
        int NINS;
        int NFX;
        int NFXVAL;
        for (int ichan=0;ichan<chans;ichan++) {
          // import pattern row
          fread(noteVal,1,4,mod);
          NPERIOD=((noteVal[0]&0x0f)<<8)+noteVal[1];
          NINS=(noteVal[0]&0xf0)+(noteVal[2]>>4);
          NFX=noteVal[2]&0x0f;
          NFXVAL=noteVal[3];
          // conversion stuff
          if (NPERIOD==0) {
            p->data[indxr][ichan][0]=0;
          } else {
            p->data[indxr][ichan][0]=hscale(round((log(float(894841.0/NPERIOD)/65.406391325149658669)/log(2.0))*12));
          }
          p->data[indxr][ichan][1]=NINS;
          switch(NFX) {
            case 0: if (NFXVAL!=0) {p->data[indxr][ichan][3]=10;p->data[indxr][ichan][4]=NFXVAL;} else {p->data[indxr][ichan][3]=0;p->data[indxr][ichan][4]=0;}; break;
            case 1:
              if (karsten) {
                p->data[indxr][ichan][3]=10;
              } else {
                p->data[indxr][ichan][3]=6;
              }
              p->data[indxr][ichan][4]=NFXVAL;
              break;
            case 2:
              if (karsten) {
                if (NFXVAL<16) {
                  p->data[indxr][ichan][3]=6;
                  p->data[indxr][ichan][4]=NFXVAL;
                } else {
                  p->data[indxr][ichan][3]=5;
                  p->data[indxr][ichan][4]=NFXVAL>>4;
                }
              } else {
                p->data[indxr][ichan][3]=5;
                p->data[indxr][ichan][4]=NFXVAL;
              }
              break;
            case 3: p->data[indxr][ichan][3]=7;p->data[indxr][ichan][4]=NFXVAL; break;
            case 4: p->data[indxr][ichan][3]=8;p->data[indxr][ichan][4]=NFXVAL; break;
            case 5: p->data[indxr][ichan][3]=12;p->data[indxr][ichan][4]=NFXVAL; break;
            case 6: p->data[indxr][ichan][3]=11;p->data[indxr][ichan][4]=NFXVAL; break;
            case 7: p->data[indxr][ichan][3]=18;p->data[indxr][ichan][4]=NFXVAL; break;
            case 8: p->data[indxr][ichan][3]=24;p->data[indxr][ichan][4]=NFXVAL; break;
            case 9: p->data[indxr][ichan][3]=15;p->data[indxr][ichan][4]=NFXVAL; break;
            case 10: p->data[indxr][ichan][3]=4;p->data[indxr][ichan][4]=NFXVAL; break;
            case 11: p->data[indxr][ichan][3]=2;p->data[indxr][ichan][4]=NFXVAL; break;
            case 12: p->data[indxr][ichan][2]=0x40+minval(NFXVAL,0x3f);p->data[indxr][ichan][4]=0; break;
            case 13: p->data[indxr][ichan][3]=3;p->data[indxr][ichan][4]=NFXVAL-(NFXVAL>>4)*6; break;
            case 14: p->data[indxr][ichan][3]=19;switch (NFXVAL>>4) {
              case 1: p->data[indxr][ichan][4]=0xf0+(NFXVAL%16); p->data[indxr][ichan][3]=6; break;
              case 2: p->data[indxr][ichan][4]=0xf0+(NFXVAL%16); p->data[indxr][ichan][3]=5; break;
              case 3: p->data[indxr][ichan][4]=0x20+(NFXVAL%16); break;
              case 4: p->data[indxr][ichan][4]=0x30+(NFXVAL%16); break;
              case 6: p->data[indxr][ichan][4]=0xb0+(NFXVAL%16); break;
              case 7: p->data[indxr][ichan][4]=0x40+(NFXVAL%16); break;
              case 9: p->data[indxr][ichan][4]=(NFXVAL%16); p->data[indxr][ichan][3]=17; break;
              case 10: p->data[indxr][ichan][4]=0x0f+((NFXVAL%16)<<4); p->data[indxr][ichan][3]=4; break;
              case 11: p->data[indxr][ichan][4]=0xf0+(NFXVAL%16); p->data[indxr][ichan][3]=4; break;
              default: p->data[indxr][ichan][4]=NFXVAL; break;
            }; break;
            case 15: p->data[indxr][ichan][3]=1;p->data[indxr][ichan][4]=NFXVAL;if (NFXVAL>0x20) {p->data[indxr][ichan][3]=20;}; break;
          }
        }
      }
    }
  }
  else {/*cout << "error while importing file! file doesn't exist\n";*/ return 1;}
  song->detune=0x1b; // Amiga compat
  if (!playermode && !fileswitch) {player.pat=0;}
  if (name=="") {
    SDL_SetWindowTitle(sdlWin,PROGRAM_NAME);
  } else {
    SDL_SetWindowTitle(sdlWin,(name+S(" - ")+S(PROGRAM_NAME)).c_str());
  }
  song->channels=chans;
  song->orders--;
  return 0;
}
int ImportS3M(FILE* s3m) {
  // import S3M file
  size_t size;
  char* memblock;
  int sk;
  int NextByte;
  int NextChannel;
  int CurrentRow;
  int insparas[99];
  int patparas[256];
  if (s3m!=NULL) { // read the file
    printf("loading S3M file, ");
    size=fsize(s3m);
    printf("%zu bytes\n",size);
    memblock=new char[size];
    fseek(s3m,0,SEEK_SET);
    fread(memblock,1,size,s3m);
    fclose(s3m);
    printf("success, now importing file\n");
    CleanupPatterns();
  // module name
  for (sk=0;sk<28;sk++) {
    if (memblock[sk]==0) break;
    name+=memblock[sk];
  }
  printf("module name is %s\n",name.c_str());
  song->orders=memblock[0x20];
  instruments=memblock[0x22];
  patterns=memblock[0x24]*2;
  printf("%d orders, %d instruments, %d patterns\n",song->orders,instruments,patterns);
  // order list
  printf("---ORDER LIST---\n");
  for (sk=0x60;sk<song->orders+0x60;sk++) {
    song->order[sk-0x60]=memblock[sk];
    switch(memblock[sk]) {
      case -2: printf("+++ "); break;
      case -1: printf("--- "); break;
      default: printf("%d ",(int)(unsigned char)memblock[sk]); break;
      }
    }
  // pointers
  printf("\n---POINTERS---\n");
  for (sk=0x60+song->orders;sk<(0x60+song->orders+instruments);sk+=2) {
    insparas[(sk-(0x60+song->orders))/2]=((unsigned char)memblock[sk]*16)+((unsigned char)memblock[sk+1]*4096);
    printf("instrument %d offset: ",(sk-(0x60+song->orders))/2);
    printf("%d\n",insparas[(sk-(0x60+song->orders))/2]);
    }
  for (sk=0x60+song->orders+(instruments*2);sk<(0x60+song->orders+(instruments*2)+patterns);sk+=2) {
    patparas[(sk-(0x60+song->orders+(instruments*2)))/2]=((unsigned char)memblock[sk]*16)+((unsigned char)memblock[sk+1]*4096);
    printf("pattern %d offset: ",(sk-(0x60+song->orders+(instruments*2)))/2);
    printf("%d\n",patparas[(sk-(0x60+song->orders+(instruments*2)))/2]);
    }
  // unpack patterns
  for (int pointer=0;pointer<(patterns/2);pointer++) {
    printf("-unpacking pattern %d-\n",pointer);
    Pattern* p=song->getPattern(pointer,true);
    CurrentRow=0;
    sk=patparas[pointer];
    int patsize=(unsigned char)memblock[sk]+((unsigned char)memblock[sk+1]*256);
    printf("%d bytes in pattern\n",patsize);
    sk=patparas[pointer]+2;
    for (int a=0;a<patsize;a++) {
    NextByte=(unsigned char)memblock[sk+a];
    if (NextByte==0) {
      CurrentRow++;
      if (CurrentRow==64) {break;}
      continue;
    }
    NextChannel=NextByte%32;
    if ((NextByte>>5)%2) {
      a++;
      p->data[CurrentRow][NextChannel][0]=(unsigned char)memblock[sk+a]+17;
      a++;
      p->data[CurrentRow][NextChannel][1]=(unsigned char)memblock[sk+a];
      //p->data[CurrentRow][NextChannel][2]=127;
    }
    if ((NextByte>>6)%2) {
      a++;
      p->data[CurrentRow][NextChannel][2]=minval(127,(unsigned char)64+memblock[sk+a]);
    }
    if ((NextByte>>7)%2) {
      a++;
      p->data[CurrentRow][NextChannel][3]=(unsigned char)memblock[sk+a];
      a++;
      p->data[CurrentRow][NextChannel][4]=(unsigned char)memblock[sk+a];
    }
    }
  }

  delete[] memblock;
  }
  origin="Scream Tracker 3";
  return 0;
}

// THE FINAL ONE
struct XMHeader {
  char name[37];
  char x1a;
  char program[20];
  short ver;
  int size;
  short orders, loop, chans, pats, instrs, flags, speed, tempo;
  char ord[256];
  char padding[256];
};

struct XMPatternHeader {
  int size;
  unsigned char packType;
  unsigned char rows[2];
  unsigned char len[2];
  char padding[256];
};

struct XMInstrHeader {
  int size;
  char name[22];
  char type;
  char samples[2];
  char padding[256];
};

struct XMEnvPoint {
  unsigned short pos, val;
};

struct XMInstrContHeader {
  int size;
  char sample[96];
  XMEnvPoint envVol[12];
  XMEnvPoint envPan[12];
  unsigned char envVolCount;
  unsigned char envPanCount;
  unsigned char volSus, volLoopStart, volLoopEnd;
  unsigned char panSus, panLoopStart, panLoopEnd;
  unsigned char volType, panType;
  char vibType, vibSweep, vibDepth, vibSpeed;
  short volFade, unused;
  char padding[256];
};

struct XMSampleHeader {
  int size;
  int loopStart, loopSize;
  char vol, pitch;
  char flags;
  char pan;
  char note;
  char unused;
  char padding[256];
};

int XMVolume(int val) {
  switch (val>>4) {
    case 0: return 0; break;
    case 1: case 2: case 3: case 4: case 5:
      return 0x40+minval(0x3f,val-16); break;
    case 6: return 0x30+(val&0x0f); break;
    case 7: return 0x20+(val&0x0f); break;
    case 8: return 0x10+(val&0x0f); break;
    case 9: return 0x00+(val&0x0f); break;
    case 10: return 0x1e0+(val&0x0f); break;
    case 11: return 0xe0+(val&0x0f); break;
    case 12: return 0x80+(val&0x0f)*4; break;
    case 13: return 0x1c0+(val&0x0f); break;
    case 14: return 0x1d0+(val&0x0f); break;
    case 15: return 0xf0+(val&0x0f); break;
  }
  return 0;
}

int XMEffect(int fx, int fxv) {
  switch (fx) {
    case 0:
      if (fxv!=0) {
        return 10;
      } else {
        return 0;
      }
      break;
    case 1: return 6; break;
    case 2: return 5; break;
    case 3: return 7; break;
    case 4: return 8; break;
    case 5: return 12; break;
    case 6: return 11; break;
    case 7: return 18; break;
    case 8: return 24; break;
    case 9: return 15; break;
    case 10: return 4; break;
    case 11: return 2; break;
    case 12: return 27; break;
    case 13: return 3; break;
    case 14:
      return 19; // todo
      break;
    case 15:
      if (fxv>31) {
        return 20;
      } else {
        return 1;        
      }
      break;
    case 16: return 22; break;
    case 17: return 23; break;
    case 19: return 19; break;
    case 20: return 15; break;
    case 24: return 16; break;
    case 26: return 17; break;
    case 28: return 9; break;
    case 32:
      return 0; // todo
      break;
  }
  printf("this effect\n");
  return 0;
}

int ImportXM(FILE* xm) {
  XMHeader h;
  XMPatternHeader ph;
  XMInstrHeader ih;
  XMInstrContHeader ich;
  XMSampleHeader sh;
  unsigned char patData[65536];
  int sk, curChan, curRow, nextNote, vol, fx, fxval;
  short delta;
  int sampleSeek=0;
  printf("loading XM\n");
  fseek(xm,0,SEEK_SET);
  fread(&h,1,336,xm);
  
  CleanupPatterns();
  name="";
  for (int i=0; i<20; i++) {
    if (h.name[i+17]==0) break;
    name+=h.name[i+17];
  }
  
  origin="";
  for (int i=0; i<20; i++) {
    if (h.name[i+17]==0) break;
    origin+=h.name[i+17];
  }

  for (int i=0; i<32; i++) {
    song->defaultPan[i]=0;
  }
  
  for (int i=0; i<h.orders; i++) {
    song->order[i]=h.ord[i];
  }
  song->orders=h.orders-1;
  song->channels=h.chans;
  song->speed=h.speed;
  song->tempo=h.tempo;
  
  printf("seeking to %d\n",60+h.size);
  fseek(xm,60+h.size,SEEK_SET);
  for (int i=0; i<h.pats; i++) {
    fread(&ph,1,9,xm);
    if (ph.size!=9) {
      popbox=PopupBox("Error",strFormat("pattern %d header size mismatch! %d != 9",i,ph.size));
      triggerfx(1);
      fclose(xm);
      return 1;
    }
    Pattern* p=song->getPattern(i,true);
    p->length=ph.rows[0];
    fread(patData,1,ph.len[0]+(ph.len[1]<<8),xm);
    // decode pattern!
    sk=0;
    curChan=0; curRow=0;
    do {
      nextNote=patData[sk++];
      if (nextNote&0x80) {
        if (nextNote&1) {
          if (patData[sk]>=96) {
            p->data[curRow][curChan][0]=0x0d;
          } else {
            p->data[curRow][curChan][0]=1+(((patData[sk]-1)/12)<<4)+((patData[sk]-1)%12);
          }
          sk++;
        }
        if (nextNote&2) p->data[curRow][curChan][1]=patData[sk++];
        fx=0; fxval=0; vol=0;
        if (nextNote&4) vol=patData[sk++];
        if (nextNote&8) fx=patData[sk++];
        if (nextNote&16) fxval=patData[sk++];
        
        p->data[curRow][curChan][2]=XMVolume(vol);
        p->data[curRow][curChan][3]=((XMVolume(vol)&0x100)>>1)|XMEffect(fx,fxval);
        p->data[curRow][curChan][4]=fxval;
      } else {
        if (nextNote>=96) {
          p->data[curRow][curChan][0]=0x0d;
        } else {
          p->data[curRow][curChan][0]=1+(((nextNote-1)/12)<<4)+((nextNote-1)%12);
        }
        p->data[curRow][curChan][1]=patData[sk++];
        vol=patData[sk++];
        fx=patData[sk++];
        fxval=patData[sk++];
        p->data[curRow][curChan][2]=XMVolume(vol);
        p->data[curRow][curChan][3]=XMEffect(fx,fxval);
        p->data[curRow][curChan][4]=fxval;
      }
      curChan++;
      if (curChan>=h.chans) {
        curChan=0;
        curRow++;
      }
    } while (sk<(ph.len[0]+(ph.len[1]<<8)));
  }

  // decode instruments
  for (int i=0; i<h.instrs; i++) {
    fread(&ih,1,29,xm);
    memcpy(song->ins[i+1]->name,ih.name,22);
    printf("ins %d: %d samples. header is %d bytes\n",i,ih.samples[0],ih.size);
    fread(&ich,1,214,xm);
    fseek(xm,-243,SEEK_CUR);
    // convert envelopes
    printf("volenv: %d panenv: %d\n",ich.envVolCount,ich.envPanCount);
    if (ich.volType&1) {
      int point=0;
      Macro* m=new Macro;
      for (int j=0; (point<ich.envVolCount); j++) {
        if (point<1) {
          m->cmds.push_back(MacroCommand(cmdSet,minval(255,ich.envVol[point].val*4),true));
        } else {
          m->cmds.push_back(MacroCommand(cmdSet,minval(255,interpolate(float(ich.envVol[point-1].val),float(ich.envVol[point].val),(float(j-ich.envVol[point-1].pos)/float(ich.envVol[point].pos-ich.envVol[point-1].pos)))*4),true));
        }
        if (j>=ich.envVol[point].pos) {
          if (point==ich.volSus && ich.volType&2) m->cmds.push_back(MacroCommand(cmdWaitRel,0,false));
          point++;
        }
      }
      song->ins[i]->volMacro=song->macros.size();
      song->macros.push_back(m);
      printf("volsus %d\n",ich.volSus);
    }
    // load samples (only the first one is loaded)
    fseek(xm,ih.size,SEEK_CUR);
    for (int j=0; j<ih.samples[0]; j++) {
      printf("%lx\n",ftell(xm));
      fread(&sh,1,40,xm);
      if (sh.flags&16) {
        printf("16-bit sample!\n");
        sh.loopStart>>=1;
        sh.loopSize>>=1;
        sh.size>>=1;
      }
      printf("sample %d: %d %d %d vol %d fine %d\n",j,sh.size,sh.loopStart,sh.loopSize,sh.vol,sh.pitch);
      if (j==0 && sampleSeek<SOUNDCHIP_PCM_SIZE) {
        song->ins[i+1]->pcmPos=sampleSeek;
        if (sh.flags&3) {
          song->ins[i+1]->pcmLen=minval(sh.size,sh.loopStart+sh.loopSize);
        } else {
          song->ins[i+1]->pcmLen=sh.size;
        }
        song->ins[i+1]->pcmLoop=sh.loopStart;
        delta=0;
        for (int k=0; k<sh.size; k++) {
          if (sh.flags&16) {
            delta+=fgetsh(xm);
            if (sampleSeek<SOUNDCHIP_PCM_SIZE) {
              chip[0].pcm[sampleSeek]=delta>>8;
              chip[1].pcm[sampleSeek]=delta>>8;
              chip[2].pcm[sampleSeek]=delta>>8;
              chip[3].pcm[sampleSeek]=delta>>8;
            }
          } else {
            delta+=(signed char)(fgetc(xm));
            if (sampleSeek<SOUNDCHIP_PCM_SIZE) {
              chip[0].pcm[sampleSeek]=delta;
              chip[1].pcm[sampleSeek]=delta;
              chip[2].pcm[sampleSeek]=delta;
              chip[3].pcm[sampleSeek]=delta;
            }
          }
          sampleSeek++;
          if (sampleSeek>=SOUNDCHIP_PCM_SIZE) {
            popbox=PopupBox("Warning","out of PCM memory to load all samples!");
          }
        }
        if (sh.flags&3) {
          song->ins[i+1]->pcmMult|=128;
        }
        song->ins[i+1]->filterMode|=8;
        song->ins[i+1]->noteOffset=sh.note+24;
      } else {
        fseek(xm,(sh.flags&16)?(sh.size*2):(sh.size),SEEK_CUR);
      }
    }
  }
  
  fclose(xm);
  
  if (!playermode && !fileswitch) {player.pat=0;}
  if (name=="") {
    SDL_SetWindowTitle(sdlWin,PROGRAM_NAME);
  } else {
    SDL_SetWindowTitle(sdlWin,(name+S(" - ")+S(PROGRAM_NAME)).c_str());
  }
  return 0;
}

#ifdef _WIN32
int print_entry(const char* filepath) {
  HANDLE flist;
  WIN32_FIND_DATAW next;
  string actualfp;
  FileInList neext;
  int increment=0;
  if (strcmp(filepath,"")==0) {
    unsigned int drivemask;
    drivemask=GetLogicalDrives();
    filenames.clear();
    for (int i=0; i<26; i++) {
      if ((drivemask>>i)&1) {
        neext.name="";
        neext.name+='A'+i;
	neext.name+=":\\";
        neext.isdir=true;
        filenames.push_back(neext);
        increment++;
      }
    }
    return true;
  }
  actualfp=filepath;
  actualfp+="\\*";
  flist=FindFirstFileW(utf8To16(actualfp.c_str()).c_str(),&next);
  printf("listing dir...\n");
  neext.name="";
  neext.isdir=false;
  // clean file list
  filenames.clear();
  if (flist!=INVALID_HANDLE_VALUE) {
    do {
      if (wcscmp(next.cFileName,L".")==0) continue;
      if (wcscmp(next.cFileName,L"..")==0) continue;
      neext.name=utf16To8(next.cFileName);
      neext.isdir=next.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY;
      filenames.push_back(neext);
      increment++;
    } while (FindNextFileW(flist,&next));
    FindClose(flist);
  } else {
    return -GetLastError();
  }
  printf("finish.\n");
  return true;
}
#else
int print_entry(const char* filepath) {
  DIR* flist;
  struct dirent* next;
  flist=opendir(filepath);
  // clean file list
  filenames.clear();
  if (flist==NULL) {
    return -errno;
  }
  printf("listing dir...\n");
  int increment=0;
  FileInList neext;
  neext.name="";
  neext.isdir=false;
  
  while ((next=readdir(flist))!=NULL) {
    if (strcmp(next->d_name,".")==0) continue;
    if (strcmp(next->d_name,"..")==0) continue;
    neext.name=next->d_name;
    neext.isdir=next->d_type&DT_DIR;
    filenames.push_back(neext);
    increment++;
  }
  closedir(flist);
  // sort the files
  string tempname;
  bool tempisdir=false;
  tempname="";
  for (size_t ii=0; ii<filenames.size(); ii++) {
    for (size_t j=0; j<filenames.size()-1; j++) {
      if (strcmp(filenames[j].name.c_str(), filenames[j+1].name.c_str()) > 0) {
        tempname=filenames[j].name;
        tempisdir=filenames[j].isdir;
        filenames[j].name=filenames[j+1].name;
        filenames[j].isdir=filenames[j+1].isdir;
        filenames[j+1].name=tempname;
        filenames[j+1].isdir=tempisdir;
      }
    }
  }
  printf("finish.\n");
  return true;
}
#endif

int SaveFile() {
  // save file
  FILE *sfile;
  //printf("\nplease write filename? ");
  //char rfn[256];
  int sk=0;
  int maskdata=0; // mask byte
  int CPL=0; // current pattern packlength
  int insparas[256];
  int patparas[256];
  int macroparas[8192];
  int commentpointer=0;
  int pcmpointer=0;
  bool IS_INS_BLANK[256];
  bool IS_PAT_BLANK[256];
  char rfn[4096];
  int oldseek=0;
  
  // temporary, gonna get replaced by a better thing soon
  // just for the sake of linux
  strcpy(rfn,(S(curdir)+S(SDIR_SEPARATOR)+curfname).c_str());
  
  sfile=ps_fopen(rfn,"wb");
  if (sfile!=NULL) { // write the file
    fseek(sfile,0,SEEK_SET); // seek to 0

    printf("writing header...\n");
    song->version=TRACKER_VER;
    song->macrosC=song->macros.size();
    fwrite(song,1,384,sfile); // write header

    printf("writing instruments...\n");
    sk=384+256*4+256*4+song->macrosC*4;
    fseek(sfile,sk,SEEK_SET); // start writing the instruments
    for (int ii=0; ii<256; ii++) {
      IS_INS_BLANK[ii]=true;
      // check if the instrument is blank
      if (memcmp(song->ins[ii],&blankIns,96)!=0) {IS_INS_BLANK[ii]=false;}
      if (IS_INS_BLANK[ii]) {
        insparas[ii]=0;continue;
      }
      insparas[ii]=ftell(sfile);
      fwrite(song->ins[ii],1,96,sfile);
    }

    printf("writing macros...\n");
    for (int i=0; i<song->macrosC; i++) {
      Macro* m=song->macros[i];
      unsigned int len=m->cmds.size();
      macroparas[i]=ftell(sfile);
      fwrite(&len,4,1,sfile); // write length
      fwrite(&m->jumpRelease,4,1,sfile); // write jumpRelease
      fwrite("\0\0\0\0\0\0\0\0",1,8,sfile); // reserved
      for (MacroCommand& j: m->cmds) {
        fputc(j.type,sfile);
        fwrite(&j.value,4,1,sfile);
      }
    }

    printf("writing patterns...\n");
    // pattern packer
    for (int ii=0; ii<256; ii++) {
      Pattern* p=song->getPattern(ii,false);
      IS_PAT_BLANK[ii]=true;
      for (int ii1=0; ii1<256; ii1++) {
        for (int ii2=0; ii2<32; ii2++) {
          for (int ii3=0; ii3<5; ii3++) {
            if (p->data[ii1][ii2][ii3]!=0) {IS_PAT_BLANK[ii]=false;break;}
          }
          if (!IS_PAT_BLANK[ii]) {break;}
        }
        if (!IS_PAT_BLANK[ii]) {break;}
      }
      if (IS_PAT_BLANK[ii]) {
        patparas[ii]=0;continue;
      }
      CPL=0; // reset the size
      sk=ftell(sfile);
      patparas[ii]=sk;
      sk+=5; // increase the seek
      fseek(sfile,sk,SEEK_SET);
      fputc(p->length,sfile);
      sk+=11;
      fseek(sfile,sk,SEEK_SET);
      for (int jj=0; jj<p->length; jj++) {
        // pack row
        for (int cpack=0; cpack<32; cpack++) {
          // first check if channel is used
          if (p->data[jj][cpack][0]==0 &&
            p->data[jj][cpack][1]==0 &&
            p->data[jj][cpack][2]==0 &&
            p->data[jj][cpack][3]==0 &&
            p->data[jj][cpack][4]==0) {continue;} // channel is unused
          // then pack this channel
          maskdata=cpack; // set maskbyte to channel number
          if (p->data[jj][cpack][0]!=0 || p->data[jj][cpack][1]!=0) {maskdata=maskdata|32;} // note AND/OR instrument
          if (p->data[jj][cpack][2]!=0) {maskdata=maskdata|64;} // volume value
          if (p->data[jj][cpack][3]!=0 || p->data[jj][cpack][4]!=0) {maskdata=maskdata|128;} // effect AND/OR effect value
          fputc(maskdata,sfile); CPL++; // write maskbyte
          if (maskdata&32) {fputc(p->data[jj][cpack][0],sfile);fputc(p->data[jj][cpack][1],sfile); CPL+=2;} // write NOTE and INSTRUMENT if required
          if (maskdata&64) {fputc(p->data[jj][cpack][2],sfile); CPL++;} // write VOLUME if required
          if (maskdata&128) {fputc(p->data[jj][cpack][3],sfile);fputc(p->data[jj][cpack][4],sfile); CPL+=2;} // write EFFECT and EFFECT VALUE if required
        }
        fputc(0,sfile); // write end of row
        CPL++;
      }
      oldseek=ftell(sfile);
      fseek(sfile,patparas[ii]+1,SEEK_SET);
      fwrite(&CPL,4,1,sfile);
      fseek(sfile,oldseek,SEEK_SET);
    }
    // write comments
    commentpointer=ftell(sfile);
    fwrite(comments.c_str(),1,comments.size(),sfile);
    fseek(sfile,comments.size()+2,SEEK_CUR);
    // write PCM data
    pcmpointer=ftell(sfile);
    bool IS_PCM_DATA_BLANK=true;
    int maxpcmwrite=0;
    for (int ii=0;ii<SOUNDCHIP_PCM_SIZE;ii++) {
      if (chip[0].pcm[ii]!=0) {IS_PCM_DATA_BLANK=false; maxpcmwrite=ii;}
    }
    if (!IS_PCM_DATA_BLANK) {
      fwrite(&maxpcmwrite,4,1,sfile);
      fwrite(chip[0].pcm,1,maxpcmwrite,sfile);
    } else {pcmpointer=0;}
    // write pointers
    printf("writing offsets...\n");
    fseek(sfile,0x180,SEEK_SET);
    fwrite(insparas,4,256,sfile);
    fwrite(macroparas,4,song->macrosC,sfile);
    fwrite(patparas,4,256,sfile);

    fseek(sfile,0x3a,SEEK_SET);
    fwrite(&commentpointer,4,1,sfile);
    
    fseek(sfile,0x36,SEEK_SET);
    fwrite(&pcmpointer,4,1,sfile);
    
    fclose(sfile);
    printf("done\n");
    print_entry(curdir);
    return 0;
  }
  return 1;
}

// TODO: this
int volOldToNew(int orig) {
  if (orig==0) return 0;
  if (orig>=0x40 && orig<0xc0) {
    return orig;
  }
  switch ((orig-1)/10) {
    case 0: return orig-1; break; // 1-10
    case 1: return 0x10+orig-11; break; // 11-20
    case 2: return 0x20+orig-21; break; // 21-30
    case 3: return 0x30+orig-31; break; // 31-40
    case 4: return 0xc0+orig-41; break; // 41-50
    case 5: return 0xd0+orig-51; break; // 51-60
  }
  switch ((orig-193)/10) {
    case 0: return 0xe0+orig-193; break; // 193-202
    case 1: return 0xf0+orig-203; break; // 203-212
    case 2: return 0x1e0+orig-213; break; // 213-222
  }
  return 0;
}

int getFormat(FILE* sfile) {
  char ident[32];
  ClassicMODHeader cmh;
  
  fseek(sfile,0,SEEK_SET);
  fread(ident,1,32,sfile);
  if (memcmp(ident,"TRACK8BT",8)==0) return FormatTRACK;
  if (memcmp(ident,"TRACKINS",8)==0) return FormatTRACKINS;
  if (memcmp(ident,"IMPM",4)==0) return FormatIT;
  if (memcmp(ident,"Extended Module:",16)==0) return FormatXM;
  if (memcmp(ident,"RIFF",4)==0) return FormatAudio;
  if (memcmp(ident,"FORM",4)==0) return FormatAudio;
  
  // S3M check
  fseek(sfile,0x2c,SEEK_SET);
  fread(ident,1,4,sfile);
  if (memcmp(ident,"SCRM",4)==0) return FormatS3M;
  
  // MOD check
  fseek(sfile,1080,SEEK_SET);
  fread(ident,1,4,sfile);
  if (memcmp(ident,"M.K.",4)==0) return FormatMOD;
  if (memcmp(ident,"M!K!",4)==0) return FormatMOD;
  if (memcmp(ident,"M&K!",4)==0) return FormatMOD;
  if (memcmp(ident,"FLT",3)==0) return FormatMOD;
  if (memcmp(ident,"TDZ",3)==0) return FormatMOD;
  if (memcmp(ident,"OKTA",4)==0) return FormatMOD;
  if (memcmp(ident,"OCTA",4)==0) return FormatMOD;
  if (memcmp(ident,"CD81",4)==0) return FormatMOD;
  if (memcmp(&ident[1],"CHN",3)==0) return FormatMOD;
  if (memcmp(&ident[2],"CH",2)==0) return FormatMOD;
  if (memcmp(&ident[2],"CN",2)==0) return FormatMOD;
  
  // Karsten check
  fseek(sfile,0,SEEK_SET);
  if (fread(&cmh,1,sizeof(cmh),sfile)!=sizeof(cmh)) return FormatUnknown;
  // check whether it makes sense
  for (int i=0; i<15; i++) {
    if (cmh.ins[i].pitch!=0) return FormatUnknown;
  }
  
  return FormatMOD;
}

void dumpMacro(Macro* m) {
  printf("---MACRO DUMP---\n");
  printf("- jumpRelease: %d\n",m->jumpRelease);
  printf("- commands:\n");
  int index=0;
  for (MacroCommand& i: m->cmds) {
    if (i.type&128) {
      switch (i.type&127) {
        case cmdEnd:
          printf("  * %.2x: End\n",index);
          break;
        case cmdSet:
          printf("  * %.2x: Set to   %d\n",index,i.value);
          break;
        case cmdWait:
          printf("  * %.2x: Wait for %d ticks\n",index,i.value);
          break;
        case cmdWaitRel:
          printf("  * %.2x: WaitRel\n",index);
          break;
        case cmdLoop:
          printf("  * %.2x: Loop on  %.2x\n",index,i.value);
          break;
        case cmdLoopRel:
          printf("  * %.2x: LoopRel  %.2x\n",index,i.value);
          break;
        case cmdAdd:
          printf("  * %.2x: Add by   %d\n",index,i.value);
          break;
        case cmdSub:
          printf("  * %.2x: Subtract %d\n",index,i.value);
          break;
        default:
          printf("  * %.2x: Unk %.2x %d\n",index,i.type,i.value);
          break;
      }
    } else {
      switch (i.type&127) {
        case cmdEnd:
          printf("  - %.2x: End\n",index);
          break;
        case cmdSet:
          printf("  - %.2x: Set to   %d\n",index,i.value);
          break;
        case cmdWait:
          printf("  - %.2x: Wait for %d ticks\n",index,i.value);
          break;
        case cmdWaitRel:
          printf("  - %.2x: WaitRel\n",index);
          break;
        case cmdLoop:
          printf("  - %.2x: Loop on  %.2x\n",index,i.value);
          break;
        case cmdLoopRel:
          printf("  - %.2x: LoopRel  %.2x\n",index,i.value);
          break;
        case cmdAdd:
          printf("  - %.2x: Add by   %d\n",index,i.value);
          break;
        case cmdSub:
          printf("  - %.2x: Subtract %d\n",index,i.value);
          break;
        default:
          printf("  - %.2x: Unk %.2x %d\n",index,i.type,i.value);
          break;
      }
    }
   
    index++;
  }
}

int LoadFile(const char* filename) {
  // load file
  FILE *sfile;
  int sk=0;
  int CurrentRow=0;
  int NextByte=0;
  int NextChannel=0;
  int votn=0;
  int insparas[256];
  int patparas[256];
  int seqparas[8192];
  int commentpointer=0;
  int pcmpointer=0;
  size_t maxpcmread=0;
  int oplaymode;
  bool IS_SEQ_BLANK[256];
  bool detectChans=false;
  char *checkstr=new char[8];

  sfile=ps_fopen(filename,"rb");
  if (sfile!=NULL) { // LOAD the file
    fseek(sfile,0,SEEK_SET); // seek to 0
    printf("loading file...\n");
    
    switch (getFormat(sfile)) {
      case FormatTRACK:
        break;
      case FormatTRACKINS:
        triggerfx(1);
        popbox=PopupBox("Error","todo");
        return 1;
        break;
      case FormatMOD:
        return ImportMOD(sfile);
        break;
      case FormatS3M:
        return ImportS3M(sfile);
        break;
      case FormatIT:
        return ImportIT(sfile);
        break;
      case FormatXM:
        return ImportXM(sfile);
        break;
      default:
        printf("error: not a compatible file!\n");fclose(sfile);
        triggerfx(1);
        popbox=PopupBox("Error","not a compatible file!");
        return 1;
        break;
    }
    
    // stop the song
    oplaymode=player.playMode;
    CleanupPatterns();

    fseek(sfile,0,SEEK_SET);
    if (fread(song,1,384,sfile)!=384) {
      printf("error: invalid song header!\n");
      fclose(sfile);
      triggerfx(1);
      popbox=PopupBox("Error","invalid song header!");
      return 1;
    }

    printf("module version %d\n",song->version);
    origin=strFormat("soundtracker dev%d\n",song->version);
    if (song->version<60) {printf("-applying filter mode compatibility\n");}
    if (song->version<65) {printf("-applying volume column compatibility\n");}
    if (song->version<106) {printf("-applying loop point fix compatibility\n");}
    if (song->version<143) {printf("-applying old sequence format compatibility\n");}
    if (song->version<144) {printf("-applying endianness compatibility\n");}
    if (song->version<145) {printf("-applying channel pan/vol compatibility\n");}
    if (song->version<146) {printf("-applying no channel count compatibility\n");}
    if (song->version<147) {printf("-applying no song length compatibility\n");}
    if (song->version<148) {printf("-applying instrument volume compatibility\n");}
    if (song->version<150) {
      printf("-applying old volume effects compatibility\n");
      printf("-applying no tempo compatibility\n");
    }
    if (song->version<151) {
      printf("-applying old cutoff curve compatibility\n");
    }
    if (song->version<152) {
      printf("-applying legacy instrument and sequence compatibility\n");
      printf("-applying legacy noise frequency compatibility\n");
    }

    instruments=song->insC; // instruments
    patterns=song->patC; // patterns
    if (song->version<152) {
      seqs=song->flags; // sequences
    } else {
      seqs=song->macrosC;
    }
    if (song->version<150) {
      song->tempo=0; // tempo
    }
    name="";
    for (int i=0; i<32; i++) {
      if (song->name[i]==0) break;
      name+=song->name[i];
    }
    if (song->version<146) {
      detectChans=true;
      song->channels=1;
    }
    if (song->version<147) {
      // detect song length.
      song->orders=255;
      for (int i=255; i>0; i--) {
        if (song->order[i]==0) {
          song->orders=i;
        } else {
          break;
        }
      }
      song->orders--;
    }
    comments=""; // clean comments
    commentpointer=song->commentPtr[0]|(song->commentPtr[1]<<16);
    if (commentpointer!=0) {
      int v;
      fseek(sfile,commentpointer,SEEK_SET);
      while (1) {
        v=fgetc(sfile);
        if (v==0 || v==EOF) break;
        comments+=v;
      }
    }
    memset(chip[0].pcm,0,SOUNDCHIP_PCM_SIZE); // clean PCM memory
    memset(chip[1].pcm,0,SOUNDCHIP_PCM_SIZE); // clean PCM memory
    memset(chip[2].pcm,0,SOUNDCHIP_PCM_SIZE); // clean PCM memory
    memset(chip[3].pcm,0,SOUNDCHIP_PCM_SIZE); // clean PCM memory
    pcmpointer=song->pcmPtr[0]|(song->pcmPtr[1]<<16);
    if (pcmpointer!=0) {
      fseek(sfile,pcmpointer,SEEK_SET);
      fread(&maxpcmread,4,1,sfile);
      if (maxpcmread>SOUNDCHIP_PCM_SIZE) maxpcmread=SOUNDCHIP_PCM_SIZE;
      fread(chip[0].pcm,1,maxpcmread,sfile);
      memcpy(chip[1].pcm,chip[0].pcm,SOUNDCHIP_PCM_SIZE);
      memcpy(chip[2].pcm,chip[0].pcm,SOUNDCHIP_PCM_SIZE);
      memcpy(chip[3].pcm,chip[0].pcm,SOUNDCHIP_PCM_SIZE);
    }

    // version<145 panning
    if (song->version<145) {
      for (int j=0; j<32; j++) {
        song->defaultPan[j]=((j+1)&2)?96:-96;
        song->defaultVol[j]=0x80;
      }
    }
    
    // version<75 mono
    if (song->version<75) {
      for (int j=0; j<32; j++) {
        song->defaultPan[j]=0;
      }
    }

    // version<152 noise range
    if (song->version<152) {
      song->flags=4;
    }

    fseek(sfile,0x180,SEEK_SET);
    for (int ii=0;ii<256;ii++) {
      insparas[ii]=fgeti(sfile);
    }
    if (song->version<152) {
      for (int ii=0;ii<256;ii++) {
        seqparas[ii]=fgeti(sfile);
      }
    } else {
      for (int ii=0;ii<song->macrosC;ii++) {
        seqparas[ii]=fgeti(sfile);
      }
    }
    for (int ii=0;ii<256;ii++) {
      patparas[ii]=fgeti(sfile);
    }

    // MACROS/SEQUENCES //
    short macroMap[8][256];
    memset(macroMap,0,8*256*2);
    // version<152 legacy sequences
    if (song->version<152) {
      unsigned char bytable[8][256][256];
      memset(bytable,0,8*256*256);
      if (song->version<143) { // old sequence format
        for (int ii=0; ii<256; ii++) { // right now this is a full dump... we'll later fix this
          fseek(sfile,seqparas[ii],SEEK_SET);
          // is it blank?
          if (seqparas[ii]==0) {continue;}
          for (int jj=0; jj<8; jj++) {
            for (int kk=0; kk<256; kk++) {
              bytable[jj][ii][kk]=fgetc(sfile); // seqtable
            }
          }
          // version<106 loop point fix conversion
          if (song->version<106) {
            IS_SEQ_BLANK[ii]=true;
            for (int ii1=0; ii1<8; ii1++) {
              for (int ii2=0; ii2<256; ii2++) {
                  if (ii2==254 || ii2==255) {
                    if (bytable[ii1][ii][ii2]!=255) {IS_SEQ_BLANK[ii]=false;break;}
                  } else {
                    if (bytable[ii1][ii][ii2]!=0) {IS_SEQ_BLANK[ii]=false;break;}
                  }
              }
              if (!IS_SEQ_BLANK[ii]) {break;}
            }
            if (!IS_SEQ_BLANK[ii]) {
              if (bytable[0][ii][254]<252) {bytable[0][ii][254]=1;}
              if (bytable[1][ii][254]<252) {bytable[1][ii][254]=1;}
              if (bytable[2][ii][254]<252) {bytable[2][ii][254]=1;}
              if (bytable[3][ii][254]<252) {bytable[3][ii][254]=1;}
              if (bytable[4][ii][254]<252) {bytable[4][ii][254]=1;}
              if (bytable[5][ii][254]<252) {bytable[5][ii][254]=1;}
              if (bytable[6][ii][254]<252) {bytable[6][ii][254]=1;}
            }
          }
        }
      } else {
        for (int ii=0; ii<256; ii++) {
          fseek(sfile,seqparas[ii],SEEK_SET);
          // is it blank?
          if (seqparas[ii]==0) {continue;}
          for (int jj=0; jj<8; jj++) {
            bytable[jj][ii][253]=fgetc(sfile);
            bytable[jj][ii][254]=fgetc(sfile);
            bytable[jj][ii][255]=fgetc(sfile);
            for (int kk=0; kk<bytable[jj][ii][253]+1; kk++) {
              // seqtable
              bytable[jj][ii][kk]=fgetc(sfile);
            }
          }
        }
      }
      // convert sequences into macros
      int macroIndex=1;
      Macro* emptyMacro=new Macro();
      emptyMacro->cmds.push_back(MacroCommand(cmdSet,0,true));
      song->macros.push_back(emptyMacro);
      for (int i=0; i<256; i++) {
        if (seqparas[i]==0) continue;
        for (int j=0; j<8; j++) {
          Macro* m=new Macro();
          for (int k=0; k<bytable[j][i][253]+1; k++) {
            switch (j) {
              case 4: // shape
                m->cmds.push_back(MacroCommand(cmdSet,bytable[j][i][k]>>5,true));
                break;
              default:
                m->cmds.push_back(MacroCommand(cmdSet,bytable[j][i][k],true));
                break;
            }
            if (k==bytable[j][i][255]) {
              if (bytable[j][i][254]<=bytable[j][i][255]) {
                m->cmds.push_back(MacroCommand(cmdLoopRel,bytable[j][i][254],false));
              } else {
                m->cmds.push_back(MacroCommand(cmdWaitRel,0,false));
              }
              m->cmds.push_back(MacroCommand(cmdWait,1,false));
            }
          }
          if (bytable[j][i][254]!=255 && (bytable[j][i][254]>bytable[j][i][255] || bytable[j][i][255]==255)) m->cmds.push_back(MacroCommand(cmdLoop,bytable[j][i][254],false));
          song->macros.push_back(m);
          macroMap[j][i]=macroIndex++;
        }
      }
    } else {
      // version>=152 macros
      for (int i=0; i<song->macrosC; i++) {
        if (seqparas[i]==0) {
          Macro* m=new Macro();
          song->macros.push_back(m);
          continue;
        }
        printf("%d: seek to %x\n",i,seqparas[i]);
        fseek(sfile,seqparas[i],SEEK_SET);
        Macro* m=new Macro();
        unsigned int len=fgeti(sfile);
        m->jumpRelease=fgeti(sfile);
        int reserved1=fgeti(sfile);
        int reserved2=fgeti(sfile);
        reserved1=reserved2; // shut the compiler up
        reserved2=reserved1;
        for (unsigned int j=0; j<len; j++) {
          unsigned char cType=fgetc(sfile);
          unsigned int cValue=fgeti(sfile);
          m->cmds.push_back(MacroCommand(cType&0x7f,cValue,cType&0x80?1:0));
        }
        song->macros.push_back(m);
      }
    }

    // INSTRUMENTS //
    for (int ii=0; ii<256; ii++) {
      fseek(sfile,insparas[ii],SEEK_SET);
      // is it blank?
      if (insparas[ii]==0) {continue;}
      // check for legacy instrument
      if (song->version<152) {
        LegacyInstrument li;
        fread(&li,1,64,sfile);
        // version<60 filter mode fix
        if (song->version<60) {
          if (li.activeEnv&2) {li.DFM^=1;}
        }
        // version<144 endianness
        if (song->version<144) {
          li.pcmLen=bswapu16(li.pcmLen);
          li.pcmPos[0]^=li.pcmPos[1];
          li.pcmPos[1]^=li.pcmPos[0];
          li.pcmPos[0]^=li.pcmPos[1];
          li.pcmLoop[0]^=li.pcmLoop[1];
          li.pcmLoop[1]^=li.pcmLoop[0];
          li.pcmLoop[0]^=li.pcmLoop[1];
          li.filterH=bswapu16(li.filterH);
        }
        // version<148 instrument volume
        if (song->version<148) {
          li.vol=64;
        }
        // version<151 cutoff curve
        if (song->version<151) {
          li.filterH=65535-(unsigned short)(2.0*sin(3.141592653589*(((double)(65535-li.filterH))/2.5)/297500.0)*65535.0);
        }

        // convert legacy instrument
        memcpy(song->ins[ii]->name,li.name,32);
        song->ins[ii]->id=li.id;
        song->ins[ii]->pcmMult=li.pcmMult;
        song->ins[ii]->volMacro=(li.activeEnv&1)?macroMap[0][li.env[0]]:-1;
        song->ins[ii]->cutMacro=(li.activeEnv&2)?macroMap[1][li.env[1]]:-1;
        song->ins[ii]->resMacro=(li.activeEnv&4)?macroMap[2][li.env[2]]:-1;
        song->ins[ii]->dutyMacro=(li.activeEnv&8)?macroMap[3][li.env[3]]:-1;
        song->ins[ii]->shapeMacro=(li.activeEnv&16)?macroMap[4][li.env[4]]:-1;
        song->ins[ii]->pitchMacro=(li.activeEnv&32)?macroMap[5][li.env[5]]:-1;
        song->ins[ii]->finePitchMacro=(li.activeEnv&64)?macroMap[6][li.env[6]]:-1;
        song->ins[ii]->panMacro=(li.activeEnv&128)?macroMap[7][li.env[7]]:-1;
        song->ins[ii]->noteOffset=li.noteOffset;
        song->ins[ii]->FPt=li.FPt;
        song->ins[ii]->FPR=li.FPR;
        song->ins[ii]->filterMode=li.DFM;
        song->ins[ii]->LFO=li.LFO;
        song->ins[ii]->vol=li.vol;
        song->ins[ii]->pitch=li.pitch;
        song->ins[ii]->pcmLen=li.pcmLen;
        song->ins[ii]->filterH=65535-li.filterH;
        song->ins[ii]->res=li.res;
        song->ins[ii]->FTm=li.FTm;
        song->ins[ii]->pcmPos=li.pcmPos[0]|(li.pcmPos[1]<<8);
        song->ins[ii]->pcmLoop=li.pcmLoop[0]|(li.pcmLoop[1]<<8);
        song->ins[ii]->ver=li.ver;
        song->ins[ii]->flags=li.flags;
        song->ins[ii]->RMf=li.RMf;
      } else {
        fread(song->ins[ii],1,96,sfile);
      }
    }

    // PATTERNS //
    for (int pointer=0;pointer<256;pointer++) {
      // is it blank?
      if (patparas[pointer]==0) continue;
      //printf("-unpacking pattern %d-\n",pointer);
      Pattern* p=song->getPattern(pointer,true);
      CurrentRow=0;
      sk=patparas[pointer];
      fseek(sfile,sk+1,SEEK_SET);
      int patsize=fgeti(sfile);
      //printf("%d bytes in pattern\n",patsize);
      p->length=fgetc(sfile);
      if (p->length==0) p->length=256;
      sk=patparas[pointer]+16;
      fseek(sfile,sk,SEEK_SET);
      for (int a=0;a<patsize;a++) {
        NextByte=fgetc(sfile);
        if (NextByte==0) {
          CurrentRow++;
          if (CurrentRow==p->length) {break;}
          continue;
        }
        NextChannel=NextByte%32;
        if (detectChans) {
          if (NextChannel>=song->channels) {
            song->channels=NextChannel+1;
          }
        }
        if ((NextByte>>5)%2) {
          a++;
          p->data[CurrentRow][NextChannel][0]=fgetc(sfile);
          a++;
          p->data[CurrentRow][NextChannel][1]=fgetc(sfile);
        }
        if ((NextByte>>6)%2) {
          a++;
          p->data[CurrentRow][NextChannel][2]=fgetc(sfile);
          // version<65 volume fix
          if (song->version<65) {
            if (p->data[CurrentRow][NextChannel][0]!=0 && p->data[CurrentRow][NextChannel][2]==0x7f) {p->data[CurrentRow][NextChannel][2]=0;}
          }
          // version<150 old volume effects
          if (song->version<150) {
            votn=volOldToNew(p->data[CurrentRow][NextChannel][2]);
            p->data[CurrentRow][NextChannel][2]=votn;
          }
        }
        if ((NextByte>>7)%2) {
          a++;
          p->data[CurrentRow][NextChannel][3]=fgetc(sfile);
          a++;
          p->data[CurrentRow][NextChannel][4]=fgetc(sfile);
        }
        if (song->version<150) {
          p->data[CurrentRow][NextChannel][3]&=0x7f;
          p->data[CurrentRow][NextChannel][3]|=(votn&0x100)>>1;
          votn=0;
        }
      }
    }
    //printf("%d ",ftell(sfile));
    fclose(sfile);
    printf("done\n");
    if (!playermode && !fileswitch) {player.pat=0;}
    if (oplaymode==1) {Play();}
    if (name=="") {
      SDL_SetWindowTitle(sdlWin,PROGRAM_NAME);
    } else {
      SDL_SetWindowTitle(sdlWin,(name+S(" - ")+S(PROGRAM_NAME)).c_str());
    }
    
    return 0;
  } else {
    perror("can't open file");
    popbox=PopupBox("Error","can't open file!");
    #ifdef SOUNDS
    triggerfx(1);
    #endif
    }
    delete[] checkstr;
  return 1;
}
void LoadSample(const char* filename,int position) {
  printf("TODO\n");
}
void LoadRawSample(const char* filename,int position) {
  FILE *sfile;
  int samplelength=0;
  sfile=ps_fopen(filename,"rb");
  samplelength=fsize(sfile);
  printf("%x bytes",samplelength);
  if (samplelength<(SOUNDCHIP_PCM_SIZE-position)) {
    //for (int ii=0;ii<samplelength;ii++) {
      fseek(sfile,position,SEEK_SET);
      fread(chip[0].pcm+position,1,samplelength,sfile);
    //}
  } else {
    printf(" don't fit!");
  }
  fclose(sfile);
}

void triggerfx(int num) {
  if (sfxdata[num]==NULL) return;
  cursfx=num;
  sfxpos=0;
  // if effect was playing, reset channel
  if (sfxplaying) {
    memset(&chip[0].chan[chantoplayfx],0,16);
  }
  chantoplayfx=FreeChannel();
  sfxInst.setChan(chantoplayfx);
  sfxplaying=true;
}

int playfx(const char* fxdata,int fxpos,int achan) {
  // returns number if not in the end, otherwise -1
  int toret=fxpos;
  sfxInst.init(&chip[0]);
  if (!sfxInst.next(fxdata,toret,strlen(fxdata))) {
    return -1;
  }
  
  return toret;
}

/// SEPARATOR

#ifdef HAVE_GUI
double getScale() {
  char* env;
  // try with environment variable
  env=getenv("TRACKER_SCALE");
  if (env!=NULL) {
    return atof(env);
  }
#if defined(ANDROID)
  // android code here
  float dpi;
  if (SDL_GetDisplayDPI(0,&dpi,NULL,NULL)==-1) {
    return 2.0;
  }
  return fmax(1,round(dpi/180));
#elif defined(__linux__)
  // linux (wayland) code here
#elif defined(_WIN32)
  // windows code here
  HDC disp;
  int dpi;
  disp=GetDC(NULL);
  if (disp==NULL) {
    return 1;
  }
  dpi=GetDeviceCaps(disp,LOGPIXELSX);
  ReleaseDC(NULL,disp);
  return (double)dpi/96.0;
#elif defined(__APPLE__)
  // macOS code here
  double dpi;
  if ((dpi=nsStubDPI())>0) {
    return dpi;
  }
#endif
#if defined(__unix__) && !defined(ANDROID)
  // X11
  Display* disp;
  int dpi;
  disp=XOpenDisplay(NULL);
  if (disp!=NULL) {
    dpi=(int)(0.5+(25.4*(double)XDisplayWidth(disp,XDefaultScreen(disp))/(double)XDisplayWidthMM(disp,XDefaultScreen(disp))));
    XCloseDisplay(disp);
    return (double)dpi/96.0;
  }
#endif
  // assume 1
  return 1;
}

enum colType {
  colRowNumber,
  colInstrument,
  colVolume,
  colVolEffect,
  colPanEffect,
  colPitchEffect,
  colSongEffect,
  colSpeedEffect,
  colChanEffect,
  colInvalidEffect,
  colMiscEffect,

  colMax
};

ImVec4 colors[colMax]={
  ImVec4(0.5f,0.5f,0.5f,1.0f),
  ImVec4(0.2f,0.8f,1.0f,1.0f),
  ImVec4(0.2f,0.7f,1.0f,1.0f),
  ImVec4(0.0f,1.0f,0.0f,1.0f),
  ImVec4(0.0f,1.0f,1.0f,1.0f),
  ImVec4(1.0f,1.0f,0.0f,1.0f),
  ImVec4(1.0f,0.0f,0.0f,1.0f),
  ImVec4(1.0f,0.0f,0.0f,1.0f),
  ImVec4(0.5f,0.5f,1.0f,1.0f),
  ImVec4(0.4f,0.4f,0.4f,1.0f),
  ImVec4(1.0f,0.0f,1.0f,1.0f),
};

colType getFXColor(unsigned char fx) {
  switch (fx+0x40) {
    case 'A': case 'T':
      return colSpeedEffect;
      break;
    case 'B': case 'C': case 'V': case 'W':
      return colSongEffect;
      break;
    case 'D': case 'M': case 'N': case 'R':
      return colVolEffect;
      break;
    case 'E': case 'F': case 'G': case 'H': case 'K': case 'L': case 'U':
      return colPitchEffect;
      break;
    case 'I': case 'J': case 'O': case 'Q':
      return colChanEffect;
      break;
    case 'S': case 'Z':
      return colMiscEffect;
      break;
    case 'P': case 'X': case 'Y':
      return colPanEffect;
      break;
  }
  return colInvalidEffect;
}

void drawPatterns(float ypos) {
  ImGui::Begin("Pattern View",NULL,ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoBringToFrontOnFocus);
  ImGui::SetWindowPos("Pattern View",ImVec2(0.0f,ypos));
  ImGui::SetWindowSize("Pattern View",ImVec2(scrW*dpiScale,scrH*dpiScale-ypos),ImGuiCond_Always);

  Pattern* p=song->getPattern(song->order[player.pat],false);
  int playerStep=player.step;
  float lineHeight=(ImGui::GetTextLineHeight()+4*dpiScale);
  if (ImGui::BeginTable("Pattern",song->channels+1,ImGuiTableFlags_BordersInnerV|ImGuiTableFlags_ScrollX|ImGuiTableFlags_ScrollY)) {
    ImGui::SetScrollY(lineHeight*playerStep);
    ImGui::TableSetupColumn("pos",ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupScrollFreeze(1,1);
    for (int i=0; i<song->channels; i++) {
      ImGui::TableSetupColumn(fmt::sprintf("c%d",i).c_str(),ImGuiTableColumnFlags_WidthFixed);
    }
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::Text("%d",song->order[player.pat]);
    for (int i=0; i<song->channels; i++) {
      ImGui::TableNextColumn();
      ImGui::Button(fmt::sprintf("%d",i).c_str());
    }
    // dummy rows
    for (int i=0; i<12; i++) {
      ImGui::TableNextRow(0,lineHeight);
    }
    for (int i=0; i<p->length; i++) {
      ImGui::TableNextRow(0,lineHeight);
      ImGui::TableNextColumn();
      ImGui::TextColored(colors[colRowNumber],"%.2X",i);
      //if (i==playerStep) ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0,0x40ffffff);
      for (int j=0; j<song->channels; j++) {
        ImGui::TableNextColumn();
        if (p->data[i][j][0]==0) {
          ImGui::TextColored(colors[colInvalidEffect],"...");
        } else {
          ImGui::Text("%s%s",getnote(p->data[i][j][0]),getoctave(p->data[i][j][0]));
        }
        ImGui::SameLine(0.0f,0.0f);
        if (p->data[i][j][1]==0) {
          ImGui::TextColored(colors[colInvalidEffect],"..");
        } else {
          ImGui::TextColored(colors[colInstrument],"%.2X",p->data[i][j][1]);
        }
        ImGui::SameLine(0.0f,0.0f);
        if (p->data[i][j][2]==0) {
          ImGui::TextColored(colors[colInvalidEffect],"...");
        } else {
          ImGui::TextColored(colors[colVolume],"%s%.2X",getVFX(p->data[i][j][2]),getVFXVal(p->data[i][j][2]));
        }
        ImGui::SameLine(0.0f,0.0f);
        ImGui::PushStyleColor(ImGuiCol_Text,colors[getFXColor(p->data[i][j][3])]);
        ImGui::Text("%s",getFX(p->data[i][j][3]));
        ImGui::SameLine(0.0f,0.0f);
        if (p->data[i][j][4]==0) {
          ImGui::Text("..");
        } else {
          ImGui::Text("%.2X",p->data[i][j][4]);
        }
        ImGui::PopStyleColor();
      }
    }
    // dummy rows
    for (int i=0; i<20; i++) {
      ImGui::TableNextRow(0,lineHeight);
    }
    ImGui::EndTable();
  }
  ImGui::End();
}

#define rangedInput(label,var,tempvar,min,max) \
  int tempvar=var; \
  if (ImGui::InputInt(label,&tempvar,1,16)) { \
    if (tempvar<min) tempvar=min; \
    if (tempvar>max) tempvar=max; \
    var=tempvar; \
  }

#define macroSelector(label,var) \
  ImGui::PushID(#var); \
  ImGui::Text(label); \
  ImGui::NextColumn(); \
  if (var>-1) { \
    ImGui::Text("%d",var); \
  } else { \
    ImGui::TextColored(ImVec4(0.5f,0.5f,0.5f,1.0f),"OFF"); \
  } \
  ImGui::NextColumn(); \
  ImGui::PushButtonRepeat(true); \
  if (ImGui::ArrowButton("##down",ImGuiDir_Down)) { \
    if (var>-1) var--; \
  } \
  ImGui::SameLine(); \
  if (ImGui::ArrowButton("##up",ImGuiDir_Up)) { \
    if (var<short(song->macros.size()-1)) var++; \
  } \
  ImGui::SameLine(); \
  ImGui::Button("New"); \
  ImGui::SameLine(); \
  ImGui::Button("Go"); \
  ImGui::PopButtonRepeat(); \
  ImGui::NextColumn(); \
  ImGui::PopID();

void drawInsEditor() {
  if (!insEditOpen) return;
  ImGui::Begin("Instrument Editor",&insEditOpen);
  if (ImGui::InputInt("Instrument",&curins)) {
    if (curins<1) curins=1;
    if (curins>255) curins=1;
  }

  Instrument* ins=song->ins[curins];

  ImGui::InputText("Name",ins->name,32);
  rangedInput("Volume",ins->vol,tempVol,0,64);
  rangedInput("Pitch",ins->pitch,tempPitch,-128,127);

  ImGui::Text("Filter Mode");
  ImGui::SameLine();
  ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(0.2f,(ins->filterMode&1)?0.6f:0.2f,0.2f,1.0f));
  if (ImGui::Button("low")) {
    ins->filterMode^=1;
  }
  ImGui::PopStyleColor();
  ImGui::SameLine();
  ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(0.2f,(ins->filterMode&2)?0.6f:0.2f,0.2f,1.0f));
  if (ImGui::Button("high")) {
    ins->filterMode^=2;
  }
  ImGui::PopStyleColor();
  ImGui::SameLine();
  ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(0.2f,(ins->filterMode&4)?0.6f:0.2f,0.2f,1.0f));
  if (ImGui::Button("band")) {
    ins->filterMode^=4;
  }
  ImGui::PopStyleColor();

  ImGui::BeginChild("Macros",ImVec2(0,0),true,ImGuiWindowFlags_MenuBar);
  ImGui::BeginMenuBar();
  ImGui::Text("Macros");
  ImGui::EndMenuBar();
  ImGui::Columns(3);
  macroSelector("Volume",ins->volMacro);
  macroSelector("Cutoff",ins->cutMacro);
  macroSelector("Resonance",ins->resMacro);
  macroSelector("Duty",ins->dutyMacro);
  macroSelector("Shape",ins->shapeMacro);
  macroSelector("Pitch",ins->pitchMacro);
  macroSelector("FinePitch",ins->finePitchMacro);
  macroSelector("Panning",ins->panMacro);
  macroSelector("VolSweep",ins->volSweepMacro);
  macroSelector("FreqSweep",ins->freqSweepMacro);
  macroSelector("CutSweep",ins->cutSweepMacro);
  macroSelector("PCM Position",ins->pcmPosMacro);
  ImGui::EndChild();

  ImGui::End();
}

bool updateDisp() {
  SDL_Event ev;
  while (SDL_PollEvent(&ev)) {
    ImGui_ImplSDL2_ProcessEvent(&ev);
    switch (ev.type) {
      case SDL_WINDOWEVENT:
        switch (ev.window.event) {
          case SDL_WINDOWEVENT_RESIZED:
            printf("I am resized\n");
            scrW=ev.window.data1/dpiScale;
            scrH=ev.window.data2/dpiScale;
            break;
        }
        break;
      case SDL_QUIT:
        return false;
        break;
    }
  }
  ImGui_ImplSDLRenderer_NewFrame();
  ImGui_ImplSDL2_NewFrame(sdlWin);
  ImGui::NewFrame();

  ImGui::BeginMainMenuBar();
  if (ImGui::MenuItem("Help")) {
  }
  ImGui::EndMainMenuBar();

  ImGui::SetNextWindowSizeConstraints(ImVec2(scrW*dpiScale,32*dpiScale),ImVec2(scrW*dpiScale,160*dpiScale));
  ImGui::Begin("Controls",NULL,ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoBringToFrontOnFocus);
  ImGui::SetWindowPos("Controls",ImVec2(0.0f,ImGui::GetTextLineHeight()+ImGui::GetStyle().ItemSpacing.y*2));

  ImGui::Columns(4);
  
  ImGui::BeginChild("EditControls",ImVec2(0,0),true);

  int ONE=1;
  ImGui::Columns(2);
  if (ImGui::InputScalar("speed",ImGuiDataType_U8,&player.speed,&ONE,&ONE)) {
    if (player.speed<1) player.speed=1;
  }
  ImGui::NextColumn();
  if (ImGui::InputScalar("tempo",ImGuiDataType_U8,&player.tempo,&ONE,&ONE)) {
    if (player.tempo<32) player.tempo=32;
  }
  ImGui::NextColumn();
  if (ImGui::InputScalar("order",ImGuiDataType_U8,&player.pat,&ONE,&ONE)) {
    if (player.pat>=song->orders) player.pat=0;
    Play();
  }
  ImGui::NextColumn();
  ImGui::InputScalar("pattern",ImGuiDataType_U8,&(song->order[player.pat]),&ONE,&ONE);
  ImGui::NextColumn();
  if (ImGui::InputScalar("octave",ImGuiDataType_U8,&curoctave,&ONE,&ONE)) {
    if (curoctave>9) curoctave=9;
  }
  ImGui::NextColumn();
  ImGui::InputScalar("length",ImGuiDataType_U8,&song->getPattern(player.pat,true)->length,&ONE,&ONE);
  ImGui::Columns(1);

  ImGui::Separator();

  if (ImGui::BeginCombo("instrument",song->ins[curins]->name)) {
    for (int i=1; i<255; i++) {
      string insName=fmt::sprintf("[%d] %s",i,song->ins[i]->name);
      if (ImGui::Selectable(insName.c_str(),curins==i)) {
        curins=i;
      }
    }
    ImGui::EndCombo();
  }

  ImGui::EndChild();

  ImGui::NextColumn();

  ImGui::BeginChild("PlayControls",ImVec2(0,0),true);
  if (ImGui::Button("Play")) {
    Play();
  }
  ImGui::SameLine();
  if (ImGui::Button("Stop")) {
    player.stop();
  }
  if (ImGui::Checkbox("NTSC",&ntsc)) {
    if (!tempolock) {
      if (ntsc) {
        if (player.tempo==125) {
          player.tempo=150;
        }
      } else {
        if (player.tempo==150) {
          player.tempo=125;
        }
      }
    }
  }
  ImGui::EndChild();

  ImGui::NextColumn();

  ImGui::BeginChild("PlayPos",ImVec2(80*dpiScale,0),true);
  ImGui::Text("%d/%d",player.tick,player.speed);
  ImGui::Text("%d/%d",player.step,song->getPattern(player.pat,false)->length);
  ImGui::Text("%d:%d/%d",song->order[player.pat],player.pat,song->orders);

  ImGui::EndChild();

  float where=ImGui::GetWindowPos().y+ImGui::GetWindowSize().y;

  ImGui::End();

  drawPatterns(where);

  drawInsEditor();

  // end of frame
  ImGui::Render();
  SDL_RenderClear(sdlRend);
  ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
  SDL_RenderPresent(sdlRend);

  return true;
}

bool initGUI() {
  sdlWin=SDL_CreateWindow("soundtracker",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,scrW*dpiScale,scrH*dpiScale,SDL_WINDOW_RESIZABLE|SDL_WINDOW_ALLOW_HIGHDPI);
  if (sdlWin==NULL) {
    printf("error while opening window!\n");
    return false;
  }

  sdlRend=SDL_CreateRenderer(sdlWin,-1,SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC|SDL_RENDERER_TARGETTEXTURE);

  if (sdlRend==NULL) {
    printf("error while opening renderer! %s\n",SDL_GetError());
    return false;
  }

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  ImGui::StyleColorsDark();
  ImGui_ImplSDL2_InitForSDLRenderer(sdlWin);
  ImGui_ImplSDLRenderer_Init(sdlRend);

  ImGui::GetStyle().ScaleAllSizes(dpiScale);

  if (!ImGui::GetIO().Fonts->AddFontFromFileTTF("font.ttf",18*dpiScale)) {
    printf("could not load UI font!\n");
    return false;
  }

  return true;
}
#endif

int main(int argc, char **argv) {
  int filearg=0;
  printf("soundtracker (r%d)\n",ver);
  chip[0].Init();
  chip[1].Init();
  chip[2].Init();
  chip[3].Init();
  player.bindChips(chip);
  framecounter=0;
  playermode=false;
  
  // new variables
#ifdef ANDROID
  iface=UIMobile;
#else
  iface=UIClassic;
#endif
  mobAltView=false;
  pageSelectShow=false;
  noStoragePerm=false;
  mobScroll=0;
  topScroll=0;
  popbox=PopupBox(false);
  
  if (argc>1) {
    // for each argument
    for (int i=1; i<argc; i++) {
      // is it an option?
      if (argv[i][0]=='-') {
        // is it a multi-char option?
        if (argv[i][1]=='-') {
          if (!strcmp(&argv[i][2],"help")) {
            printf("usage: soundtracker [--help] [-o order] [-f song] [-v] [] [song]\n");
            return 0;
          }
        }
        // -n
        if (argv[i][1]=='n') {
          ntsc=true;
        }
        // -o ORDER
        if (argv[i][1]=='o') {
          player.pat=atoi(argv[i+1]);
          i++;
        }
        // -f FILE
        if (argv[i][1]=='f') {
          fileswitch=true;
          filearg=++i;
          playermode=true;
        }
      } else {
        filearg=i;
      }
    }
  }
  if (ntsc) {
    player.tempo=150;
  } else {
    player.tempo=125;
  }
  filessorted.resize(1024);
  filenames.resize(1024);
  if (iface==UIMobile) {
    scrW=360;
    scrH=640;
  } else {
    scrW=1280;
    scrH=800;
  }

  CleanupPatterns();

  printf("initializing SDL\n");
  SDL_Init(SDL_INIT_VIDEO);
   
  curdir=new char[4096];
  memset(curdir,0,4096);
#ifdef ANDROID
  // TODO: find the actual path
  strcpy(curdir,"/storage/emulated/0");
#elif defined(_WIN32)
  GetCurrentDirectory(4095,curdir);
#else
  getcwd(curdir,4095);
#endif
  int peerrno=print_entry(curdir);

#ifdef ANDROID
  if (peerrno<0) {
    popbox=PopupBox("Error","you need to grant Storage permission to this app.");
    noStoragePerm=true;
  }
#else
  if (peerrno<0) {
    popbox=PopupBox("Error","you need to grant Storage permission to this app.");
  }
#endif

  if (!playermode) {
#ifdef HAVE_GUI
    printf("creating display\n");
    dpiScale=getScale();
    if (!initGUI()) {
      printf("couldn't initialize display...\n");
      return 1;
    }
#ifdef ANDROID
    curzoom=dpiScale-1;
    if (curzoom<1) curzoom=1;
#else
    curzoom=dpiScale;
#endif
#else
    printf("GUI not available!\n");
    return 1;
#endif
  }

  printf("initializing audio channels\n");
  initaudio();
  // clear to black
  if (playermode || fileswitch) {
    if (LoadFile(argv[filearg])) return 1;
      if (playermode) {
        Play();
        printf("playing: %s\n",name.c_str());
      }
  } else {
    if (filearg!=0) {
      if (LoadFile(argv[filearg])) return 1;
    }
  }
  printf("run\n");
#ifdef ANDROID
  if (noStoragePerm) triggerfx(1);
#endif

#ifndef JACK
  SDL_PauseAudioDevice(audioID,0);
#endif
  printf("done\n");
  // MAIN LOOP
  if (playermode) {
    while (true) usleep(100000);
  } else {
#ifdef HAVE_GUI
    while (updateDisp());
    quit=true;
#else
    while (true) usleep(100000);
#endif
  }
  printf("destroying audio system\n");
#ifdef JACK
  jack_deactivate(jclient);
#endif
  printf("destroying display\n");
  // TODO
  printf("finished\n");
  return 0;
}
