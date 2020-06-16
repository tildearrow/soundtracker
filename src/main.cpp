//>[[[[[[[[[[[[[[[[<~>]]]]]]]]]]]]]]]]<//
//+++++++++++++++++++++++++++++++++++++//
//     sound tracker for chiptunes     //
//+++++++++++++++++++++++++++++++++++++//
// written by tildearrow in C++ during //
//             2014-2015.              //
//>++++++++++-+-++-*.*-++-+-++++++++++<//

// add 2016, 2017, 2018, 2019 and 2020 to the list.

#define PROGRAM_NAME "soundtracker"
float rt1=0;
float rt2=0;

float rt3=0;
float rt4=0;
double time1=0;
double time2=0;

//// DEFINITIONS ////
#define minval(a,b) (((a)<(b))?(a):(b)) // for Linux compatibility
#define maxval(a,b) (((a)>(b))?(a):(b)) // for Linux compatibility
#define sign(a) ((a>0)?(1):((a<0)?(-1):(0)))
//#define MOUSE_GRID
//#define NTSC // define for NTSC mode
#define SOUNDS
int dpiScale;
//#define PRESERVE_INS

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

#include "soundchip.h"
#include "blip_buf.h"
soundchip chip[4]; // up to 4 soundchips

blip_buffer_t* bb[2];
int prevSample[2]={0,0};

short bbOut[2][32768];

const bool verbose=false; // change this to turn on verbose mode
signed char songdf=0;
double FPS=50;
int tempo;

int doframe;
unsigned char colorof[6]={0x00, 0x5f, 0x87, 0xaf, 0xd7, 0xff};
// init sound stuff
unsigned int cfreq[32]={1,1,1,1,1,1,1,1,
         1,1,1,1,1,1,1,1,
               1,1,1,1,1,1,1,1,
               1,1,1,1,1,1,1,1};
short cvol[32]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
short chanvol[32]={128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128};
unsigned char cshape[32]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
unsigned char cshapeprev[32]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
unsigned char cduty[32]={31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,
               31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31};
int crstep[32]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int crrmstep[32]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int coff[32]={262144,262144,262144,262144,262144,262144,262144,
              262144,262144,262144,262144,262144,262144,262144,
              262144,262144,262144,262144,262144,262144,262144,
              262144,262144,262144,262144,262144,262144,262144,
        262144,262144,262144,262144};
unsigned char creso[32]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
unsigned char cfmode[32]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
unsigned char cmode[32]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
signed char cpan[32]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
bool cfsweep[32]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
double cpcmpos[32]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
unsigned short crmfreq[32]={100,1,1,1,1,1,1,1,
         1,1,1,1,1,1,1,1,
               1,1,1,1,1,1,1,1,
               1,1,1,1,1,1,1,1};
unsigned char crmshape[32]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
signed char crm[32]={0,0,0,0,0,0,0,0,
         0,0,0,0,0,0,0,0,
               0,0,0,0,0,0,0,0,
               0,0,0,0,0,0,0,0};
unsigned char crmduty[32]={31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,
               31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31};
int cpcmstart[32]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int cbound[32]={131071,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
bool cloop[32]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int cglobvol=128;
bool AlreadySkipped=false;
enum filters {
  fNone, fLowPass, fHighPass, fNotch, fBandPass, fLowBand, fHighBand, fAllPass
};
float cseek[32]={};
float crseek[32]={};
int rngpgv[32]={0};
int scrollpos=0;
int valuewidth=4;
int oldpat=-1;
unsigned char CurrentIns=1;
unsigned char CurrentEnv=0;
// init tracker stuff
int pattern=0;
unsigned char patid[256]={};
unsigned char* patlength;
string tempInsName;
struct Instrument {
  char name[32];
  unsigned char id, pcmMult, activeEnv;
  unsigned char env[8];
  unsigned char noteOffset;
  unsigned char FPt, FPR, DFM, LFO;
  unsigned char vol, pitch;
  unsigned short pcmLen, filterH;
  unsigned char res;
  unsigned char pcmPos[2]; // alignment
  unsigned char pcmLoop[2];
  unsigned char FTm;
  unsigned short ver;
  unsigned char flags, RMf;
};
Instrument instrument[256]; // instrument[id][position]
Instrument blankIns;
unsigned char bytable[8][256][256]={}; // bytable[table][indextab][position]
unsigned char pat[256][256][32][5]={}; // pat[patid][patpos][channel][bytepos]
int scroll[32][7]={}; // scroll[channel][envelope]
unsigned char songlength=255;
int instruments=0;
int patterns=0;
int seqs=255;
signed char chpan[32]={};
signed char chvol[32]={};
bool muted[32]={};
bool leftpress=false;
bool rightpress=false;
bool leftrelease=false;
bool rightrelease=false;
bool hexmode=false;
bool quit=false;
// channel memory stuff
unsigned char Msmp[32]={};
int Mvol[32]={};
int Mport[32]={};
int Mvib[32]={};
int Mtrem[32]={};
int Marp[32]={};
int Mins[32]={};
bool EnvelopesRunning[32][8]={}; // EnvelopesRunning[channel][envelope]

string name; // song name
unsigned char defspeed=6; // default song speed
unsigned char speed=6; // current speed
signed char playmode=0; // playmode (-1: reverse, 0: stopped, 1: playing, 2: paused)
int curstep=0; // current step
unsigned char curpat=0; // current pattern
int curtick=0; // current tick
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
int curpatrow=0;
int chanstodisplay=8;
int maxCTD=8;
double patseek=0;
int screen=0; // 0=patterns, 1=instruments 2=diskop 3=song 4=mixer 5=config 6=help 7=about
float diskopscrollpos=0;
bool kb[256]={false};
bool kblast[256]={false};
bool kbpressed[256]={false};
bool IRP[32][8]={}; // in release point
bool released[32]={}; // note has been released
bool edittype=true; // 0=ModPlug 1=FT2
int ins[32]={}; // current playing instrument on each channel
int insnote[32]={}; // current playing note on a channel
int insvol[32]={}; // current volume on a channel
bool insfading[32]={}; // checks if the instrument is fading
int insrelease[32]={}; // checks if the instrument is on its release state
int inspos[32][8]={}; // current instrument position per envelope, inspos[chan][env]
int curvol[32]={}; // current volume of a step
int curchanvol[32]={}; // current volume of a channel
float curnote[32]={}; // current note of a channel
int curvibdepth[32]={}; // current vibrato offset
int curvibspeed[32]={}; // current vibrato speed
int curvibpos[32]={};
int curvibshape[32]={};
int curtrmdepth[32]={}; // current tremolo offset
int curtrmspeed[32]={}; // current tremolo speed
int curtrmpos[32]={};
int curtrmshape[32]={};
int curpandepth[32]={}; // current panbrello offset
int curpanspeed[32]={}; // current panbrello speed
int curpanpos[32]={};
int curpanshape[32]={};
int slidememory[32]={}; // current Dxx value
int chanslidememory[32]={}; // current Nxx value
int globslidememory[32]={}; // current Wxx value
int tcmdmemory[32]={}; // current Zxx value
int curhandle[32]={}; // current Zxx handle number
int defshape[32]={0,0,0,0,0,0,0,0,
                  0,0,0,0,0,0,0,0,
                  0,0,0,0,0,0,0,0,
                  0,0,0,0,0,0,0,0}; // default shape per channel (Zc8-Zcf)
int cutcount[32]={0,0,0,0,0,0,0,0,
                  0,0,0,0,0,0,0,0,
                  0,0,0,0,0,0,0,0,
                  0,0,0,0,0,0,0,0}; // SCx ticks to cut
int retrigger[32]={0,0,0,0,0,0,0,0,
                  0,0,0,0,0,0,0,0,
                  0,0,0,0,0,0,0,0,
                  0,0,0,0,0,0,0,0}; // Qxx memory
int retrig[32]={0,0,0,0,0,0,0,0,
                  0,0,0,0,0,0,0,0,
                  0,0,0,0,0,0,0,0,
                  0,0,0,0,0,0,0,0}; // Qxx processor
int plpos[32]={0,0,0,0,0,0,0,0,
                  0,0,0,0,0,0,0,0,
                  0,0,0,0,0,0,0,0,
                  0,0,0,0,0,0,0,0}; // SBx processor - position
int plcount[32]={0,0,0,0,0,0,0,0,
                  0,0,0,0,0,0,0,0,
                  0,0,0,0,0,0,0,0,
                  0,0,0,0,0,0,0,0}; // SBx processor - count
signed char chanpan[32]={0,0,0,0,0,0,0,0,
                  0,0,0,0,0,0,0,0,
                  0,0,0,0,0,0,0,0,
                  0,0,0,0,0,0,0,0}; // channel panning
int linex1=0;
int liney1=0;
int finepitch[32]={0,0,0,0,0,0,0,0,
                  0,0,0,0,0,0,0,0,
                  0,0,0,0,0,0,0,0,
                  0,0,0,0,0,0,0,0}; // fine pitch envelope value
int finedelay=0;
unsigned char defchanvol[32]={128,128,128,128,128,128,128,128,
                  128,128,128,128,128,128,128,128,
                  128,128,128,128,128,128,128,128,
                  128,128,128,128,128,128,128,128}; // default channel volume
signed char defchanpan[32]={0,0,0,0,0,0,0,0,
                  0,0,0,0,0,0,0,0,
                  0,0,0,0,0,0,0,0,
                  0,0,0,0,0,0,0,0}; // default channel panning
bool doretrigger[32]={0,0,0,0,0,0,0,0,
                  0,0,0,0,0,0,0,0,
                  0,0,0,0,0,0,0,0,
                  0,0,0,0,0,0,0,0};
bool tickstart=false;
bool linearslides=true;
bool playermode=false;
bool fileswitch=false;
bool reversemode=false;
unsigned char nvolu[32]={}; // next volume value
unsigned char nfxid[32]={}; // next effect
unsigned char nfxvl[32]={}; // next effect value
int portastatic[32]={}; // current note
int sfxpos=-1; // sound effect position
const char* sfxdata[32]={
  // pause
  "$x!O6V7fEM1v03e00100RRRRRR"
  "V7fCRRRRRR"
  "V7fERRRRRR"
  "V7fCRRRRRRRRRRRRRRRRRRRRR!",
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

const int sine[256]={
        0,  2,  3,  5,  6,  8,  9, 11, 12, 14, 16, 17, 19, 20, 22, 23,
                24, 26, 27, 29, 30, 32, 33, 34, 36, 37, 38, 39, 41, 42, 43, 44,
                45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 56, 57, 58, 59,
                59, 60, 60, 61, 61, 62, 62, 62, 63, 63, 63, 64, 64, 64, 64, 64,
                64, 64, 64, 64, 64, 64, 63, 63, 63, 62, 62, 62, 61, 61, 60, 60,
                59, 59, 58, 57, 56, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46,
                45, 44, 43, 42, 41, 39, 38, 37, 36, 34, 33, 32, 30, 29, 27, 26,
                24, 23, 22, 20, 19, 17, 16, 14, 12, 11,  9,  8,  6,  5,  3,  2,
                 0, -2, -3, -5, -6, -8, -9,-11,-12,-14,-16,-17,-19,-20,-22,-23,
               -24,-26,-27,-29,-30,-32,-33,-34,-36,-37,-38,-39,-41,-42,-43,-44,
               -45,-46,-47,-48,-49,-50,-51,-52,-53,-54,-55,-56,-56,-57,-58,-59,
               -59,-60,-60,-61,-61,-62,-62,-62,-63,-63,-63,-64,-64,-64,-64,-64,
               -64,-64,-64,-64,-64,-64,-63,-63,-63,-62,-62,-62,-61,-61,-60,-60,
               -59,-59,-58,-57,-56,-56,-55,-54,-53,-52,-51,-50,-49,-48,-47,-46,
               -45,-44,-43,-42,-41,-39,-38,-37,-36,-34,-33,-32,-30,-29,-27,-26,
               -24,-23,-22,-20,-19,-17,-16,-14,-12,-11, -9, -8, -6, -5, -3, -2
}; // taken directly from Jeffrey Lim's fine sine table
const char HEXVALS[17]="0123456789ABCDEF"; // 17 for the null
const int modeOff[6]={
  24,48,64,88,96,112
};

SSInter sfxInst;

bool sfxplaying=false;

namespace ASC {
  int interval=119000;
  int currentclock=0;
}
namespace fakeASC {
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

struct {
  int x, y, z, buttons;
} mstate;
Texture logo;
bool leftclick=false;
bool leftclickprev=false;
bool rightclick=false;  
bool rightclickprev=false;
int prevZ=0;
int channels=8;
int hover[16]={}; // hover time per button
int16_t ver=149; // version number
unsigned char chs0[5000];
char* helptext;
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
float *buf;
int pitch = 0x20;
int val = 0;
int i;
int sr;
double targetSR;
double noProc;
double procPos;

float nsL[33];
float nsR[33];
float ns[33];

double raster1, raster2, maxrasterdelta;

#define mufflef 0.08
#define mufflefb 0
float muffleb0[2]={0,0};
float muffleb1[2]={0,0};
int selectedfileindex=-1;
struct FileInList {
  string name;
  bool isdir;
};
std::vector<FileInList> filenames;
std::vector<FileInList> filessorted;
int scrW,scrH;
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
  int buffersize=5950;
  // importer settings
  bool names=true;
  bool split=false;
  bool samples=true;
  // filter settings
  bool nofilters=false;
  bool muffle=true;
}

// NEW VARIABLES BEGIN //
Graphics g;

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

const int pageMap[]={
  0, 1, 9, 3, 2, 10, 4, 5, 12, 7
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

// NEW VARIABLES END //

void Playback();
void MuteAllChannels();
int playfx(const char* fxdata,int fxpos,int achan);
void triggerfx(int num);
#define interpolatee(aa,bb,cc) (aa+((bb-aa)*cc))

bool reservedevent[32]={0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,
       0,0,0,0,0,0,0,0,
       0,0,0,0,0,0,0,0};
int newperiod[32]={0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,
       0,0,0,0,0,0,0,0,
       0,0,0,0,0,0,0,0};
int oldperiod[32]={0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,
       0,0,0,0,0,0,0,0,
       0,0,0,0,0,0,0,0};
bool offinstead[32]={0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,
       0,0,0,0,0,0,0,0,
       0,0,0,0,0,0,0,0};
bool midion[32]={0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,
       0,0,0,0,0,0,0,0,
       0,0,0,0,0,0,0,0};

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
    ASC::interval=(int)(6180000/FPS);
    if (tempo==150) ASC::interval=103103;
    targetSR=309000;
    noProc=sr/targetSR;
  } else {
    ASC::interval=(int)(5950000/FPS);
    targetSR=297500;
    noProc=sr/targetSR;
  }
  blip_set_rates(bb[0],targetSR,jacksr);
  blip_set_rates(bb[1],targetSR,jacksr);
  if (kb[SDL_SCANCODE_ESCAPE] || (PIR((scrW/2)+21,37,(scrW/2)+61,48,mstate.x,mstate.y) && leftclick && iface!=UIMobile)) {
    ASC::interval=16384;
  }
  
  // high quality rewrite
  int runtotal=blip_clocks_needed(bb[0],nframes);
  
  for (int i=0; i<runtotal; i++) {
    ASC::currentclock-=20; // 20 CPU cycles per sound output cycle
    if (ASC::currentclock<=0) {
      for (int ii=0;ii<32;ii++) {
        cshapeprev[ii]=cshape[ii];
      }
      if (playmode>0) {
        Playback();
      } else {
         MuteAllChannels();
      }
      if (sfxplaying) {
        sfxpos=playfx(sfxdata[cursfx],sfxpos,chantoplayfx);
        if (sfxpos==-1) {
          sfxplaying=false;
        }
      }
      for (int updateindex1=0;updateindex1<32;updateindex1++) {
        if (muted[updateindex1]) {
          cvol[updateindex1]=0;
          if (updateindex1<(8*(1+((channels-1)>>3)))) {
            chip[updateindex1>>3].chan[updateindex1&7].vol=0;
          }
        }
      }
      ASC::currentclock+=ASC::interval;
    }
    temp[0]=0; temp[1]=0;
    for (int j=0; j<(1+((channels-1)>>3)); j++) {
      chip[j].NextSample(&stemp[0],&stemp[1]);
      temp[0]+=stemp[0]; temp[1]+=stemp[1];
    }
    blip_add_delta(bb[0],i,(short)(temp[0]-prevSample[0]));
    blip_add_delta(bb[1],i,(short)(temp[1]-prevSample[1]));
    prevSample[0]=temp[0];
    prevSample[1]=temp[1];
  }
  
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

Color getucol(unsigned char thecol) {
  if (thecol<16) {
    if (thecol==8) {return (g._WRAP_map_rgb(128,128,128));} else {
      if (thecol<8) {g._WRAP_map_rgb((thecol&1)*192,((thecol&2)>>1)*192,((thecol&3)>>2)*192);}
      else {g._WRAP_map_rgb((thecol&1)*256,((thecol&2)>>1)*256,((thecol&3)>>2)*256);}
    }} else {
  if (thecol>231) {
    // shade of gray/grey
    return (g._WRAP_map_rgb(8+(10*(thecol-232)),8+(10*(thecol-232)),8+(10*(thecol-232))));
  }
  else {
    return (g._WRAP_map_rgb(colorof[((thecol-16)/36)],colorof[((thecol-16)/6)%6],colorof[(thecol-16)%6]));
  }}
  return g._WRAP_map_rgb(255,255,255);
}
unsigned char GetFXColor(unsigned char fxval) {
  switch (fxval) {
  case 1: case 20: return 164; break; // speed control
  case 2: case 3: case 22: case 23: return 9; break; // song control
  case 4: case 13: case 14: case 18: return 10; break; // volume control
  case 5: case 6: case 7: case 8: case 11: case 12: case 21: return 11; break; // pitch control
  case 9: case 10: case 15: case 17: return 63; break; // note control
  case 19: case 26: return 13; break; // special commands
  case 16: case 24: case 25: return 14; break; // panning commands
  default: return 8; break; // unknown commands
  }
}
int getpatlen(int thelen) {
  if (patlength[thelen]==0) {return 256;}
  return patlength[thelen];
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
  if (fxval<128 && fxval>63) {return "v";} // 64-127
  if (fxval<193 && fxval>127) {return "p";} // 128-192
  switch((fxval-1)/10) {
  case 0: return "a"; break; // 1-10
  case 1: return "b"; break; // 11-20
  case 2: return "c"; break; // 21-30
  case 3: return "d"; break; // 31-40
  case 4: return "e"; break; // 41-50
  case 5: return "f"; break; // 51-60
  }
  switch((fxval-193)/10) {
  case 0: return "g"; break; // 193-202
  case 1: return "h"; break; // 203-212
  case 2: return "o"; break; // 213-222
  }
  return "?";
}
unsigned char getVFXval(int nval) {
  if (nval==0) {return 255;} // 0
  if (nval<128 && nval>63) {return nval-64;} // 64-127
  if (nval<193 && nval>127) {return nval-128;} // 128-192
  switch((nval-1)/10) {
  case 0: return nval-1; break; // 1-10
  case 1: return nval-11; break; // 11-20
  case 2: return nval-21; break; // 21-30
  case 3: return nval-31; break; // 31-40
  case 4: return nval-41; break; // 41-50
  case 5: return nval-51; break; // 51-60
  }
  switch((nval-193)/10) {
  case 0: return nval-193; break; // 193-202
  case 1: return nval-203; break; // 203-212
  case 2: return nval-213; break; // 213-222
  }
  return 254;
}
char getVFXL(int nval) {
  if (nval==0) {return '.';}
  else{return getlnibble(nval&0x3f);}
}
char getVFXH(int nval) {
  if (nval==0) {return '.';}
  else{return gethnibble(nval&0x3f);}
}
unsigned char getVFXColor(int fxval) {
  if (fxval==0) {return 81;} // 0
  if (fxval<128 && fxval>63) {return 27;} // 64-127
  if (fxval<193 && fxval>127) {return 6;} // 128-192
  switch((fxval-1)/10) {
  case 0: return 2; break; // 1-10
  case 1: return 2; break; // 11-20
  case 2: return 2; break; // 21-30
  case 3: return 2; break; // 31-40
  case 4: return 3; break; // 41-50
  case 5: return 3; break; // 51-60
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
unsigned char getmixerposcol(int channel,int envid) {
  if (!EnvelopesRunning[channel][envid]) {return 8;}
  if (inspos[channel][envid]>=bytable[envid][instrument[Mins[channel]].env[envid]][254]) {return 11;}
  if (IRP[channel][envid] && released[channel]) {return 10;}
  if (released[channel]) {return 9;}
  if (IRP[channel][envid]) {return 12;}
  return 14;
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
  return (int)((6203.34-(songdf*2))*(pow(2.0f,(float)(((float)note-58)/12.0f))));
}

int msnoteperiod(float note, int chan) {
  return ((297500+(songdf*100))/(440*(pow(2.0f,(float)(((float)note-58)/12)))));
}

int AllocateSequence(int seqid) {
  bool nonfree[256];
  // finds the next free sequence for seqid
  for (int usedindex=0;usedindex<256;usedindex++) {
    nonfree[usedindex]=false;
  }
  for (int usedindex=0;usedindex<256;usedindex++) {
    // finds what sequences are used
    nonfree[instrument[usedindex].env[seqid]]=true;
  }
  for (int usedindex=0;usedindex<256;usedindex++) {
    // finds what sequences don't have their default values
    for (int checkpos=0;checkpos<253;checkpos++) {
      if (bytable[seqid][usedindex][checkpos]!=0) {nonfree[usedindex]=true;break;}
    }
    if (bytable[seqid][usedindex][253]!=0) {
      nonfree[usedindex]=true;
    }
    if (bytable[seqid][usedindex][254]!=255) {
      nonfree[usedindex]=true;
    }
    if (bytable[seqid][usedindex][255]!=255) {
      nonfree[usedindex]=true;
    }
  }
  for (int usedindex=0;usedindex<256;usedindex++) {
    // using the previous variables, find the first free envelope
    if (!nonfree[usedindex]) {return usedindex;}
  }
  printf("could not allocate sequence! %d\n",seqid); 
  #ifdef SOUNDS
  triggerfx(1);
  #endif
  popbox=PopupBox("Error","no free sequence found!");
  return 0;
}
int ProcessPitch(int insnumb, float offset) {
  // output (pitch logic)
  if (((instrument[Mins[insnumb]].activeEnv&32)>>5)) {
    if (bytable[5][instrument[Mins[insnumb]].env[envPitch]][inspos[insnumb][5]]<0x40) {
    return mnoteperiod(curnote[insnumb]+(bytable[5][instrument[Mins[insnumb]].env[envPitch]][inspos[insnumb][5]])+(((unsigned char)instrument[Mins[insnumb]].noteOffset)-47)+((float)finepitch[insnumb]/64)+offset,insnumb);
    } else {
    if (bytable[5][instrument[Mins[insnumb]].env[envPitch]][inspos[insnumb][5]]<0x80) {
    return mnoteperiod(curnote[insnumb]-((bytable[5][instrument[Mins[insnumb]].env[envPitch]][inspos[insnumb][5]]-64))+(((unsigned char)instrument[Mins[insnumb]].noteOffset)-47)+((float)finepitch[insnumb]/64)+offset,insnumb);
    } else {
    return mnoteperiod(bytable[5][instrument[Mins[insnumb]].env[envPitch]][inspos[insnumb][5]]-128+((float)finepitch[insnumb]/64)+offset,insnumb);
    }}
  }
  else {return mnoteperiod(curnote[insnumb]+(((unsigned char)instrument[Mins[insnumb]].noteOffset)-47)+((float)finepitch[insnumb]/64)+offset,insnumb);}
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
  // returns which channel is free
  // routine is similar to IT's next free channel routine
  // 1. find first inactive channel
  // failed? 2. find first channel with zero volume
  for (int ii=0;ii<8;ii++) {
    if (cvol[ii]==0) {return ii;}
  }
  // failed? 3. find channel with lowest volume
  int candidate=0;
  for (int ii=1;ii<32;ii++) {
    if (cvol[candidate]>cvol[ii]) {candidate=ii;}
  }
  return candidate;
}

// convert sequence into ssinter format
string seqToSS(int insid, int seqid) {
  string ret;
  if (!(instrument[insid].activeEnv&(1<<seqid))) return "";
  int iseq=instrument[insid].env[seqid];
  int len=bytable[seqid][iseq][253];
  ret="$~";
  switch (seqid) {
    case 0: // volume
      for (int i=0; i<=len; i++) {
        ret+=strFormat("V%.2x;",bytable[seqid][iseq][i]>>1);
      }
      break;
    case 1: // cutoff
      ret+=strFormat("I%d",instrument[insid].DFM&7);
      if (!(instrument[insid].activeEnv&4)) {
        ret+="r30";
      }
      for (int i=0; i<=len; i++) {
        ret+=strFormat("c%.4x;",(bytable[seqid][iseq][i]*(65535-instrument[insid].filterH))/255);
      }
      break;
    case 2: // resonance
      for (int i=0; i<=len; i++) {
        ret+=strFormat("r%.2x;",bytable[seqid][iseq][i]);
      }
      break;
    case 3: // duty
      for (int i=0; i<=len; i++) {
        ret+=strFormat("Y%.2x;",bytable[seqid][iseq][i]>>1);
      }
      break;
    case 4: // shape
      for (int i=0; i<=len; i++) {
        ret+=strFormat("S%d;",bytable[seqid][iseq][i]>>5);
      }
      break;
    case 5: // pitch
      for (int i=0; i<=len; i++) {
        if (instrument[insid].activeEnv&64) {
          if (bytable[seqid][iseq][i]<0x40) { // up
            ret+=strFormat("f(* xf (trans (+ f %d));",bytable[seqid][iseq][i]);
          } else if (bytable[seqid][iseq][i]<0x80) { // down
            ret+=strFormat("f(* xf (trans (+ f -%d)));",bytable[seqid][iseq][i]-0x40);
          } else { // note
            ret+=strFormat("f(note (+ f %d));",bytable[seqid][iseq][i]-0x80);
          }
        } else {
          if (bytable[seqid][iseq][i]<0x40) { // up
            ret+=strFormat("f(* xf (trans %d));",bytable[seqid][iseq][i]);
          } else if (bytable[seqid][iseq][i]<0x80) { // down
            ret+=strFormat("f(* xf (trans -%d));",bytable[seqid][iseq][i]-0x40);
          } else { // note
            ret+=strFormat("f(note %d);",bytable[seqid][iseq][i]-0x80);
          }
        }
      }
      break;
    case 6: // fine pitch
      ret+="=(setq f 0)";
      for (int i=0; i<=len; i++) {
        ret+=strFormat("=(setq f (+ f %d))",(signed char)bytable[seqid][iseq][i]);
        if (instrument[insid].activeEnv&32) {
          ret+=";";
        } else {
          ret+="f(* xf (trans f))";
        }
      }
      break;
    case 7: // pan
      for (int i=0; i<=len; i++) {
        ret+=strFormat("P%d;",bytable[seqid][iseq][i]);
      }
      break;
    default: // invalid
      return "";
  }
  return ret;
}

void NextRow() {
  //// PROCESS NEXT ROW ////
  // forward code
  if (!reversemode) {
  // increase step counter
  curstep++;
  // did we reach end of pattern?
  if (curstep>(getpatlen(patid[curpat])-1)) {
       // set current step to 0 and go to next pattern
       curstep=0; 
       curpat++;
       // reset pattern loop stuff
       for (int ii=0;ii<32;ii++) {
         plcount[ii]=0;
         plpos[ii]=0;
       }
       // did we reach end of song? if yes then restart song
       if (curpat>songlength) {curpat=0;}
  }
  } else {
    // backward code
    curstep--;
    // did we reach start of pattern?
    if (curstep<0) {
      // are we NOT at the beginning of the song?
      if (curpat!=0) {
        // set current step to 0 and go to next pattern
        curstep=getpatlen(patid[curpat-1])-1; 
        curpat--;
        // reset pattern loop stuff
        for (int ii=0;ii<32;ii++) {
          plcount[ii]=0;
          plpos[ii]=0;
        }
      } else {
        curstep=-1;
      }
      // did we reach end of song? if yes then restart song
      if (curpat>songlength) {curpat=0;}
    }
  }
  // MAKE SURE PATTERNS ARE UPDATED
  UPDATEPATTERNS=true;
  // next note and instrument
  unsigned char nnote;
  unsigned char nnnote;
  unsigned char ninst;
  unsigned char SEMINOTE;
  finedelay=0; // S6x value
  // run playback routine over 32 channels
  for (int loop=0;loop<32;loop++) {
    // get next row variables
    nnote=pat[patid[curpat]][curstep][loop][0]; // finds out next note
    if (curstep!=(getpatlen(patid[curpat])-1)) {
    nnnote=pat[patid[curpat]][curstep+1][loop][0]; // finds out next note past next note
    } else {nnnote=0;}
    if ((nnnote%16)!=0 && (nnnote%16)!=15 && (nnnote%16)!=14 && (nnnote%16)!=13 && ((instrument[Mins[loop]].flags>>6)!=0)) {
      cutcount[loop]=speed-(instrument[Mins[loop]].flags>>6);
    }
    ninst=pat[patid[curpat]][curstep][loop][1]; // finds out next instrument
    nvolu[loop]=pat[patid[curpat]][curstep][loop][2]; // finds out next volume value
    // is there a note and instrument, but no volume value? assume instrument volume
    if ((nnote%16)!=0 && (nnote%16)!=15 && (nnote%16)!=14 && (nnote%16)!=13 && nvolu[loop]==0 && ninst!=0) {nvolu[loop]=0x40+minval(instrument[ninst].vol,63);}
    nfxid[loop]=pat[patid[curpat]][curstep][loop][3]; // finds out next effect
    nfxvl[loop]=pat[patid[curpat]][curstep][loop][4]; // finds out next effect value
    // Txx
    if (nfxid[loop]==20)
    {if (nfxvl[loop]!=0)
    {if (!tempolock) {tempo=maxval(31,nfxvl[loop]);FPS=(double)tempo/2.5;}
    }}
    // volume value set?
    if (nvolu[loop]!=0 && nvolu[loop]>63 && nvolu[loop]<128 && !(nfxid[loop]==19 && (nfxvl[loop]>>4)==0x0d)) {
      // set note volume
      Mvol[loop]=(nvolu[loop]%64)*2;
      // volume envelope enabled?
      if (instrument[Mins[loop]].activeEnv&1) {
      // set current volume to (volume in envelope)*(note volume/127)
      // else set current volume to note volume
      cvol[loop]=((bytable[0][instrument[Mins[loop]].env[envVolume]][inspos[loop][0]]/2)*(Mvol[loop]))/127;
      EnvelopesRunning[loop][0]=true;}
      else {EnvelopesRunning[loop][0]=false;cvol[loop]=Mvol[loop];}
    }
    // is there a new instrument? if yes then reset volume
    if (ninst!=0 && (nnote%16)!=15 && (nnote%16)!=14 && (nnote%16)!=13) {
      if (nvolu[loop]==0) {
        if (EnvelopesRunning[loop][0]) {
          Mvol[loop]=minval(instrument[ninst].vol,63)*2;
        } else {
          cvol[loop]=minval(instrument[ninst].vol,63)*2;
        }
      }
    }
    if (ninst!=0) {
      Mins[loop]=ninst;
    }
    if ((nnote%16)==15) {
      Mvol[loop]=0;
      cvol[loop]=0;
      EnvelopesRunning[loop][0]=false;EnvelopesRunning[loop][1]=false;
      EnvelopesRunning[loop][2]=false;EnvelopesRunning[loop][3]=false;
      EnvelopesRunning[loop][4]=false;EnvelopesRunning[loop][5]=false;
      EnvelopesRunning[loop][6]=false;EnvelopesRunning[loop][7]=false;
      reservedevent[loop]=1; offinstead[loop]=1;
      continue;
    }
    // is there SDx? if yes then don't trigger note
    // is there a new note?
    if ((nnote%16)!=0 && !(nfxid[loop]==19 && (nfxvl[loop]>>4)==0x0d)) {
      // is new note a release note? set released flag to true and exit if yes
      if ((nnote%16)==13) {released[loop]=true; reservedevent[loop]=1; offinstead[loop]=1; continue;}
      // is new note a NOTE, and effect isn't Gxx/Lxx/gxx?
      if ((nnote%16)!=13 && (nnote%16)!=14 && (nnote%16)!=15 && nfxid[loop]!=7 && nfxid[loop]!=12 && !(nvolu[loop]>192 && nvolu[loop]<203)) {
      // set current note
      curnote[loop]=mscale(nnote);
      }
      // reset the "release" flags
      // this won't happen if note is a release note
      released[loop]=0;
      // is there a new instrument value, along with the new note? if yes then change instrument
      if (ninst!=0) {
      Mins[loop]=ninst;
      }
      // is there a new volume value, along with the new note?
      if (nvolu[loop]!=0 && nvolu[loop]>63 && nvolu[loop]<128) {
      // set note volume
      Mvol[loop]=(nvolu[loop]%64)*2;
      } else {if ((nnote%16)!=15 && ninst!=0) {Mvol[loop]=126;}} // no volume, assuming v3f if not a note release
      // set seminote
      SEMINOTE=((nnote%16)+((nnote>>4)*12));
      // set portamento note
      portastatic[loop]=SEMINOTE;
      // reset all envelope cursors if effect isn't Gxx/Lxx/gxx
      if (nfxid[loop]!=7 && nfxid[loop]!=12 && !(nvolu[loop]>192 && nvolu[loop]<203)) {
      // is it a pcm instrument? (pcm check)
      if (instrument[Mins[loop]].DFM&8) {
        // set channel mode to PCM
        chip[0].chan[loop].flags.pcm=1;
                                //chip[0].pcmmult[loop]=127;
                                chip[0].chan[loop].flags.pcmloop=instrument[Mins[loop]].pcmMult>>7;
        //chip[0].pcmmult[loop]|=(instrument[Mins[loop]][0x21]&128);
        chip[0].chan[loop].pcmrst=*(unsigned short*)instrument[Mins[loop]].pcmPos+*(unsigned short*)instrument[Mins[loop]].pcmLoop;
        // set respective PCM pointers
        chip[0].chan[loop].pcmpos=*(unsigned short*)instrument[Mins[loop]].pcmPos;
        chip[0].chan[loop].pcmbnd=*(unsigned short*)instrument[Mins[loop]].pcmPos+instrument[Mins[loop]].pcmLen;
      } else {chip[0].chan[loop].flags.pcm=0;}
      // is ringmod on? (ringmod check)
      if (instrument[Mins[loop]].DFM&16) {
        // set ring modulation to on
        crm[loop]=1;
        crmduty[loop]=63;
        crmfreq[loop]=msnoteperiod(SEMINOTE+((instrument[Mins[loop]].LFO<0x40)?(instrument[Mins[loop]].LFO):(64-instrument[Mins[loop]].LFO)),loop);
        crrmstep[loop]=0;
        // is sync on?
        if (instrument[Mins[loop]].flags&32) {crm[loop]=2;}
      } else {crm[loop]=0;}
      // is oscreset on? (osc reset check)
      if (instrument[Mins[loop]].flags&1) {crstep[loop]=1;} // osc reset
      // volume (if turned on and effect isn't S77, or effect is S78)
      if (((instrument[Mins[loop]].activeEnv&1) || ((nfxid[loop]==19) && (nfxvl[loop]==0x78))) && !((nfxid[loop]==19) && (nfxvl[loop]==0x77))) {
      cvol[loop]=((bytable[0][instrument[Mins[loop]].env[envVolume]][0]/2)*(Mvol[loop]))/127;
      EnvelopesRunning[loop][0]=true;}
      else {EnvelopesRunning[loop][0]=false;cvol[loop]=Mvol[loop];}
      // duty
      if ((instrument[Mins[loop]].activeEnv&8)>>3) {
      cduty[loop]=bytable[3][instrument[Mins[loop]].env[envDuty]][0]/2;
      EnvelopesRunning[loop][3]=true;}
      else {EnvelopesRunning[loop][3]=false;cduty[loop]=63;}
      // shape
      if ((instrument[Mins[loop]].activeEnv&16)>>4) {
      cshape[loop]=bytable[4][instrument[Mins[loop]].env[envShape]][0]/32;
      EnvelopesRunning[loop][4]=true;}
      else {EnvelopesRunning[loop][4]=false;cshape[loop]=0;}
      // cutoff
      if ((instrument[Mins[loop]].activeEnv&2)>>1) { 
      if (nfxid[loop]!=15) {
      coff[loop]=(bytable[1][instrument[Mins[loop]].env[envCutoff]][0]*(0xffff-instrument[Mins[loop]].filterH))>>7;cfmode[loop]=instrument[Mins[loop]].DFM&7;
      } else {
      coff[loop]=(bytable[1][instrument[Mins[loop]].env[envCutoff]][(int)((float)bytable[1][instrument[Mins[loop]].env[envCutoff]][253]*((float)nfxvl[loop]/256))]*(0xffff-instrument[Mins[loop]].filterH))>>7;
      cfmode[loop]=instrument[Mins[loop]].DFM&7;
      //coff[loop]=bytable[1][instrument[Mins[loop]].env[envCutoff]][(int)((float)bytable[1][instrument[Mins[loop]].env[envCutoff]][253]*((float)nfxvl[loop]/256))]*512;cfmode[loop]=instrument[Mins[loop]].DFM&7;
      }
      EnvelopesRunning[loop][1]=true;}
      else {EnvelopesRunning[loop][1]=false;coff[loop]=262144;cfmode[loop]=fNone;}
      // resonance
      if ((instrument[Mins[loop]].activeEnv&4)>>2) {
      creso[loop]=bytable[2][instrument[Mins[loop]].env[envReson]][0];
      EnvelopesRunning[loop][2]=true;}
      else {EnvelopesRunning[loop][2]=false;creso[loop]=48;finepitch[loop]=0;}
      // panning
      if ((instrument[Mins[loop]].activeEnv&128)>>7) {
      cpan[loop]=bytable[7][instrument[Mins[loop]].env[envPan]][0]-128;
      EnvelopesRunning[loop][7]=true;}
      else {EnvelopesRunning[loop][7]=false;
        cpan[loop]=chanpan[loop];
      }
      // finepitch
      if ((instrument[Mins[loop]].activeEnv&64)>>6) {
      finepitch[loop]=(signed char)bytable[6][instrument[Mins[loop]].env[envHiPitch]][0];
      cfreq[loop]=ProcessPitch(loop,0);
      EnvelopesRunning[loop][6]=true;}
      else {EnvelopesRunning[loop][6]=false;finepitch[loop]=0;}
      // pitch
      if ((instrument[Mins[loop]].activeEnv&32)>>5) {
      // output (pitch logic)
      if (bytable[5][instrument[Mins[loop]].env[envPitch]][0]<0x40) {
      cfreq[loop]=mnoteperiod(curnote[loop]+(bytable[5][instrument[Mins[loop]].env[envPitch]][0])+(((unsigned char)instrument[Mins[loop]].noteOffset)-47),loop);
      } else {
      if (bytable[5][instrument[Mins[loop]].env[envPitch]][0]<0x80) {
      cfreq[loop]=mnoteperiod(curnote[loop]-((bytable[5][instrument[Mins[loop]].env[envPitch]][0]-64))+(((unsigned char)instrument[Mins[loop]].noteOffset)-47),loop);
      } else {
      cfreq[loop]=mnoteperiod(bytable[5][instrument[Mins[loop]].env[envPitch]][0]-128,loop);
      }}
      EnvelopesRunning[loop][5]=true;}
      else {EnvelopesRunning[loop][5]=false;}
      }
      // is it a note cut? if yes, set volume to 0
      if ((nnote%16)==15) {Mvol[loop]=0;} else 
      {
        // set note frequency if effect isn't Gxx/Lxx/gxx and pitch/finepitch envelopes are disabled
      if (nfxid[loop]!=7 && nfxid[loop]!=12 && !(nvolu[loop]>192 && nvolu[loop]<203) && !((instrument[Mins[loop]].activeEnv&32)>>5) && !((instrument[Mins[loop]].activeEnv&64)>>6)) {
      SEMINOTE=((nnote%16)+((nnote>>4)*12))+(((unsigned char)instrument[Mins[loop]].noteOffset)-48);
      cfreq[loop]=mnoteperiod(SEMINOTE,loop); // sets the frequency to match the current note and applies instrument transposition
      oldperiod[loop]=newperiod[loop];
      newperiod[loop]=cfreq[loop];
      }
      }
      // only reset envelope positions if effect isn't Gxx/Lxx/gxx
      if (nfxid[loop]!=7 && nfxid[loop]!=12 && !(nvolu[loop]>192 && nvolu[loop]<203)) {
      // check for Oxx effect
      if (nfxid[loop]!=15) {
      inspos[loop][0]=0; // sets the instrument position to 0
      inspos[loop][1]=0; // sets the instrument position to 0
      inspos[loop][2]=0; // sets the instrument position to 0
      inspos[loop][3]=0; // sets the instrument position to 0
      inspos[loop][4]=0; // sets the instrument position to 0
      inspos[loop][5]=0; // sets the instrument position to 0
      inspos[loop][6]=0; // sets the instrument position to 0
      inspos[loop][7]=0; // sets the instrument position to 0
      } else { // Oxx
      inspos[loop][0]=nfxvl[loop]; // sets the instrument position to 0
      inspos[loop][1]=(int)((float)bytable[1][instrument[Mins[loop]].env[envCutoff]][253]*((float)nfxvl[loop]/256)); // sets the instrument position to 0
      inspos[loop][2]=nfxvl[loop]; // sets the instrument position to 0
      inspos[loop][3]=nfxvl[loop]; // sets the instrument position to 0
      inspos[loop][4]=0; // sets the instrument position to 0
      inspos[loop][5]=0; // sets the instrument position to 0
      inspos[loop][6]=0; // sets the instrument position to 0
      inspos[loop][7]=nfxvl[loop]; // sets the instrument position to 0
      }
      // set In-Release-Point (IRP) flags to false
      IRP[loop][0]=0;
      IRP[loop][1]=0;
      IRP[loop][2]=0;
      IRP[loop][3]=0;
      IRP[loop][4]=0;
      IRP[loop][5]=0;
      IRP[loop][6]=0;
      IRP[loop][7]=0;
      // note is not released yet
      released[loop]=false;
      reservedevent[loop]=1;
      // reset vibrato position
      curvibpos[loop]=0;
      }
      // Qxx
      if (nfxid[loop]==17)
      {if (nfxvl[loop]!=0) {doretrigger[loop]=true;retrigger[loop]=nfxvl[loop];retrig[loop]=(retrigger[loop]%16);}
      else {retrig[loop]=(retrigger[loop]%16);}}
    } else {
      if (nfxid[loop]==19 && (nfxvl[loop]>>4)==0x0d) {
        curnote[loop]=mscale(nnote);
        doretrigger[loop]=true;retrigger[loop]=128+(nfxvl[loop]%16);retrig[loop]=maxval(1,retrigger[loop]%16);
      }
    }
  }
  // effect checks
  for (int looper=0;looper<32;looper++) {
  // Axx
  if (nfxid[looper]==1)
    {if (nfxvl[looper]!=0 && !speedlock)
    {speed=nfxvl[looper];}}
  // Dfx - Dxf - Kfx - Kxf - Lfx - Lxf
  if (nfxid[looper]==4 || nfxid[looper]==11 || nfxid[looper]==12)
    {if (nfxvl[looper]!=0) {slidememory[looper]=nfxvl[looper];}
    if (EnvelopesRunning[looper][0]) {
    if (slidememory[looper]>0xf0) {Mvol[looper]-=((slidememory[looper]%16)*2);
    if (Mvol[looper]<0) {Mvol[looper]=0;}} else {if (slidememory[looper]%16==0x0f && slidememory[looper]!=0x0f) {Mvol[looper]+=((slidememory[looper]>>4)*2);if (Mvol[looper]>127) {Mvol[looper]=127;}}}}
    else {
    if (slidememory[looper]>0xf0) {cvol[looper]-=((slidememory[looper]%16)*2);
    if (cvol[looper]<0) {cvol[looper]=0;}} else {if (slidememory[looper]%16==0x0f && slidememory[looper]!=0x0f) {cvol[looper]+=((slidememory[looper]>>4)*2);if (cvol[looper]>127) {cvol[looper]=127;}}}}
    }
  // Nfx - Nxf
  if (nfxid[looper]==14)
    {if (nfxvl[looper]!=0) {chanslidememory[looper]=nfxvl[looper];}
    if (chanslidememory[looper]>0xf0) {chanvol[looper]-=((chanslidememory[looper]%16)*2);
    if (chanvol[looper]<0) {chanvol[looper]=0;}} else {if (chanslidememory[looper]%16==0x0f && chanslidememory[looper]!=0x0f) {chanvol[looper]+=((chanslidememory[looper]>>4)*2);if (chanvol[looper]>127) {chanvol[looper]=127;}}}
    }
  // Efx - Eex
  if (nfxid[looper]==5)
    {if (nfxvl[looper]>0xef) {curnote[looper]-=((float)(nfxvl[looper]%16)/16);
      if (!EnvelopesRunning[looper][5]) {
            cfreq[looper]=mnoteperiod(curnote[looper]+((unsigned char)instrument[Mins[looper]].noteOffset)-47,looper)+(int)((float)sine[(curvibpos[looper]*curvibspeed[looper]*4)%256]*((float)curvibdepth[looper]/16));
            } else {
            cfreq[looper]=ProcessPitch(looper,0)+(int)((float)sine[(curvibpos[looper]*curvibspeed[looper]*4)%256]*((float)curvibdepth[looper]/16));
            }} else {
      if (nfxvl[looper]>0xdf) {curnote[looper]-=((float)(nfxvl[looper]%16)/64);
          if (!EnvelopesRunning[looper][5]) {
            cfreq[looper]=mnoteperiod(curnote[looper]+((unsigned char)instrument[Mins[looper]].noteOffset)-47,looper)+(int)((float)sine[(curvibpos[looper]*curvibspeed[looper]*4)%256]*((float)curvibdepth[looper]/16));
            } else {
            cfreq[looper]=ProcessPitch(looper,0)+(int)((float)sine[(curvibpos[looper]*curvibspeed[looper]*4)%256]*((float)curvibdepth[looper]/16));
            }}
    }}
  // Ffx - Fex
  if (nfxid[looper]==6)
    {if (nfxvl[looper]>0xef) {curnote[looper]+=((float)(nfxvl[looper]%16)/16);
      if (!EnvelopesRunning[looper][5]) {
            cfreq[looper]=mnoteperiod(curnote[looper]+((unsigned char)instrument[Mins[looper]].noteOffset)-47,looper)+(int)((float)sine[(curvibpos[looper]*curvibspeed[looper]*4)%256]*((float)curvibdepth[looper]/16));
            } else {
            cfreq[looper]=ProcessPitch(looper,0)+(int)((float)sine[(curvibpos[looper]*curvibspeed[looper]*4)%256]*((float)curvibdepth[looper]/16));
            }} else {
      if (nfxvl[looper]>0xdf) {curnote[looper]+=((float)(nfxvl[looper]%16)/64);
          if (!EnvelopesRunning[looper][5]) {
            cfreq[looper]=mnoteperiod(curnote[looper]+((unsigned char)instrument[Mins[looper]].noteOffset)-47,looper)+(int)((float)sine[(curvibpos[looper]*curvibspeed[looper]*4)%256]*((float)curvibdepth[looper]/16));
            } else {
            cfreq[looper]=ProcessPitch(looper,0)+(int)((float)sine[(curvibpos[looper]*curvibspeed[looper]*4)%256]*((float)curvibdepth[looper]/16));
            }}
    }}
  // Mxx
  if (nfxid[looper]==13)
  {chanvol[looper]=minval(nfxvl[looper]*2,128);}
  // Vxx
  if (nfxid[looper]==22)
  {cglobvol=minval(nfxvl[looper],128);}
  // Qxx
  if (nfxid[looper]==17)
  {if (nfxvl[looper]!=0) {if (nfxvl[looper]%16!=retrigger[looper]%16) {
    if (retrig[looper]==(retrigger[looper]%16)) {retrig[looper]=(nfxvl[looper]%16);}
    retrigger[looper]=nfxvl[looper];}}} else {
    if (!(nfxid[looper]==19 && (nfxvl[looper]>>4)==0x0d)) {retrig[looper]=0;}
  }
  // Sxx
  if (nfxid[looper]==19)
    {
    // S3x
    if ((nfxvl[looper]>>4)==0x03) {
      curvibshape[looper]=nfxvl[looper]%4;
    }
    // S8x
    if ((nfxvl[looper]>>4)==0x08)
      {cpan[looper]=(((nfxvl[looper]%16)*16)+(nfxvl[looper]%16))-128;
       chanpan[looper]=(((nfxvl[looper]%16)*16)+(nfxvl[looper]%16))-128;}
    // SEx
    if ((nfxvl[looper]>>4)==0x0e) {finedelay+=(nfxvl[looper]%16)*speed;}
    // S6x
    if ((nfxvl[looper]>>4)==0x06) {finedelay+=(nfxvl[looper]%16);}
    // SCx
    if ((nfxvl[looper]>>4)==0x0c) {
      if ((nfxvl[looper]%16)<speed) {
      cutcount[looper]=maxval(1,nfxvl[looper]%16);
      }
    }
    // SBx
    if ((nfxvl[looper]>>4)==0x0b) {
      if ((nfxvl[looper]%16)==0) {plpos[looper]=curstep;} else {
        if (plcount[looper]<(nfxvl[looper]%16)) {
          plcount[looper]++;
          curstep=plpos[looper]-1;
        } else {plcount[looper]=0;}
      }}}
  // Xxx
  if (nfxid[looper]==24)
    {cpan[looper]=nfxvl[looper]-128;
     chanpan[looper]=nfxvl[looper]-128;}
  // Zxx
  if (nfxid[looper]==26)
    {Zxx(nfxvl[looper]);}
  }
  curtick=speed+finedelay; // sets the current tick to the song speed
  //curstep++; // increases the step counter
  // did we reach end of pattern?
  /*if (curstep>(getpatlen(patid[curpat])-1)) { // yes
       curstep=0; // set current step to 0
       curpat++; // increases current pattern
       for (int ii=0;ii<32;ii++) {
         plcount[ii]=0;
         plpos[ii]=0;
       }
       if (curpat>songlength) {curpat=0;}
  }*/
}
void SkipPattern(int chanval) {
  if (!AlreadySkipped) {
    AlreadySkipped=true;curpat++;curstep=nfxvl[chanval]-1;NextRow();
  }
}

void UpdateEnvelope(int channel, int envid) {
  if (!IRP[channel][envid] || released[channel] ||
    (bytable[envid][instrument[Mins[channel]].env[envid]][254])<(bytable[envid][instrument[Mins[channel]].env[envid]][255])) {
  inspos[channel][envid]++;
  if (inspos[channel][envid]==bytable[envid][instrument[Mins[channel]].env[envid]][255]) {IRP[channel][envid]=true;}
  }
  if (inspos[channel][envid]>
    ((IRP[channel][envid] && !released[channel])?(minval(252,bytable[envid][instrument[Mins[channel]].env[envid]][255])):(minval(252,bytable[envid][instrument[Mins[channel]].env[envid]][253]))
    )) // if envelope exceeds 252, or the length
    {if ((bytable[envid][instrument[Mins[channel]].env[envid]][254]<252) &&
    (!released[channel])
    ) { // if loop is defined
      inspos[channel][envid]=bytable[envid][instrument[Mins[channel]].env[envid]][254];
  } else {
    if (!((bytable[envid][instrument[Mins[channel]].env[envid]][254]<252) &&
      (bytable[envid][instrument[Mins[channel]].env[envid]][254])>(bytable[envid][instrument[Mins[channel]].env[envid]][255])
      ) && 
      (bytable[envid][instrument[Mins[channel]].env[envid]][255]<252)
      ) {
    // end of envelope
    inspos[channel][envid]=minval(252,bytable[envid][instrument[Mins[channel]].env[envid]][253]);
    EnvelopesRunning[channel][envid]=false;
    } else {
      if ((bytable[envid][instrument[Mins[channel]].env[envid]][254]>252)) {
      // end of envelope
      inspos[channel][envid]=minval(252,bytable[envid][instrument[Mins[channel]].env[envid]][253]);
      EnvelopesRunning[channel][envid]=false;
      } else {
      inspos[channel][envid]=bytable[envid][instrument[Mins[channel]].env[envid]][254];
      }
    }
  }
  }
}

void NextTick() {
  // process the next tick
  curtick--;
  // run envelopes
  for (int loop2=0;loop2<32;loop2++) {
  // Qxx
  if (doretrigger[loop2])
    {retrig[loop2]--;
     if (retrig[loop2]==0) {retrig[loop2]=retrigger[loop2]%16;
     doretrigger[loop2]=false;
     if (nfxid[loop2]==17) {doretrigger[loop2]=true;} else {
      if (nvolu[loop2]!=0 && nvolu[loop2]>63 && nvolu[loop2]<128) {
      // set note volume
      Mvol[loop2]=(nvolu[loop2]%64)*2;
    }
     }
     if (nfxid[loop2]==17) {
      // process RVCT
      switch (retrigger[loop2]>>4) {
        case 1: Mvol[loop2]-=2; if (Mvol[loop2]<0) {Mvol[loop2]=0;}; break;
        case 2: Mvol[loop2]-=4; if (Mvol[loop2]<0) {Mvol[loop2]=0;}; break;
        case 3: Mvol[loop2]-=8; if (Mvol[loop2]<0) {Mvol[loop2]=0;}; break;
        case 4: Mvol[loop2]-=16; if (Mvol[loop2]<0) {Mvol[loop2]=0;}; break;
        case 5: Mvol[loop2]-=32; if (Mvol[loop2]<0) {Mvol[loop2]=0;}; break;
        case 6: Mvol[loop2]*=2; Mvol[loop2]/=3; if (Mvol[loop2]<0) {Mvol[loop2]=0;}; break;
        case 7: Mvol[loop2]/=2; if (Mvol[loop2]<0) {Mvol[loop2]=0;}; break;
        case 9: Mvol[loop2]+=2; if (Mvol[loop2]>126) {Mvol[loop2]=126;}; break;
        case 10: Mvol[loop2]+=4; if (Mvol[loop2]>126) {Mvol[loop2]=126;}; break;
        case 11: Mvol[loop2]+=8; if (Mvol[loop2]>126) {Mvol[loop2]=126;}; break;
        case 12: Mvol[loop2]+=16; if (Mvol[loop2]>126) {Mvol[loop2]=126;}; break;
        case 13: Mvol[loop2]+=32; if (Mvol[loop2]>126) {Mvol[loop2]=126;}; break;
        case 14: Mvol[loop2]*=3; Mvol[loop2]/=2; if (Mvol[loop2]>126) {Mvol[loop2]=126;}; break;
        case 15: Mvol[loop2]*=2; if (Mvol[loop2]>126) {Mvol[loop2]=126;}; break;
      }
     }
      // reset all envelope cursors if effect isn't Gxx/Lxx/gxx
      if (nfxid[loop2]!=7 && nfxid[loop2]!=12 && !(nvolu[loop2]>192 && nvolu[loop2]<203)) {
      // is there a new instrument value, along with the new note? if yes then change instrument
      if (pat[patid[curpat]][curstep][loop2][1]!=0) {
      Mins[loop2]=pat[patid[curpat]][curstep][loop2][1];
      }
      // is it a pcm instrument? (pcm check)
      if (instrument[Mins[loop2]].DFM&8) {
        // set channel mode to PCM
        chip[0].chan[loop2].flags.pcm=1;
        // set respective PCM pointers
        chip[0].chan[loop2].pcmpos=*(unsigned short*)instrument[Mins[loop2]].pcmPos;
        chip[0].chan[loop2].pcmbnd=*(unsigned short*)instrument[Mins[loop2]].pcmPos+instrument[Mins[loop2]].pcmLen;
      } else {chip[0].chan[loop2].flags.pcm=0;}
      // is oscreset on? (osc reset check)
      if (instrument[Mins[loop2]].flags&1) {crstep[loop2]=1;} // osc reset
      // volume (if turned on and effect isn't S77, or effect is S78)
      if (((instrument[Mins[loop2]].activeEnv&1) || ((nfxid[loop2]==19) && (nfxvl[loop2]==0x78))) && !((nfxid[loop2]==19) && (nfxvl[loop2]==0x77))) {
      cvol[loop2]=((bytable[0][instrument[Mins[loop2]].env[envVolume]][0]/2)*(Mvol[loop2]))/127;
      EnvelopesRunning[loop2][0]=true;}
      else {EnvelopesRunning[loop2][0]=false;cvol[loop2]=Mvol[loop2];}
      // duty
      if ((instrument[Mins[loop2]].activeEnv&8)>>3) {
      cduty[loop2]=bytable[3][instrument[Mins[loop2]].env[envDuty]][0]/2;
      EnvelopesRunning[loop2][3]=true;}
      else {EnvelopesRunning[loop2][3]=false;cduty[loop2]=63;}
      // shape
      if ((instrument[Mins[loop2]].activeEnv&16)>>4) {
      cshape[loop2]=bytable[4][instrument[Mins[loop2]].env[envShape]][0]/32;
      EnvelopesRunning[loop2][4]=true;}
      else {EnvelopesRunning[loop2][4]=false;cshape[loop2]=0;}
      // cutoff
      if ((instrument[Mins[loop2]].activeEnv&2)>>1) {
      if (nfxid[loop2]!=15) {
      coff[loop2]=(bytable[1][instrument[Mins[loop2]].env[envCutoff]][0]*(0xffff-instrument[Mins[loop2]].filterH))>>7;cfmode[loop2]=instrument[Mins[loop2]].DFM&7;
      } else {
      coff[loop2]=(bytable[1][instrument[Mins[loop2]].env[envCutoff]][(int)((float)bytable[1][instrument[Mins[loop2]].env[envCutoff]][253]*((float)nfxvl[loop2]/256))]*(0xffff-instrument[Mins[loop2]].filterH))>>7;
      cfmode[loop2]=instrument[Mins[loop2]].DFM&7;
      }
      EnvelopesRunning[loop2][1]=true;}
      else {EnvelopesRunning[loop2][1]=false;coff[loop2]=262144;cfmode[loop2]=fNone;}
      // resonance
      if ((instrument[Mins[loop2]].activeEnv&4)>>2) {
      creso[loop2]=bytable[2][instrument[Mins[loop2]].env[envReson]][0];
      EnvelopesRunning[loop2][2]=true;}
      else {EnvelopesRunning[loop2][2]=false;creso[loop2]=48;finepitch[loop2]=0;}
      // panning
      if ((instrument[Mins[loop2]].activeEnv&128)>>7) {
      cpan[loop2]=bytable[7][instrument[Mins[loop2]].env[envPan]][0]-128;
      EnvelopesRunning[loop2][7]=true;}
      else {EnvelopesRunning[loop2][7]=false;
        cpan[loop2]=chanpan[loop2];
      }
      // finepitch
      if ((instrument[Mins[loop2]].activeEnv&64)>>6) {
      finepitch[loop2]=(signed char)bytable[6][instrument[Mins[loop2]].env[envHiPitch]][0];
      cfreq[loop2]=ProcessPitch(loop2,0);
      EnvelopesRunning[loop2][6]=true;}
      else {EnvelopesRunning[loop2][6]=false;finepitch[loop2]=0;}
      // pitch
      if ((instrument[Mins[loop2]].activeEnv&32)>>5) {
      // output (pitch logic)
      if (bytable[5][instrument[Mins[loop2]].env[envPitch]][0]<0x40) {
      cfreq[loop2]=mnoteperiod(curnote[loop2]+(bytable[5][instrument[Mins[loop2]].env[envPitch]][0])+(((unsigned char)instrument[Mins[loop2]].noteOffset)-47),loop2);
      } else {
      if (bytable[5][instrument[Mins[loop2]].env[envPitch]][0]<0x80) {
      cfreq[loop2]=mnoteperiod(curnote[loop2]-((bytable[5][instrument[Mins[loop2]].env[envPitch]][0]-64))+(((unsigned char)instrument[Mins[loop2]].noteOffset)-47),loop2);
      } else {
      cfreq[loop2]=mnoteperiod(bytable[5][instrument[Mins[loop2]].env[envPitch]][0]-128,loop2);
      }}
      EnvelopesRunning[loop2][5]=true;}
      else {EnvelopesRunning[loop2][5]=false;}
      }
      if (nfxid[loop2]!=7 && nfxid[loop2]!=12 && !(nvolu[loop2]>192 && nvolu[loop2]<203) && !((instrument[Mins[loop2]].activeEnv&32)>>5) && !((instrument[Mins[loop2]].activeEnv&64)>>6)) {
      cfreq[loop2]=mnoteperiod(((hscale(curnote[loop2])%16)+((hscale(curnote[loop2])>>4)*12))+(((unsigned char)instrument[Mins[loop2]].noteOffset)-48),loop2);
      }
      // only reset envelope positions if effect isn't Gxx/Lxx/gxx
      if (nfxid[loop2]!=7 && nfxid[loop2]!=12 && !(nvolu[loop2]>192 && nvolu[loop2]<203)) {
      // check for Oxx effect
      if (nfxid[loop2]!=15) {
      inspos[loop2][0]=0; // sets the instrument position to 0
      inspos[loop2][1]=0; // sets the instrument position to 0
      inspos[loop2][2]=0; // sets the instrument position to 0
      inspos[loop2][3]=0; // sets the instrument position to 0
      inspos[loop2][4]=0; // sets the instrument position to 0
      inspos[loop2][5]=0; // sets the instrument position to 0
      inspos[loop2][6]=0; // sets the instrument position to 0
      inspos[loop2][7]=0; // sets the instrument position to 0
      } else { // Oxx
      inspos[loop2][0]=nfxvl[loop2]; // sets the instrument position to 0
      inspos[loop2][1]=(int)((float)bytable[1][instrument[Mins[loop2]].env[envCutoff]][253]*((float)nfxvl[loop2]/256)); // sets the instrument position to 0
      inspos[loop2][2]=nfxvl[loop2]; // sets the instrument position to 0
      inspos[loop2][3]=nfxvl[loop2]; // sets the instrument position to 0
      inspos[loop2][4]=0; // sets the instrument position to 0
      inspos[loop2][5]=0; // sets the instrument position to 0
      inspos[loop2][6]=0; // sets the instrument position to 0
      inspos[loop2][7]=nfxvl[loop2]; // sets the instrument position to 0
      }
      // set In-Release-Point (IRP) flags to false
      IRP[loop2][0]=0;
      IRP[loop2][1]=0;
      IRP[loop2][2]=0;
      IRP[loop2][3]=0;
      IRP[loop2][4]=0;
      IRP[loop2][5]=0;
      IRP[loop2][6]=0;
      IRP[loop2][7]=0;
      // note is not released yet
      released[loop2]=false;
      // reset vibrato position
      curvibpos[loop2]=0;
      }
      continue;
     }}
    // volume
    if (EnvelopesRunning[loop2][0]) {
      UpdateEnvelope(loop2,0);
      cvol[loop2]=((bytable[0][instrument[Mins[loop2]].env[envVolume]][inspos[loop2][0]]/2)*(Mvol[loop2]))/127;
    }
    // duty
    if (EnvelopesRunning[loop2][3]) {
      UpdateEnvelope(loop2,3);
      // output
      cduty[loop2]=bytable[3][instrument[Mins[loop2]].env[envDuty]][inspos[loop2][3]]/2;
    }
    // shape
    if (EnvelopesRunning[loop2][4]) {
      UpdateEnvelope(loop2,4);
      // output
      cshape[loop2]=bytable[4][instrument[Mins[loop2]].env[envShape]][inspos[loop2][4]]/32;
    }
    // cutoff
    if (EnvelopesRunning[loop2][1]) {
      UpdateEnvelope(loop2,1);
      // output
      coff[loop2]=(bytable[1][instrument[Mins[loop2]].env[envCutoff]][inspos[loop2][1]]*(0xffff-instrument[Mins[loop2]].filterH))>>7;
    }
    // resonance
    if (EnvelopesRunning[loop2][2]) {
      UpdateEnvelope(loop2,2);
      // output
      creso[loop2]=bytable[2][instrument[Mins[loop2]].env[envReson]][inspos[loop2][2]];
    }
    // panning
    if (EnvelopesRunning[loop2][7]) {
      UpdateEnvelope(loop2,7);
      // output
      cpan[loop2]=bytable[7][instrument[Mins[loop2]].env[envPan]][inspos[loop2][7]]-128;
    }
    // pitch
    if (EnvelopesRunning[loop2][5]) {
      UpdateEnvelope(loop2,5);
      // output (pitch logic)
      if (bytable[5][instrument[Mins[loop2]].env[envPitch]][inspos[loop2][5]]<0x40) {
      cfreq[loop2]=mnoteperiod(curnote[loop2]+(bytable[5][instrument[Mins[loop2]].env[envPitch]][inspos[loop2][5]])+(((unsigned char)instrument[Mins[loop2]].noteOffset)-47),loop2);
      } else {
      if (bytable[5][instrument[Mins[loop2]].env[envPitch]][inspos[loop2][5]]<0x80) {
      cfreq[loop2]=mnoteperiod(curnote[loop2]-((bytable[5][instrument[Mins[loop2]].env[envPitch]][inspos[loop2][5]]-64))+(((unsigned char)instrument[Mins[loop2]].noteOffset)-47),loop2);
      } else {
      cfreq[loop2]=mnoteperiod(bytable[5][instrument[Mins[loop2]].env[envPitch]][inspos[loop2][5]]-128,loop2);
      }}
    }
    // finepitch
    if (EnvelopesRunning[loop2][6]) {
      UpdateEnvelope(loop2,6);
      // output
      if (EnvelopesRunning[loop2][6]) {
      finepitch[loop2]+=(signed char)bytable[6][instrument[Mins[loop2]].env[envHiPitch]][inspos[loop2][6]];
      cfreq[loop2]=ProcessPitch(loop2,0);
      }
    }
  }
  // run effects
  for (int looper=0;looper<32;looper++) {
  // Bxx
  if (nfxid[looper]==2) {
    if (curtick==0) {
      curpat=nfxvl[looper];
      curstep=-1;
      for (int l=0; l<32; l++) {
        // check for Cxx's
        if (nfxid[l]==3) {
          curstep=nfxvl[l]-1;
        }
      }
      NextRow();
      
    }
  }
  // Cxx
  if (nfxid[looper]==3)
    {if (curtick==0) {SkipPattern(looper);}}
  // Dxx
  if (nfxid[looper]==4 || nfxid[looper]==11 || nfxid[looper]==12)
    {if (EnvelopesRunning[looper][0]) {
    if (slidememory[looper]<0x10) {Mvol[looper]-=(slidememory[looper]*2);
    if (Mvol[looper]<0) {Mvol[looper]=0;}} else {if ((slidememory[looper]%16)!=0x0f && slidememory[looper]<0xf1) {Mvol[looper]+=((slidememory[looper]>>4)*2);if (Mvol[looper]>127) {Mvol[looper]=127;}}}}
    else {
    if (slidememory[looper]<0x10) {cvol[looper]-=(slidememory[looper]*2);
    if (cvol[looper]<0) {cvol[looper]=0;}} else {if ((slidememory[looper]%16)!=0x0f && slidememory[looper]<0xf1) {cvol[looper]+=((slidememory[looper]>>4)*2);if (cvol[looper]>127) {cvol[looper]=127;}}}}}
  // Nxx
  if (nfxid[looper]==14)
    {if (chanslidememory[looper]<0x10) {chanvol[looper]-=(chanslidememory[looper]*2);
    if (chanvol[looper]<0) {chanvol[looper]=0;}} else {if ((chanslidememory[looper]%16)!=0x0f && chanslidememory[looper]<0xf1) {chanvol[looper]+=((chanslidememory[looper]>>4)*2);if (chanvol[looper]>127) {chanvol[looper]=127;}}}}
  // Exx
  if (nfxid[looper]==5)
    {if (nfxvl[looper]<0xe0) {
      if (!linearslides) {cfreq[looper]+=nfxvl[looper];}
      else {curnote[looper]-=((float)nfxvl[looper]/16);
          if (!EnvelopesRunning[looper][5]) {
            cfreq[looper]=mnoteperiod(curnote[looper]+((unsigned char)instrument[Mins[looper]].noteOffset)-47,looper)+(int)((float)sine[(curvibpos[looper]*curvibspeed[looper]*4)%256]*((float)curvibdepth[looper]/16));
            } else {
            cfreq[looper]=ProcessPitch(looper,0)+(int)((float)sine[(curvibpos[looper]*curvibspeed[looper]*4)%256]*((float)curvibdepth[looper]/16));
            }}}}
  // Fxx
  if (nfxid[looper]==6)
    {if (nfxvl[looper]<0xe0) {
      if (!linearslides) {cfreq[looper]-=nfxvl[looper];}
      else {curnote[looper]+=((float)nfxvl[looper]/16);
          cfreq[looper]=mnoteperiod(curnote[looper]+1,looper);
          crmfreq[looper]=msnoteperiod(curnote[looper]+1+((instrument[Mins[looper]].LFO<0x40)?(instrument[Mins[looper]].LFO):(64-instrument[Mins[looper]].LFO)),looper);
          if (!EnvelopesRunning[looper][5]) {
            cfreq[looper]=mnoteperiod(curnote[looper]+((unsigned char)instrument[Mins[looper]].noteOffset)-47,looper)+(int)((float)sine[(curvibpos[looper]*curvibspeed[looper]*4)%256]*((float)curvibdepth[looper]/16));
            } else {
            cfreq[looper]=ProcessPitch(looper,0)+(int)((float)sine[(curvibpos[looper]*curvibspeed[looper]*4)%256]*((float)curvibdepth[looper]/16));
            }}}}
  // Gxx
  if (nfxid[looper]==7 || nfxid[looper]==12 || (nvolu[looper]>192 && nvolu[looper]<203))
  {if (nfxvl[looper]!=0 && nfxid[looper]!=12 && !(nvolu[looper]>193 && nvolu[looper]<203)) {Mport[looper]=nfxvl[looper];} // portamento memory
  if (nvolu[looper]>193 && nvolu[looper]<203) {
    switch (nvolu[looper]) {
      case 194: Mport[looper]=1; break;
      case 195: Mport[looper]=4; break;
      case 196: Mport[looper]=8; break;
      case 197: Mport[looper]=16; break;
      case 198: Mport[looper]=32; break;
      case 199: Mport[looper]=64; break;
      case 200: Mport[looper]=96; break;
      case 201: Mport[looper]=128; break;
      case 202: Mport[looper]=255; break;
  }}
    if (!linearslides) {
    if (cfreq[looper]>mnoteperiod(portastatic[looper],looper)) {
      cfreq[looper]=maxval(cfreq[looper]-(Mport[looper]*4),mnoteperiod(portastatic[looper],looper));
    } else {if (cfreq[looper]<mnoteperiod(portastatic[looper],looper)) {
      cfreq[looper]=minval(cfreq[looper]+(Mport[looper]*4),mnoteperiod(portastatic[looper],looper));
    }}
    } else {
    if (curnote[looper]>(portastatic[looper])) {
      curnote[looper]=maxval(curnote[looper]-((float)Mport[looper]/16),portastatic[looper]-1);
      cfreq[looper]=ProcessPitch(looper,0);
    } else {if (curnote[looper]<(portastatic[looper])) {
      curnote[looper]=minval(curnote[looper]+((float)Mport[looper]/16),portastatic[looper]-1);
      cfreq[looper]=ProcessPitch(looper,0);
    } else {curnote[looper]=portastatic[looper]-1;
      cfreq[looper]=ProcessPitch(looper,0);}}
      crmfreq[looper]=msnoteperiod(curnote[looper]+1+((instrument[Mins[looper]].LFO<0x40)?(instrument[Mins[looper]].LFO):(64-instrument[Mins[looper]].LFO)),looper);
    }
  }
  // Hxx
  if (nfxid[looper]==8 || nfxid[looper]==11) {
    if (nfxid[looper]!=11) {
    if ((nfxvl[looper]%16)!=0) {curvibdepth[looper]=nfxvl[looper]%16;}
    if ((nfxvl[looper]/16)!=0) {curvibspeed[looper]=nfxvl[looper]/16;}
    }
    curvibpos[looper]++;
    switch (curvibshape[looper]) {
    case 0: cfreq[looper]=ProcessPitch(looper,-(float)(sin(((float)curvibpos[looper]*(float)curvibspeed[looper]*4/256)*2*M_PI))*((float)curvibdepth[looper]/8)); break; // sine
    case 1: cfreq[looper]=ProcessPitch(looper,0)+
    (int)((float)((((curvibpos[looper]*curvibspeed[looper]*4)%256)/2)-64)*((float)curvibdepth[looper]/16)); break; // saw
    case 2: cfreq[looper]=ProcessPitch(looper,0)+
    (int)((float)((((curvibpos[looper]*curvibspeed[looper]*4)%256)>127)?(64):(-64))*((float)curvibdepth[looper]/16)); break; // square
    case 3: cfreq[looper]=ProcessPitch(looper,0)+
    (int)((float)((rand()%128)-64)*((float)curvibdepth[looper]/16)); break; // random
    }
  }
  // Jxx
  if (nfxid[looper]==10) {
    // cycle between the 3 frequencies
    switch ((speed-curtick)%3) {
      case 0: cfreq[looper]=mnoteperiod(curnote[looper]+((unsigned char)instrument[Mins[looper]].noteOffset)-47,looper); break;
      case 1: cfreq[looper]=mnoteperiod(curnote[looper]+(nfxvl[looper]/16)+(((unsigned char)instrument[Mins[looper]].noteOffset)-47),looper); break;
      case 2: cfreq[looper]=mnoteperiod(curnote[looper]+(nfxvl[looper]%16)+(((unsigned char)instrument[Mins[looper]].noteOffset)-47),looper); break;
    }
  }
  // Kxx
  /*if (nfxid[looper]==11) {
    if (EnvelopesRunning[looper][0]) {
    if (nfxvl[looper]<0x10) {Mvol[looper]-=(nfxvl[looper]*2);
    if (Mvol[looper]<0) {Mvol[looper]=0;}} else {Mvol[looper]+=((nfxvl[looper]>>4)*2);if (Mvol[looper]>127) {Mvol[looper]=127;}}}
    else {
    if (nfxvl[looper]<0x10) {cvol[looper]-=(nfxvl[looper]*2);
    if (cvol[looper]<0) {cvol[looper]=0;}} else {cvol[looper]+=((nfxvl[looper]>>4)*2);if (cvol[looper]>127) {cvol[looper]=127;}}}
    curvibpos[looper]++;
    if (!EnvelopesRunning[looper][5]) {
    cfreq[looper]=mnoteperiod(curnote[looper]+((unsigned char)instrument[Mins[looper]].noteOffset)-47)+(int)((float)sine[(curvibpos[looper]*curvibspeed[looper]*4)%256]*((float)curvibdepth[looper]/16));
    } else {
    cfreq[looper]=ProcessPitch(looper)+(int)((float)sine[(curvibpos[looper]*curvibspeed[looper]*4)%256]*((float)curvibdepth[looper]/16));
    }
  }*/
  // SCx
  cutcount[looper]--;
  if (cutcount[looper]==0) {
    if (EnvelopesRunning[looper][0]) {Mvol[looper]=0;} else {cvol[looper]=0;}
  }
  if (cutcount[looper]<-1) {cutcount[looper]=-1;}
  /*if (nfxid[looper]==19)
    {if ((nfxvl[looper]>>4)==0x0c) {cutcount[looper]--;
      if (cutcount[looper]==0) {
        if (EnvelopesRunning[looper][0]) {Mvol[looper]=0;} else {cvol[looper]=0;}
      }}}*/
  // Uxx
  if (nfxid[looper]==21) {
    if ((nfxvl[looper]%16)!=0) {curvibdepth[looper]=nfxvl[looper]%16;}
    if ((nfxvl[looper]/16)!=0) {curvibspeed[looper]=nfxvl[looper]/16;}
    curvibpos[looper]++;
    cfreq[looper]=mnoteperiod(curnote[looper]+((unsigned char)instrument[Mins[looper]].noteOffset)-47,looper)+(int)((float)sine[(curvibpos[looper]*curvibspeed[looper]*4)%256]*((float)curvibdepth[looper]/64));
  }
  // Yxx
  if (nfxid[looper]==25) {
    if ((nfxvl[looper]%16)!=0) {curpandepth[looper]=nfxvl[looper]%16;}
    if ((nfxvl[looper]/16)!=0) {curpanspeed[looper]=nfxvl[looper]/16;}
    curpanpos[looper]++;
    cpan[looper]=(signed char)((float)sine[(curpanpos[looper]*curpanspeed[looper]*4)%256]*((float)curpandepth[looper]/16));
  }
  }
  AlreadySkipped=false;
  if (curtick==0) { // is tick counter 0?
    if (playmode==1 || playmode==4) {NextRow();} else {curtick=1;}
  }
}
void Playback() {
  NextTick();
  for (int i=0; i<8*(1+((channels-1)>>3)); i++) {
    if (sfxplaying && i==chantoplayfx) continue;
    chip[i>>3].chan[i&7].vol=(cvol[i]*chanvol[i])>>7;
    chip[i>>3].chan[i&7].pan=cpan[i];
          if (cshape[i]==4) {
            chip[i>>3].chan[i&7].freq=minval(cfreq[i]>>2,0xffff);
          } else {
            chip[i>>3].chan[i&7].freq=cfreq[i];
          }
          chip[i>>3].chan[i&7].restimer=crmfreq[i];
    chip[i>>3].chan[i&7].flags.fmode=cfmode[i];
          chip[i>>3].chan[i&7].flags.shape=cshape[i];
          chip[i>>3].chan[i&7].flags.ring=crm[i];
          chip[i>>3].chan[i&7].flags.resosc=crstep[i];
          crstep[i]=0;
          chip[i>>3].chan[i&7].flags.restim=(crm[i]&2)?(1):(0);
    chip[i>>3].chan[i&7].duty=cduty[i];
    chip[i>>3].chan[i&7].cutoff=coff[i]/4;
    chip[i>>3].chan[i&7].reson=creso[i];
  }
}

void CleanupPatterns() {
  // patterns
  memset(pat,0,256*256*32*5);
  // pattern details
  memset(patlength,0x40,256);
  memset(patid,0,256);
  // envelopes
  memset(bytable,0,8*256*256);
  for (int kk=0;kk<8;kk++) {
    for (int jj=0;jj<256;jj++) {
      bytable[kk][jj][254]=255;
      bytable[kk][jj][255]=255;
    }
  }
  // instruments
  memset(instrument,0,256*64);
  memset(&blankIns,0,64);
  for (int jj=0;jj<256;jj++) {
    instrument[jj].noteOffset=48;
    instrument[jj].vol=64;
  }
  blankIns.noteOffset=48;
  blankIns.vol=64;
  // default vol/pan
  for (int j=0; j<32; j++) {
    defchanvol[j]=0x80;
    defchanpan[j]=((j+1)&2)?96:-96;
  }
  // default variables
  defspeed=6;
  songdf=0;
  channels=8;
  songlength=255;
  origin="Unknown";
}

Color mapHSV(float hue,float saturation,float value) {
  float c=value*saturation;
  float x=c*(1-fabs(fmod(hue/60,2)-1));
  float m=value-c;
  float r=0,gR=0,b=0;
  if (hue<60) {r=c;gR=x;b=0;}
  else if (hue<120) {r=x;gR=c;b=0;}
  else if (hue<180) {r=0;gR=c;b=x;}
  else if (hue<240) {r=0;gR=x;b=c;}
  else if (hue<300) {r=x;gR=0;b=c;}
  else if (hue<360) {r=c;gR=0;b=x;}
  return g._WRAP_map_rgba_f(r+m,gR+m,b+m,64);
}

void inputCursor(float x, float y1, float y2) {
  size_t ad=utf8cpos(inputvar->c_str(),inputcurpos);
  g._WRAP_draw_line(x+(ad*8),y1,x+(ad*8),y2,g._WRAP_map_rgb(255,255,0),1);
}

void drawpatterns(bool force) {
  if (playermode) {return;}
  if (follow) {curpatrow=curstep; if (curpatrow<0) curpatrow=0;}
  //curpatrow=0;
  // will be replaced
  if ((!UPDATEPATTERNS || playmode==0 || playmode==1) && !force && oldpat==curpat) {oldpat=curpat;return;}
  oldpat=curpat;
  UPDATEPATTERNS=true;
  g._WRAP_destroy_bitmap(patternbitmap);
  patternbitmap=g._WRAP_create_bitmap(24+chanstodisplay*96,(((patlength[patid[curpat]]==0)?(256):(patlength[patid[curpat]]))*12)+4);
  //popbox=PopupBox("Bitmap Size","bitmap size: "+std::to_string(24+chanstodisplay*96)+"x"+std::to_string((((patlength[patid[curpat]]==0)?(256):(patlength[patid[curpat]]))*12)+4));
  g.setTarget(patternbitmap);
  g._WRAP_clear_to_color(g._WRAP_map_rgb(0,0,0));
  g._WRAP_draw_filled_rectangle(0,60,scrW,scrH,g._WRAP_map_rgb(0,0,0));
  for (int i=0;i<getpatlen(patid[curpat]);i++) {
    //if (i>curpatrow+15+((scrH-450)/12)) {continue;}
    //if (i<curpatrow-16) {continue;}
    g.tColor(8);
    g.tPos(0,i);
    g.printf("%.2X",i);
    // channel drawing routine, replicated 8 times
    for (int j=0;j<chanstodisplay;j++) {
      g.tColor(15);
      g.printf("|");
      if (pat[patid[curpat]][i][j+curedpage][0]==0 &&
          pat[patid[curpat]][i][j+curedpage][1]==0 &&
          pat[patid[curpat]][i][j+curedpage][2]==0 &&
          pat[patid[curpat]][i][j+curedpage][3]==0 &&
          pat[patid[curpat]][i][j+curedpage][4]==0) {
        g.tColor(245);
        g.printf("...........");
        continue;
      }
      // note
      g.tColor(250);
      g.printf("%s%s",getnote(pat[patid[curpat]][i][j+curedpage][0]),getoctave(pat[patid[curpat]][i][j+curedpage][0]));
      g.tColor(81);
      g.printf("%c%c",getinsH(pat[patid[curpat]][i][j+curedpage][1]),getinsL(pat[patid[curpat]][i][j+curedpage][1]));
      // instrument
      if (pat[patid[curpat]][i][j+curedpage][2]==0 && pat[patid[curpat]][i][j+curedpage][0]!=0) {
        g.printf("v%.2X",instrument[pat[patid[curpat]][i][j+curedpage][1]].vol);
      } else {
        g.tColor(getVFXColor(pat[patid[curpat]][i][j+curedpage][2]));
        g.printf("%s%c%c",getVFX(pat[patid[curpat]][i][j+curedpage][2]),getVFXH(pat[patid[curpat]][i][j+curedpage][2]),getVFXL(pat[patid[curpat]][i][j+curedpage][2]));
      }
      // effect
      g.tColor(GetFXColor(pat[patid[curpat]][i][j+curedpage][3]));
      g.printf("%s%c%c",getFX(pat[patid[curpat]][i][j+curedpage][3]),getinsH(pat[patid[curpat]][i][j+curedpage][4]),getinsL(pat[patid[curpat]][i][j+curedpage][4]));
    }
    g.tColor(15);
    g.printf("|");
  }
  g.resetTarget();
}

void drawinsedit() {
  // draws the instrument edit dialog
  g.tPos(0,5);
  g.tColor(15);
  g.printf("Instrument Editor|INS   ^v|+|-|Save|Load|                   |HEX|\n");
  g.printf("  |Volume|Cutoff|Reson|Duty|Shape|Pitch|HiPitch|Pan|Seq  ^v|NF|X|\n");

  g.tNLPos((float)(scrW-272)/8.0);
  g.tPos((float)(scrW-272)/8.0,7); g.printf("RelNote    ^v+-\n");
  g.printf("VibType sin|squ|saw|\n");
  g.printf("TrmType sin|squ|saw|tri\n\n");
  
  g.printf("________________________________\n\n");
  
  g.printf("Filter low|high|band\n\n");
  
  g.printf("PCM|pos $     |length $    |\n");
  g.printf("   |loop$     |seekmult x00|\n\n");
  
  g.printf("filterH \n\n\n");
  
  
  g.printf("RM|freq   ^v+-|shape squ^v|\n");
  g.printf("Sy|duty 00^v+-|\n\n");
  
  g.printf("ResetOsc|ResetFilter|\n");
  g.printf("ResetRMOsc|\n\n");
  
  g.printf("AutoCut  ^v\n");
  g.printf("DefVol   ^v\n");
  g.printf("Pitch    ^v\n");

  g.tNLPos(0);
  g.tPos(0,(float)(scrH-18)/12);
  g.printf("< Loop   ^v+- Release   ^v+- Length   ^v+-                     >");
  
  g._WRAP_draw_rectangle(0,94,516+(scrW-800),scrH-34,g._WRAP_map_rgb(255,255,255),1);
  // draws envelope waveform
  if (!hexmode) {
  for (int ii=scrollpos;ii<minval(253,(516/valuewidth)+scrollpos+((scrW-800)/valuewidth));ii++) {
  if (CurrentEnv==5) { // pitch
    if (bytable[5][instrument[CurrentIns].env[CurrentEnv]][ii]!=0) {
    if (bytable[5][instrument[CurrentIns].env[CurrentEnv]][ii]<0x40) {
      // up
      g._WRAP_draw_pixel((ii-scrollpos)*valuewidth,145,g._WRAP_map_rgb(128,255,128));
      g._WRAP_draw_pixel((ii-scrollpos)*valuewidth,146,g._WRAP_map_rgb(128,255,128));
      g._WRAP_draw_pixel((ii-scrollpos)*valuewidth,147,g._WRAP_map_rgb(128,255,128));
      g._WRAP_draw_pixel((ii-scrollpos)*valuewidth,148,g._WRAP_map_rgb(128,255,128));
      g._WRAP_draw_pixel((ii-scrollpos)*valuewidth,149,g._WRAP_map_rgb(128,255,128));
      g._WRAP_draw_pixel(1+((ii-scrollpos)*valuewidth),149,g._WRAP_map_rgb(128,255,128));
      g._WRAP_draw_pixel(2+((ii-scrollpos)*valuewidth),149,g._WRAP_map_rgb(128,255,128));
      g._WRAP_draw_pixel(2+((ii-scrollpos)*valuewidth),148,g._WRAP_map_rgb(128,255,128));
      g._WRAP_draw_pixel(2+((ii-scrollpos)*valuewidth),147,g._WRAP_map_rgb(128,255,128));
      g._WRAP_draw_pixel(2+((ii-scrollpos)*valuewidth),146,g._WRAP_map_rgb(128,255,128));
      g._WRAP_draw_pixel(2+((ii-scrollpos)*valuewidth),145,g._WRAP_map_rgb(128,255,128));
      } else {
      if (bytable[5][instrument[CurrentIns].env[CurrentEnv]][ii]<0x80) {
      // down
      g._WRAP_draw_pixel((ii-scrollpos)*valuewidth,145,g._WRAP_map_rgb(255,128,128));
      g._WRAP_draw_pixel((ii-scrollpos)*valuewidth,146,g._WRAP_map_rgb(255,128,128));
      g._WRAP_draw_pixel((ii-scrollpos)*valuewidth,147,g._WRAP_map_rgb(255,128,128));
      g._WRAP_draw_pixel((ii-scrollpos)*valuewidth,148,g._WRAP_map_rgb(255,128,128));
      g._WRAP_draw_pixel((ii-scrollpos)*valuewidth,149,g._WRAP_map_rgb(255,128,128));
      g._WRAP_draw_pixel(1+((ii-scrollpos)*valuewidth),149,g._WRAP_map_rgb(255,128,128));
      g._WRAP_draw_pixel(2+((ii-scrollpos)*valuewidth),148,g._WRAP_map_rgb(255,128,128));
      g._WRAP_draw_pixel(2+((ii-scrollpos)*valuewidth),147,g._WRAP_map_rgb(255,128,128));
      g._WRAP_draw_pixel(2+((ii-scrollpos)*valuewidth),146,g._WRAP_map_rgb(255,128,128));
      g._WRAP_draw_pixel(1+((ii-scrollpos)*valuewidth),145,g._WRAP_map_rgb(255,128,128));
      } else {
      // absolute
      g._WRAP_draw_pixel((ii-scrollpos)*valuewidth,146,g._WRAP_map_rgb(255,255,128));
      g._WRAP_draw_pixel((ii-scrollpos)*valuewidth,147,g._WRAP_map_rgb(255,255,128));
      g._WRAP_draw_pixel((ii-scrollpos)*valuewidth,148,g._WRAP_map_rgb(255,255,128));
      g._WRAP_draw_pixel((ii-scrollpos)*valuewidth,149,g._WRAP_map_rgb(255,255,128));
      g._WRAP_draw_pixel(1+((ii-scrollpos)*valuewidth),145,g._WRAP_map_rgb(255,255,128));
      g._WRAP_draw_pixel(2+((ii-scrollpos)*valuewidth),146,g._WRAP_map_rgb(255,255,128));
      g._WRAP_draw_pixel(2+((ii-scrollpos)*valuewidth),147,g._WRAP_map_rgb(255,255,128));
      g._WRAP_draw_pixel(2+((ii-scrollpos)*valuewidth),148,g._WRAP_map_rgb(255,255,128));
      g._WRAP_draw_pixel(2+((ii-scrollpos)*valuewidth),149,g._WRAP_map_rgb(255,255,128));
      g._WRAP_draw_pixel(1+((ii-scrollpos)*valuewidth),147,g._WRAP_map_rgb(255,255,128));
      }
    }}
  }
  g._WRAP_draw_filled_rectangle((ii-scrollpos)*valuewidth,
    (CurrentEnv==7)?402-bytable[CurrentEnv][instrument[CurrentIns].env[CurrentEnv]][ii]:
    (CurrentEnv==6)?interpolate(94,scrH-42,0.5):(scrH-42),
    ((ii-scrollpos)*valuewidth)+valuewidth,
    (CurrentEnv==6)?interpolate(94,scrH-42,0.5)-(signed char)bytable[CurrentEnv][instrument[CurrentIns].env[CurrentEnv]][ii]:
    (scrH-43)-(bytable[CurrentEnv][instrument[CurrentIns].env[CurrentEnv]][ii]*((scrH-128)/256)),
    (ii>bytable[CurrentEnv][instrument[CurrentIns].env[CurrentEnv]][253])?g._WRAP_map_rgb(63,63,63): // out of range
    (ii>bytable[CurrentEnv][instrument[CurrentIns].env[CurrentEnv]][255])?g._WRAP_map_rgb(192,0,192): // release
    (ii>=bytable[CurrentEnv][instrument[CurrentIns].env[CurrentEnv]][254])?g._WRAP_map_rgb(0,192,192): // loop
    g._WRAP_map_rgb(255,255,255));
  }
  for (int jj=0;jj<32;jj++) {
  if (EnvelopesRunning[jj][CurrentEnv] && Mins[jj]==CurrentIns) {
    g._WRAP_draw_filled_rectangle((inspos[jj][CurrentEnv]-scrollpos)*valuewidth,
      (CurrentEnv==7)?402-bytable[CurrentEnv][instrument[CurrentIns].env[CurrentEnv]][inspos[jj][CurrentEnv]]:
    (CurrentEnv==6)?interpolate(94,scrH-42,0.5):(scrH-42),
    ((inspos[jj][CurrentEnv]-scrollpos)*valuewidth)+valuewidth,
    (CurrentEnv==6)?interpolate(94,scrH-42,0.5)-(signed char)bytable[CurrentEnv][instrument[CurrentIns].env[CurrentEnv]][inspos[jj][CurrentEnv]]:
    (scrH-43)-(bytable[CurrentEnv][instrument[CurrentIns].env[CurrentEnv]][inspos[jj][CurrentEnv]]*((scrH-128)/256)),
    g._WRAP_map_rgb(0,255,0)); continue;
  }
  }
  } else {
  for (int ii=0;ii<253;ii++) {
    g.tColor((ii>bytable[CurrentEnv][instrument[CurrentIns].env[CurrentEnv]][253])?240:15);
    g.tPos(1+(ii%21)*3,8.5+(ii/21)*2);
    g.printf("%.2x",bytable[CurrentEnv][instrument[CurrentIns].env[CurrentEnv]][ii]);
  }
  }
  // draws some GUI stuff
  g.tPos((float)(scrW-272)/8.0,11);
  g.tColor((inputwhere==2)?11:15);
  g.printf("%s",instrument[CurrentIns].name);
  if (inputwhere==2) {
    inputCursor((scrW-272),133,145);
  }
  
  g.tPos(55,6);
  g.tColor(10);
  g.printf("%.2X",instrument[CurrentIns].env[CurrentEnv]);
  
  g.tPos(22,5);
  g.printf("%.2X",CurrentIns);
  
  // the right pane
  g.tColor(11);
  g.tPos((float)(scrW-208)/8.0,7);
  g.printf("%s%s\n",getnotetransp(instrument[CurrentIns].noteOffset),getoctavetransp(instrument[CurrentIns].noteOffset));
  // the thing at the bottom
  g.tPos(7,(float)(scrH-18)/12.0);
  g.printf("%.2X\n",bytable[CurrentEnv][instrument[CurrentIns].env[CurrentEnv]][254]);
  g.tPos(22,(float)(scrH-18)/12.0);
  g.printf("%.2X\n",bytable[CurrentEnv][instrument[CurrentIns].env[CurrentEnv]][255]);
  g.tPos(36,(float)(scrH-18)/12.0);
  g.printf("%.2X\n",bytable[CurrentEnv][instrument[CurrentIns].env[CurrentEnv]][253]);
  
  g.tPos(0,6);
  if (((instrument[CurrentIns].activeEnv)>>CurrentEnv)&1) {
    g.tColor(10);
    g.printf("ON");
  } else {
    g.tColor(9);
    g.printf("NO");
  }
  
  g.tColor(10);
  switch (CurrentEnv) {
    case 0:
      g.tPos(3,6);
      g.printf("Volume");
      break;
    case 1:
      g.tPos(10,6);
      g.printf("Cutoff");
      break;
    case 2:
      g.tPos(17,6);
      g.printf("Reson");
      break;
    case 3:
      g.tPos(23,6);
      g.printf("Duty");
      break;
    case 4:
      g.tPos(28,6);
      g.printf("Shape");
      break;
    case 5:
      g.tPos(34,6);
      g.printf("Pitch");
      break;
    case 6:
      g.tPos(40,6);
      g.printf("HiPitch");
      break;
    case 7:
      g.tPos(48,6);
      g.printf("Pan");
      break;
  }
  // the right pane is back
  
  g.tColor(11);
  if (instrument[CurrentIns].DFM&1) {
    g.tPos((float)(scrW-216)/8.0,13);
    g.printf("low");
  }
  if (instrument[CurrentIns].DFM&2) {
    g.tPos((float)(scrW-184)/8.0,13);
    g.printf("high");
  }
  if (instrument[CurrentIns].DFM&4) {
    g.tPos((float)(scrW-144)/8.0,13);
    g.printf("band");
  }
  if (instrument[CurrentIns].DFM&8) {
    g.tPos((float)(scrW-272)/8.0,15);
    g.printf("PCM");
  }
  if (instrument[CurrentIns].DFM&16) {
    g.tPos((float)(scrW-272)/8.0,21);
    g.printf("RM");
  }
  if (instrument[CurrentIns].flags&32) {
    g.tPos((float)(scrW-272)/8.0,22);
    g.printf("Sy");
  }
  if (instrument[CurrentIns].flags&1) {
    g.tPos((float)(scrW-272)/8.0,24);
    g.printf("ResetOsc");
  }
  
  g.tPos((float)(scrW-192)/8.0,15);
  g.printf("%.2x%.2x",instrument[CurrentIns].pcmPos[1],instrument[CurrentIns].pcmPos[0]);
  
  g.tPos((float)(scrW-192)/8.0,16);
  g.printf("%.2x%.2x",instrument[CurrentIns].pcmLoop[1],instrument[CurrentIns].pcmLoop[0]);
  
  g.tPos((float)(scrW-88)/8.0,15);
  g.printf("%.4x",instrument[CurrentIns].pcmLen);
  
  g.tPos((float)(scrW-208)/8.0,21);
  g.printf("%.2x",instrument[CurrentIns].LFO);
  
  g.tPos((float)(scrW-208)/8.0,18);
  g.printf("%.4x",0xffff-instrument[CurrentIns].filterH);
  
  g.tPos((float)(scrW-208)/8.0,27);
  g.printf("%.1x",instrument[CurrentIns].flags>>6);
  
  if (hexmode) {
    g.tPos(61,5);
    g.printf("HEX");
  }
  
  if (rightclick && PIR(0,90,515,420,mstate.x,mstate.y) && !hexmode) {
    g._WRAP_draw_circle((linex1/4)*4,liney1,4,g._WRAP_map_rgb(255,255,255),1);
    g._WRAP_draw_circle((mstate.x/4)*4,mstate.y,4,g._WRAP_map_rgb(255,255,255),1);
    g._WRAP_draw_line((linex1/4)*4,liney1,(mstate.x/4)*4,mstate.y,g._WRAP_map_rgb(255,255,255),1);
  }
}

void EditSkip() {
  // autoinstrument
  if (pat[patid[curpat]][curstep][curedpage+curedchan][0]!=0 && (pat[patid[curpat]][curstep][curedpage+curedchan][0]%16)<13) {
    pat[patid[curpat]][curstep][curedpage+curedchan][1]=curins;
  }
  // skipping
  if (playmode==0) {
    curtick=1;
    curstep++;
    if (curstep>(getpatlen(patid[curpat])-1)) {
      curstep=0;
      curpat++;
    }
    selStart=curstep;
    selEnd=curstep;
    curpatrow=curstep;
    curselchan=curedchan;
    curselmode=curedmode;
  }
  drawpatterns(true);
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
void drawmixerlayer() {
  g.setTarget(mixer);
  g._WRAP_clear_to_color(g._WRAP_map_rgba(0,0,0,0));
  int mixerdrawoffset=(scrW/2)-chanstodisplay*48-12;
  for (int chantodraw=0;chantodraw<chanstodisplay;chantodraw++) {
    g._WRAP_draw_line(12.5+(chantodraw*96)+mixerdrawoffset,60,12.5+(chantodraw*96)+mixerdrawoffset,scrH-0.5,g._WRAP_map_rgb(255,255,255),1);
    g.tColor(15);
    g.tNLPos(2+chantodraw*12+((float)mixerdrawoffset/8.0));
    g.tPos(5+2);
    g.printf("Vol  ^v\n");
    g.printf("Pan  ^v\n\n");
    g.printf("Freq\n");
    g.printf("Cut\n");
    g.printf("Ins  |Vol  \n");
    g.printf("EP0  |EP1  \n");
    g.printf("EP2  |EP3  \n");
    g.printf("EP4  |EP5  \n");
    g.printf("EP6  |EP7  \n");
    g.printf("Not  |Por  \n");
    g.printf("Vib  |Trm  \n");
    g.printf("Trr  |Pbr  ");
  }
  g.tNLPos(0);
  g._WRAP_draw_line(0,21.5+60,scrW,21.5+60,g._WRAP_map_rgb(255,255,255),1);
  g._WRAP_draw_line(0,57.5+60,scrW,57.5+60,g._WRAP_map_rgb(255,255,255),1);
  g._WRAP_draw_line(12.5+(chanstodisplay*96)+mixerdrawoffset,60,12.5+(chanstodisplay*96)+mixerdrawoffset,scrW-0.5,g._WRAP_map_rgb(255,255,255),1);
  g.resetTarget();
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

void drawmixer() {
  // draws the mixer dialog
  int mixerdrawoffset=(scrW/2)-chanstodisplay*48-12;
  g._WRAP_draw_bitmap(mixer,0,0,0);
  for (int chantodraw=0;chantodraw<chanstodisplay;chantodraw++) {
    if (midion[chantodraw+curedpage]) {
      g.tColor(9);
    } else {
      g.tColor(15);
    }
    g.tNLPos(2+chantodraw*12+((float)mixerdrawoffset/8.0));
    g.tPos(5);
    g.printf(" Channel ");
    if (!muted[chantodraw+curedpage]) {
      g.tColor(14);
    } else {
      g.tColor(8);
    }
    g.printf("%d\n\n",chantodraw+curedpage);

    g._WRAP_draw_filled_rectangle(16+(chantodraw*96)+mixerdrawoffset-1,scrH-4-1,58+(chantodraw*96)+mixerdrawoffset+1,(scrH-4)-(((float)cvol[chantodraw+curedpage]*((127-(maxval(0,(float)cpan[chantodraw+curedpage])))/127))*((scrH-244)/128))+1,g._WRAP_map_rgb(0,200,0));
    g._WRAP_draw_filled_rectangle(62+(chantodraw*96)+mixerdrawoffset-1,scrH-4-1,104+(chantodraw*96)+mixerdrawoffset+1,(scrH-4)-(((float)cvol[chantodraw+curedpage]*((128+(minval(0,(float)cpan[chantodraw+curedpage])))/128))*((scrH-244)/128))+1,g._WRAP_map_rgb(0,200,0));
    g._WRAP_draw_filled_rectangle(16+(chantodraw*96)+mixerdrawoffset,scrH-4,58+(chantodraw*96)+mixerdrawoffset,(scrH-4)-(((float)cvol[chantodraw+curedpage]*((127-(maxval(0,(float)cpan[chantodraw+curedpage])))/127))*((scrH-244)/128)),g._WRAP_map_rgb(0,255,0));
    g._WRAP_draw_filled_rectangle(62+(chantodraw*96)+mixerdrawoffset,scrH-4,104+(chantodraw*96)+mixerdrawoffset,(scrH-4)-(((float)cvol[chantodraw+curedpage]*((128+(minval(0,(float)cpan[chantodraw+curedpage])))/128))*((scrH-244)/128)),g._WRAP_map_rgb(0,255,0));
    
    g.tColor(14);
    g.printf("   %.2x",defchanvol[chantodraw+curedpage]);
    g.tColor(12);
    g.printf("   %.2x%c\n",cduty[chantodraw+curedpage]%256,shapeSym(cshape[chantodraw+curedpage]));
    g.tColor(14);
    g.printf("   %.2x   ",(unsigned char)defchanpan[chantodraw+curedpage]);
    switch (cfmode[chantodraw+curedpage]) {
      case 0: g.printf("   \n\n"); break;
      case 1: g.printf("  l\n\n"); break;
      case 2: g.printf(" h \n\n"); break;
      case 3: g.printf(" hl\n\n"); break;
      case 4: g.printf("b  \n\n"); break;
      case 5: g.printf("b l\n\n"); break;
      case 6: g.printf("bh \n\n"); break;
      case 7: g.printf("bhl\n\n"); break;
    }
    g.printf("      %.4x\n",cfreq[chantodraw+curedpage]);
    g.tColor(12);
    g.printf("    %.1f\n",(float)coff[chantodraw+curedpage]/10);

    g.tColor(14);
    g.printf("   %.2x",Mins[chantodraw+curedpage]);
    g.printf("    %.2x\n",Mvol[chantodraw+curedpage]);

    for (int j=0; j<8; j++) {
      g.tColor(getmixerposcol(chantodraw+curedpage,j));
      g.printf(j&1?"    %.2x\n":"   %.2x",inspos[chantodraw+curedpage][j]);
    }

    g.tColor(14);
    g.printf("   %.2x",(int)(curnote[chantodraw+curedpage])%256);
    g.printf("    %.2x",portastatic[chantodraw+curedpage]%256);

  }
  g.tNLPos(0);
}
void drawdiskop() {
  // draws the file dialog
  g.tPos(0,5);
  g.tColor(15);
  g.printf("Disk Operations| New|Open|Save                                      |LoadSample|LoadRawSample\n\n");
  g._WRAP_draw_line(0,80,scrW,80,g._WRAP_map_rgb(255,255,255),1);
  g.printf("FilePath\n\n");
  g._WRAP_draw_line(0,104,scrW,104,g._WRAP_map_rgb(255,255,255),1);
  g.tColor(7);
  g.printf("<go up>");
  if (iface!=UIMobile) {
    // TODO: we don't need this if there's nothing to scroll
    g.tPos((scrW/8.0f)-1,9);
    g.printf("^");
    g.tPos((scrW/8.0f)-1,((scrH-4)/12.0f)-3);
    g.printf("v");
    
    // draw scroll bar
    if ((filenames.size()-(int(scrH/12)-12))>0) {
      g._WRAP_draw_filled_rectangle(
        scrW-7,
        120+(scrH-130-38)*((float)diskopscrollpos/(float)(filenames.size()-(int(scrH/12)-12))),
        scrW-1,
        130+(scrH-130-38)*((float)diskopscrollpos/(float)(filenames.size()-(int(scrH/12)-12))),
        g._WRAP_map_rgb(160,160,160)
      );
    }
  }
  g.tColor(15);
  
  g.tPos(9,7);
  if (strcmp(curdir,"")==0) {
    g.printf("(?)");
  } else {
    g.printf("%s",curdir);
  }

  if (iface==UIMobile) {
    // more spacing
    if (selectedfileindex>(diskopscrollpos)) {
      g._WRAP_draw_filled_rectangle(0,111+16+((selectedfileindex-1-diskopscrollpos)*36),scrW,123+24+16+((selectedfileindex-1-diskopscrollpos)*36),g._WRAP_map_rgb(128,128,128));
    }
    for (int i=diskopscrollpos; i<minval(diskopscrollpos+((int(scrH/12)-9)/3),filenames.size()); i++) {
      g.tPos(1,11.25+(i-diskopscrollpos)*3);
      g.tColor(filenames[i].isdir?14:15);
      g.printf(filenames[i].name.c_str());
    }
  } else {
    if (selectedfileindex>(diskopscrollpos)) {
      g._WRAP_draw_filled_rectangle(0,111+((selectedfileindex-diskopscrollpos)*12),scrW,123+((selectedfileindex-diskopscrollpos)*12),g._WRAP_map_rgb(128,128,128));
    }
    for (int i=diskopscrollpos; i<minval(diskopscrollpos+(int(scrH/12)-9),filenames.size()); i++) {
      g.tPos(0,10+i-diskopscrollpos);
      g.tColor(filenames[i].isdir?14:15);
      g.printf(filenames[i].name.c_str());
    }
  }
  
  g._WRAP_draw_filled_rectangle(0,scrH-24,scrW,scrH,g._WRAP_map_rgb(0,0,0));
  g._WRAP_draw_line(0,scrH-24,scrW,scrH-24,g._WRAP_map_rgb(255,255,255),1);
  g.tPos(0,((scrH-8.0f)/12.0f)-1.0f);
  g.tColor(15);
  g.printf("FileName");
  g.tPos(9,((scrH-8.0f)/12.0f)-1.0f);
  g.tColor((inputwhere==5)?11:15);
  g.printf("%s",curfname.c_str());
  if (inputwhere==5) {
      inputCursor(73,scrH-20,scrH-4);
  }
}

void drawmemory() {
  // draws soundchip memory view
  g.tPos(0,5);
  g.tColor(15);
  g.printf("Memory  00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f\n");
  g.printf("   $00\n");
  g.printf("   $10\n");
  g.printf("   $20\n");
  g.printf("   $30\n");
  g.printf("   $40\n");
  g.printf("   $50\n");
  g.printf("   $60\n");
  g.printf("   $70\n");
  g.printf("   $80\n");
  g.printf("   $90\n");
  g.printf("   $a0\n");
  g.printf("   $b0\n");
  g.printf("   $c0\n");
  g.printf("   $d0\n");
  g.printf("   $e0\n");
  g.printf("   $f0\n");
  g.tColor(14);
  for (int j=0; j<16; j++) {
    g.tPos(8,6+j);
    for (int i=0; i<16; i++) {
      g.printf("%.2x ",((unsigned char*)(chip[curedpage].chan))[i+(j<<4)]);
    }
  }
}

void drawsong() {
  // draws the disk operations dialog
  g.tColor(15);
  g.tPos(0,5);
  g.printf("Tuning     v^|Channels   v^|beat   v^|bar   v^|detune   v^-+|length   v^-+|tempo    v^-+|speed   v^\n");
  g.printf("---------------------------------------------------------------------------------------------------\n");
  g.printf("|Song Name\n");
  g.printf("---------------------------------------------------------------------------------------------------\n");
  g.printf(comments.c_str());
  
  g.tPos(0,18);
  g.printf("---------------------------------------------------------------------------------------------------\n");
  g.printf("PlaybackMode Normal|IT|FT2|PT|ST3\n");
  g.printf("CompatibleGxx|DoubleFilter|\n");
  g.printf("Slides Linear|Periods|Amiga\n");
  g.printf("\n");
  g.printf("PCM|Edit|Load|SaveDump|PlayDump\n");
  g.printf("\n");
  
  g.tColor((inputwhere==1)?11:15);
  g.tPos(11,7);
  g.printf(name.c_str());
  if (inputwhere==1) {
    inputCursor(89,85,97);
  }
  g.tColor(11);
  
  // tuning
  g.tPos(7,5);
  g.printf(" PAL");
  
  // channels
  g.tPos(22,5);
  g.printf("%2d",channels);
  
  // detune
  g.tPos(54,5);
  g.printf("%.2x",(unsigned char)songdf);
  
  // length
  g.tPos(68,5);
  g.printf("%.2x",songlength);
  
  // tempo
  g.tPos(81,5);
  g.printf("%d",125);
  
  // speed
  g.tPos(95,5);
  g.printf("%.2X",defspeed);
}
void drawhelp() {
  // draws the help screen
  g.tColor(15);
  g.tPos(0,5);
  g.printf(helptext);
}
void drawconfig() {
  g.tColor(15);
  g.tPos(0,6);
  
  g.printf(
  "Color Palette          |Load|Save| Audio Settings\n"
  "                                 |\n"
  "Note              |          C-4 | simulate distortion\n"
  "Instrument        |          01  | \n"
  "Volume            |          v20 | \n"
  "Effect (tempo)    |          A06 | cubic spline PCM\n"
  "Effect (song)     |          B02 |---------------------\n"
  "Effect (volume)   |          D06 | Importer Settings\n"
  "Effect (pitch)    |          GFF | \n"
  "Effect (note)     |          J37 | import samples\n"
  "Effect (special)  |          SC1 | split instruments\n"
  "Effect (pan)      |          Y64 |---------------------\n"
  "Effect (unknown)  |          ?FF | Filter Settings\n"
  "Blank Row         |          ... | \n"
  "Selected 1        |          abc | disable filters\n"
  "Selected 2        |          abc | high quality\n"
  "Selected 3        |          abc |\n"
  "Peak Meter        |          abc |\n"
  "Default           |          abc |\n"
  "Dark              |          abc |\n"
  "Loop Highlight    |          abc |\n"
  "Release Highlight |          abc |\n"
  "CurPos Highlight  |          abc |\n"
  "On/Up             |          ON  |\n"
  "Off/Down          |          NO  |\n"
  "Absolute          |          A   |\n"
  "CurrentRow HL     |          abc |\n"
  "Selection         |          abc |\n"
  "No Volume         |          v40 |"
  );
  
  g.tNLPos(35);
  g.tPos(8);
  g.tColor((settings::distortion)?11:15);
  g.printf("simulate distortion\n\n\n");
  g.tColor((settings::cubicspline)?11:15);
  g.printf("cubic spline PCM\n\n\n\n\n\n\n\n\n");
  g.tColor((settings::nofilters)?11:15);
  g.printf("disable filters\n");
  g.tColor((settings::muffle)?11:15);
  g.printf("high quality");
  g.tNLPos(0);
}
void drawabout() {
  // draws about screen
  g.tColor(15);
  g.tPos((float)scrW/16.0,5);
  g.tAlign(0.5);
  g.printf(PROGRAM_NAME);
  g.tPos((float)scrW/16.0,6);
  g.tColor(14);
  g.printf("r%d",ver);
  g.tAlign(0);
  if (logo.actual==NULL) {
    g.tColor(15);
    g.tPos(0,10);
    g.printf("it seems you don't have the logo file!");
  } else {
    g._WRAP_draw_rotated_bitmap(logo,180,86.5,scrW/2,scrH/2,(sin((((float)curstep*(float)speed)+((float)speed-(float)curtick))/(8*(float)speed)*2*M_PI)/8)*(playmode!=0),0);
  }
}

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

void drawpiano() {
  double prefreq;
  int postfreq;
  g._WRAP_draw_scaled_bitmap(piano,0,0,700,60,(scrW/2)-(((scrW)/700)*700)/2,scrH-(60*((scrW)/700)),((scrW)/700)*700,((scrW)/700)*60,0);
  g.setTarget(pianoroll_temp);
  g._WRAP_draw_bitmap(pianoroll,0,-1,0);
  g.setTarget(pianoroll);
  g._WRAP_clear_to_color(g._WRAP_map_rgb(0,0,0));
  g._WRAP_draw_bitmap(pianoroll_temp,0,0,0);
  g.resetTarget();
  //printf("--------------\n");
  for (int ii=0;ii<32;ii++) {
    if (muted[ii] || cvol[ii]==0) continue;
    prefreq=((log(((4.53948974609375*(double)cfreq[ii])/440.0)/64)/log(2.0))*12.0)+57.5;
    if (prefreq<0 || prefreq>120) continue;
    if (
      (int)prefreq%12==0 || (int)prefreq%12==2 || (int)prefreq%12==4 ||
      (int)prefreq%12==5 || (int)prefreq%12==7 || (int)prefreq%12==9 ||
      (int)prefreq%12==11
      ) {
      postfreq=round(((int)prefreq%12)/2.0);
      // upper key
      g._WRAP_draw_filled_rectangle(
        ((((int)prefreq/12)*70+((postfreq*10)+getLKeyOff(postfreq)))*((scrW)/700))+(scrW/2)-(((scrW)/700)*700)/2,
        ((scrW)/700)+scrH-(((scrW)/700)*60),
        ((((int)prefreq/12)*70+((postfreq*10)+getRKeyOff(postfreq)))*((scrW)/700))+(scrW/2)-(((scrW)/700)*700)/2,
        35*((scrW)/700)+scrH-(((scrW)/700)*60),
        g._WRAP_map_rgba(
        (cshape[ii]==4 || cshape[ii]==1 || cshape[ii]==5)?(255):(0),
        (cshape[ii]!=5)?(255):(0),
        (cshape[ii]!=1 && cshape[ii]!=2)?(255):(0),
        cvol[ii]*2));
      // lower key
      g._WRAP_draw_filled_rectangle(
        ((((int)prefreq/12)*70+(((postfreq*10)+1)))*((scrW)/700))+(scrW/2)-(((scrW)/700)*700)/2,
        35*((scrW)/700)+scrH-(((scrW)/700)*60),
        ((((int)prefreq/12)*70+(((postfreq*10)+10)))*((scrW)/700))+(scrW/2)-(((scrW)/700)*700)/2,
        59*((scrW)/700)+scrH-(((scrW)/700)*60),
        g._WRAP_map_rgba(
        (cshape[ii]==4 || cshape[ii]==1 || cshape[ii]==5)?(255):(0),
        (cshape[ii]!=5)?(255):(0),
        (cshape[ii]!=1 && cshape[ii]!=2)?(255):(0),
        cvol[ii]*2));
    } else {
      postfreq=((int)prefreq%12)/2.0;
      g._WRAP_draw_filled_rectangle(
        ((((int)prefreq/12)*70+(((postfreq*10)+8)))*((scrW)/700))+(scrW/2)-(((scrW)/700)*700)/2,
        ((scrW)/700)+scrH-(((scrW)/700)*60),
        ((((int)prefreq/12)*70+(((postfreq*10)+13)))*((scrW)/700))+(scrW/2)-(((scrW)/700)*700)/2,
        (34*((scrW)/700))+scrH-(((scrW)/700)*60),
        g._WRAP_map_rgba(
        (cshape[ii]==4 || cshape[ii]==1 || cshape[ii]==5)?(cvol[ii]*2):(0),
        (cshape[ii]!=5)?(cvol[ii]*2):(0),
        (cshape[ii]!=1 && cshape[ii]!=2)?(cvol[ii]*2):(0),
        255));
    }
    g.setTarget(pianoroll);
                g._WRAP_set_blender(SDL_BLENDMODE_ADD);
                if (cshape[ii]==0) {
                 g._WRAP_draw_filled_rectangle(((prefreq-0.5)*6)+1-0.5,127,((prefreq-0.5)*6)+5,128,
      mapHSV(240-(cduty[ii]),1,(float)cvol[ii]/127)); 
                } else {
    g._WRAP_draw_filled_rectangle(((prefreq-0.5)*5.83333333)+1-0.5,127,((prefreq-0.5)*5.833333333)+5,128,
      g._WRAP_map_rgb(
      (cshape[ii]==4 || cshape[ii]==1 || cshape[ii]==5)?(cvol[ii]*2):(0),
      (cshape[ii]!=5)?(cvol[ii]*2):(0),
      (cshape[ii]!=1 && cshape[ii]!=2)?(cvol[ii]*2):(0)
      ));
                }
                g._WRAP_set_blender(SDL_BLENDMODE_BLEND);
    g.resetTarget();
  }
  g._WRAP_draw_scaled_bitmap(pianoroll,0,0,700,128,(scrW/2)-((((scrW)/700)*700)/2),scrH-(((scrH-(60+((((scrW)/700)*60))))/128)*128)-(((scrW)/700)*60),((scrW)/700)*700,((scrH-(60+((((scrW)/700)*60))))/128)*128,0);
  for (int ii=0;ii<32;ii++) {
    if (muted[ii] || cvol[ii]==0) continue;
    prefreq=((log(((4.53948974609375*(double)cfreq[ii])/440.0)/64)/log(2.0))*12.0)+57.5;
    if (prefreq<0 || prefreq>120) continue;
    g.tColor(15);
    g.tPos(0,5+ii);
    g.printf("%d: %s%s %c%.2d",ii,getnotetransp((int)prefreq),getoctavetransp((int)prefreq),(sign((int)(fmod(prefreq,1)*100)-50)>-1)?('+'):('-'),abs((int)(fmod(prefreq,1)*100)-50));
  }
}

void drawsfxpanel() {
  // TODO after the SDL port
}

void drawpcmeditor() {
  pcmeditseek=mstate.x;
  g.tColor(15);
  g.tPos(0,5);
  if (sign(pcmeditscale)>-1) {
    g.printf("scale: %d:1\n",(int)pow(2.0f,pcmeditscale));
  } else {
    g.printf("scale: 1:%d\n",(int)pow(2.0f,-pcmeditscale));
  }
  g.printf("position: %.5x",(int)((float)pcmeditseek*pow(2.0f,-pcmeditscale)));
  g._WRAP_draw_line((int)((float)pcmeditseek*pow(2.0f,-pcmeditscale))*pow(2.0f,pcmeditscale),(scrH/2)-128,
    (int)((float)pcmeditseek*pow(2.0f,-pcmeditscale))*pow(2.0f,pcmeditscale),(scrH/2)+128,g._WRAP_map_rgb(255,255,0),1);
  for (float ii=0;ii<(scrW*pow(2.0f,-pcmeditscale));ii+=pow(2.0f,-pcmeditscale)) {
    g._WRAP_draw_pixel(ii*pow(2.0f,pcmeditscale),(scrH/2)+chip[0].pcm[(int)ii+pcmeditoffset],(pcmeditenable)?g._WRAP_map_rgb(255,255,0):g._WRAP_map_rgb(255,255,255));
  }
  for (int ii=0;ii<32;ii++) {
    if (cmode[ii]==1 && cvol[ii]>0) {
      g._WRAP_draw_line(((float)cpcmpos[ii]*pow(2.0f,pcmeditscale)),(scrH/2)-140,
    ((float)cpcmpos[ii]*pow(2.0f,pcmeditscale)),(scrH/2)-129,g._WRAP_map_rgb(0,255,255),1);
      g.tColor(11);
      g.tPos((float)(cpcmpos[ii]*pow(2.0f,pcmeditscale))/8.0,(float)((scrH/2)-153)/12);
      g.printf("%d",ii);
    }
  }
}
void StepPlay() {
  NextRow();
  playmode=2;
}
void Play() {
  //// PLAY SONG ////
  // set speed to song speed and other variables
  if (!speedlock) {speed=defspeed;}
  if (!tempolock) {
  if (ntsc) {
    tempo=150;
  } else {
    tempo=125;
  }
  }
  for (int ii=0;ii<32;ii++) {
    if (pat[patid[curpat]][0][ii][3]==20)
    {if (pat[patid[curpat]][0][ii][4]!=0 && !tempolock)
    {tempo=maxval(31,pat[patid[curpat]][0][ii][4]);FPS=(double)tempo/2.5;
    }}
  }
  FPS=tempo/2.5;
  // reset cursor position
  curtick=2;curstep=-1;playmode=1;
  tickstart=true;
  // reset channels
  for (int su=0;su<32;su++) {
  cvol[su]=0;
  cfreq[su]=1;
  cduty[su]=63;
  cshape[su]=0;
  coff[su]=262144;
  creso[su]=0;
  cfmode[su]=0;
  Mvol[su]=0;
  Mins[su]=0;
  curvibshape[su]=0;
  EnvelopesRunning[su][0]=false;
  EnvelopesRunning[su][1]=false;
  EnvelopesRunning[su][2]=false;
  EnvelopesRunning[su][3]=false;
  EnvelopesRunning[su][4]=false;
  EnvelopesRunning[su][5]=false;
  EnvelopesRunning[su][6]=false;
  EnvelopesRunning[su][7]=false;
  inspos[su][0]=0;
  inspos[su][1]=0;
  inspos[su][2]=0;
  inspos[su][3]=0;
  inspos[su][4]=0;
  inspos[su][5]=0;
  inspos[su][6]=0;
  inspos[su][7]=0;
  released[su]=false;
  plcount[su]=0;
  plpos[su]=0;
  chanvol[su]=defchanvol[su];
  doretrigger[su]=false;
  chanpan[su]=defchanpan[su];
  //if ((su+1)&2) {chanpan[su]=96;} else {chanpan[su]=-96;} // amiga auto-pan logic
  //if (su&1) {chanpan[su]=96;} else {chanpan[su]=-96;} // normal auto-pan logic
  finedelay=0;
  cutcount[su]=-1;
  }
  // reset global volume
  cglobvol=128;
  // process next row
  NextTick();
}
unsigned char ITVolumeConverter(unsigned char itvol) {
  if (itvol<65) {return minval(itvol+64,127);} // 64-127
  if (itvol<193 && itvol>127) {return itvol;} // 128-192
  switch((itvol-65)/10) {
  case 0: return itvol-65; break; // 1-10
  case 1: return itvol-65; break; // 11-20
  case 2: return itvol-65; break; // 21-30
  case 3: return itvol-65; break; // 31-40
  case 4: return itvol-65; break; // 41-50
  case 5: return itvol-65; break; // 51-60
  }
  return itvol;
}
int ImportIT(FILE* it) {
  // import IT file, after YEARS I wasn't able to do this.
  // check out http://schismtracker.org/wiki/ITTECH.TXT for specs in IT format
    int64_t size;
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
  printf("%ld bytes\n",size);
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
    songlength=(unsigned char)memblock[0x20]; instruments=(unsigned char)memblock[0x22]; patterns=(unsigned char)memblock[0x26]; samples=(unsigned char)memblock[0x24];
    //cout << (int)memblock[0x29] << "." << (int)memblock[0x28];
    printf("\n");
    //cout << "volumes: global " << (int)(unsigned char)memblock[0x30] << ", mixing " << (int)(unsigned char)memblock[0x31] << "\n";
    //cout << "speeds: " << (int)memblock[0x32] << ":" << (int)(unsigned char)memblock[0x33] << "\n";
    defspeed=memblock[0x32];
    printf("---pans---\n");
    for (sk=0x40;sk<0x60;sk++) {
      defchanpan[sk-64]=memblock[sk];
      printf("%d ",(int)memblock[sk]);
    }
    printf("\n");
    printf("---volumes---\n");
    for (sk=0x80;sk<0xa0;sk++) {
      defchanvol[sk-128]=memblock[sk]*2;
      printf("%d ",(int)memblock[sk]);
    }
    printf("\n");
    printf("---ORDER LIST---\n");
    for (sk=0xc0;sk<(0xc0+songlength);sk++) {
      patid[sk-0xc0]=memblock[sk];
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
    patparas[sk]=((unsigned char)memblock[0xc0+songlength+(instruments*4)+(samples*4)+(sk*4)])+(((unsigned char)memblock[0xc0+songlength+(instruments*4)+(samples*4)+(sk*4)+1])*256)+(((unsigned char)memblock[0xc0+songlength+(instruments*4)+(samples*4)+(sk*4)+2])*65536)+(((unsigned char)memblock[0xc0+songlength+(instruments*4)+(samples*4)+(sk*4)+3])*16777216);
    printf("pattern %d offset: ",sk);
    printf("%d\n",patparas[sk]);
    }
    // load/unpack patterns
    channels=1;
    for (int pointer=0;pointer<patterns;pointer++) {
    printf("-unpacking pattern %d-\n",pointer);
    CurrentRow=0;
    sk=patparas[pointer];
    int patsize=(unsigned char)memblock[sk]+((unsigned char)memblock[sk+1]*256);
    int plength=(unsigned char)memblock[sk+2]+((unsigned char)memblock[sk+3]*256);
    printf("%d bytes in pattern\n",patsize);
    patlength[pointer]=plength; // set length
    sk=patparas[pointer]+8;
    for (int a=0;a<patsize;a++) {
    NextByte=(unsigned char)memblock[sk+a];
    if (NextByte==0) {
      CurrentRow++;
      if (CurrentRow==plength) {break;}
      continue;
    }
    NextChannel=(NextByte-1)&31;
    if (NextChannel>=channels) {
      channels=NextChannel+1;
      if (channels>32) channels=32;
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
        case 255: pat[pointer][CurrentRow][NextChannel][0]=13; break;
        case 254: pat[pointer][CurrentRow][NextChannel][0]=15; break;
        default: if (LastNote[NextChannel]<120) {pat[pointer][CurrentRow][NextChannel][0]=((LastNote[NextChannel]/12)*16)+(LastNote[NextChannel]%12)+1;/*pat[pointer][CurrentRow][NextChannel][2]=0x7f;*/} else {pat[pointer][CurrentRow][NextChannel][0]=14;}; break;
      }
    }
    if (NextMask[NextChannel]&2) {
      a++;
      LastIns[NextChannel]=(unsigned char)memblock[sk+a];
      pat[pointer][CurrentRow][NextChannel][1]=LastIns[NextChannel];
    }
    if (NextMask[NextChannel]&4) {
      a++;
      LastVol[NextChannel]=(unsigned char)memblock[sk+a];
      pat[pointer][CurrentRow][NextChannel][2]=ITVolumeConverter(LastVol[NextChannel]);
    }
    if (NextMask[NextChannel]&8) {
      a++;
      LastFX[NextChannel]=(unsigned char)memblock[sk+a];
      pat[pointer][CurrentRow][NextChannel][3]=LastFX[NextChannel];
      a++;
      LastFXVal[NextChannel]=(unsigned char)memblock[sk+a];
      pat[pointer][CurrentRow][NextChannel][4]=LastFXVal[NextChannel];
    }
    if (NextMask[NextChannel]&16) {
      switch (LastNote[NextChannel]) {
        case 255: pat[pointer][CurrentRow][NextChannel][0]=13; break;
        case 254: pat[pointer][CurrentRow][NextChannel][0]=15; break;
        default: if (LastNote[NextChannel]<120) {pat[pointer][CurrentRow][NextChannel][0]=((LastNote[NextChannel]/12)*16)+(LastNote[NextChannel]%12)+1;/*pat[pointer][CurrentRow][NextChannel][2]=0x7f;*/} else {pat[pointer][CurrentRow][NextChannel][0]=14;}; break;
      }
    }
    if (NextMask[NextChannel]&32) {
      pat[pointer][CurrentRow][NextChannel][1]=LastIns[NextChannel];
    }
    if (NextMask[NextChannel]&64) {
      pat[pointer][CurrentRow][NextChannel][2]=ITVolumeConverter(LastVol[NextChannel]);
    }
    if (NextMask[NextChannel]&128) {
      pat[pointer][CurrentRow][NextChannel][3]=LastFX[NextChannel];
      pat[pointer][CurrentRow][NextChannel][4]=LastFXVal[NextChannel];
    }
    }
  }

  } else {printf("error while importing file! not an IT module file\n"); delete[] memblock; return 1;}
    delete[] memblock;
  }
  else {printf("error while importing file! file doesn't exist\n"); return 1;}
  if (!playermode && !fileswitch) {curpat=0;}
  if (playmode==1) {Play();}
  if (name=="") {
    g.setTitle(PROGRAM_NAME);
  } else {
    g.setTitle(name+S(" - ")+S(PROGRAM_NAME));
  }
  songlength--;
  return 0;
}
int ImportMOD(FILE* mod) {
  // import MOD file
  // check out http://www.fileformat.info/format/mod/corion.htm for specs in MOD format
    int64_t size;
  char * memblock;
  int sk;
  //string fn;
  int chans;
  int CurrentSampleSeek=0;
  printf("\nplease write filename? ");
  if (mod!=NULL) { // read the file
  printf("loading MOD file, ");
  CleanupPatterns();
    size=fsize(mod);
  printf("%ld bytes\n",size);
    memblock=new char[size];
    fseek(mod,0,SEEK_SET);
    fread(memblock,1,size,mod);
    fclose(mod);
    printf("success, now importing file\n");
  for (int nonsense=0;nonsense<256;nonsense++) {
    patlength[nonsense]=64;
    //instrument[nonsense].env[envPan]=48;
  }
#ifndef PRESERVE_INS
  for (int ii=0;ii<31;ii++) {
    for (int jj=0;jj<22;jj++) {
      instrument[ii+1].name[jj]=memblock[0x14+(ii*30)+jj];
    }
    instrument[ii+1].vol=memblock[0x14+(ii*30)+25];
    if (settings::samples) {
      instrument[ii+1].pcmPos[1]=CurrentSampleSeek>>8;
      instrument[ii+1].pcmPos[0]=CurrentSampleSeek%256;
      instrument[ii+1].DFM|=(CurrentSampleSeek>>16)?(128):(0);
      int tempsize;
      tempsize=(((unsigned char)(memblock[0x14+(ii*30)+0x16])<<8)+(unsigned char)(memblock[0x14+(ii*30)+0x17]))*2;
      int repeatpos;
      repeatpos=(((unsigned char)(memblock[0x14+(ii*30)+0x1a])<<8)+(unsigned char)(memblock[0x14+(ii*30)+0x1b]))*2;
      int repeatlen;
      repeatlen=(((unsigned char)(memblock[0x14+(ii*30)+0x1c])<<8)+(unsigned char)(memblock[0x14+(ii*30)+0x1d]))*2;
      printf("sample %d size: %.5x repeat: %.4x replen: %.4x\n",ii,tempsize,repeatpos,repeatlen);
      instrument[ii+1].pcmLen=(repeatpos>0 || repeatlen>2)?(minval(tempsize,repeatpos+repeatlen)):(tempsize);
      instrument[ii+1].pcmLoop[1]=repeatpos>>8;
      instrument[ii+1].pcmLoop[0]=repeatpos&0xff;
      instrument[ii+1].pcmMult|=(repeatpos>0 || repeatlen>2)?(128):(0);
      CurrentSampleSeek+=tempsize;
      instrument[ii+1].noteOffset=12;
      instrument[ii+1].DFM|=8;
                        /*
    instrument[ii+1].pcmPos[0]=CurrentSampleSeek>>8;
    instrument[ii+1].pcmPos[1]=CurrentSampleSeek%256;
    instrument[ii+1].DFM|=(CurrentSampleSeek>>16)<<7;
    CurrentSampleSeek+=(((memblock[0x14+(ii*30)+22]<<8)+memblock[0x14+(ii*30)+23])*2);
    if ((((((memblock[0x14+(ii*30)+22]<<8)+memblock[0x14+(ii*30)+23])*2)>>4)%8)>3) {CurrentSampleSeek+=256;}
    instrument[ii+1][0x33]=(((memblock[0x14+(ii*30)+23]<<8)+memblock[0x14+(ii*30)+22])*2)>>8;
    instrument[ii+1][0x32]=(((memblock[0x14+(ii*30)+23]<<8)+memblock[0x14+(ii*30)+22])*2)%256;*/
    }
  }
#endif
  if ((memblock[1080]=='M' && memblock[1081]=='.' && memblock[1082]=='K' && memblock[1083]=='.')||
    (memblock[1080]=='M' && memblock[1081]=='!' && memblock[1082]=='K' && memblock[1083]=='!')||
     (memblock[1081]=='C' && memblock[1082]=='H' && memblock[1083]=='N')||
     (memblock[1082]=='C' && memblock[1083]=='H')) {
       switch(memblock[1080]) {
      case 'M': printf("4-channel original MOD module detected\n"); origin="ProTracker"; chans=4; break;
      default: printf("multi-channel MOD module detected\n"); origin="FastTracker or similar"; break;
       }
       if (memblock[1082]=='C' && memblock[1083]=='H') {chans=(NumberLetter(memblock[1080])*10)+NumberLetter(memblock[1081]);}
       if (memblock[1081]=='C' && memblock[1082]=='H' && memblock[1083]=='N') {chans=NumberLetter(memblock[1080]);}
    // name
    name="";
    for (sk=0;sk<20;sk++) {
      if (memblock[sk]==0) break;
      name+=memblock[sk];
    }
    printf("module name is %s\n",name.c_str());
    printf("---ORDER LIST---\n");
    for (sk=952;sk<1080;sk++) {
      patid[sk-952]=memblock[sk];
      switch(memblock[sk]) {
      case -2: printf("+++ "); break;
      case -1: printf("--- "); break;
      default: printf("%d ",(int)(unsigned char)memblock[sk]); break;
      }
    }
    printf("\nretrieving pattern count\n");
    patterns=0;
    for (sk=0;sk<128;sk++) {
      if (patid[sk]>patterns) {patterns=patid[sk];}
    }
    printf("%d patterns\n",patterns);
    songlength=memblock[950]-1;
    if (settings::samples) {
    printf("putting samples to PCM memory if possible\n");
    printf("%ld bytes",size-1084-(patterns*chans*64*4));
    memcpy(chip[0].pcm,memblock+1084+((patterns+1)*chans*64*4),minval(65280,size-1084-((patterns+1)*chans*64*4)));
    if (size-1084-((patterns+1)*chans*64*4)>65280) {
      popbox=PopupBox("Warning","out of PCM memory to load all samples!");
    }
    }
    printf("---PATTERNS---\n");
    for (int importid=0;importid<patterns+1;importid++) {
    printf("-PATTERN %d-\n",importid);
    /*if (verbose) {cout << "-Channel1- -Channel2- -Channel3- -Channel4- ";
    if (chans==6) {cout << "-Channel5- -Channel6-\n";} else {
      if (chans==8) {cout << "-Channel5- -Channel6- -Channel7- -Channel8-\n";} else {cout << "\n";}
    }}*/
    sk=1084; // import position
    for (int indxr=0;indxr<64;indxr++) {
      int NPERIOD;
      int NINS;
      int NFX;
      int NFXVAL;
      for (int ichan=0;ichan<chans;ichan++) {
        // import pattern row
        NPERIOD=(((unsigned char)memblock[sk+(importid*(chans*256))+(indxr*(chans*4))+(ichan*4)])%16*256)+((unsigned char)memblock[sk+(importid*(chans*256))+(indxr*(chans*4))+(ichan*4)+1]);
        NINS=(((unsigned char)memblock[sk+(importid*(chans*256))+(indxr*(chans*4))+(ichan*4)]>>4)*16)+((unsigned char)memblock[sk+(importid*(chans*256))+(indxr*(chans*4))+(ichan*4)+2]>>4);
        NFX=(unsigned char)memblock[sk+(importid*(chans*256))+(indxr*(chans*4))+(ichan*4)+2]%16;
        NFXVAL=(unsigned char)memblock[sk+(importid*(chans*256))+(indxr*(chans*4))+(ichan*4)+3];
        // conversion stuff
        switch(NPERIOD) {
          case 56: if (verbose) printf("B-7 "); pat[importid][indxr][ichan][0]=0x7c; break;
          case 60: if (verbose) printf("A#7 "); pat[importid][indxr][ichan][0]=0x7b; break;
          case 63: if (verbose) printf("A-7 "); pat[importid][indxr][ichan][0]=0x7a; break;
          case 67: if (verbose) printf("G#7 "); pat[importid][indxr][ichan][0]=0x79; break;
          case 71: if (verbose) printf("G-7 "); pat[importid][indxr][ichan][0]=0x78; break;
          case 75: if (verbose) printf("F#7 "); pat[importid][indxr][ichan][0]=0x77; break;
          case 80: if (verbose) printf("F-7 "); pat[importid][indxr][ichan][0]=0x76; break;
          case 85: if (verbose) printf("E-7 "); pat[importid][indxr][ichan][0]=0x75; break;
          case 90: if (verbose) printf("D#7 "); pat[importid][indxr][ichan][0]=0x74; break;
          case 95: if (verbose) printf("D-7 "); pat[importid][indxr][ichan][0]=0x73; break;
          case 101: if (verbose) printf("C#7 "); pat[importid][indxr][ichan][0]=0x72; break;
          case 107: if (verbose) printf("C-7 "); pat[importid][indxr][ichan][0]=0x71; break;
          case 113: if (verbose) printf("B-6 "); pat[importid][indxr][ichan][0]=0x6c; break;
          case 120: if (verbose) printf("A#6 "); pat[importid][indxr][ichan][0]=0x6b; break;
          case 127: if (verbose) printf("A-6 "); pat[importid][indxr][ichan][0]=0x6a; break;
          case 135: if (verbose) printf("G#6 "); pat[importid][indxr][ichan][0]=0x69; break;
          case 143: if (verbose) printf("G-6 "); pat[importid][indxr][ichan][0]=0x68; break;
          case 151: if (verbose) printf("F#6 "); pat[importid][indxr][ichan][0]=0x67; break;
          case 160: if (verbose) printf("F-6 "); pat[importid][indxr][ichan][0]=0x66; break;
          case 170: if (verbose) printf("E-6 "); pat[importid][indxr][ichan][0]=0x65; break;
          case 180: if (verbose) printf("D#6 "); pat[importid][indxr][ichan][0]=0x64; break;
          case 190: if (verbose) printf("D-6 "); pat[importid][indxr][ichan][0]=0x63; break;
          case 202: if (verbose) printf("C#6 "); pat[importid][indxr][ichan][0]=0x62; break;
          case 214: if (verbose) printf("C-6 "); pat[importid][indxr][ichan][0]=0x61; break;
          case 226: if (verbose) printf("B-5 "); pat[importid][indxr][ichan][0]=0x5c; break;
          case 240: if (verbose) printf("A#5 "); pat[importid][indxr][ichan][0]=0x5b; break;
          case 254: if (verbose) printf("A-5 "); pat[importid][indxr][ichan][0]=0x5a; break;
          case 269: if (verbose) printf("G#5 "); pat[importid][indxr][ichan][0]=0x59; break;
          case 285: if (verbose) printf("G-5 "); pat[importid][indxr][ichan][0]=0x58; break;
          case 302: if (verbose) printf("F#5 "); pat[importid][indxr][ichan][0]=0x57; break;
          case 320: if (verbose) printf("F-5 "); pat[importid][indxr][ichan][0]=0x56; break;
          case 339: if (verbose) printf("E-5 "); pat[importid][indxr][ichan][0]=0x55; break;
          case 360: if (verbose) printf("D#5 "); pat[importid][indxr][ichan][0]=0x54; break;
          case 381: if (verbose) printf("D-5 "); pat[importid][indxr][ichan][0]=0x53; break;
          case 404: if (verbose) printf("C#5 "); pat[importid][indxr][ichan][0]=0x52; break;
          case 428: if (verbose) printf("C-5 "); pat[importid][indxr][ichan][0]=0x51; break;
          case 453: if (verbose) printf("B-4 "); pat[importid][indxr][ichan][0]=0x4c; break;
          case 480: if (verbose) printf("A#4 "); pat[importid][indxr][ichan][0]=0x4b; break;
          case 508: if (verbose) printf("A-4 "); pat[importid][indxr][ichan][0]=0x4a; break;
          case 538: if (verbose) printf("G#4 "); pat[importid][indxr][ichan][0]=0x49; break;
          case 570: if (verbose) printf("G-4 "); pat[importid][indxr][ichan][0]=0x48; break;
          case 604: if (verbose) printf("F#4 "); pat[importid][indxr][ichan][0]=0x47; break;
          case 640: if (verbose) printf("F-4 "); pat[importid][indxr][ichan][0]=0x46; break;
          case 678: if (verbose) printf("E-4 "); pat[importid][indxr][ichan][0]=0x45; break;
          case 720: if (verbose) printf("D#4 "); pat[importid][indxr][ichan][0]=0x44; break;
          case 762: if (verbose) printf("D-4 "); pat[importid][indxr][ichan][0]=0x43; break;
          case 808: if (verbose) printf("C#4 "); pat[importid][indxr][ichan][0]=0x42; break;
          case 856: if (verbose) printf("C-4 "); pat[importid][indxr][ichan][0]=0x41; break;
          case 906: if (verbose) printf("B-3 "); pat[importid][indxr][ichan][0]=0x3c; break;
          case 907: if (verbose) printf("B-3 "); pat[importid][indxr][ichan][0]=0x3c; break; // OpenMPT?
          case 960: if (verbose) printf("A#3 "); pat[importid][indxr][ichan][0]=0x3b; break;
          case 1016: if (verbose) printf("A-3 "); pat[importid][indxr][ichan][0]=0x3a; break;
          case 1076: if (verbose) printf("G#3 "); pat[importid][indxr][ichan][0]=0x39; break;
          case 1140: if (verbose) printf("G-3 "); pat[importid][indxr][ichan][0]=0x38; break;
          case 1208: if (verbose) printf("F#3 "); pat[importid][indxr][ichan][0]=0x37; break;
          case 1280: if (verbose) printf("F-3 "); pat[importid][indxr][ichan][0]=0x36; break;
          case 1356: if (verbose) printf("E-3 "); pat[importid][indxr][ichan][0]=0x35; break;
          case 1440: if (verbose) printf("D#3 "); pat[importid][indxr][ichan][0]=0x34; break;
          case 1524: if (verbose) printf("D-3 "); pat[importid][indxr][ichan][0]=0x33; break;
          case 1616: if (verbose) printf("C#3 "); pat[importid][indxr][ichan][0]=0x32; break;
          case 1712: if (verbose) printf("C-3 "); pat[importid][indxr][ichan][0]=0x31; break;
          case 0: if (verbose) printf("--- "); pat[importid][indxr][ichan][0]=0x00; break;
          default: if (verbose) printf("??? "); pat[importid][indxr][ichan][0]=0x00; printf("invalid note! %d at row %d channel %d\n",NPERIOD,indxr,ichan); break;
        }
        pat[importid][indxr][ichan][1]=NINS;
        switch(NFX) {
          case 0: if (NFXVAL!=0) {pat[importid][indxr][ichan][3]=10;pat[importid][indxr][ichan][4]=NFXVAL;} else {pat[importid][indxr][ichan][3]=0;pat[importid][indxr][ichan][4]=0;}; break;
          case 1: pat[importid][indxr][ichan][3]=6;pat[importid][indxr][ichan][4]=NFXVAL; break;
          case 2: pat[importid][indxr][ichan][3]=5;pat[importid][indxr][ichan][4]=NFXVAL; break;
          case 3: pat[importid][indxr][ichan][3]=7;pat[importid][indxr][ichan][4]=NFXVAL; break;
          case 4: pat[importid][indxr][ichan][3]=8;pat[importid][indxr][ichan][4]=NFXVAL; break;
          case 5: pat[importid][indxr][ichan][3]=12;pat[importid][indxr][ichan][4]=NFXVAL; break;
          case 6: pat[importid][indxr][ichan][3]=11;pat[importid][indxr][ichan][4]=NFXVAL; break;
          case 7: pat[importid][indxr][ichan][3]=18;pat[importid][indxr][ichan][4]=NFXVAL; break;
          case 8: pat[importid][indxr][ichan][3]=24;pat[importid][indxr][ichan][4]=NFXVAL; break;
          case 9: pat[importid][indxr][ichan][3]=15;pat[importid][indxr][ichan][4]=NFXVAL; break;
          case 10: pat[importid][indxr][ichan][3]=4;pat[importid][indxr][ichan][4]=NFXVAL; break;
          case 11: pat[importid][indxr][ichan][3]=2;pat[importid][indxr][ichan][4]=NFXVAL; break;
          case 12: pat[importid][indxr][ichan][2]=0x40+minval(NFXVAL,0x3f);pat[importid][indxr][ichan][4]=0; break;
          case 13: pat[importid][indxr][ichan][3]=3;pat[importid][indxr][ichan][4]=NFXVAL-(NFXVAL>>4)*6; break;
          case 14: pat[importid][indxr][ichan][3]=19;switch (NFXVAL>>4) {
            case 1: pat[importid][indxr][ichan][4]=0xf0+(NFXVAL%16); pat[importid][indxr][ichan][3]=6; break;
            case 2: pat[importid][indxr][ichan][4]=0xf0+(NFXVAL%16); pat[importid][indxr][ichan][3]=5; break;
            case 3: pat[importid][indxr][ichan][4]=0x20+(NFXVAL%16); break;
            case 4: pat[importid][indxr][ichan][4]=0x30+(NFXVAL%16); break;
            case 6: pat[importid][indxr][ichan][4]=0xb0+(NFXVAL%16); break;
            case 7: pat[importid][indxr][ichan][4]=0x40+(NFXVAL%16); break;
            case 9: pat[importid][indxr][ichan][4]=(NFXVAL%16); pat[importid][indxr][ichan][3]=17; break;
            case 10: pat[importid][indxr][ichan][4]=0x0f+((NFXVAL%16)<<4); pat[importid][indxr][ichan][3]=4; break;
            case 11: pat[importid][indxr][ichan][4]=0xf0+(NFXVAL%16); pat[importid][indxr][ichan][3]=4; break;
            default: pat[importid][indxr][ichan][4]=NFXVAL; break;
            }; break;
          case 15: pat[importid][indxr][ichan][3]=1;pat[importid][indxr][ichan][4]=NFXVAL;if (NFXVAL>0x20) {pat[importid][indxr][ichan][3]=20;}; break;
        }
      }
      //if (verbose) cout << "\n";
    }
    }

  } else {/*cout << "error while importing file! not a MOD module file\n";*/ delete[] memblock; return 1;}
    delete[] memblock;
  }
  else {/*cout << "error while importing file! file doesn't exist\n";*/ return 1;}
  songdf=0x1b; // Amiga compat
  if (!playermode && !fileswitch) {curpat=0;}
  if (playmode==1) {Play();}
  if (name=="") {
    g.setTitle(PROGRAM_NAME);
  } else {
    g.setTitle(name+S(" - ")+S(PROGRAM_NAME));
  }
  channels=chans;
  return 0;
}
int ImportS3M() {
  // import S3M file
  int64_t size;
  char * memblock;
  int sk;
  int NextByte;
  int NextChannel;
  int CurrentRow;
  FILE *s3m;
  int insparas[99];
  int patparas[256];
  //string fn;
  printf("\nplease write filename? ");
  char rfn[256];
  //gets(rfn);
  s3m=ps_fopen(rfn,"rb");
  if (s3m!=NULL) { // read the file
    printf("loading S3M file, ");
    size=fsize(s3m);
    printf("%ld bytes\n",size);
    memblock=new char[size];
    fseek(s3m,0,SEEK_SET);
    fread(memblock,1,size,s3m);
    fclose(s3m);
    printf("success, now importing file\n");
  for (int nonsense=0;nonsense<256;nonsense++) {
    patlength[nonsense]=64;
    //instrument[nonsense].env[envPan]=48;
  }
  // module name
  for (sk=0;sk<28;sk++) {
    if (memblock[sk]==0) break;
    name+=memblock[sk];
  }
  printf("module name is %s\n",name.c_str());
  songlength=memblock[0x20];
  instruments=memblock[0x22];
  patterns=memblock[0x24]*2;
  printf("%d orders, %d instruments, %d patterns\n",songlength,instruments,patterns);
  // order list
  printf("---ORDER LIST---\n");
  for (sk=0x60;sk<songlength+0x60;sk++) {
    patid[sk-0x60]=memblock[sk];
    switch(memblock[sk]) {
      case -2: printf("+++ "); break;
      case -1: printf("--- "); break;
      default: printf("%d ",(int)(unsigned char)memblock[sk]); break;
      }
    }
  // pointers
  printf("\n---POINTERS---\n");
  for (sk=0x60+songlength;sk<(0x60+songlength+instruments);sk+=2) {
    insparas[(sk-(0x60+songlength))/2]=((unsigned char)memblock[sk]*16)+((unsigned char)memblock[sk+1]*4096);
    printf("instrument %d offset: ",(sk-(0x60+songlength))/2);
    printf("%d\n",insparas[(sk-(0x60+songlength))/2]);
    }
  for (sk=0x60+songlength+(instruments*2);sk<(0x60+songlength+(instruments*2)+patterns);sk+=2) {
    patparas[(sk-(0x60+songlength+(instruments*2)))/2]=((unsigned char)memblock[sk]*16)+((unsigned char)memblock[sk+1]*4096);
    printf("pattern %d offset: ",(sk-(0x60+songlength+(instruments*2)))/2);
    printf("%d\n",patparas[(sk-(0x60+songlength+(instruments*2)))/2]);
    }
  // unpack patterns
  for (int pointer=0;pointer<(patterns/2);pointer++) {
    printf("-unpacking pattern %d-\n",pointer);
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
      pat[pointer][CurrentRow][NextChannel][0]=(unsigned char)memblock[sk+a]+17;
      a++;
      pat[pointer][CurrentRow][NextChannel][1]=(unsigned char)memblock[sk+a];
      //pat[pointer][CurrentRow][NextChannel][2]=127;
    }
    if ((NextByte>>6)%2) {
      a++;
      pat[pointer][CurrentRow][NextChannel][2]=minval(127,(unsigned char)64+memblock[sk+a]);
    }
    if ((NextByte>>7)%2) {
      a++;
      pat[pointer][CurrentRow][NextChannel][3]=(unsigned char)memblock[sk+a];
      a++;
      pat[pointer][CurrentRow][NextChannel][4]=(unsigned char)memblock[sk+a];
    }
    }
  }


  }
  delete[] memblock;
  origin="Scream Tracker 3";
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
  int seqparas[256];
  int commentpointer=0;
  int pcmpointer=0;
  bool IS_INS_BLANK[256];
  bool IS_PAT_BLANK[256];
  bool IS_SEQ_BLANK[256];
  char rfn[4096];
  int oldseek=0;
  
  // temporary, gonna get replaced by a better thing soon
  // just for the sake of linux
  strcpy(rfn,(S(curdir)+S(SDIR_SEPARATOR)+curfname).c_str());
  
  sfile=ps_fopen(rfn,"wb");
  if (sfile!=NULL) { // write the file
    fseek(sfile,0,SEEK_SET); // seek to 0
    printf("writing headers...\n");
    fwrite("TRACK8BT",1,8,sfile); // magic number
    fwrite(&ver,2,1,sfile); // version
    fputc(instruments,sfile); // instruments
    fputc(patterns,sfile); // patterns
    fputc(songlength,sfile); // orders
    fputc(defspeed,sfile); // speed
    fputc(seqs,sfile); // sequences
    fputc(125,sfile); // tempo
    fputs(name.c_str(),sfile); // name
    fseek(sfile,48,SEEK_SET); // seek to 0x30
    fputc(0,sfile); // default filter mode
    fputc(channels,sfile); // channels
    fwrite("\0\0",2,1,sfile); // flags
    fputc(128,sfile); // global volume
    fputc(0,sfile); // global panning
    fwrite("\0\0\0\0",4,1,sfile); // mute flags
    fwrite("\0\0\0\0",4,1,sfile); // PCM data pointer
    fwrite("\0\0",2,1,sfile); // reserved
    fseek(sfile,0x3e,SEEK_SET); // seek to 0x3e
    fputc(songdf,sfile); // detune factor
    fseek(sfile,0x40,SEEK_SET); // seek to 0x40
    fwrite(defchanvol,1,32,sfile); // channel volume
    fwrite(defchanpan,1,32,sfile); // channel panning
    fseek(sfile,0x80,SEEK_SET); // seek to 0x80
    for (int ii=0; ii<256; ii++) {
      fputc(patid[ii],sfile); // order list
    }
    printf("writing instruments...\n");
    fseek(sfile,0xd80,SEEK_SET); // seek to 0xD80, and start writing the instruments
    sk=0xd80;
    for (int ii=0; ii<256; ii++) {
      IS_INS_BLANK[ii]=true;
      // check if the instrument is blank
      if (memcmp(&instrument[ii],&blankIns,64)!=0) {IS_INS_BLANK[ii]=false;}
      if (IS_INS_BLANK[ii]) {
        insparas[ii]=0;continue;
      }
      insparas[ii]=ftell(sfile);
      fwrite(&instrument[ii],1,64,sfile);
    }
    printf("writing sequences...\n");
    for (int ii=0; ii<256; ii++) {
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
      if (IS_SEQ_BLANK[ii]) {
        seqparas[ii]=0;continue;
      }
      seqparas[ii]=ftell(sfile);
      for (int jj=0; jj<8; jj++) {
        fputc(bytable[jj][ii][253],sfile);
        fputc(bytable[jj][ii][254],sfile);
        fputc(bytable[jj][ii][255],sfile);
        for (int kk=0; kk<(bytable[jj][ii][253]+1); kk++) {
          fputc(bytable[jj][ii][kk],sfile); // seqtable
        }
      }
    }
    printf("packing/writing patterns...\n");
    // pattern packer
    for (int ii=0; ii<256; ii++) {
      IS_PAT_BLANK[ii]=true;
      for (int ii1=0; ii1<256; ii1++) {
        for (int ii2=0; ii2<32; ii2++) {
          for (int ii3=0; ii3<5; ii3++) {
            if (pat[ii][ii1][ii2][ii3]!=0) {IS_PAT_BLANK[ii]=false;break;}
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
      fputc(patlength[ii],sfile);
      sk+=11;
      fseek(sfile,sk,SEEK_SET);
      for (int jj=0; jj<getpatlen(ii); jj++) {
        // pack row
        for (int cpack=0; cpack<32; cpack++) {
          // first check if channel is used
          if (pat[ii][jj][cpack][0]==0 &&
            pat[ii][jj][cpack][1]==0 &&
            pat[ii][jj][cpack][2]==0 &&
            pat[ii][jj][cpack][3]==0 &&
            pat[ii][jj][cpack][4]==0) {continue;} // channel is unused
          // then pack this channel
          maskdata=cpack; // set maskbyte to channel number
          if (pat[ii][jj][cpack][0]!=0 || pat[ii][jj][cpack][1]!=0) {maskdata=maskdata|32;} // note AND/OR instrument
          if (pat[ii][jj][cpack][2]!=0) {maskdata=maskdata|64;} // volume value
          if (pat[ii][jj][cpack][3]!=0 || pat[ii][jj][cpack][4]!=0) {maskdata=maskdata|128;} // effect AND/OR effect value
          fputc(maskdata,sfile); CPL++; // write maskbyte
          if (maskdata&32) {fputc(pat[ii][jj][cpack][0],sfile);fputc(pat[ii][jj][cpack][1],sfile); CPL+=2;} // write NOTE and INSTRUMENT if required
          if (maskdata&64) {fputc(pat[ii][jj][cpack][2],sfile); CPL++;} // write VOLUME if required
          if (maskdata&128) {fputc(pat[ii][jj][cpack][3],sfile);fputc(pat[ii][jj][cpack][4],sfile); CPL+=2;} // write EFFECT and EFFECT VALUE if required
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
    for (int ii=0;ii<65280;ii++) {
      if (chip[0].pcm[ii]!=0) {IS_PCM_DATA_BLANK=false; maxpcmwrite=ii;}
    }
    if (!IS_PCM_DATA_BLANK) {
      fwrite(&maxpcmwrite,4,1,sfile);
      fwrite(chip[0].pcm,1,maxpcmwrite,sfile);
    } else {pcmpointer=0;}
    // write pointers
    printf("writing offsets...\n");
    fseek(sfile,0x180,SEEK_SET);
    for (int ii=0;ii<256;ii++) {
      fwrite(&insparas[ii],4,1,sfile);
    }
    for (int ii=0;ii<256;ii++) {
      fwrite(&seqparas[ii],4,1,sfile);
    }
    for (int ii=0;ii<256;ii++) {
      fwrite(&patparas[ii],4,1,sfile);
    }
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
int LoadFile(const char* filename) {
  // load file
  FILE *sfile;
  int sk=0;
  int CurrentRow=0;
  int NextByte=0;
  int NextChannel=0;
  int insparas[256];
  int patparas[256];
  int seqparas[256];
  int commentpointer=0;
  int pcmpointer=0;
  size_t maxpcmread=0;
  int TVER;
  int oplaymode;
  bool IS_SEQ_BLANK[256];
  bool detectChans=false;
  char *checkstr=new char[8];
  sfile=ps_fopen(filename,"rb");
  if (sfile!=NULL) { // LOAD the file
    fseek(sfile,0,SEEK_SET); // seek to 0
    printf("loading file...\n");
    //printf("%d ",ftell(sfile));
    fread(checkstr,1,8,sfile); // magic number
    //printf("%s",checkstr);
    if (checkstr[0]!='T' ||
      checkstr[1]!='R' ||
      checkstr[2]!='A' ||
      checkstr[3]!='C' ||
      checkstr[4]!='K' ||
      checkstr[5]!='8' ||
      checkstr[6]!='B' ||
      checkstr[7]!='T') {
    // check if this is a different module type just in case.
    if (checkstr[0]=='I' &&
      checkstr[1]=='M' &&
      checkstr[2]=='P' &&
      checkstr[3]=='M') { // IT
      return ImportIT(sfile);
    } else {
      fseek(sfile,1080,SEEK_SET);
      fread(checkstr,1,8,sfile); // magic number for MOD
      if ((checkstr[0]=='M' && checkstr[1]=='.' && checkstr[2]=='K' && checkstr[3]=='.')||
        (checkstr[0]=='M' && checkstr[1]=='!' && checkstr[2]=='K' && checkstr[3]=='!')||
     (checkstr[1]=='C' && checkstr[2]=='H' && checkstr[3]=='N')||
     (checkstr[2]=='C' && checkstr[3]=='H')) {
        return ImportMOD(sfile);
      } else {
        printf("error: not a compatible file!\n");fclose(sfile);
    #ifdef SOUNDS
    triggerfx(1);
    #endif
    popbox=PopupBox("Error","not a compatible file!");
    return 1;
      }
    }
    }
    oplaymode=playmode;
    playmode=0;
    CleanupPatterns();
    //printf("%d ",ftell(sfile));
    TVER=fgetsh(sfile); // version
    printf("module version %d\n",TVER);
    origin=strFormat("soundtracker dev%d\n",TVER);
    if (TVER<60) {printf("-applying filter mode compatibility\n");}
    if (TVER<65) {printf("-applying volume column compatibility\n");}
    if (TVER<106) {printf("-applying loop point fix compatibility\n");}
    if (TVER<143) {printf("-applying old sequence format compatibility\n");}
    if (TVER<144) {printf("-applying endianness compatibility\n");}
    if (TVER<145) {printf("-applying channel pan/vol compatibility\n");}
    if (TVER<146) {printf("-applying no channel count compatibility\n");}
    if (TVER<147) {printf("-applying no song length compatibility\n");}
    if (TVER<148) {printf("-applying instrument volume compatibility\n");}
    //if (TVER<??) {printf("-applying legacy instrument compatibility\n");}
    //printf("%d ",ftell(sfile));
    instruments=fgetc(sfile); // instruments
    patterns=fgetc(sfile); // patterns
    songlength=fgetc(sfile); // orders
    defspeed=fgetc(sfile); // speed
    seqs=fgetc(sfile); // sequences
    //fputc(sfile,125); // tempo
    fseek(sfile,16,SEEK_SET);
    name="";
    int ncache;
    for (int i=0; i<32; i++) {
      ncache=fgetc(sfile);
      if (ncache==0) break;
      name+=ncache;
    }
    //printf("%d ",ftell(sfile));
    fseek(sfile,49,SEEK_SET); // seek to 0x31
    //printf("%d ",ftell(sfile));
    //fputc(sfile,0); // default filter mode
    if (TVER<146) {
      detectChans=true;
      channels=1;
    } else {
      channels=fgetc(sfile); // channels
    }
    //fputsh(sfile,0); // flags
    //fputc(sfile,128); // global volume
    //fputc(sfile,0); // global panning
    //fputi(sfile,0); // mute flags
    //fputi(sfile,0); // PCM data pointer
    //fputsh(sfile,0); // reserved
    fseek(sfile,0x3e,SEEK_SET); // seek to 0x3e
    songdf=fgetc(sfile); // detune factor
    fseek(sfile,0x40,SEEK_SET); // seek to 0x40
    fread(defchanvol,1,32,sfile); // channel volume
    fread(defchanpan,1,32,sfile); // channel panning
    fseek(sfile,0x80,SEEK_SET); // seek to 0x80
    for (int ii=0; ii<256; ii++) {
      patid[ii]=fgetc(sfile); // order list
    }
    if (TVER<147) {
      // detect song length.
      songlength=255;
      for (int i=255; i>0; i--) {
        if (patid[i]==0) {
          songlength=i;
        } else {
          break;
        }
      }
      songlength--;
    }
    fseek(sfile,0x3a,SEEK_SET); // seek to 0x3a
    comments=""; // clean comments
    commentpointer=fgeti(sfile);
    if (commentpointer!=0) {
      int v;
      fseek(sfile,commentpointer,SEEK_SET);
      while (1) {
        v=fgetc(sfile);
        if (v==0 || v==EOF) break;
        comments+=v;
      }
    }
    fseek(sfile,0x36,SEEK_SET); // seek to 0x36
    memset(chip[0].pcm,0,65280); // clean PCM memory
    pcmpointer=fgeti(sfile);
    if (pcmpointer!=0) {
      fseek(sfile,pcmpointer,SEEK_SET);
      fread(&maxpcmread,4,1,sfile);
      if (maxpcmread>65280) maxpcmread=65280;
      fread(chip[0].pcm,1,maxpcmread,sfile);
    }
    fseek(sfile,0x180,SEEK_SET);
    for (int ii=0;ii<256;ii++) {
      insparas[ii]=fgeti(sfile);
    }
    for (int ii=0;ii<256;ii++) {
      seqparas[ii]=fgeti(sfile);
    }
    for (int ii=0;ii<256;ii++) {
      patparas[ii]=fgeti(sfile);
    }
    //printf("reading instruments...\n");
    for (int ii=0; ii<256; ii++) {
      fseek(sfile,insparas[ii],SEEK_SET);
      // is it blank?
      if (insparas[ii]==0) {continue;}
      fread(&instrument[ii],1,64,sfile);
      // version<60 filter mode fix
      if (TVER<60) {
        if (instrument[ii].activeEnv&2) {instrument[ii].DFM^=1;}
      }
      // version<144 endianness
      if (TVER<144) {
        instrument[ii].pcmLen=bswapu16(instrument[ii].pcmLen);
        instrument[ii].pcmPos[0]^=instrument[ii].pcmPos[1];
        instrument[ii].pcmPos[1]^=instrument[ii].pcmPos[0];
        instrument[ii].pcmPos[0]^=instrument[ii].pcmPos[1];
        instrument[ii].pcmLoop[0]^=instrument[ii].pcmLoop[1];
        instrument[ii].pcmLoop[1]^=instrument[ii].pcmLoop[0];
        instrument[ii].pcmLoop[0]^=instrument[ii].pcmLoop[1];
        instrument[ii].filterH=bswapu16(instrument[ii].filterH);
      }
      // version<148 instrument volume
      if (TVER<148) {
        instrument[ii].vol=64;
      }

      // version<145 panning
      if (TVER<145) {
        for (int j=0; j<32; j++) {
          defchanpan[j]=((j+1)&2)?96:-96;
          defchanvol[j]=0x80;
        }
      }
      
      // version<75 mono
      if (TVER<75) {
        for (int j=0; j<32; j++) {
          defchanpan[j]=0;
        }
      }
      
      // version<?? force legacy instrument
      /*
      if (TVER<144) {
        instrument[ii].ver&=~0x8000;
      }
      if (instrument[ii].ver&0x8000) { // new instrument
        
      }*/
    }
    //printf("reading sequences...\n");
    if (TVER<143) { // old sequence format
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
      if (TVER<106) {
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
  // unpack patterns
  for (int pointer=0;pointer<256;pointer++) {
    // is it blank?
    if (patparas[pointer]==0) {patlength[pointer]=64;continue;}
    //printf("-unpacking pattern %d-\n",pointer);
    CurrentRow=0;
    sk=patparas[pointer];
    fseek(sfile,sk+1,SEEK_SET);
    int patsize=fgeti(sfile);
    //printf("%d bytes in pattern\n",patsize);
    patlength[pointer]=fgetc(sfile);
    sk=patparas[pointer]+16;
    fseek(sfile,sk,SEEK_SET);
    for (int a=0;a<patsize;a++) {
    NextByte=fgetc(sfile);
    if (NextByte==0) {
      CurrentRow++;
      if (CurrentRow==patlength[pointer]) {break;}
      continue;
    }
    NextChannel=NextByte%32;
    if (detectChans) {
      if (NextChannel>=channels) {
        channels=NextChannel+1;
      }
    }
    if ((NextByte>>5)%2) {
      a++;
      pat[pointer][CurrentRow][NextChannel][0]=fgetc(sfile);
      a++;
      pat[pointer][CurrentRow][NextChannel][1]=fgetc(sfile);
    }
    if ((NextByte>>6)%2) {
      a++;
      pat[pointer][CurrentRow][NextChannel][2]=fgetc(sfile);
      // version<65 volume fix
      if (TVER<65) {
        if (pat[pointer][CurrentRow][NextChannel][0]!=0 && pat[pointer][CurrentRow][NextChannel][2]==0x7f) {pat[pointer][CurrentRow][NextChannel][2]=0;}
      }
    }
    if ((NextByte>>7)%2) {
      a++;
      pat[pointer][CurrentRow][NextChannel][3]=fgetc(sfile);
      a++;
      pat[pointer][CurrentRow][NextChannel][4]=fgetc(sfile);
    }
    }
  }
    //printf("%d ",ftell(sfile));
    fclose(sfile);
    printf("done\n");
    if (!playermode && !fileswitch) {curpat=0;}
    if (oplaymode==1) {Play();}
    if (name=="") {
      g.setTitle(PROGRAM_NAME);
    } else {
      g.setTitle(name+S(" - ")+S(PROGRAM_NAME));
    }
    
    // debug conversion
    for (int i=1; i<255; i++) {
      if (instrument[i].activeEnv) {
        printf("--INSTRUMENT %d--\n",i);
        printf("vol: %s\n",seqToSS(i,0).c_str());
        printf("cut: %s\n",seqToSS(i,1).c_str());
        printf("res: %s\n",seqToSS(i,2).c_str());
        printf("duty: %s\n",seqToSS(i,3).c_str());
        printf("shape: %s\n",seqToSS(i,4).c_str());
        printf("finepitch: %s\n",seqToSS(i,6).c_str());
        printf("pitch: %s\n",seqToSS(i,5).c_str());
        printf("pan: %s\n",seqToSS(i,7).c_str());
      }
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
void SaveInstrument() {
  // save instrument
  FILE *sfile;
  printf("\nplease write filename? ");
  char rfn[256];
  //gets(rfn);
  sfile=ps_fopen(rfn,"wb");
  if (sfile!=NULL) { // write the file
    fseek(sfile,0,SEEK_SET); // seek to 0
    printf("writing header...\n");
    fwrite("TRACKINS",1,8,sfile); // magic number
    fwrite(&instrument[CurrentIns],1,64,sfile);
    printf("writing envelopes...\n");
    for (int jj=0; jj<8; jj++) {
      for (int kk=0; kk<256; kk++) {
        fputc(bytable[jj][instrument[CurrentIns].env[jj]][kk],sfile); // seqtable
      }
    }
    fclose(sfile);
    printf("done\n");
  } else {fprintf(stderr,"error: couldn't open file for writing!\n");}
}
void LoadInstrument() {
  // load instrument
  FILE *sfile;
  printf("\nplease write filename? ");
  char rfn[256];
  int NextFree=0;
  //gets(rfn);
  sfile=ps_fopen(rfn,"rb");
  if (sfile!=NULL) { // read the file
    fseek(sfile,8,SEEK_SET); // seek to 8
    printf("reading header...\n");
    fread(&instrument[CurrentIns],1,64,sfile);
    printf("reading envelopes...\n");
    for (int jj=0; jj<8; jj++) {
      NextFree=AllocateSequence(jj);
      instrument[CurrentIns].env[jj]=NextFree;
      for (int kk=0; kk<256; kk++) {
        bytable[jj][NextFree][kk]=fgetc(sfile); // seqtable
      }
    }
    fclose(sfile);
    printf("done\n");
  } else {fprintf(stderr,"error: couldn't open file for reading!\n");}
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
  if (samplelength<(65280-position)) {
    //for (int ii=0;ii<samplelength;ii++) {
      fseek(sfile,position,SEEK_SET);
      fread(chip[0].pcm+position,1,samplelength,sfile);
    //}
  } else {
    printf(" don't fit!");
  }
  fclose(sfile);
}

void setInputRect() {
  SDL_Rect finalRect;
  finalRect=inputRefRect;
  finalRect.x*=dpiScale;
  finalRect.y*=dpiScale;
  finalRect.w*=dpiScale;
  finalRect.h*=dpiScale;
  SDL_SetTextInputRect(&finalRect);
}

void ClickEvents() {
  reversemode=false;
  // click events
  leftpress=false;
  rightpress=false;
  leftrelease=false;
  rightrelease=false;

  leftclickprev=leftclick;
  leftclick=(mstate.buttons&1);
  if (leftclick!=leftclickprev && leftclick==true) {
    leftpress=true;
  }
  if (leftclick!=leftclickprev && leftclick==false) {
    leftrelease=true;
  }

  rightclickprev=rightclick;
  rightclick=(mstate.buttons&2);
  if (rightclick!=rightclickprev && rightclick==true) {
    rightpress=true;
  }
  if (rightclick!=rightclickprev && rightclick==false) {
    rightrelease=true;
  }
  
  if (popbox.isVisible()) {
    if (leftpress) {
      popbox.hide();
#ifdef ANDROID
      if (noStoragePerm) quit=true;
#endif
    }
    return;
  }

  // hover functions
  for (int ii=0; ii<16; ii++) {
    // decrease hover amount
    hover[ii]--;
    // is hover negative? bound to 0
    if (hover[ii]<0) {hover[ii]=0;}
  }
  if (PIR(32,12,56,24,mstate.x,mstate.y)) {hover[1]+=2;}
  if (PIR(0,12,24,24,mstate.x,mstate.y)) {hover[0]+=2;}
  if (PIR(32,24,56,36,mstate.x,mstate.y)) {hover[2]+=2;}
  if (PIR(0,24,24,36,mstate.x,mstate.y)) {hover[3]+=2;}
  if (PIR(0,36,24,48,mstate.x,mstate.y)) {hover[4]+=2;}
  if (PIR(32,36,56,48,mstate.x,mstate.y)) {hover[5]+=2;}
  if (PIR(640,12,672,28,mstate.x,mstate.y)) {hover[6]+=2;}
  if (PIR(scrW-80,12,scrW-56,28,mstate.x,mstate.y)) {hover[7]+=2;}
  if (PIR(scrW-24,12,scrW,28,mstate.x,mstate.y)) {hover[8]+=2;}
  if (PIR(scrW-48,12,scrW-32,28,mstate.x,mstate.y)) {hover[9]+=2;}
  for (int ii=0; ii<16; ii++) {
    if (hover[ii]>8) {hover[ii]=8;}
  }

  // screen event
  if (iface==UIMobile) {
    if (leftpress) {
      // nullify input
      if (!PIR(inputRefRect.x,inputRefRect.y,inputRefRect.x+inputRefRect.w,inputRefRect.y+inputRefRect.h,mstate.x,mstate.y)) {
        if (inputvar!=NULL) {
          inputvar=NULL;
          inputcurpos=0;
          maxinputsize=0;
          
          SDL_StopTextInput();
          SDL_SetTextInputRect(NULL);
          
          if (inputwhere==1) {
            if (name=="") {
              g.setTitle(PROGRAM_NAME);
            } else {
              g.setTitle(name+S(" - ")+S(PROGRAM_NAME));
            }
          }
          inputwhere=0;
        }
      }
      if (pageSelectShow) {
        if (PIR(0,0,scrW,59,mstate.x,mstate.y)) {
          swX.setOut(&topScroll);
          swX.setRange(0,820-scrW);
          swX.start(mstate.x);
        }
      } else {
        // play button
        if (PIR((scrW/2)-71,13,(scrW/2)-31,47,mstate.x,mstate.y)) {
          if (curtick==0) {
            Play();
          } else {
            playmode=1;
          }
        }
        // pattern play button
        if (PIR((scrW/2)-20,13,(scrW/2)+20,47,mstate.x,mstate.y)) {
          Play();
        }
        // stop button
        if (PIR((scrW/2)+31,13,(scrW/2)+71,47,mstate.x,mstate.y)) {
          playmode=0;
        }
        // skip left button
        if (PIR((scrW/2)-112,13,(scrW/2)-82,47,mstate.x,mstate.y)) {
          if (curpat>0) curpat--;
          if (playmode==1) Play();
        }
        // skip right button
        if (PIR((scrW/2)+82,13,(scrW/2)+112,47,mstate.x,mstate.y)) {
          if (curpat<songlength) {
            curpat++;
          } else {
            curpat=0;
          }
          if (playmode==1) Play();
        }
      }
    }
    if (leftrelease) {
      if (pageSelectShow) {
        if (swX.getStatus()==swHolding) {
          for (int i=0; i<10; i++) {
            if (PIR(((1-mobScroll)*-scrW)+16+(10*8*i)-topScroll,12,((1-mobScroll)*-scrW)+16+72+(10*8*i)-topScroll,48,mstate.x,mstate.y)) {
              screen=pageMap[i];
              if (screen==0) drawpatterns(true);
              pageSelectShow=false;
              break;
            }
          }
        }
      } else {
        // page select
        if (PIR(0,0,48,59,mstate.x,mstate.y)) {
          pageSelectShow=true;
        }
        // alternate view
        if (PIR(scrW-48,0,scrW,59,mstate.x,mstate.y)) {
          mobAltView=!mobAltView;
        }
      }
      swX.end(mstate.x);
    }
    swX.update(mstate.x);
  } else {
    if (leftclick) {
      if (PIR(32,12,56,24,mstate.x,mstate.y)) {screen=1;}
      if (PIR(0,12,24,24,mstate.x,mstate.y)) {screen=0;drawpatterns(true);}
      if (PIR(32,24,56,36,mstate.x,mstate.y)) {screen=2;}
      if (PIR(0,24,24,36,mstate.x,mstate.y)) {screen=3;}
      if (PIR(0,36,24,48,mstate.x,mstate.y)) {screen=4;}
      if (PIR(32,36,56,48,mstate.x,mstate.y)) {screen=5;}
      /*
      if (PIR(640,12,672,28,mstate.x,mstate.y)) {screen=6;}
      */
      if (PIR(0,0,128,11,mstate.x,mstate.y)) {screen=7;}
      if (PIR(64,12,88,24,mstate.x,mstate.y)) {screen=9;}
      if (PIR(64,24,88,36,mstate.x,mstate.y)) {screen=10;}
      if (PIR(64,36,88,48,mstate.x,mstate.y)) {screen=12;}
      if (PIR((scrW/2)-61,13,(scrW/2)-21,37,mstate.x,mstate.y)) {
        if (curtick==0) {
          Play();
        } else {
          playmode=1;
        }
      }
      if (PIR((scrW/2)-61,37,(scrW/2)-21,48,mstate.x,mstate.y)) {reversemode=true;}
      if (PIR((scrW/2)+21,13,(scrW/2)+61,37,mstate.x,mstate.y)) {playmode=0;}
    }
    if (leftpress) {
      if (!PIR(inputRefRect.x,inputRefRect.y,inputRefRect.x+inputRefRect.w,inputRefRect.y+inputRefRect.h,mstate.x,mstate.y)) {
        if (inputvar!=NULL) {
          inputvar=NULL;
          inputcurpos=0;
          maxinputsize=0;
          
          SDL_StopTextInput();
          SDL_SetTextInputRect(NULL);
          
          if (inputwhere==1) {
            if (name=="") {
              g.setTitle(PROGRAM_NAME);
            } else {
              g.setTitle(name+S(" - ")+S(PROGRAM_NAME));
            }
          }
          inputwhere=0;
        }
      }
      if (PIR(272,24,279,36,mstate.x,mstate.y)) {curoctave--; if (curoctave<0) {curoctave=0;}}
      if (PIR(280,24,288,36,mstate.x,mstate.y)) {curoctave++; if (curoctave>8) {curoctave=8;}}
      if (PIR(160,12,167,23,mstate.x,mstate.y)) {speed--; if (speed<1) {speed=1;}}
      if (PIR(168,12,176,23,mstate.x,mstate.y)) {speed++; if (speed>31) {speed=31;}; if (speed<1) {speed=1;}}
      if (PIR(160,24,167,35,mstate.x,mstate.y)) {tempo--; if (tempo<31) {tempo=31;}; FPS=tempo/2.5;}
      if (PIR(168,24,176,35,mstate.x,mstate.y)) {tempo++; if (tempo>255) {tempo=255;}; FPS=tempo/2.5;}
      if (PIR(160,36,167,48,mstate.x,mstate.y)) {if (curpat>0) curpat--; if (playmode==1) Play();}
      if (PIR(168,36,176,48,mstate.x,mstate.y)) {
        if (curpat<songlength) {
          curpat++;
        } else {
          curpat=0;
        }
        if (playmode==1) Play();
      }
      if (PIR(272,12,279,24,mstate.x,mstate.y)) {patid[curpat]--;}
      if (PIR(280,12,288,24,mstate.x,mstate.y)) {patid[curpat]++;}
      if (PIR(272,36,279,48,mstate.x,mstate.y)) {patlength[patid[curpat]]--;}
      if (PIR(280,36,288,48,mstate.x,mstate.y)) {patlength[patid[curpat]]++;}
      if (PIR((scrW/2)-20,37,(scrW/2)+20,48,mstate.x,mstate.y)) {StepPlay();}
      if (PIR((scrW/2)-20,13,(scrW/2)+20,37,mstate.x,mstate.y)) {Play();}
      if (PIR(scrW-34*8,24,scrW-28*8,36,mstate.x,mstate.y)) {follow=!follow;}
      
      if (PIR(scrW-28*8,12,scrW-27*8,24,mstate.x,mstate.y)) {
        curins--;
        if (curins<0) curins=0;
      }
      if (PIR(scrW-27*8,12,scrW-26*8,24,mstate.x,mstate.y)) {
        curins++;
        if (curins>255) curins=255;
      }
      
      if (PIR(96,12,136,24,mstate.x,mstate.y)) {speedlock=!speedlock;}
      if (PIR(96,24,136,36,mstate.x,mstate.y)) {tempolock=!tempolock;}
    }
  }

  // events only in pattern view
  if (screen==0) {
    float patStartX, patStartY;
    patStartX=(scrW*((float)dpiScale)-(24+chanstodisplay*96)*curzoom)/2;
    patStartY=(60+((scrH*dpiScale)-60)/2);
    if (mstate.y>60) {
      if (leftpress) {
        selStart=(int)((mstate.y*dpiScale-patStartY-(3*curzoom)+curpatrow*12*curzoom)/(12*curzoom));
        if (selStart<0) selStart=0;
        if (selStart>=getpatlen(patid[curpat])) selStart=getpatlen(patid[curpat])-1;
        
        curedchan=(mstate.x*dpiScale-16*curzoom-patStartX)/(96*curzoom);
        if (curedchan<0) curedchan=0;
        switch ((int)((mstate.x*dpiScale-16*curzoom-patStartX)/(8*curzoom))%12) {
        case 0: curedmode=0; break;
        case 1: curedmode=0; break;
        case 2: curedmode=0; break;
        case 3: curedmode=0; break;
        case 4: curedmode=1; break;
        case 5: curedmode=1; break;
        case 6: curedmode=2; break;
        case 7: curedmode=2; break;
        case 8: curedmode=2; break;
        case 9: curedmode=3; break;
        case 10: curedmode=4; break;
        case 11: curedmode=4; break;
        }
        if (curedchan>=chanstodisplay) {
          curedchan=chanstodisplay-1;
          curedmode=4;
        }
      }
      if (leftclick) {
        selEnd=(int)((mstate.y*dpiScale-patStartY-(3*curzoom)+curpatrow*12*curzoom)/(12*curzoom));
        if (selEnd<0) selEnd=0;
        if (selEnd>=getpatlen(patid[curpat])) selEnd=getpatlen(patid[curpat])-1;
        
        curselchan=(mstate.x*dpiScale-16*curzoom-patStartX)/(96*curzoom);
        if (curselchan<0) curselchan=0;
        switch ((int)((mstate.x*dpiScale-16*curzoom-patStartX)/(8*curzoom))%12) {
        case 0: curselmode=0; break;
        case 1: curselmode=0; break;
        case 2: curselmode=0; break;
        case 3: curselmode=0; break;
        case 4: curselmode=1; break;
        case 5: curselmode=1; break;
        case 6: curselmode=2; break;
        case 7: curselmode=2; break;
        case 8: curselmode=2; break;
        case 9: curselmode=3; break;
        case 10: curselmode=4; break;
        case 11: curselmode=4; break;
        }
        if (curselchan>=chanstodisplay) {
          curselchan=chanstodisplay-1;
          curselmode=4;
        }
        drawpatterns(true);
      }
    }
    if ((mstate.z-prevZ)<0) {
      if (follow) {
        curstep-=(mstate.z-prevZ);
        if (curstep>(getpatlen(patid[curpat])-1)) {
          curstep-=patlength[patid[curpat]];
          curpat++;
        }
      } else {
        curpatrow-=(mstate.z-prevZ)*3;
        curpatrow=fmin(curpatrow,getpatlen(patid[curpat])-1);
      }
      drawpatterns(true);
    }
    if ((mstate.z-prevZ)>0) {
      if (follow) {
        curstep-=(mstate.z-prevZ);
        if (curstep<0) {
          if (curpat!=0) {
            curpat--;
            curstep+=patlength[patid[curpat]];
          } else {
            curstep=-1;
          }
        }
      } else {
        curpatrow-=(mstate.z-prevZ)*3;
        curpatrow=fmax(curpatrow,0);
      }
      drawpatterns(true);
    }
  }
  // events only in instrument view
  if (screen==1) {
    // set the envelope ONLY in normal mode
    if (!hexmode) {
    if (mstate.buttons&1 && PIR(0,90,scrW-285,scrH-30,mstate.x,mstate.y)) {
      if (CurrentEnv==6) {bytable[CurrentEnv][instrument[CurrentIns].env[CurrentEnv]][minval(minval(252,(512/valuewidth)+(scrollpos)+scrW-285),((scrollpos*valuewidth)+mstate.x)/valuewidth)]=maxval(minval(127,interpolate(94,scrH-42,0.5)-mstate.y),-128);}
      else {bytable[CurrentEnv][instrument[CurrentIns].env[CurrentEnv]][minval(minval(252,(512/valuewidth)+(scrollpos)+scrW-285),((scrollpos*valuewidth)+mstate.x)/valuewidth)]=maxval(minval(255,((scrH-43)-mstate.y)/((scrH-128)/256)),0);}
    }
    // line draw tool
    if (PIR(0,90,scrW-285,scrH-30,mstate.x,mstate.y)) {
    if (rightpress) {
      linex1=mstate.x;
      liney1=mstate.y;
    }
    if (rightclick) {
      // make line
      for (int ii=(linex1/valuewidth)+scrollpos;ii<minval(253,(mstate.x/valuewidth)+scrollpos);ii++) {
        bytable[CurrentEnv][instrument[CurrentIns].env[CurrentEnv]][ii]=interpolate(maxval(minval(255,(scrH-42)-liney1),0),
        maxval(minval(255,(scrH-42)-mstate.y),0),
        ((float)ii-((float)linex1/valuewidth)-(float)scrollpos)/(minval(253,(mstate.x/valuewidth)+scrollpos)-(linex1/valuewidth)-scrollpos));
      }
    }
    }
    } else {
    for (int ii=0;ii<253;ii++) {
      if (PIR(8+((ii%21)*24),102+((ii/21)*24),15+((ii%21)*24),114+((ii/21)*24),mstate.x,mstate.y)) {
        if (leftpress) bytable[CurrentEnv][instrument[CurrentIns].env[CurrentEnv]][ii]+=16;
        if (rightpress) bytable[CurrentEnv][instrument[CurrentIns].env[CurrentEnv]][ii]-=16;}
      if (PIR(16+((ii%21)*24),102+((ii/21)*24),32+((ii%21)*24),114+((ii/21)*24),mstate.x,mstate.y)) {
        if (leftpress) bytable[CurrentEnv][instrument[CurrentIns].env[CurrentEnv]][ii]+=1;
        if (rightpress) bytable[CurrentEnv][instrument[CurrentIns].env[CurrentEnv]][ii]-=1;}
    }
    }
    // change envelopes
    if (leftpress) {
      // TODO this
      if (PIR(528,132,784,144,mstate.x,mstate.y)) {
        inputvar=&tempInsName;
        inputcurpos=utf8findcpos(inputvar->c_str(),float(mstate.x-524)/8);
        maxinputsize=32;
        inputwhere=2;
        
        inputRefRect.x=528;
        inputRefRect.y=132;
        inputRefRect.w=784-528;
        inputRefRect.h=144-132;
        setInputRect();
        SDL_StartTextInput();
      }
      if (PIR(24,72,72,84,mstate.x,mstate.y)) {CurrentEnv=0;}
      if (PIR(80,72,128,84,mstate.x,mstate.y)) {CurrentEnv=1;}
      if (PIR(136,72,176,84,mstate.x,mstate.y)) {CurrentEnv=2;}
      if (PIR(184,72,216,84,mstate.x,mstate.y)) {CurrentEnv=3;}
      if (PIR(224,72,264,84,mstate.x,mstate.y)) {CurrentEnv=4;}
      if (PIR(272,72,312,84,mstate.x,mstate.y)) {CurrentEnv=5;}
      if (PIR(320,72,376,84,mstate.x,mstate.y)) {CurrentEnv=6;}
      if (PIR(384,72,408,84,mstate.x,mstate.y)) {CurrentEnv=7;}
      if (PIR(456,72,463,84,mstate.x,mstate.y)) {instrument[CurrentIns].env[CurrentEnv]++;}
      if (PIR(464,72,471,84,mstate.x,mstate.y)) {instrument[CurrentIns].env[CurrentEnv]--;}
      if (PIR(480,72,496,84,mstate.x,mstate.y)) {instrument[CurrentIns].env[CurrentEnv]=AllocateSequence(CurrentEnv);}
      if (PIR(504,72,512,84,mstate.x,mstate.y)) {
        for (int ii=0;ii<254;ii++) {
          bytable[CurrentEnv][instrument[CurrentIns].env[CurrentEnv]][ii]=0;
        }
        bytable[CurrentEnv][instrument[CurrentIns].env[CurrentEnv]][254]=255;
        bytable[CurrentEnv][instrument[CurrentIns].env[CurrentEnv]][255]=255;}
      if (PIR(192,60,199,72,mstate.x,mstate.y)) {CurrentIns++;}
      if (PIR(200,60,207,72,mstate.x,mstate.y)) {CurrentIns--; if (CurrentIns<1) CurrentIns=255;}
      // the right pane buttons
      if (PIR(scrW-184,84,scrW-177,96,mstate.x,mstate.y)) {instrument[CurrentIns].noteOffset++;}
      if (PIR(scrW-176,84,scrW-169,96,mstate.x,mstate.y)) {instrument[CurrentIns].noteOffset--;}
      if (PIR(scrW-168,84,scrW-161,96,mstate.x,mstate.y)) {instrument[CurrentIns].noteOffset+=12;}
      if (PIR(scrW-160,84,scrW-153,96,mstate.x,mstate.y)) {instrument[CurrentIns].noteOffset-=12;}
      if (PIR(0,72,16,84,mstate.x,mstate.y)) {instrument[CurrentIns].activeEnv=(instrument[CurrentIns].activeEnv)^(1<<CurrentEnv);}
      // bottom thing
      if (PIR(88,scrH-18,95,scrH-6,mstate.x,mstate.y)) {bytable[CurrentEnv][instrument[CurrentIns].env[CurrentEnv]][254]++;}
      if (PIR(96,scrH-18,104,scrH-6,mstate.x,mstate.y)) {bytable[CurrentEnv][instrument[CurrentIns].env[CurrentEnv]][254]--;}
      if (PIR(208,scrH-18,215,scrH-6,mstate.x,mstate.y)) {bytable[CurrentEnv][instrument[CurrentIns].env[CurrentEnv]][255]++;}
      if (PIR(216,scrH-18,224,scrH-6,mstate.x,mstate.y)) {bytable[CurrentEnv][instrument[CurrentIns].env[CurrentEnv]][255]--;}
      if (PIR(320,scrH-18,327,scrH-6,mstate.x,mstate.y)) {bytable[CurrentEnv][instrument[CurrentIns].env[CurrentEnv]][253]++;}
      if (PIR(328,scrH-18,336,scrH-6,mstate.x,mstate.y)) {bytable[CurrentEnv][instrument[CurrentIns].env[CurrentEnv]][253]--;}
      // the right pane buttons
      if (PIR(scrW-216,156,scrW-192,168,mstate.x,mstate.y)) {instrument[CurrentIns].DFM^=1;} // LOW
      if (PIR(scrW-184,156,scrW-152,168,mstate.x,mstate.y)) {instrument[CurrentIns].DFM^=2;} // HIGH
      if (PIR(scrW-144,156,scrW-112,168,mstate.x,mstate.y)) {instrument[CurrentIns].DFM^=4;} // BAND
      if (PIR(scrW-272,180,scrW-248,192,mstate.x,mstate.y)) {instrument[CurrentIns].DFM^=8;} // pcm flag
      if (PIR(scrW-272,252,scrW-256,264,mstate.x,mstate.y)) {instrument[CurrentIns].DFM^=16;} // rm flag
      if (PIR(scrW-272,264,scrW-256,276,mstate.x,mstate.y)) {instrument[CurrentIns].flags^=32;} // sync flag
      if (PIR(scrW-272,288,scrW-208,300,mstate.x,mstate.y)) {instrument[CurrentIns].flags^=1;} // reset osc flag
      // not this one
      if (PIR(488,60,512,72,mstate.x,mstate.y)) {hexmode=!hexmode;}
      // but yes these ones
      if (PIR(scrW-200,180,scrW-193,191,mstate.x,mstate.y)) {instrument[CurrentIns].DFM^=128;} 
      if (PIR(scrW-192,180,scrW-185,191,mstate.x,mstate.y)) {instrument[CurrentIns].pcmPos[1]+=16;}
      if (PIR(scrW-184,180,scrW-177,191,mstate.x,mstate.y)) {instrument[CurrentIns].pcmPos[1]++;}
      if (PIR(scrW-176,180,scrW-169,191,mstate.x,mstate.y)) {instrument[CurrentIns].pcmPos[0]+=16;}
      if (PIR(scrW-168,180,scrW-160,191,mstate.x,mstate.y)) {instrument[CurrentIns].pcmPos[0]++;}
      if (PIR(scrW-88,180,scrW-81,191,mstate.x,mstate.y)) {instrument[CurrentIns].pcmLen+=4096;}
      if (PIR(scrW-80,180,scrW-73,191,mstate.x,mstate.y)) {instrument[CurrentIns].pcmLen+=256;}
      if (PIR(scrW-72,180,scrW-63,191,mstate.x,mstate.y)) {instrument[CurrentIns].pcmLen+=16;}
      if (PIR(scrW-64,180,scrW-56,191,mstate.x,mstate.y)) {instrument[CurrentIns].pcmLen++;}

      // filter. somebody screwed up the order.
      if (PIR(scrW-(800-592),216,scrW-(800-599),228,mstate.x,mstate.y)) {instrument[CurrentIns].filterH-=4096;}
      if (PIR(scrW-(800-600),216,scrW-(800-607),228,mstate.x,mstate.y)) {instrument[CurrentIns].filterH-=256;}
      if (PIR(scrW-(800-608),216,scrW-(800-615),228,mstate.x,mstate.y)) {instrument[CurrentIns].filterH-=16;}
      if (PIR(scrW-(800-616),216,scrW-(800-623),228,mstate.x,mstate.y)) {instrument[CurrentIns].filterH--;}

      if (PIR(scrW-(800-624),252,scrW-(800-631),264,mstate.x,mstate.y)) {instrument[CurrentIns].LFO++;}
      if (PIR(scrW-(800-632),252,scrW-(800-640),264,mstate.x,mstate.y)) {instrument[CurrentIns].LFO--;}

      if (PIR(scrW-(800-600),324,scrW-(800-607),336,mstate.x,mstate.y)) {instrument[CurrentIns].flags+=64;}
      if (PIR(scrW-(800-608),324,scrW-(800-616),336,mstate.x,mstate.y)) {instrument[CurrentIns].flags-=64;}
    }
    if (rightpress) {
      if (PIR(scrW-(800-600),180,scrW-(800-607),191,mstate.x,mstate.y)) {instrument[CurrentIns].DFM^=128;}
      if (PIR(scrW-(800-608),180,scrW-(800-615),191,mstate.x,mstate.y)) {instrument[CurrentIns].pcmPos[1]-=16;}
      if (PIR(scrW-(800-616),180,scrW-(800-623),191,mstate.x,mstate.y)) {instrument[CurrentIns].pcmPos[1]--;}
      if (PIR(scrW-(800-624),180,scrW-(800-631),191,mstate.x,mstate.y)) {instrument[CurrentIns].pcmPos[0]-=16;}
      if (PIR(scrW-(800-632),180,scrW-(800-640),191,mstate.x,mstate.y)) {instrument[CurrentIns].pcmPos[0]--;}
      if (PIR(scrW-(800-712),180,scrW-(800-719),191,mstate.x,mstate.y)) {instrument[CurrentIns].pcmLen-=4096;}
      if (PIR(scrW-(800-720),180,scrW-(800-727),191,mstate.x,mstate.y)) {instrument[CurrentIns].pcmLen-=256;}
      if (PIR(scrW-(800-728),180,scrW-(800-735),191,mstate.x,mstate.y)) {instrument[CurrentIns].pcmLen-=16;}
      if (PIR(scrW-(800-736),180,scrW-(800-744),191,mstate.x,mstate.y)) {instrument[CurrentIns].pcmLen--;}
      
      // ditto and mimikyu
      if (PIR(scrW-(800-592),216,scrW-(800-599),228,mstate.x,mstate.y)) {instrument[CurrentIns].filterH+=4096;}
      if (PIR(scrW-(800-600),216,scrW-(800-607),228,mstate.x,mstate.y)) {instrument[CurrentIns].filterH+=256;}
      if (PIR(scrW-(800-608),216,scrW-(800-615),228,mstate.x,mstate.y)) {instrument[CurrentIns].filterH+=16;}
      if (PIR(scrW-(800-616),216,scrW-(800-623),228,mstate.x,mstate.y)) {instrument[CurrentIns].filterH++;}
    }
    if (leftclick) {
      if (PIR(0,scrH-18,8,scrH-6,mstate.x,mstate.y)) {scrollpos--; if (scrollpos<0) {scrollpos=0;};}
      if (PIR(500,scrH-18,512,scrH-6,mstate.x,mstate.y)) {scrollpos++; if (scrollpos>124) {scrollpos=124;};}
      // bottom thing
      if (PIR(72,scrH-18,79,scrH-6,mstate.x,mstate.y)) {bytable[CurrentEnv][instrument[CurrentIns].env[CurrentEnv]][254]++;}
      if (PIR(80,scrH-18,87,scrH-6,mstate.x,mstate.y)) {bytable[CurrentEnv][instrument[CurrentIns].env[CurrentEnv]][254]--;}
      if (PIR(192,scrH-18,199,scrH-6,mstate.x,mstate.y)) {bytable[CurrentEnv][instrument[CurrentIns].env[CurrentEnv]][255]++;}
      if (PIR(200,scrH-18,207,scrH-6,mstate.x,mstate.y)) {bytable[CurrentEnv][instrument[CurrentIns].env[CurrentEnv]][255]--;}
      if (PIR(304,scrH-18,311,scrH-6,mstate.x,mstate.y)) {bytable[CurrentEnv][instrument[CurrentIns].env[CurrentEnv]][253]++;}
      if (PIR(312,scrH-18,319,scrH-6,mstate.x,mstate.y)) {bytable[CurrentEnv][instrument[CurrentIns].env[CurrentEnv]][253]--;}
      // other stuff
      if (PIR(240,60,272,71,mstate.x,mstate.y)) {SaveInstrument();}
      if (PIR(280,60,312,71,mstate.x,mstate.y)) {LoadInstrument();}
      if (PIR(scrW-(800-608),252,scrW-(800-615),264,mstate.x,mstate.y)) {instrument[CurrentIns].LFO++;}
      if (PIR(scrW-(800-616),252,scrW-(800-623),264,mstate.x,mstate.y)) {instrument[CurrentIns].LFO--;}
    }

  }
  // events only in diskop view
  if (screen==2) {
    if (leftpress) {
      // filename input
      if (PIR(72,scrH-24,scrW,scrH,mstate.x,mstate.y)) {
        inputvar=&curfname;
        inputcurpos=utf8findcpos(inputvar->c_str(),float(mstate.x-72)/8);
        maxinputsize=4095;
        inputwhere=5;

        inputRefRect.x=72;
        inputRefRect.y=scrH-24;
        inputRefRect.w=scrW-72;
        inputRefRect.h=24;
        setInputRect();
        SDL_StartTextInput();
      }
      
      if (PIR(168,60,200,72,mstate.x,mstate.y)) {
        int success=LoadFile((S(curdir)+S(SDIR_SEPARATOR)+curfname).c_str());
        if (success==0) {
          loadedfname=S(curdir)+S(SDIR_SEPARATOR)+curfname;
        }
      }
      if (PIR(208,60,240,72,mstate.x,mstate.y)) {
        int success;
        if (loadedfname==(S(curdir)+S(SDIR_SEPARATOR)+curfname)) {
          success=SaveFile();
          if (success==0) {
            triggerfx(4);
          }
        } else {
          bool ffound;
          ffound=false;
          for (size_t i=0; i<filenames.size(); i++) {
            if (curfname==filenames[i].name) {
              popbox=PopupBox("Warning","overwrite file "+curfname+"? click on Save again to confirm.");
              triggerfx(1);
              ffound=true;
              break;
            }
          }
          if (!ffound) {
            success=SaveFile();
            loadedfname=S(curdir)+S(SDIR_SEPARATOR)+curfname;
            if (success==0) triggerfx(4);
          }
        }
      }
      /*if (PIR(248,60,320,72,mstate.x,mstate.y)) {int success=ImportMOD(filenames[selectedfileindex-1].name.c_str());}*/
      //if (PIR(328,60,400,72,mstate.x,mstate.y)) {int success=ImportS3M();}
      //if (PIR(408,60,472,72,mstate.x,mstate.y)) {int success=ImportIT();}
      if (PIR(552,60,632,72,mstate.x,mstate.y)) {
        printf("\nplease write filename? ");
        char rfn[256];
        printf("\nwrite position? ");
        int writeposition=0;
        writeposition=0;
        LoadSample(rfn,writeposition);}
      if (PIR(640,60,744,72,mstate.x,mstate.y)) {
        printf("\nplease write filename? ");
        char rfn[256];
        printf("\nwrite position? ");
        int writeposition=0;
        writeposition=0;
        LoadRawSample(rfn,writeposition);
      }
      if (iface!=UIMobile && !PIR(0,scrH-24,scrW,scrH,mstate.x,mstate.y)) for (int ii=diskopscrollpos; ii<minval(diskopscrollpos+((int)(scrH/12)-12),filenames.size()); ii++) {
        if (PIR(0,120+(ii*12)-(diskopscrollpos*12),scrW-8,131+(ii*12)-(diskopscrollpos*12),mstate.x,mstate.y)) {
          if (selectedfileindex==ii+1) {
            if (filenames[ii].isdir) {
              if (strcmp(curdir,"")!=0) {
                if (curdir[strlen(curdir)-1]!=DIR_SEPARATOR) {
                  strcat(curdir,SDIR_SEPARATOR);
                }
              }
              strcat(curdir,filenames[ii].name.c_str());
              diskopscrollpos=0;
              selectedfileindex=-1;
              curfname="";
              int peerrno=print_entry(curdir);
              if (peerrno<0) {
                popbox=PopupBox("Error","can't read directory! ("+S(strerror(-peerrno))+")");
                triggerfx(1);
                break;
              }
            } else {
              int success;
              success=LoadFile((S(curdir)+S(SDIR_SEPARATOR)+curfname).c_str());
              if (success==0) {
                loadedfname=S(curdir)+S(SDIR_SEPARATOR)+curfname;
                // adjust channel count if needed
                maxCTD=(scrW*dpiScale-24*curzoom)/(96*curzoom);
                if (maxCTD>channels) maxCTD=channels;
                if (maxCTD<1) maxCTD=1;
                chanstodisplay=maxCTD;
                drawmixerlayer();
              }
            }
          } else {
            selectedfileindex=ii+1;
            curfname=filenames[ii].name;
          }
        }
      }
      if (iface!=UIMobile && PIR(0,108,790,119,mstate.x,mstate.y)) {
        ParentDir(curdir);
        print_entry(curdir);
        diskopscrollpos=0;
        selectedfileindex=-1;
      }
    }
    if (iface!=UIMobile && leftclick) {
      if (PIR(scrW-8,108,scrW,119,mstate.x,mstate.y)) {diskopscrollpos--;if (diskopscrollpos<0) {diskopscrollpos=0;}}
      if (PIR(scrW-8,scrH-36,scrW,scrH-24,mstate.x,mstate.y)) {diskopscrollpos++;if (diskopscrollpos>maxval(0,(int)filenames.size()-((int)(scrH/12)-12))) {diskopscrollpos=maxval(0,(int)filenames.size()-((int)(scrH/12)-12));}}
    }
    if ((mstate.z-prevZ)<0) {
      diskopscrollpos-=(mstate.z-prevZ)*4;if (diskopscrollpos>maxval(0,(int)filenames.size()-((int)(scrH/12)-12))) {diskopscrollpos=maxval(0,(int)filenames.size()-((int)(scrH/12)-12));}
    }
    if ((mstate.z-prevZ)>0) {
      diskopscrollpos-=(mstate.z-prevZ)*4;if (diskopscrollpos<0) {diskopscrollpos=0;}
    }

    // mobile UI scroll thingy
    if (iface==UIMobile) {
      if (!PIR(0,scrH-24,scrW,scrH,mstate.x,mstate.y)) {
        if (leftpress) {
          diskopSwiper.setOut(&doScroll);
          diskopSwiper.setRange(0,36*maxval(0,(int)filenames.size()-((int)(scrH/12)-12)/3));
          diskopSwiper.setFrict(0.5);
          diskopSwiper.start(mstate.y);
        }
        if (diskopSwiper.getStatus()==swHolding) {
          // selection check here
          for (int ii=diskopscrollpos; ii<minval(diskopscrollpos+((int)(scrH/12)-12)/3,filenames.size()); ii++) {
            if (PIR(0,125+(ii*36)-(diskopscrollpos*36),scrW-8,136+24+(ii*36)-(diskopscrollpos*36),mstate.x,mstate.y)) {
              selectedfileindex=ii+1;
            }
          }
        } else {
          selectedfileindex=-1;
        }
        if (leftrelease) {
          if (diskopSwiper.getStatus()!=swDragging) {
            diskopSwiper.end(mstate.y);
            if (PIR(0,108,790,119,mstate.x,mstate.y)) {
              ParentDir(curdir);
              print_entry(curdir);
              diskopSwiper.setRange(0,36*maxval(0,(int)filenames.size()-((int)(scrH/12)-12)/3));
              diskopscrollpos=0;
              selectedfileindex=-1;
            }
            if (selectedfileindex>=0) {
              curfname=filenames[selectedfileindex-1].name;
              if (filenames[selectedfileindex-1].isdir) {
                if (strcmp(curdir,"")!=0) {
                  if (curdir[strlen(curdir)-1]!=DIR_SEPARATOR) {
                    strcat(curdir,SDIR_SEPARATOR);
                  }
                }
                strcat(curdir,curfname.c_str());
                diskopscrollpos=0;
                selectedfileindex=-1;
                curfname="";
                int peerrno=print_entry(curdir);
                diskopSwiper.setRange(0,36*maxval(0,(int)filenames.size()-((int)(scrH/12)-12)/3));
                if (peerrno<0) {
                  popbox=PopupBox("Error","can't read directory! ("+S(strerror(-peerrno))+")");
                  triggerfx(1);
                }
              } else {
                int success;
                selectedfileindex=-1;
                success=LoadFile((S(curdir)+S(SDIR_SEPARATOR)+curfname).c_str());
                if (success==0) {
                  loadedfname=S(curdir)+S(SDIR_SEPARATOR)+curfname;
                  // adjust channel count if needed
                  maxCTD=(scrW*dpiScale-24*curzoom)/(96*curzoom);
                  if (maxCTD>channels) maxCTD=channels;
                  if (maxCTD<1) maxCTD=1;
                  chanstodisplay=maxCTD;
                  drawmixerlayer();
                }
              }
            }
          } else {
            diskopSwiper.end(mstate.y);
          }
        }
      }
      diskopSwiper.update(mstate.y);
      diskopscrollpos=doScroll/36;
    }
  }
  // events only in song view
  if (screen==3) {
    if (leftpress) {
    if (PIR(88,84,344,96,mstate.x,mstate.y)) {
      inputvar=&name;
      inputcurpos=utf8findcpos(inputvar->c_str(),float(mstate.x-84)/8);
      maxinputsize=32;
      inputwhere=1;

      inputRefRect.x=88;
      inputRefRect.y=84;
      inputRefRect.w=344-88;
      inputRefRect.h=96-84;
      setInputRect();
      SDL_StartTextInput();
    }
    if (PIR(760,60,767,72,mstate.x,mstate.y)) {defspeed--;if (defspeed<1) {defspeed=1;};speed=defspeed;}
    if (PIR(768,60,776,72,mstate.x,mstate.y)) {defspeed++;if (defspeed<1) {defspeed=1;};speed=defspeed;}
    if (PIR(456,60,463,72,mstate.x,mstate.y)) {songdf--;}
    if (PIR(464,60,472,72,mstate.x,mstate.y)) {songdf++;}
    if (PIR(568,60,575,72,mstate.x,mstate.y)) {songlength--;}
    if (PIR(576,60,584,72,mstate.x,mstate.y)) {songlength++;}
    if (PIR(184,276,248,288,mstate.x,mstate.y)) {playmode=2;curtick=1;cvol[0]=127;cpcmpos[0]=0;cmode[0]=1;cfreq[0]=2300;cbound[0]=131071;cloop[0]=0;}
    if (PIR(32,276,64,288,mstate.x,mstate.y)) {screen=11;}
    }
    if (leftclick) {
    if (PIR(440,60,447,72,mstate.x,mstate.y)) {songdf--;}
    if (PIR(448,60,455,72,mstate.x,mstate.y)) {songdf++;}
    if (PIR(552,60,559,72,mstate.x,mstate.y)) {songlength--;}
    if (PIR(560,60,567,72,mstate.x,mstate.y)) {songlength++;}
    }
  }
  // events only in mixer view
  if (screen==4) {
    int mixerdrawoffset=(scrW/2)-chanstodisplay*48-12;
    if (leftclick) {
      for (int ii=0;ii<chanstodisplay;ii++) {
        if (PIR(56+(ii*96)+mixerdrawoffset,84,63+(ii*96)+mixerdrawoffset,95,mstate.x,mstate.y)) {defchanvol[ii+curedpage]++;if (defchanvol[ii+curedpage]>128) {defchanvol[ii+curedpage]=128;}}
        if (PIR(64+(ii*96)+mixerdrawoffset,84,72+(ii*96)+mixerdrawoffset,95,mstate.x,mstate.y)) {defchanvol[ii+curedpage]--;if (defchanvol[ii+curedpage]>250) {defchanvol[ii+curedpage]=0;}}
        if (PIR(56+(ii*96)+mixerdrawoffset,96,63+(ii*96)+mixerdrawoffset,108,mstate.x,mstate.y)) {defchanpan[ii+curedpage]++;}
        if (PIR(64+(ii*96)+mixerdrawoffset,96,72+(ii*96)+mixerdrawoffset,108,mstate.x,mstate.y)) {defchanpan[ii+curedpage]--;}
      }
    }
    if (leftpress) {
      for (int ii=0;ii<chanstodisplay;ii++) {
        if (!kb[SDL_SCANCODE_LSHIFT]) {if (PIR(16+(ii*96)+mixerdrawoffset,60,96+(ii*96)+mixerdrawoffset,72,mstate.x,mstate.y)) {muted[ii+curedpage]=!muted[ii+curedpage];}}
        else {if (PIR(16+(ii*96)+mixerdrawoffset,60,96+(ii*96)+mixerdrawoffset,72,mstate.x,mstate.y)) {midion[ii+curedpage]=!midion[ii+curedpage];}}
      }
    }
    if (rightpress) {
      for (int ii=0;ii<chanstodisplay;ii++) {
        if (PIR(16+(ii*96)+mixerdrawoffset,60,96+(ii*96)+mixerdrawoffset,72,mstate.x,mstate.y)) {
          bool solomode=true;
          for (int jj=0;jj<32;jj++) {
            if (jj==ii+curedpage) {continue;}
            if (!muted[jj]) {solomode=false;break;}
          }
          if (solomode) {
            for (int jj=0;jj<32;jj++) {
              muted[jj]=false;
            }
          } else {
            for (int jj=0;jj<32;jj++) {
              muted[jj]=true;
            }
            muted[ii+curedpage]=false;
          }
        }
      }
    }
  }
  // events only in config view
  if (screen==5) {
    if (leftpress) {
      if (PIR(280,96,368,107,mstate.x,mstate.y)) {settings::distortion=!settings::distortion;}
      if (PIR(280,132,368,143,mstate.x,mstate.y)) {settings::cubicspline=!settings::cubicspline;}
      if (PIR(280,240,368,251,mstate.x,mstate.y)) {settings::nofilters=!settings::nofilters;}
      if (PIR(280,252,368,264,mstate.x,mstate.y)) {settings::muffle=!settings::muffle;}
    }
  }
  // events only in sound effect editor
  if (screen==10) {
  }
  // events only in PCM editor
  if (screen==11) {
    if (leftclick) {
      if (pcmeditenable) chip[0].pcm[(int)(((float)mstate.x)*pow(2.0f,-pcmeditscale))+pcmeditoffset]=minval(127,maxval(-127,mstate.y-(scrH/2)));
    }
  }
  prevZ=mstate.z;
}
void FastTracker() {
  // FT2-like pattern editor
  int firstChan, firstMode;
  int lastChan, lastMode;
  int selTop, selBottom;
  // scroll code
  if (kbpressed[SDL_SCANCODE_UP]) {
    // go back
    if (playmode==0) {
      curtick=1;
      curstep--;
      if (curstep<0) {
        curstep=0;
      }
      selEnd=curstep;
      if (!kb[SDL_SCANCODE_LSHIFT]) {
        selStart=curstep;
        curselchan=curedchan;
        curselmode=curedmode;
      }
    }
    drawpatterns(true);
  }
  if (kbpressed[SDL_SCANCODE_DOWN]) {
    // skipping
    if (playmode==0) {
      curtick=1;
      curstep++;
      if (curstep>(getpatlen(patid[curpat])-1)) {
        curstep=0;
        curpat++;
      }
      selEnd=curstep;
      if (!kb[SDL_SCANCODE_LSHIFT]) {
        selStart=curstep;
        curselchan=curedchan;
        curselmode=curedmode;
      }
    }
    drawpatterns(true);
  }
  if (kbpressed[SDL_SCANCODE_RIGHT]) {
    curedmode++;
    if (curedmode>4) {
      curedchan++;
      curedmode=0;
      if (curedchan>=chanstodisplay) {
        curedchan=chanstodisplay-1;
        curedmode=4;
      }
    }
    if (!kb[SDL_SCANCODE_LSHIFT]) {
      curselchan=curedchan;
      curselmode=curedmode;
      selEnd=selStart;
    }
    drawpatterns(true);
  }
  if (kbpressed[SDL_SCANCODE_LEFT]) {
    curedmode--;
    if (curedmode<0) {
      curedchan--;
      curedmode=4;
      if (curedchan<0) {
        curedchan=0;
        curedmode=0;
      }
    }
    if (!kb[SDL_SCANCODE_LSHIFT]) {
      curselchan=curedchan;
      curselmode=curedmode;
      selEnd=selStart;
    }
    drawpatterns(true);
  }
  if (kbpressed[SDL_SCANCODE_DELETE]) {
    // delete from selection start to end
    firstChan=curedchan; firstMode=curedmode;
    lastChan=curselchan; lastMode=curselmode;
    selTop=selStart; selBottom=selEnd;
    if (lastChan<firstChan) {
      lastChan^=firstChan;
      firstChan^=lastChan;
      lastChan^=firstChan;
      
      lastMode^=firstMode;
      firstMode^=lastMode;
      lastMode^=firstMode;
    } else if (lastChan==firstChan) {
      if (lastMode<firstMode) {
        lastMode^=firstMode;
        firstMode^=lastMode;
        lastMode^=firstMode;
      }
    }
    if (selBottom<selTop) {
      selBottom^=selTop;
      selTop^=selBottom;
      selBottom^=selTop;
    }
    for (int i=firstChan; i<=lastChan; i++) {
      for (int j=(i==firstChan)?firstMode:0; (j<5 && (i<lastChan || j<=lastMode)); j++) {
        for (int k=selTop; k<=selBottom; k++) {
          pat[patid[curpat]][k][curedpage+i][j]=0x00;
        }
      }
    }
    drawpatterns(true);
    selStart=curstep;
    selEnd=curstep;
    curselchan=curedchan;
    curselmode=curedmode;
  }
  if (curedmode==0) {
  // silences and stuff
  if (kbpressed[SDL_SCANCODE_EQUALS]) {pat[patid[curpat]][curstep][curedpage+curedchan][0]=0x0d;EditSkip();}
  if (kbpressed[SDL_SCANCODE_1]) {pat[patid[curpat]][curstep][curedpage+curedchan][0]=0x0f;EditSkip();}
  if (kbpressed[SDL_SCANCODE_BACKSLASH]) {pat[patid[curpat]][curstep][curedpage+curedchan][0]=0x0e;EditSkip();}
  // notes, main octave
  if (kbpressed[SDL_SCANCODE_Z]) {pat[patid[curpat]][curstep][curedpage+curedchan][0]=0x01+minval(curoctave<<4,0x90);EditSkip();}
  if (kbpressed[SDL_SCANCODE_X]) {pat[patid[curpat]][curstep][curedpage+curedchan][0]=0x03+minval(curoctave<<4,0x90);EditSkip();}
  if (kbpressed[SDL_SCANCODE_C]) {pat[patid[curpat]][curstep][curedpage+curedchan][0]=0x05+minval(curoctave<<4,0x90);EditSkip();}
  if (kbpressed[SDL_SCANCODE_V]) {pat[patid[curpat]][curstep][curedpage+curedchan][0]=0x06+minval(curoctave<<4,0x90);EditSkip();}
  if (kbpressed[SDL_SCANCODE_B]) {pat[patid[curpat]][curstep][curedpage+curedchan][0]=0x08+minval(curoctave<<4,0x90);EditSkip();}
  if (kbpressed[SDL_SCANCODE_N]) {pat[patid[curpat]][curstep][curedpage+curedchan][0]=0x0a+minval(curoctave<<4,0x90);EditSkip();}
  if (kbpressed[SDL_SCANCODE_M]) {pat[patid[curpat]][curstep][curedpage+curedchan][0]=0x0c+minval(curoctave<<4,0x90);EditSkip();}
  if (kbpressed[SDL_SCANCODE_S]) {pat[patid[curpat]][curstep][curedpage+curedchan][0]=0x02+minval(curoctave<<4,0x90);EditSkip();}
  if (kbpressed[SDL_SCANCODE_D]) {pat[patid[curpat]][curstep][curedpage+curedchan][0]=0x04+minval(curoctave<<4,0x90);EditSkip();}
  if (kbpressed[SDL_SCANCODE_G]) {pat[patid[curpat]][curstep][curedpage+curedchan][0]=0x07+minval(curoctave<<4,0x90);EditSkip();}
  if (kbpressed[SDL_SCANCODE_H]) {pat[patid[curpat]][curstep][curedpage+curedchan][0]=0x09+minval(curoctave<<4,0x90);EditSkip();}
  if (kbpressed[SDL_SCANCODE_J]) {pat[patid[curpat]][curstep][curedpage+curedchan][0]=0x0b+minval(curoctave<<4,0x90);EditSkip();}
  // notes, 2nd octave
  if (kbpressed[SDL_SCANCODE_Q]) {pat[patid[curpat]][curstep][curedpage+curedchan][0]=0x01+minval((curoctave+1)<<4,0x90);EditSkip();}
  if (kbpressed[SDL_SCANCODE_W]) {pat[patid[curpat]][curstep][curedpage+curedchan][0]=0x03+minval((curoctave+1)<<4,0x90);EditSkip();}
  if (kbpressed[SDL_SCANCODE_E]) {pat[patid[curpat]][curstep][curedpage+curedchan][0]=0x05+minval((curoctave+1)<<4,0x90);EditSkip();}
  if (kbpressed[SDL_SCANCODE_R]) {pat[patid[curpat]][curstep][curedpage+curedchan][0]=0x06+minval((curoctave+1)<<4,0x90);EditSkip();}
  if (kbpressed[SDL_SCANCODE_T]) {pat[patid[curpat]][curstep][curedpage+curedchan][0]=0x08+minval((curoctave+1)<<4,0x90);EditSkip();}
  if (kbpressed[SDL_SCANCODE_Y]) {pat[patid[curpat]][curstep][curedpage+curedchan][0]=0x0a+minval((curoctave+1)<<4,0x90);EditSkip();}
  if (kbpressed[SDL_SCANCODE_U]) {pat[patid[curpat]][curstep][curedpage+curedchan][0]=0x0c+minval((curoctave+1)<<4,0x90);EditSkip();}
  if (kbpressed[SDL_SCANCODE_2]) {pat[patid[curpat]][curstep][curedpage+curedchan][0]=0x02+minval((curoctave+1)<<4,0x90);EditSkip();}
  if (kbpressed[SDL_SCANCODE_3]) {pat[patid[curpat]][curstep][curedpage+curedchan][0]=0x04+minval((curoctave+1)<<4,0x90);EditSkip();}
  if (kbpressed[SDL_SCANCODE_5]) {pat[patid[curpat]][curstep][curedpage+curedchan][0]=0x07+minval((curoctave+1)<<4,0x90);EditSkip();}
  if (kbpressed[SDL_SCANCODE_6]) {pat[patid[curpat]][curstep][curedpage+curedchan][0]=0x09+minval((curoctave+1)<<4,0x90);EditSkip();}
  if (kbpressed[SDL_SCANCODE_7]) {pat[patid[curpat]][curstep][curedpage+curedchan][0]=0x0b+minval((curoctave+1)<<4,0x90);EditSkip();}
  if (kbpressed[SDL_SCANCODE_I]) {pat[patid[curpat]][curstep][curedpage+curedchan][0]=0x01+minval((curoctave+2)<<4,0x90);EditSkip();}
  if (kbpressed[SDL_SCANCODE_9]) {pat[patid[curpat]][curstep][curedpage+curedchan][0]=0x02+minval((curoctave+2)<<4,0x90);EditSkip();}
  if (kbpressed[SDL_SCANCODE_O]) {pat[patid[curpat]][curstep][curedpage+curedchan][0]=0x03+minval((curoctave+2)<<4,0x90);EditSkip();}
  if (kbpressed[SDL_SCANCODE_0]) {pat[patid[curpat]][curstep][curedpage+curedchan][0]=0x04+minval((curoctave+2)<<4,0x90);EditSkip();}
  if (kbpressed[SDL_SCANCODE_P]) {pat[patid[curpat]][curstep][curedpage+curedchan][0]=0x05+minval((curoctave+2)<<4,0x90);EditSkip();}
  /*
  if (kbpressed[66]) {pat[patid[curpat]][curstep][curedpage+curedchan][0]=0x06+minval((curoctave+2)<<4,0x90);EditSkip();}
  if (kbpressed[65]) {pat[patid[curpat]][curstep][curedpage+curedchan][0]=0x07+minval((curoctave+2)<<4,0x90);EditSkip();}
  */
  }
  if (curedmode==1) {
  if (kbpressed[SDL_SCANCODE_0]) {pat[patid[curpat]][curstep][curedpage+curedchan][1]=(pat[patid[curpat]][curstep][curedpage+curedchan][1]<<4);drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_1]) {pat[patid[curpat]][curstep][curedpage+curedchan][1]=(pat[patid[curpat]][curstep][curedpage+curedchan][1]<<4)+1;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_2]) {pat[patid[curpat]][curstep][curedpage+curedchan][1]=(pat[patid[curpat]][curstep][curedpage+curedchan][1]<<4)+2;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_3]) {pat[patid[curpat]][curstep][curedpage+curedchan][1]=(pat[patid[curpat]][curstep][curedpage+curedchan][1]<<4)+3;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_4]) {pat[patid[curpat]][curstep][curedpage+curedchan][1]=(pat[patid[curpat]][curstep][curedpage+curedchan][1]<<4)+4;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_5]) {pat[patid[curpat]][curstep][curedpage+curedchan][1]=(pat[patid[curpat]][curstep][curedpage+curedchan][1]<<4)+5;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_6]) {pat[patid[curpat]][curstep][curedpage+curedchan][1]=(pat[patid[curpat]][curstep][curedpage+curedchan][1]<<4)+6;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_7]) {pat[patid[curpat]][curstep][curedpage+curedchan][1]=(pat[patid[curpat]][curstep][curedpage+curedchan][1]<<4)+7;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_8]) {pat[patid[curpat]][curstep][curedpage+curedchan][1]=(pat[patid[curpat]][curstep][curedpage+curedchan][1]<<4)+8;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_9]) {pat[patid[curpat]][curstep][curedpage+curedchan][1]=(pat[patid[curpat]][curstep][curedpage+curedchan][1]<<4)+9;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_A]) {pat[patid[curpat]][curstep][curedpage+curedchan][1]=(pat[patid[curpat]][curstep][curedpage+curedchan][1]<<4)+10;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_B]) {pat[patid[curpat]][curstep][curedpage+curedchan][1]=(pat[patid[curpat]][curstep][curedpage+curedchan][1]<<4)+11;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_C]) {pat[patid[curpat]][curstep][curedpage+curedchan][1]=(pat[patid[curpat]][curstep][curedpage+curedchan][1]<<4)+12;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_D]) {pat[patid[curpat]][curstep][curedpage+curedchan][1]=(pat[patid[curpat]][curstep][curedpage+curedchan][1]<<4)+13;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_E]) {pat[patid[curpat]][curstep][curedpage+curedchan][1]=(pat[patid[curpat]][curstep][curedpage+curedchan][1]<<4)+14;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_F]) {pat[patid[curpat]][curstep][curedpage+curedchan][1]=(pat[patid[curpat]][curstep][curedpage+curedchan][1]<<4)+15;drawpatterns(true);}
  }
  if (curedmode==2) {
  if (kbpressed[SDL_SCANCODE_0]) {pat[patid[curpat]][curstep][curedpage+curedchan][2]=(pat[patid[curpat]][curstep][curedpage+curedchan][2]<<4);drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_1]) {pat[patid[curpat]][curstep][curedpage+curedchan][2]=(pat[patid[curpat]][curstep][curedpage+curedchan][2]<<4)+1;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_2]) {pat[patid[curpat]][curstep][curedpage+curedchan][2]=(pat[patid[curpat]][curstep][curedpage+curedchan][2]<<4)+2;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_3]) {pat[patid[curpat]][curstep][curedpage+curedchan][2]=(pat[patid[curpat]][curstep][curedpage+curedchan][2]<<4)+3;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_4]) {pat[patid[curpat]][curstep][curedpage+curedchan][2]=(pat[patid[curpat]][curstep][curedpage+curedchan][2]<<4)+4;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_5]) {pat[patid[curpat]][curstep][curedpage+curedchan][2]=(pat[patid[curpat]][curstep][curedpage+curedchan][2]<<4)+5;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_6]) {pat[patid[curpat]][curstep][curedpage+curedchan][2]=(pat[patid[curpat]][curstep][curedpage+curedchan][2]<<4)+6;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_7]) {pat[patid[curpat]][curstep][curedpage+curedchan][2]=(pat[patid[curpat]][curstep][curedpage+curedchan][2]<<4)+7;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_8]) {pat[patid[curpat]][curstep][curedpage+curedchan][2]=(pat[patid[curpat]][curstep][curedpage+curedchan][2]<<4)+8;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_9]) {pat[patid[curpat]][curstep][curedpage+curedchan][2]=(pat[patid[curpat]][curstep][curedpage+curedchan][2]<<4)+9;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_A]) {pat[patid[curpat]][curstep][curedpage+curedchan][2]=(pat[patid[curpat]][curstep][curedpage+curedchan][2]<<4)+10;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_B]) {pat[patid[curpat]][curstep][curedpage+curedchan][2]=(pat[patid[curpat]][curstep][curedpage+curedchan][2]<<4)+11;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_C]) {pat[patid[curpat]][curstep][curedpage+curedchan][2]=(pat[patid[curpat]][curstep][curedpage+curedchan][2]<<4)+12;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_D]) {pat[patid[curpat]][curstep][curedpage+curedchan][2]=(pat[patid[curpat]][curstep][curedpage+curedchan][2]<<4)+13;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_E]) {pat[patid[curpat]][curstep][curedpage+curedchan][2]=(pat[patid[curpat]][curstep][curedpage+curedchan][2]<<4)+14;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_F]) {pat[patid[curpat]][curstep][curedpage+curedchan][2]=(pat[patid[curpat]][curstep][curedpage+curedchan][2]<<4)+15;drawpatterns(true);}
  }
  if (curedmode==3) {
  if (kbpressed[SDL_SCANCODE_A]) {pat[patid[curpat]][curstep][curedpage+curedchan][3]=1;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_B]) {pat[patid[curpat]][curstep][curedpage+curedchan][3]=2;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_C]) {pat[patid[curpat]][curstep][curedpage+curedchan][3]=3;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_D]) {pat[patid[curpat]][curstep][curedpage+curedchan][3]=4;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_E]) {pat[patid[curpat]][curstep][curedpage+curedchan][3]=5;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_F]) {pat[patid[curpat]][curstep][curedpage+curedchan][3]=6;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_G]) {pat[patid[curpat]][curstep][curedpage+curedchan][3]=7;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_H]) {pat[patid[curpat]][curstep][curedpage+curedchan][3]=8;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_I]) {pat[patid[curpat]][curstep][curedpage+curedchan][3]=9;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_J]) {pat[patid[curpat]][curstep][curedpage+curedchan][3]=10;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_K]) {pat[patid[curpat]][curstep][curedpage+curedchan][3]=11;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_L]) {pat[patid[curpat]][curstep][curedpage+curedchan][3]=12;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_M]) {pat[patid[curpat]][curstep][curedpage+curedchan][3]=13;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_N]) {pat[patid[curpat]][curstep][curedpage+curedchan][3]=14;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_O]) {pat[patid[curpat]][curstep][curedpage+curedchan][3]=15;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_P]) {pat[patid[curpat]][curstep][curedpage+curedchan][3]=16;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_Q]) {pat[patid[curpat]][curstep][curedpage+curedchan][3]=17;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_R]) {pat[patid[curpat]][curstep][curedpage+curedchan][3]=18;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_S]) {pat[patid[curpat]][curstep][curedpage+curedchan][3]=19;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_T]) {pat[patid[curpat]][curstep][curedpage+curedchan][3]=20;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_U]) {pat[patid[curpat]][curstep][curedpage+curedchan][3]=21;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_V]) {pat[patid[curpat]][curstep][curedpage+curedchan][3]=22;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_W]) {pat[patid[curpat]][curstep][curedpage+curedchan][3]=23;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_X]) {pat[patid[curpat]][curstep][curedpage+curedchan][3]=24;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_Y]) {pat[patid[curpat]][curstep][curedpage+curedchan][3]=25;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_Z]) {pat[patid[curpat]][curstep][curedpage+curedchan][3]=26;drawpatterns(true);}
  }
  if (curedmode==4) {
  if (kbpressed[SDL_SCANCODE_0]) {pat[patid[curpat]][curstep][curedpage+curedchan][4]=(pat[patid[curpat]][curstep][curedpage+curedchan][4]<<4);drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_1]) {pat[patid[curpat]][curstep][curedpage+curedchan][4]=(pat[patid[curpat]][curstep][curedpage+curedchan][4]<<4)+1;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_2]) {pat[patid[curpat]][curstep][curedpage+curedchan][4]=(pat[patid[curpat]][curstep][curedpage+curedchan][4]<<4)+2;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_3]) {pat[patid[curpat]][curstep][curedpage+curedchan][4]=(pat[patid[curpat]][curstep][curedpage+curedchan][4]<<4)+3;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_4]) {pat[patid[curpat]][curstep][curedpage+curedchan][4]=(pat[patid[curpat]][curstep][curedpage+curedchan][4]<<4)+4;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_5]) {pat[patid[curpat]][curstep][curedpage+curedchan][4]=(pat[patid[curpat]][curstep][curedpage+curedchan][4]<<4)+5;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_6]) {pat[patid[curpat]][curstep][curedpage+curedchan][4]=(pat[patid[curpat]][curstep][curedpage+curedchan][4]<<4)+6;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_7]) {pat[patid[curpat]][curstep][curedpage+curedchan][4]=(pat[patid[curpat]][curstep][curedpage+curedchan][4]<<4)+7;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_8]) {pat[patid[curpat]][curstep][curedpage+curedchan][4]=(pat[patid[curpat]][curstep][curedpage+curedchan][4]<<4)+8;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_9]) {pat[patid[curpat]][curstep][curedpage+curedchan][4]=(pat[patid[curpat]][curstep][curedpage+curedchan][4]<<4)+9;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_A]) {pat[patid[curpat]][curstep][curedpage+curedchan][4]=(pat[patid[curpat]][curstep][curedpage+curedchan][4]<<4)+10;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_B]) {pat[patid[curpat]][curstep][curedpage+curedchan][4]=(pat[patid[curpat]][curstep][curedpage+curedchan][4]<<4)+11;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_C]) {pat[patid[curpat]][curstep][curedpage+curedchan][4]=(pat[patid[curpat]][curstep][curedpage+curedchan][4]<<4)+12;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_D]) {pat[patid[curpat]][curstep][curedpage+curedchan][4]=(pat[patid[curpat]][curstep][curedpage+curedchan][4]<<4)+13;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_E]) {pat[patid[curpat]][curstep][curedpage+curedchan][4]=(pat[patid[curpat]][curstep][curedpage+curedchan][4]<<4)+14;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_F]) {pat[patid[curpat]][curstep][curedpage+curedchan][4]=(pat[patid[curpat]][curstep][curedpage+curedchan][4]<<4)+15;drawpatterns(true);}
  }
}
void InstrumentTest(int testnote, int testchan) {
  // code scrapped.
  // this will be fixed soon.
}
void ModPlug() {
  // ModPlug Tracker-OpenMPT-like pattern editor


}
void RunTestNote(int keycode) {
  // notes, main octave
  switch (keycode) {
  case SDL_SCANCODE_Z: InstrumentTest(0x01+minval(curoctave<<4,0x90),FreeChannel()); break;
  case SDL_SCANCODE_X: InstrumentTest(0x03+minval(curoctave<<4,0x90),FreeChannel()); break;
  case SDL_SCANCODE_C: InstrumentTest(0x05+minval(curoctave<<4,0x90),FreeChannel()); break;
  case SDL_SCANCODE_V: InstrumentTest(0x06+minval(curoctave<<4,0x90),FreeChannel()); break;
  case SDL_SCANCODE_B: InstrumentTest(0x08+minval(curoctave<<4,0x90),FreeChannel()); break;
  case SDL_SCANCODE_N: InstrumentTest(0x0a+minval(curoctave<<4,0x90),FreeChannel()); break;
  case SDL_SCANCODE_M: InstrumentTest(0x0c+minval(curoctave<<4,0x90),FreeChannel()); break;
  case SDL_SCANCODE_S: InstrumentTest(0x02+minval(curoctave<<4,0x90),FreeChannel()); break;
  case SDL_SCANCODE_D: InstrumentTest(0x04+minval(curoctave<<4,0x90),FreeChannel()); break;
  case SDL_SCANCODE_G: InstrumentTest(0x07+minval(curoctave<<4,0x90),FreeChannel()); break;
  case SDL_SCANCODE_H: InstrumentTest(0x09+minval(curoctave<<4,0x90),FreeChannel()); break;
  case SDL_SCANCODE_J: InstrumentTest(0x0b+minval(curoctave<<4,0x90),FreeChannel()); break;
  // TODO why did I not use the enum?!
  //case 72: InstrumentTest(0x01+minval((curoctave+1)<<4,0x90),FreeChannel()); break;
  //case 12: InstrumentTest(0x02+minval((curoctave+1)<<4,0x90),FreeChannel()); break;
  //case 73: InstrumentTest(0x03+minval((curoctave+1)<<4,0x90),FreeChannel()); break;
  //case 60: InstrumentTest(0x04+minval((curoctave+1)<<4,0x90),FreeChannel()); break;
  // notes, 2nd octave
  case SDL_SCANCODE_Q: InstrumentTest(0x01+minval((curoctave+1)<<4,0x90),FreeChannel()); break;
  case SDL_SCANCODE_W: InstrumentTest(0x03+minval((curoctave+1)<<4,0x90),FreeChannel()); break;
  case SDL_SCANCODE_E: InstrumentTest(0x05+minval((curoctave+1)<<4,0x90),FreeChannel()); break;
  case SDL_SCANCODE_R: InstrumentTest(0x06+minval((curoctave+1)<<4,0x90),FreeChannel()); break;
  case SDL_SCANCODE_T: InstrumentTest(0x08+minval((curoctave+1)<<4,0x90),FreeChannel()); break;
  case SDL_SCANCODE_Y: InstrumentTest(0x0a+minval((curoctave+1)<<4,0x90),FreeChannel()); break;
  case SDL_SCANCODE_U: InstrumentTest(0x0c+minval((curoctave+1)<<4,0x90),FreeChannel()); break;
  case SDL_SCANCODE_2: InstrumentTest(0x02+minval((curoctave+1)<<4,0x90),FreeChannel()); break;
  case SDL_SCANCODE_3: InstrumentTest(0x04+minval((curoctave+1)<<4,0x90),FreeChannel()); break;
  case SDL_SCANCODE_5: InstrumentTest(0x07+minval((curoctave+1)<<4,0x90),FreeChannel()); break;
  case SDL_SCANCODE_6: InstrumentTest(0x09+minval((curoctave+1)<<4,0x90),FreeChannel()); break;
  case SDL_SCANCODE_7: InstrumentTest(0x0b+minval((curoctave+1)<<4,0x90),FreeChannel()); break;
  case SDL_SCANCODE_I: InstrumentTest(0x01+minval((curoctave+2)<<4,0x90),FreeChannel()); break;
  case SDL_SCANCODE_9: InstrumentTest(0x02+minval((curoctave+2)<<4,0x90),FreeChannel()); break;
  case SDL_SCANCODE_O: InstrumentTest(0x03+minval((curoctave+2)<<4,0x90),FreeChannel()); break;
  case SDL_SCANCODE_0: InstrumentTest(0x04+minval((curoctave+2)<<4,0x90),FreeChannel()); break;
  case SDL_SCANCODE_P: InstrumentTest(0x05+minval((curoctave+2)<<4,0x90),FreeChannel()); break;
  case 66: InstrumentTest(0x06+minval((curoctave+2)<<4,0x90),FreeChannel()); break;
  case 65: InstrumentTest(0x07+minval((curoctave+2)<<4,0x90),FreeChannel()); break;
  }
}
void MuteControls() {
  if (kbpressed[SDL_SCANCODE_1]) {muted[0]=!muted[0];}
  if (kbpressed[SDL_SCANCODE_2]) {muted[1]=!muted[1];}
  if (kbpressed[SDL_SCANCODE_3]) {muted[2]=!muted[2];}
  if (kbpressed[SDL_SCANCODE_4]) {muted[3]=!muted[3];}
  if (kbpressed[SDL_SCANCODE_5]) {muted[4]=!muted[4];}
  if (kbpressed[SDL_SCANCODE_6]) {muted[5]=!muted[5];}
  if (kbpressed[SDL_SCANCODE_7]) {muted[6]=!muted[6];}
  if (kbpressed[SDL_SCANCODE_8]) {muted[7]=!muted[7];}
  if (kbpressed[SDL_SCANCODE_Q]) {muted[8]=!muted[8];}
  if (kbpressed[SDL_SCANCODE_W]) {muted[9]=!muted[9];}
  if (kbpressed[SDL_SCANCODE_E]) {muted[10]=!muted[10];}
  if (kbpressed[SDL_SCANCODE_R]) {muted[11]=!muted[11];}
  if (kbpressed[SDL_SCANCODE_T]) {muted[12]=!muted[12];}
  if (kbpressed[SDL_SCANCODE_Y]) {muted[13]=!muted[13];}
  if (kbpressed[SDL_SCANCODE_U]) {muted[14]=!muted[14];}
  if (kbpressed[SDL_SCANCODE_I]) {muted[15]=!muted[15];}
  if (kbpressed[SDL_SCANCODE_A]) {muted[16]=!muted[16];}
  if (kbpressed[SDL_SCANCODE_S]) {muted[17]=!muted[17];}
  if (kbpressed[SDL_SCANCODE_D]) {muted[18]=!muted[18];}
  if (kbpressed[SDL_SCANCODE_F]) {muted[19]=!muted[19];}
  if (kbpressed[SDL_SCANCODE_G]) {muted[20]=!muted[20];}
  if (kbpressed[SDL_SCANCODE_H]) {muted[21]=!muted[21];}
  if (kbpressed[SDL_SCANCODE_J]) {muted[22]=!muted[22];}
  if (kbpressed[SDL_SCANCODE_K]) {muted[23]=!muted[23];}
  if (kbpressed[SDL_SCANCODE_Z]) {muted[24]=!muted[24];}
  if (kbpressed[SDL_SCANCODE_X]) {muted[25]=!muted[25];}
  if (kbpressed[SDL_SCANCODE_C]) {muted[26]=!muted[26];}
  if (kbpressed[SDL_SCANCODE_V]) {muted[27]=!muted[27];}
  if (kbpressed[SDL_SCANCODE_B]) {muted[28]=!muted[28];}
  if (kbpressed[SDL_SCANCODE_N]) {muted[29]=!muted[29];}
  if (kbpressed[SDL_SCANCODE_M]) {muted[30]=!muted[30];}
  if (kbpressed[SDL_SCANCODE_COMMA]) {muted[31]=!muted[31];}
}
void MuteAllChannels() {
  for (int su=0;su<8*(1+((channels-1)>>3));su++) {
    if (sfxplaying && su==chantoplayfx) continue;
    cvol[su]=0;
    chip[su>>3].chan[su&7].vol=0;
  }
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

void SFXControls() {
  if (kbpressed[SDL_SCANCODE_1]) {triggerfx(0);}
  if (kbpressed[SDL_SCANCODE_2]) {triggerfx(1);}
  if (kbpressed[SDL_SCANCODE_3]) {triggerfx(2);}
  if (kbpressed[SDL_SCANCODE_4]) {triggerfx(3);}
  if (kbpressed[SDL_SCANCODE_5]) {triggerfx(4);}
  if (kbpressed[SDL_SCANCODE_6]) {triggerfx(5);}
  if (kbpressed[SDL_SCANCODE_7]) {triggerfx(6);}
  if (kbpressed[SDL_SCANCODE_8]) {triggerfx(7);}
  if (kbpressed[SDL_SCANCODE_Q]) {triggerfx(8);}
  if (kbpressed[SDL_SCANCODE_W]) {triggerfx(9);}
  if (kbpressed[SDL_SCANCODE_E]) {triggerfx(10);}
  if (kbpressed[SDL_SCANCODE_R]) {triggerfx(11);}
  if (kbpressed[SDL_SCANCODE_T]) {triggerfx(12);}
  if (kbpressed[SDL_SCANCODE_Y]) {triggerfx(13);}
  if (kbpressed[SDL_SCANCODE_U]) {triggerfx(14);}
  if (kbpressed[SDL_SCANCODE_I]) {triggerfx(15);}
  if (kbpressed[SDL_SCANCODE_A]) {triggerfx(16);}
  if (kbpressed[SDL_SCANCODE_S]) {triggerfx(17);}
  if (kbpressed[SDL_SCANCODE_D]) {triggerfx(18);}
  if (kbpressed[SDL_SCANCODE_F]) {triggerfx(19);}
  if (kbpressed[SDL_SCANCODE_G]) {triggerfx(20);}
  if (kbpressed[SDL_SCANCODE_H]) {triggerfx(21);}
  if (kbpressed[SDL_SCANCODE_J]) {triggerfx(22);}
  if (kbpressed[SDL_SCANCODE_K]) {triggerfx(23);}
  if (kbpressed[SDL_SCANCODE_Z]) {triggerfx(24);}
  if (kbpressed[SDL_SCANCODE_X]) {triggerfx(25);}
  if (kbpressed[SDL_SCANCODE_C]) {triggerfx(26);}
  if (kbpressed[SDL_SCANCODE_V]) {triggerfx(27);}
  if (kbpressed[SDL_SCANCODE_B]) {triggerfx(28);}
  if (kbpressed[SDL_SCANCODE_N]) {triggerfx(29);}
  // pausers
  if (kbpressed[SDL_SCANCODE_M]) {if (playmode==0) {playmode=1;} else {playmode=0;};triggerfx(30);}
  if (kbpressed[SDL_SCANCODE_COMMA]) {if (playmode==0) {playmode=1;} else {playmode=0;};triggerfx(31);}
}
void KeyboardEvents() {
  const unsigned char* ks=SDL_GetKeyboardState(NULL);
  // check for presses
  for (int cntkb=0;cntkb<255;cntkb++) {
  kblast[cntkb]=kb[cntkb];
  kbpressed[cntkb]=false;
  kb[cntkb]=ks[cntkb];
  if (kb[cntkb]!=kblast[cntkb] && kb[cntkb]==true) {
    kbpressed[cntkb]=true;
    //if (verbose) {cout << cntkb;}
  }}
  // main code here
  if (edittype && screen==0) {FastTracker();} else {ModPlug();}
  if (screen==4) {MuteControls();}
  if (kbpressed[SDL_SCANCODE_PAGEDOWN]) {curedpage++; if (curedpage>(channels-chanstodisplay)) {curedpage=(channels-chanstodisplay);
    #ifdef SOUNDS
    triggerfx(1);
    #endif
  };drawpatterns(true);drawmixerlayer();}
  if (kbpressed[SDL_SCANCODE_PAGEUP]) {curedpage--; if (curedpage<0) {
    #ifdef SOUNDS
    triggerfx(1);
    #endif
    curedpage=0;
  };drawpatterns(true);drawmixerlayer();}
  if (kbpressed[SDL_SCANCODE_END]) {
    if (kb[SDL_SCANCODE_LSHIFT]) {
      curzoom++;
      maxCTD=(scrW*dpiScale-24*curzoom)/(96*curzoom);
      if (maxCTD<1) maxCTD=1;
      if (chanstodisplay>maxCTD) {
        chanstodisplay=maxCTD;
      }
      if (curedpage>(channels-chanstodisplay)) {
        curedpage=(channels-chanstodisplay);
      }
      drawpatterns(true);
      drawmixerlayer();
    }
  }
  if (kbpressed[SDL_SCANCODE_HOME]) {
    if (kb[SDL_SCANCODE_LSHIFT]) {
      curzoom--;
      if (curzoom<1) {
        triggerfx(1);
        curzoom=1;
      } else {
        maxCTD=(scrW*dpiScale-24*curzoom)/(96*curzoom);
        if (maxCTD<1) maxCTD=1;
        if (maxCTD>=channels) maxCTD=channels;
        chanstodisplay=maxCTD;
        if (curedpage>(channels-chanstodisplay)) {
          curedpage=(channels-chanstodisplay);
        }
        drawpatterns(true);
        drawmixerlayer();
      }
    }
  }
  
  // playback
  if (kbpressed[SDL_SCANCODE_F5]) {
    if (curtick==0) {
      Play();
    } else {
      playmode=1;
    }
  }
  if (kbpressed[SDL_SCANCODE_F6]) {
    Play();
  }
  if (kbpressed[SDL_SCANCODE_F8]) {
    playmode=0;
  }
  
  if (kbpressed[SDL_SCANCODE_TAB]) {MuteAllChannels();}
  if (kbpressed[SDL_SCANCODE_APPLICATION] || kbpressed[SDL_SCANCODE_GRAVE]) {ntsc=!ntsc; if (ntsc) {
    if (tempo==125) {
      tempo=150;
      FPS=60;
    }
  } else {
    if (tempo==150) {
      tempo=125;
      FPS=50;
    }
  }
  }
  if (screen==9 || screen==7) {
    SFXControls();
  }
  if (screen==1) {
    if (kbpressed[SDL_SCANCODE_MINUS]) {valuewidth--; if (valuewidth<1) {valuewidth=1;}}
    if (kbpressed[SDL_SCANCODE_EQUALS]) {valuewidth++;}
  }
  if (screen==11) {
    if (kb[SDL_SCANCODE_LEFT]) {pcmeditoffset-=4*maxval(1,(int)pow(2.0f,-pcmeditscale));}
    if (kb[SDL_SCANCODE_RIGHT]) {pcmeditoffset+=4*maxval(1,(int)pow(2.0f,-pcmeditscale));}
    if (kbpressed[SDL_SCANCODE_MINUS]) {pcmeditscale--;}
    if (kbpressed[SDL_SCANCODE_EQUALS]) {pcmeditscale++;}
    if (kbpressed[SDL_SCANCODE_SPACE]) {pcmeditenable=!pcmeditenable;}
  }
}
void drawdisp() {
  int delta;
  int firstChan, firstMode;
  int lastChan, lastMode;
  int selTop, selBottom;
  static int pointsToDraw=735;
  
  float patStartX, patStartY, patOffY;
  
  if (playermode) {return;}
  ClickEvents();
  KeyboardEvents();
  if (screen==0) {
    patStartX=(scrW*((float)dpiScale)-(24+chanstodisplay*96)*curzoom)/2;
    patStartY=(60+((scrH*dpiScale)-60)/2);
    patOffY=(curpatrow*12)*curzoom;
    Color barcol;
    
    // top bar
    int rectX;
    for (int i=0; i<chanstodisplay; i++) {
      if (cshape[i+curedpage]==0) {
        barcol=mapHSV(250-(cduty[i+curedpage]),0.67,1);
      } else {
        barcol=g._WRAP_map_rgb(
          (cshape[i+curedpage]==4 || cshape[i+curedpage]==1 || cshape[i+curedpage]==5)?(255):(0),
          (cshape[i+curedpage]!=5)?(255):(0),
          (cshape[i+curedpage]!=1 && cshape[i+curedpage]!=2)?(255):(0)
        );
      }
      barcol.a=0.1+float(cvol[i+curedpage])/200.0f;
      rectX=patStartX/dpiScale+(3+5.5+(12*i))*8*(float(curzoom)/float(dpiScale));
      g._WRAP_draw_filled_rectangle(rectX+6,67,rectX+6+38*(float(curzoom)/float(dpiScale))*(float((cvol[i+curedpage]*(127-maxval(0,-cpan[i+curedpage])))>>7)/127.0f),73,barcol);
      g._WRAP_draw_filled_rectangle(rectX-6,67,rectX-6-38*(float(curzoom)/float(dpiScale))*(float((cvol[i+curedpage]*(127-maxval(0,cpan[i+curedpage])))>>7)/127.0f),73,barcol);
      
      g.tPos(patStartX/(8*dpiScale)+(3+5.5+(12*i))*(float(curzoom)/float(dpiScale)),5);
      if (!muted[i+curedpage]) {
        g.tColor(14);
      } else {
        g.tColor(8);
      }
      g.tAlign(0.5);
      g.printf("%d",i+curedpage);
      g.tAlign(0);
    }
    g._WRAP_draw_line(0,80,scrW,80,g._WRAP_map_rgb(255,255,255),1);
    
    g._WRAP_set_clipping_rectangle(0,81,scrW,scrH-81);
    g._WRAP_disregard_scale_draw(patternbitmap,0,
                          0,//maxval(0,curpatrow-16)*12,
                          g._WRAP_get_bitmap_width(patternbitmap),
                          scrH*dpiScale-maxval(60,252-(curpatrow*12)),
                          patStartX,
                          patStartY-patOffY,
                          curzoom,
                          0);
    firstChan=curedchan; firstMode=curedmode;
    lastChan=curselchan; lastMode=curselmode;
    selTop=selStart; selBottom=selEnd;
    if (lastChan<firstChan) {
      lastChan^=firstChan;
      firstChan^=lastChan;
      lastChan^=firstChan;
    
      lastMode^=firstMode;
      firstMode^=lastMode;
      lastMode^=firstMode;
    } else if (lastChan==firstChan) {
      if (lastMode<firstMode) {
        lastMode^=firstMode;
        firstMode^=lastMode;
        lastMode^=firstMode;
      }
    }
    if (selBottom<selTop) {
      selBottom^=selTop;
      selTop^=selBottom;
      selBottom^=selTop;
    }
    for (int i=firstChan; i<=lastChan; i++) {
      for (int j=(i==firstChan)?firstMode:0; (j<5 && (i<lastChan || j<=lastMode)); j++) {
        g._WRAP_draw_filled_rectangle((patStartX+(modeOff[j]+(i*96))*curzoom)/dpiScale,
                               (patStartY+(3*curzoom)-(curpatrow-selTop)*12*curzoom)/dpiScale,
                               (patStartX+(modeOff[j+1]+(i*96))*curzoom)/dpiScale,
                               (patStartY+(15*curzoom)-(curpatrow-selBottom)*12*curzoom)/dpiScale,
                               g._WRAP_map_rgba(128,128,128,128));
      }
    }
    g._WRAP_draw_filled_rectangle(
      0,
      ((patStartY+(3*curzoom)+((follow)?(0):(12*(maxval(0,curstep)-curpatrow)*curzoom)))/dpiScale),
      scrW+1,
      ((patStartY+(15*curzoom)+((follow)?(0):(12*(maxval(0,curstep)-curpatrow)*curzoom)))/dpiScale),
      g._WRAP_map_rgba(64,64,64,128));
    g._WRAP_reset_clipping_rectangle();
  }
  // grid markers
  #ifdef MOUSE_GRID
  g._WRAP_draw_rectangle(1+((mstate.x/8)*8),1+((mstate.y/12)*12),((mstate.x/8)*8)+9,((mstate.y/12)*12)+14,g._WRAP_map_rgb(0,255,255),1);
  g.tPos(20,0);
  g.tColor(14);
  g.printf("%d, %d",(mstate.x/8)*8,(mstate.y/12)*12);
  #endif
  
  // oscilloscope
  g.setTarget(osc);
  g._WRAP_set_blender(SDL_BLENDMODE_ADD);
  pointsToDraw=735;//(pointsToDraw*255+(signed short)(oscbufWPos-oscbufRPos))/256;
  oscbufRPos=oscbufWPos-735;
  //printf("ptd: %d\n",pointsToDraw);
  if (pointsToDraw<0) {
    pointsToDraw=0;
  }
  if (pointsToDraw) {
    g._WRAP_clear_to_color(g._WRAP_map_rgb(0,0,0));
    for (int ii=0;ii<pointsToDraw;ii++) {
      g._WRAP_draw_pixel(((ii*128/pointsToDraw))+0.5,36+(int)(oscbuf[oscbufRPos]*18)-9+0.5,g._WRAP_map_rgb_f(0,(1-(((oscbuf[oscbufRPos]+1)*18)-(int)((oscbuf[oscbufRPos]+1)*18)))*1.5,0));
      g._WRAP_draw_pixel(((ii*128/pointsToDraw))+0.5,36+(int)(oscbuf2[oscbufRPos]*18)-9+0.5,g._WRAP_map_rgb_f((1-(((oscbuf2[oscbufRPos]+1)*18)-(int)((oscbuf2[oscbufRPos]+1)*18)))*1.5,0,0));
      g._WRAP_draw_pixel(((ii*128/pointsToDraw))+0.5,36+(int)(oscbuf[oscbufRPos]*18)-8+0.5,g._WRAP_map_rgb_f(0,((((oscbuf[oscbufRPos]+1)*18)-(int)((oscbuf[oscbufRPos]+1)*18)))*1.5,0));
      g._WRAP_draw_pixel(((ii*128/pointsToDraw))+0.5,36+(int)(oscbuf2[oscbufRPos]*18)-8+0.5,g._WRAP_map_rgb_f(((((oscbuf2[oscbufRPos]+1)*18)-(int)((oscbuf2[oscbufRPos]+1)*18)))*1.5,0,0));
      oscbufRPos++;
    }
  }
  g._WRAP_set_blender(SDL_BLENDMODE_BLEND);
  g.resetTarget();
  
  if (iface==UIMobile) {
    // boundaries
    g._WRAP_draw_line(0,59,scrW,59,g._WRAP_map_rgb(255,255,255),1);
    if (mobAltView) {
      // TODO: optimize for mobile
      g.tPos(2,1);
      g.tNLPos(2);
      g.tColor(8);
      g.printf("speed   v^|  |patID   v^\n");
      g.printf("tempo   v^|  |octave  v^\n");
      g.printf("order   v^|  |length  v^\n");
      g.tPos(((float)scrW)/8.0-16,1);
      g.tNLPos(((float)scrW)/8.0-16);
      g.printf("|instr   v^\n");
      g.printf("|  follow  \n");
      g.printf("|   loop   \n");
      g.tNLPos(0);
    } else {
      // page select
      g._WRAP_draw_line(16,24,32,24,g._WRAP_map_rgb(255,255,255),1);
      g._WRAP_draw_line(16,30,32,30,g._WRAP_map_rgb(255,255,255),1);
      g._WRAP_draw_line(16,36,32,36,g._WRAP_map_rgb(255,255,255),1);
      // play/pattern/stop buttons
      g._WRAP_draw_rectangle((scrW/2)-112,13,(scrW/2)-82,47,g._WRAP_map_rgb(255,255,255),2);
      g._WRAP_draw_rectangle((scrW/2)-71,13,(scrW/2)-31,47,g._WRAP_map_rgb(255,255,255),2);
      g._WRAP_draw_rectangle((scrW/2)-20,13,(scrW/2)+20,47,g._WRAP_map_rgb(255,255,255),2);
      g._WRAP_draw_rectangle((scrW/2)+31,13,(scrW/2)+71,47,g._WRAP_map_rgb(255,255,255),2);
      g._WRAP_draw_rectangle((scrW/2)+82,13,(scrW/2)+112,47,g._WRAP_map_rgb(255,255,255),2);
      // play button
      g._WRAP_draw_line((scrW/2)-58,22,(scrW/2)-58,36,g._WRAP_map_rgb(255,255,255),2);
      g._WRAP_draw_line((scrW/2)-58,22,(scrW/2)-44,29,g._WRAP_map_rgb(255,255,255),2);
      g._WRAP_draw_line((scrW/2)-58,36,(scrW/2)-44,29,g._WRAP_map_rgb(255,255,255),2);
      // pattern button
      g._WRAP_draw_line((scrW/2)-8,21,(scrW/2)-8,37,g._WRAP_map_rgb(255,255,255),2);
      g._WRAP_draw_line((scrW/2)-5,22,(scrW/2)-5,36,g._WRAP_map_rgb(255,255,255),2);
      g._WRAP_draw_line((scrW/2)-5,22,(scrW/2)+9,29,g._WRAP_map_rgb(255,255,255),2);
      g._WRAP_draw_line((scrW/2)-5,36,(scrW/2)+9,29,g._WRAP_map_rgb(255,255,255),2);
      // stop button
      g._WRAP_draw_rectangle((scrW/2)+44,22,(scrW/2)+58,36,g._WRAP_map_rgb(255,255,255),2);
      // skip left
      g._WRAP_draw_line((scrW/2)-92,22,(scrW/2)-102,29,g._WRAP_map_rgb(255,255,255),2);
      g._WRAP_draw_line((scrW/2)-92,36,(scrW/2)-102,29,g._WRAP_map_rgb(255,255,255),2);
      // skip right
      g._WRAP_draw_line((scrW/2)+92,22,(scrW/2)+102,29,g._WRAP_map_rgb(255,255,255),2);
      g._WRAP_draw_line((scrW/2)+92,36,(scrW/2)+102,29,g._WRAP_map_rgb(255,255,255),2);
    }

    // alternate view
    if (mobAltView) {
      g._WRAP_draw_line(scrW-24,32,scrW-16,26,g._WRAP_map_rgb(255,255,255),2);
      g._WRAP_draw_line(scrW-32,26,scrW-24,32,g._WRAP_map_rgb(255,255,255),2);
    } else {
      g._WRAP_draw_line(scrW-24,26,scrW-16,32,g._WRAP_map_rgb(255,255,255),2);
      g._WRAP_draw_line(scrW-32,32,scrW-24,26,g._WRAP_map_rgb(255,255,255),2);
    }
    // oscilloscope
    //g._WRAP_draw_bitmap(osc,scrW-128,0,0);
    //g._WRAP_draw_line(scrW-128,0,scrW-128,59,g._WRAP_map_rgb(255,255,255),1);

    // page select
    if (pageSelectShow) {
      mobScroll+=fmin(0.08,(1-mobScroll)*0.2);
      if (mobScroll>0.999) mobScroll=1;
    } else {
      mobScroll=fmax(mobScroll-0.08,mobScroll*0.8);
      if (mobScroll<0.001) mobScroll=0;
    }
    if (mobScroll>0) {
      g._WRAP_set_clipping_rectangle(0,0,scrW*mobScroll,59);
      g._WRAP_draw_filled_rectangle(-scrW*(1.0f-mobScroll),0,scrW*mobScroll,59,g._WRAP_map_rgb(0,0,0));

      for (int i=0; i<10; i++) {
        g.tPos((-topScroll+(1.0f-mobScroll)*-scrW)/8+3+(10*i)+float(7-strlen(pageNames[i]))/2,1.666667);
        g.tColor(15);
        g._WRAP_draw_rectangle(((1.0f-mobScroll)*-scrW)+16+(10*8*i)-topScroll,12,((1.0f-mobScroll)*-scrW)+16+72+(10*8*i)-topScroll,48,g._WRAP_map_rgb(128,128,128),1);
        g.printf("%s",pageNames[i]);
      }
      g._WRAP_reset_clipping_rectangle();
    }
  } else {
    // header
    g.tPos(0,0);
    g.tColor(14);
    g.printf(PROGRAM_NAME);
    g.tPos(14,0);
    g.printf("dev%d",ver);
  
    // properties - buttons
    patseek+=(curpat-patseek)/4;
    if (fmod(patseek,1)>0.999 || fmod(patseek,1)<0.001) {
      patseek=round(patseek);
    }
    g.tPos(0,1);
    g.tColor(8);
    g.printf("pat|ins|sfx|speed   v^|  |patID   v^|\n");
    g.printf("sng|dsk|mem|tempo   v^|  |octave  v^|\n");
    g.printf("lvl|cfg|vis|order   v^|  |length  v^|\n");
    g.tPos(((float)scrW)/8.0-37,1);
    g.tNLPos(((float)scrW)/8.0-37);
    g.printf("|instr   v^\n");
    g.printf("|  follow  \n");
    g.printf("|   loop   \n");
    g.tNLPos(0);
    if (speedlock) {
      g.tColor(14);
      g.tPos(12,1);
      g.printf("speed");
    }
    if (tempolock) {
      g.tColor(14);
      g.tPos(12,2);
      g.printf("tempo");
    }
  
    g.tColor(14);
    g.tPos(18,1); g.printf("%.2X",speed);
    g.tPos(17,2); g.printf("%d",tempo);
    g.tPos(18,3); g.printf("%.2X",curpat);
    g.tPos(32,1); g.printf("%.2X",patid[curpat]);
    g.tPos(32,2); g.printf("%.2X",curoctave);
    g.tPos(32,3); g.printf("%.2X",patlength[patid[curpat]]);
    
    g.tNLPos(((float)scrW/8.0)-36);
    g.tPos(1);
    if (curins==0) {
      g.printf("      --\n");
    } else {
      g.printf("      %.2X\n",curins);
    }
    if (follow) {
      g.printf("  follow");
    }
    
    g.tNLPos(((float)scrW/8.0)-24);
    g.tPos(0.6666667);
    g.tColor(15);
    g.printf(" %.2x/%.2x\n",curtick,speed);
    g.printf(" %.2x/%.2x\n",maxval(0,curstep),patlength[patid[curpat]]);
    g.printf(" %.2x:%.2x",patid[curpat],curpat,songlength);
    // draw orders
    // -128, 192, 255, +191, 128
    g._WRAP_set_clipping_rectangle(184,16,16,36);
    delta=(6*fmod(patseek,1));
    g.tNLPos(23);
    g.tPos(-fmod(patseek,1));
    if (((int)patseek-1)>0) {
      g.tColor(244-delta); g.printf("%s\n",getVisPat(patid[maxval((int)patseek-2,0)]).c_str());
    } else {
      g.printf("\n");
    }
    if (((int)patseek)>0) {
      g.tColor(250-delta); g.printf("%s\n",getVisPat(patid[maxval((int)patseek-1,0)]).c_str());
    } else {
      g.printf("\n");
    }
    g.tColor(255-delta); g.printf("%s\n",getVisPat(patid[(int)patseek]).c_str());
    if (((int)patseek)<songlength) {
      g.tColor(249+delta); g.printf("%s\n",getVisPat(patid[minval((int)patseek+1,255)]).c_str());
    }
    if (((int)patseek+1)<songlength) {
      g.tColor(244+delta); g.printf("%s",getVisPat(patid[maxval((int)patseek+2,0)]).c_str());
    }
    g._WRAP_reset_clipping_rectangle();
    g.tNLPos(0);
    // boundaries
    g._WRAP_draw_line(scrW-200,0,scrW-200,59,g._WRAP_map_rgb(255,255,255),1);
    g._WRAP_draw_line(0,59,scrW,59,g._WRAP_map_rgb(255,255,255),1);
    // play/pattern/stop buttons
    g._WRAP_draw_rectangle((scrW/2)-61,13,(scrW/2)-21,37,g._WRAP_map_rgb(255,255,255),2);
    g._WRAP_draw_rectangle((scrW/2)-20,13,(scrW/2)+20,37,g._WRAP_map_rgb(255,255,255),2);
    g._WRAP_draw_rectangle((scrW/2)+21,13,(scrW/2)+61,37,g._WRAP_map_rgb(255,255,255),2);
    // reverse/step/follow buttons
    g._WRAP_draw_rectangle((scrW/2)-61,37,(scrW/2)-21,48,g._WRAP_map_rgb(255,255,255),2);
    g._WRAP_draw_rectangle((scrW/2)-20,37,(scrW/2)+20,48,g._WRAP_map_rgb(255,255,255),2);
    g._WRAP_draw_rectangle((scrW/2)+21,37,(scrW/2)+61,48,g._WRAP_map_rgb(255,255,255),2);
    // play button
    g._WRAP_draw_line((scrW/2)-48,18,(scrW/2)-48,32,g._WRAP_map_rgb(255,255,255),2);
    g._WRAP_draw_line((scrW/2)-48,18,(scrW/2)-34,25,g._WRAP_map_rgb(255,255,255),2);
    g._WRAP_draw_line((scrW/2)-48,32,(scrW/2)-34,25,g._WRAP_map_rgb(255,255,255),2);
    // pattern button
    g._WRAP_draw_line((scrW/2)-8,17,(scrW/2)-8,33,g._WRAP_map_rgb(255,255,255),2);
    g._WRAP_draw_line((scrW/2)-5,18,(scrW/2)-5,32,g._WRAP_map_rgb(255,255,255),2);
    g._WRAP_draw_line((scrW/2)-5,18,(scrW/2)+9,25,g._WRAP_map_rgb(255,255,255),2);
    g._WRAP_draw_line((scrW/2)-5,32,(scrW/2)+9,25,g._WRAP_map_rgb(255,255,255),2);
    // stop button
    g._WRAP_draw_rectangle((scrW/2)+34,18,(scrW/2)+48,32,g._WRAP_map_rgb(255,255,255),2);
    // oscilloscope
    g._WRAP_draw_bitmap(osc,scrW-128,0,0);
    g._WRAP_draw_line(scrW-128,0,scrW-128,59,g._WRAP_map_rgb(255,255,255),1);
  }
  
  switch(screen) {
    case 0: drawpatterns(false); break;
    case 1: drawinsedit(); break;
    case 2: drawdiskop(); break;
    case 3: drawsong(); break;
    case 4: drawmixer(); break;
    case 5: drawconfig(); break;
    case 6: drawhelp(); break;
    case 7: drawabout(); break;
    case 9: drawsfxpanel(); break;
    case 10: drawmemory(); break;
    case 11: drawpcmeditor(); break;
    case 12: drawpiano(); break;
  }

  if (candInput!="") {
    g.tPos(scrW/8-candInput.size(),0);
    g.tColor(10);
    g.printf("%s",candInput.c_str());
  }  

  if (popbox.isVisible()) popbox.draw();
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

void initNewBits() {
  bdNew=Button(16,76,64,32,"New",NULL,NULL);
  bdOpen=Button(88,76,64,32,"Open",NULL,NULL);
  bdSave=Button(158,76,64,32,"Save",NULL,NULL);
}

void updateDisp() {
  SDL_Event ev;
  while (g._WRAP_get_next_event(&ev)) {
    if (ev.type==SDL_QUIT) {
      printf("close button pressed\n");
      quit=true;
    } else if (ev.type==SDL_MOUSEMOTION) {
      mstate.x=ev.motion.x/dpiScale;
      mstate.y=ev.motion.y/dpiScale;
    } else if (ev.type==SDL_MOUSEBUTTONDOWN) {
      mstate.x=ev.button.x/dpiScale;
      mstate.y=ev.button.y/dpiScale;
      switch (ev.button.button) {
        case SDL_BUTTON_LEFT:
          mstate.buttons|=1;
          break;
        case SDL_BUTTON_RIGHT:
          mstate.buttons|=2;
          break;
        case SDL_BUTTON_MIDDLE:
          mstate.buttons|=4;
          break;
      }
    } else if (ev.type==SDL_MOUSEBUTTONUP) {
      mstate.x=ev.button.x/dpiScale;
      mstate.y=ev.button.y/dpiScale;
      switch (ev.button.button) {
        case SDL_BUTTON_LEFT:
          mstate.buttons&=~1;
          break;
        case SDL_BUTTON_RIGHT:
          mstate.buttons&=~2;
          break;
        case SDL_BUTTON_MIDDLE:
          mstate.buttons&=~4;
          break;
      }
    } else if (ev.type==SDL_MOUSEWHEEL) {
      mstate.z+=ev.wheel.y;
    } else if (ev.type==SDL_WINDOWEVENT) {
      if (ev.window.event==SDL_WINDOWEVENT_RESIZED) {
        g.trigResize(ev.window.data1,ev.window.data2);
        g._WRAP_destroy_bitmap(mixer);
        // recreate pattern bitmap
        g._WRAP_destroy_bitmap(patternbitmap);
        patternbitmap=g._WRAP_create_bitmap(g.getWSize().x,g.getWSize().y);
        scrW=g.getWSize().x;
        scrH=g.getWSize().y;
        mixer=g._WRAP_create_bitmap(scrW,scrH);
        // adjust channel count if needed
        maxCTD=(scrW*dpiScale-24*curzoom)/(96*curzoom);
        if (maxCTD>channels) maxCTD=channels;
        if (maxCTD<1) maxCTD=1;
        chanstodisplay=maxCTD;
        drawmixerlayer();
        drawpatterns(true);
      }
    } else if (ev.type==SDL_KEYDOWN) {
      if (inputvar!=NULL && !imeActive) {
        switch (ev.key.keysym.sym) {
          case SDLK_LEFT:
            inputcurpos--;
            if (inputcurpos<0) {
              inputcurpos=0;
              triggerfx(1);
            }
            break;
          case SDLK_RIGHT:
            inputcurpos++;
            if (inputcurpos>(signed)utf8len(inputvar->c_str())) {
              inputcurpos=(signed)utf8len(inputvar->c_str());
              triggerfx(1);
            }
            break;
          case SDLK_HOME:
            inputcurpos=0;
            break;
          case SDLK_END:
            inputcurpos=utf8len(inputvar->c_str());
            break;
          case SDLK_BACKSPACE:
            if (--inputcurpos<0) {
              inputcurpos=0;
              triggerfx(1);
            } else {
              inputvar->erase(utf8pos(inputvar->c_str(),inputcurpos),utf8csize((const unsigned char*)inputvar->c_str()+utf8pos(inputvar->c_str(),inputcurpos)));
            }
            break;
          default:
            break;
        }
      }
    } else if (ev.type==SDL_TEXTEDITING) {
      candInput=ev.edit.text;
      if (ev.edit.start>0) {
        imeActive=true;
      } else {
        imeActive=false;
      }
    } else if (ev.type==SDL_TEXTINPUT) {
      imeActive=false;
      candInput="";
      if (inputvar!=NULL) {
        if ((inputvar->size()+strlen(ev.text.text))<maxinputsize) {
          inputvar->insert(utf8pos(inputvar->c_str(),inputcurpos),ev.text.text);
          inputcurpos+=utf8len(ev.text.text);;
        } else {
          triggerfx(1);
        }
      }
    }
  }
  if (true) {
    doframe=1;
    framecounter++;
    if (!playermode) {
      scrW=g.getWSize().x;
      scrH=g.getWSize().y;
    }

    maxrasterdelta=(maxval(0,raster2-raster1)>maxrasterdelta)?(maxval(0,raster2-raster1)):(maxrasterdelta);
    if (!playermode) {
      g.clearScreen();
      drawdisp();
      if (kb[SDL_SCANCODE_LSHIFT]) {
        g.tColor(9);
        g.tAlign(0.5);
        g.tPos((float)scrW/16.0,0);
        if (ntsc) {
          g.printf("%.1fHz CLOCK: 6.18MHz i: %x t: %.5x NTSC",FPS,ASC::interval,ASC::currentclock);
        } else {
          g.printf("%.1fHz CLOCK: 5.95MHz i: %x t: %.5x PAL",FPS,ASC::interval,ASC::currentclock);
        }
        g.tAlign(0);
        g.tNLPos(0);
      }
      g._WRAP_flip_display();
    }
  }
}

int main(int argc, char **argv) {
  int filearg=0;
  printf("soundtracker (r%d)\n",ver);
  chip[0].Init();
  chip[1].Init();
  chip[2].Init();
  chip[3].Init();
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
          curpat=atoi(argv[i+1]);
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
    FPS=60;
    tempo=150;
  } else {
    FPS=50;
    tempo=125;
  }
  filessorted.resize(1024);
  filenames.resize(1024);
  if (iface==UIMobile) {
    scrW=360;
    scrH=640;
  } else {
    scrW=800;
    scrH=450;
  }
  // create memory blocks
  patlength=new unsigned char[256];
  helptext=new char[18];
  strcpy(helptext,"help.txt not found");

  printf("initializing SDL\n");
  if (!g.preinit()) {
    fprintf(stderr,"failed to initialize SDL!\n");
    return 1;
  }
   
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
    printf("creating display\n");
    if (!g.init(scrW,scrH)) {
      printf("couldn't initialize display...\n");
      return 1;
    }
    dpiScale=g._getScale();
#ifdef ANDROID
    curzoom=dpiScale-1;
    if (curzoom<1) curzoom=1;
#else
    curzoom=dpiScale;
#endif
    maxTSize=g.maxTexSize();
  }
  patternbitmap=g._WRAP_create_bitmap(scrW,scrH);
  piano=g._WRAP_create_bitmap(700,60);
  pianoroll=g._WRAP_create_bitmap(700,128);
  pianoroll_temp=g._WRAP_create_bitmap(700,128);
  mixer=g._WRAP_create_bitmap(scrW,scrH);
  osc=g._WRAP_create_bitmap(128,59);

  CleanupPatterns();
  // init colors
  Color colors[256];
  for (int lc=0; lc<256; lc++) {
    colors[lc]=getucol(lc);
  }
  g.setTarget(patternbitmap);
  g._WRAP_clear_to_color(g._WRAP_map_rgb(0,0,0));
  g.setTarget(pianoroll);
  g._WRAP_clear_to_color(g._WRAP_map_rgb(0,0,0));
  g.setTarget(pianoroll_temp);
  g._WRAP_clear_to_color(g._WRAP_map_rgb(0,0,0));
  g.setTarget(piano);
  g._WRAP_clear_to_color(g._WRAP_map_rgb(0,0,0));
  // draw a piano
  for (int ii=0; ii<10; ii++) {
    for (int jj=0; jj<7; jj++) {
      g._WRAP_draw_filled_rectangle((jj*10)+(ii*70),60-60,((jj+1)*10)+(ii*70),60,g._WRAP_map_rgb(64,64,64));
      g._WRAP_draw_filled_rectangle((jj*10)+1+(ii*70),60-59,((jj+1)*10)+(ii*70),60-1,g._WRAP_map_rgb(224,224,224));
    }
    for (int jj=0; jj<6; jj++) {
      if (jj==2) continue;
      g._WRAP_draw_filled_rectangle((jj+0.666666667)*10+(ii*70),60-60,((jj+1.36)*10)+(ii*70),60-25,g._WRAP_map_rgb(0,0,0));
    }
  }
  g.setTarget(osc);
  g._WRAP_clear_to_color(g._WRAP_map_rgb(0,0,0));
  if (!playermode) {
    g.setTarget(mixer);
    g._WRAP_clear_to_color(g._WRAP_map_rgb(0,0,0));
    drawmixerlayer();
    g.resetTarget();
    initNewBits();
  }
  printf("initializing audio channels\n");
  initaudio();
  // clear to black
  if (!playermode) {
    g._WRAP_clear_to_color(g._WRAP_map_rgb(0,0,0));
    // flip buffers
    g._WRAP_flip_display();
    SDL_StopTextInput();
    SDL_SetTextInputRect(NULL);
    drawpatterns(true);
  }
  if (playermode || fileswitch) {
    if (LoadFile(argv[filearg])) return 1;
      if (playermode) {
        Play();
        printf("playing: %s\n",name.c_str());
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
    g._WRAP_rest(3600);
  } else {
    while (1) {
      updateDisp();
      if (quit) {
        break;
      }
    }
  }
  printf("destroying audio system\n");
#ifdef JACK
  jack_deactivate(jclient);
#endif
  printf("destroying display\n");
  g.quit();
  printf("destroying some buffers\n");
  delete[] patlength;
  printf("finished\n");
  return 0;
}
