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
#define sign(a) ((a>0)?(1):((a<0)?(-1):(0)))
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

blip_buffer_t* bb[2];
int prevSample[2]={0,0};

short bbOut[2][32768];

const bool verbose=false; // change this to turn on verbose mode

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
int scrollpos=0;
int valuewidth=4;
int oldpat=-1;
unsigned char CurrentIns=1;
unsigned char CurrentEnv=0;
// init tracker stuff
int pattern=0;
string tempInsName;
Instrument blankIns;
int scroll[32][7]={}; // scroll[channel][envelope]
int instruments=0;
int patterns=0;
int seqs=255;
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
unsigned char nnote[32]={}; // next note
float curnote[32]={}; // current note of a channel
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
bool doretrigger[32]={0,0,0,0,0,0,0,0,
                  0,0,0,0,0,0,0,0,
                  0,0,0,0,0,0,0,0,
                  0,0,0,0,0,0,0,0};
bool tickstart=false;
bool linearslides=true;
bool playermode=false;
bool fileswitch=false;
bool reversemode=false;
int nvolu[32]={}; // next volume value
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
int hover[16]={}; // hover time per button
int16_t ver=TRACKER_VER; // version number
unsigned char chs0[5000];
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

// new things
Song* song=NULL;
Player player;
std::mutex canUseSong;

// NEW VARIABLES END //

void Playback();
void MuteAllChannels();
int playfx(const char* fxdata,int fxpos,int achan);
void triggerfx(int num);
#define interpolatee(aa,bb,cc) (aa+((bb-aa)*cc))

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
    // 6.18MHz * 2.5
    ASC::interval=(int)(15450000/player.tempo);
    if (player.tempo==150) ASC::interval=103103;
    targetSR=309000;
    noProc=sr/targetSR;
  } else {
    // 5.95MHz * 2.5
    ASC::interval=(int)(14875000/player.tempo);
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
  
  bool canPlay=canUseSong.try_lock();
  if (canPlay) for (int i=0; i<runtotal; i++) {
    ASC::currentclock-=20; // 20 CPU cycles per sound output cycle
    if (ASC::currentclock<=0) {
      for (int ii=0;ii<32;ii++) {
        cshapeprev[ii]=cshape[ii];
      }

      player.update();

      if (sfxplaying) {
        sfxpos=playfx(sfxdata[cursfx],sfxpos,chantoplayfx);
        if (sfxpos==-1) {
          sfxplaying=false;
        }
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
  case 4: case 13: case 14: case 18: case 27: return 10; break; // volume control
  case 5: case 6: case 7: case 8: case 11: case 12: case 21: return 11; break; // pitch control
  case 9: case 10: case 15: case 17: return 63; break; // note control
  case 19: case 26: return 13; break; // special commands
  case 16: case 24: case 25: return 14; break; // panning commands
  default: return 8; break; // unknown commands
  }
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
  // returns which channel is free
  // routine is similar to IT's next free channel routine
  // 1. find first inactive channel
  // failed? 2. find first channel with zero volume
  for (int ii=0;ii<8;ii++) {
    if (cvol[ii]==0) {return ii;}
  }
  // failed? 3. find channel with lowest volume
  int candidate=0;
  for (int ii=1;ii<8;ii++) {
    if (cvol[candidate]>cvol[ii]) {candidate=ii;}
  }
  return candidate;
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
  if (follow) {patRow=player.step; if (patRow<0) patRow=0;}
  //patRow=0;
  // will be replaced (WHEN)
  if ((!UPDATEPATTERNS || player.playMode==0 || player.playMode==1) && !force && oldpat==player.pat) {oldpat=player.pat;return;}
  oldpat=player.pat;
  UPDATEPATTERNS=true;
  g._WRAP_destroy_bitmap(patternbitmap);
  Pattern* p=song->getPattern(song->order[player.pat],true);
  patternbitmap=g._WRAP_create_bitmap(24+chanstodisplay*96,((p->length)*12)+4);
  g.setTarget(patternbitmap);
  g._WRAP_clear_to_color(g._WRAP_map_rgb(0,0,0));
  g._WRAP_draw_filled_rectangle(0,60,scrW,scrH,g._WRAP_map_rgb(0,0,0));
  for (int i=0;i<p->length;i++) {
    //if (i>patRow+15+((scrH-450)/12)) {continue;}
    //if (i<patRow-16) {continue;}
    g.tColor(8);
    g.tPos(0,i);
    g.printf("%.2X",i);
    // channel drawing routine, replicated 8 times
    for (int j=0;j<chanstodisplay;j++) {
      g.tColor(15);
      g.printf("|");
      if (p->data[i][j+curedpage][0]==0 &&
          p->data[i][j+curedpage][1]==0 &&
          p->data[i][j+curedpage][2]==0 &&
          p->data[i][j+curedpage][3]==0 &&
          p->data[i][j+curedpage][4]==0) {
        g.tColor(245);
        g.printf("...........");
        continue;
      }
      // note
      g.tColor(250);
      g.printf("%s%s",getnote(p->data[i][j+curedpage][0]),getoctave(p->data[i][j+curedpage][0]));
      g.tColor(81);
      g.printf("%c%c",getinsH(p->data[i][j+curedpage][1]),getinsL(p->data[i][j+curedpage][1]));
      // instrument
      if (p->data[i][j+curedpage][2]==0 && p->data[i][j+curedpage][0]!=0) {
        g.printf("v%.2X",song->ins[p->data[i][j+curedpage][1]]->vol);
      } else {
        g.tColor(getVFXColor(p->data[i][j+curedpage][2]|((p->data[i][j+curedpage][3]&0x80)<<1)));
        if ((p->data[i][j+curedpage][2]|((p->data[i][j+curedpage][3]&0x80)<<1))==0) {
          g.printf("...");
        } else {
          g.printf("%s%.2X",getVFX(p->data[i][j+curedpage][2]|((p->data[i][j+curedpage][3]&0x80)<<1)),getVFXVal(p->data[i][j+curedpage][2]|((p->data[i][j+curedpage][3]&0x80)<<1)));
        }
      }
      // effect
      g.tColor(GetFXColor(p->data[i][j+curedpage][3]&0x7f));
      g.printf("%s%c%c",getFX(p->data[i][j+curedpage][3]&0x7f),getinsH(p->data[i][j+curedpage][4]),getinsL(p->data[i][j+curedpage][4]));
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
  // draws some GUI stuff
  g.tPos((float)(scrW-272)/8.0,11);
  g.tColor((inputwhere==2)?11:15);
  g.printf("%s",song->ins[CurrentIns]->name);
  if (inputwhere==2) {
    inputCursor((scrW-272),133,145);
  }
  
  g.tPos(22,5);
  g.printf("%.2X",CurrentIns);
  
  // the right pane
  g.tColor(11);
  g.tPos((float)(scrW-208)/8.0,7);
  g.printf("%s%s\n",getnotetransp(song->ins[CurrentIns]->noteOffset),getoctavetransp(song->ins[CurrentIns]->noteOffset));
  
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
  if (song->ins[CurrentIns]->filterMode&1) {
    g.tPos((float)(scrW-216)/8.0,13);
    g.printf("low");
  }
  if (song->ins[CurrentIns]->filterMode&2) {
    g.tPos((float)(scrW-184)/8.0,13);
    g.printf("high");
  }
  if (song->ins[CurrentIns]->filterMode&4) {
    g.tPos((float)(scrW-144)/8.0,13);
    g.printf("band");
  }
  if (song->ins[CurrentIns]->filterMode&8) {
    g.tPos((float)(scrW-272)/8.0,15);
    g.printf("PCM");
  }
  if (song->ins[CurrentIns]->filterMode&16) {
    g.tPos((float)(scrW-272)/8.0,21);
    g.printf("RM");
  }
  if (song->ins[CurrentIns]->flags&32) {
    g.tPos((float)(scrW-272)/8.0,22);
    g.printf("Sy");
  }
  if (song->ins[CurrentIns]->flags&1) {
    g.tPos((float)(scrW-272)/8.0,24);
    g.printf("ResetOsc");
  }
  
  g.tPos((float)(scrW-192)/8.0,15);
  g.printf("%.4x",song->ins[CurrentIns]->pcmPos);
  
  g.tPos((float)(scrW-192)/8.0,16);
  g.printf("%.4x",song->ins[CurrentIns]->pcmLoop);
  
  g.tPos((float)(scrW-88)/8.0,15);
  g.printf("%.4x",song->ins[CurrentIns]->pcmLen);
  
  g.tPos((float)(scrW-208)/8.0,21);
  g.printf("%.2x",song->ins[CurrentIns]->LFO);
  
  g.tPos((float)(scrW-208)/8.0,18);
  g.printf("%.4x",0xffff-song->ins[CurrentIns]->filterH);
  
  g.tPos((float)(scrW-208)/8.0,27);
  g.printf("%.1x",song->ins[CurrentIns]->flags>>6);
  
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
  Pattern* p=song->getPattern(song->order[player.pat],true);
  // autoinstrument
  if (p->data[player.step][curedpage+curedchan][0]!=0 && (p->data[player.step][curedpage+curedchan][0]%16)<13) {
    p->data[player.step][curedpage+curedchan][1]=curins;
  }
  // skipping
  if (player.playMode==0) {
    player.tick=1;
    player.step++;
    if (player.step>(p->length-1)) {
      player.step=0;
      player.pat++;
    }
    selStart=player.step;
    selEnd=player.step;
    patRow=player.step;
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
    g.printf("   %.2x",song->defaultVol[chantodraw+curedpage]);
    g.tColor(12);
    g.printf("   %.2x%c\n",cduty[chantodraw+curedpage]%256,shapeSym(cshape[chantodraw+curedpage]));
    g.tColor(14);
    g.printf("   %.2x   ",(unsigned char)song->defaultPan[chantodraw+curedpage]);
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
      g.tColor(14);
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
  g.printf("%2d",song->channels);
  
  // detune
  g.tPos(54,5);
  g.printf("%.2x",(unsigned char)song->detune);
  
  // length
  g.tPos(68,5);
  g.printf("%.2x",song->orders);
  
  // tempo
  g.tPos(81,5);
  if (song->tempo==0) {
    g.printf("N/A");
  } else {
    g.printf("%d",song->tempo);
  }
  
  // speed
  g.tPos(95,5);
  g.printf("%.2X",song->speed);
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
void drawconfig() {
  // new config page!
  g.tColor(15);
  g.tAlign(0.5);
  g.tPos((scrW/2)/8-10,5.5);
  g.printf("General");

  g.tPos((scrW/2)/8,5.5);
  g.printf("Audio");

  g.tPos((scrW/2)/8+10,5.5);
  g.printf("Video");
  g.tAlign(0);
  
  g._WRAP_draw_line(0,92,scrW,92,g._WRAP_map_rgb(255,255,255),1);

  // draw config section
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
    g._WRAP_draw_rotated_bitmap(logo,180,86.5,scrW/2,scrH/2,(sin((((float)player.step*(float)player.speed)+((float)player.speed-(float)player.tick))/(8*(float)player.speed)*2*M_PI)/8)*(player.playMode!=0),0);
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
void Play() {
  // reset cursor position
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
  chanvol[su]=song->defaultVol[su];
  doretrigger[su]=false;
  chanpan[su]=song->defaultPan[su];
  //if ((su+1)&2) {chanpan[su]=96;} else {chanpan[su]=-96;} // amiga auto-pan logic
  //if (su&1) {chanpan[su]=96;} else {chanpan[su]=-96;} // normal auto-pan logic
  finedelay=0;
  cutcount[su]=-1;
  }
  // reset global volume
  cglobvol=128;
  
  // the code above has to be destroyed.
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
    song->orders=(unsigned char)memblock[0x20]; instruments=(unsigned char)memblock[0x22]; patterns=(unsigned char)memblock[0x26]; samples=(unsigned char)memblock[0x24];
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
    g.setTitle(PROGRAM_NAME);
  } else {
    g.setTitle(name+S(" - ")+S(PROGRAM_NAME));
  }
  song->orders--;
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
        printf("8-channel Oktalyzer/OctaMED module detected\n");
        origin="Oktalyzer/OctaMED";
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
      printf("sample %d size: %.5x repeat: %.4x replen: %.4x\n",ii,tempsize,repeatpos,repeatlen);
      printf("finetune %d vol %d\n",h.ins[ii].pitch,h.ins[ii].vol);

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
    printf("---PATTERNS---\n");
    if (karsten) {
      fseek(mod,sizeof(ClassicMODHeader),SEEK_SET);
    } else {
      fseek(mod,sizeof(MODHeader),SEEK_SET);
    }
    for (int importid=0;importid<patterns+1;importid++) {
      printf("-PATTERN %d-\n",importid);
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
    g.setTitle(PROGRAM_NAME);
  } else {
    g.setTitle(name+S(" - ")+S(PROGRAM_NAME));
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
    /*(if (ich.volType&1) {
      int point=0;
      for (int j=0; (point<ich.envVolCount && j<253); j++) {
        if (point<1) {
          bytable[0][i+1][j]=minval(255,ich.envVol[point].val*4);
        } else {
          bytable[0][i+1][j]=minval(255,interpolate(float(ich.envVol[point-1].val),float(ich.envVol[point].val),(float(j-ich.envVol[point-1].pos)/float(ich.envVol[point].pos-ich.envVol[point-1].pos)))*4);
        }
        if (j>=ich.envVol[point].pos) point++;
      }
      bytable[0][i+1][253]=minval(252,ich.envVol[ich.envVolCount-1].pos);
      printf("volsus %d\n",ich.volSus);
      if (ich.volType&2) {
        bytable[0][i+1][255]=minval(252,ich.envVol[ich.volSus].pos);
      }
    }*/
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
            chip[0].pcm[sampleSeek]=delta>>8;
            chip[1].pcm[sampleSeek]=delta>>8;
            chip[2].pcm[sampleSeek]=delta>>8;
            chip[3].pcm[sampleSeek]=delta>>8;
          } else {
            delta+=(signed char)(fgetc(xm));
            chip[0].pcm[sampleSeek]=delta;
            chip[1].pcm[sampleSeek]=delta;
            chip[2].pcm[sampleSeek]=delta;
            chip[3].pcm[sampleSeek]=delta;
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
    g.setTitle(PROGRAM_NAME);
  } else {
    g.setTitle(name+S(" - ")+S(PROGRAM_NAME));
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
    fputc(song->orders,sfile); // orders
    fputc(song->speed,sfile); // speed
    fputc(seqs,sfile); // sequences
    fputc(song->tempo,sfile); // tempo
    fputs(name.c_str(),sfile); // name
    fseek(sfile,48,SEEK_SET); // seek to 0x30
    fputc(0,sfile); // default filter mode
    fputc(song->channels,sfile); // channels
    fwrite("\0\0",2,1,sfile); // flags
    fputc(128,sfile); // global volume
    fputc(0,sfile); // global panning
    fwrite("\0\0\0\0",4,1,sfile); // mute flags
    fwrite("\0\0\0\0",4,1,sfile); // PCM data pointer
    fwrite("\0\0",2,1,sfile); // reserved
    fseek(sfile,0x3e,SEEK_SET); // seek to 0x3e
    fputc(song->detune,sfile); // detune factor
    fseek(sfile,0x40,SEEK_SET); // seek to 0x40
    fwrite(song->defaultVol,1,32,sfile); // channel volume
    fwrite(song->defaultPan,1,32,sfile); // channel panning
    fseek(sfile,0x80,SEEK_SET); // seek to 0x80
    for (int ii=0; ii<256; ii++) {
      fputc(song->order[ii],sfile); // order list
    }
    printf("writing instruments...\n");
    fseek(sfile,0xd80,SEEK_SET); // seek to 0xD80, and start writing the instruments
    sk=0xd80;
    for (int ii=0; ii<256; ii++) {
      IS_INS_BLANK[ii]=true;
      // check if the instrument is blank
      if (memcmp(&song->ins[ii],&blankIns,64)!=0) {IS_INS_BLANK[ii]=false;}
      if (IS_INS_BLANK[ii]) {
        insparas[ii]=0;continue;
      }
      insparas[ii]=ftell(sfile);
      fwrite(&song->ins[ii],1,64,sfile);
    }
    printf("writing macros...\n");
    // TODO PLEASE: MACROS
    printf("packing/writing patterns...\n");
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
      song->flags|=4;
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
        if (seqparas[i]==0) continue;
        fseek(sfile,seqparas[i],SEEK_SET);
        Macro* m=new Macro();
        unsigned int len=fgeti(sfile);
        m->jumpRelease=fgeti(sfile);
        int reserved1=fgeti(sfile);
        int reserved2=fgeti(sfile);
        for (unsigned int i=0; i<len; i++) {
          unsigned char cType=fgetc(sfile);
          unsigned int cValue=fgeti(sfile);
          m->cmds.push_back(MacroCommand(cType&0x7f,cValue,cType&0x80?1:0));
          if ((cType&0x7f)==cmdEnd) break;
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
      g.setTitle(PROGRAM_NAME);
    } else {
      g.setTitle(name+S(" - ")+S(PROGRAM_NAME));
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
          if (player.tick==0) {
            Play();
          } else {
            // TODO continue
          }
        }
        // pattern play button
        if (PIR((scrW/2)-20,13,(scrW/2)+20,47,mstate.x,mstate.y)) {
          Play();
        }
        // stop button
        if (PIR((scrW/2)+31,13,(scrW/2)+71,47,mstate.x,mstate.y)) {
          player.stop();
        }
        // skip left button
        if (PIR((scrW/2)-112,13,(scrW/2)-82,47,mstate.x,mstate.y)) {
          if (player.pat>0) player.pat--;
          if (player.playMode==1) Play();
        }
        // skip right button
        if (PIR((scrW/2)+82,13,(scrW/2)+112,47,mstate.x,mstate.y)) {
          if (player.pat<song->orders) {
            player.pat++;
          } else {
            player.pat=0;
          }
          if (player.playMode==1) Play();
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
        if (player.tick==0) {
          Play();
        } else {
          // TODO this
        }
      }
      if (PIR((scrW/2)-61,37,(scrW/2)-21,48,mstate.x,mstate.y)) {reversemode=true;}
      if (PIR((scrW/2)+21,13,(scrW/2)+61,37,mstate.x,mstate.y)) {player.stop();}
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
      if (PIR(160,12,167,23,mstate.x,mstate.y)) {player.speed--; if (player.speed<1) {player.speed=1;}}
      if (PIR(168,12,176,23,mstate.x,mstate.y)) {player.speed++; if (player.speed>31) {player.speed=31;}; if (player.speed<1) {player.speed=1;}}
      if (PIR(160,24,167,35,mstate.x,mstate.y)) {player.tempo--; if (player.tempo<31) {player.tempo=31;};}
      if (PIR(168,24,176,35,mstate.x,mstate.y)) {player.tempo++; if (player.tempo>255) {player.tempo=255;};}
      if (PIR(160,36,167,48,mstate.x,mstate.y)) {if (player.pat>0) player.pat--; if (player.playMode==1) Play();}
      if (PIR(168,36,176,48,mstate.x,mstate.y)) {
        if (player.pat<song->orders) {
          player.pat++;
        } else {
          player.pat=0;
        }
        if (player.playMode==1) Play();
      }
      if (PIR(272,12,279,24,mstate.x,mstate.y)) {song->order[player.pat]--; drawpatterns(true);}
      if (PIR(280,12,288,24,mstate.x,mstate.y)) {song->order[player.pat]++; drawpatterns(true);}
      if (PIR(272,36,279,48,mstate.x,mstate.y)) {
        Pattern* p=song->getPattern(song->order[player.pat],true);
        p->length--;
        if (p->length<1) p->length=1;
      }
      if (PIR(280,36,288,48,mstate.x,mstate.y)) {
        Pattern* p=song->getPattern(song->order[player.pat],true);
        p->length++;
        if (p->length>256) p->length=256;
      }
      //if (PIR((scrW/2)-20,37,(scrW/2)+20,48,mstate.x,mstate.y)) {StepPlay();}
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
    Pattern* p=song->getPattern(song->order[player.pat],true);
    patStartX=(scrW*((float)dpiScale)-(24+chanstodisplay*96)*curzoom)/2;
    patStartY=(60+((scrH*dpiScale)-60)/2);
    if (mstate.y>60) {
      if (leftpress) {
        selStart=(int)((mstate.y*dpiScale-patStartY-(3*curzoom)+patRow*12*curzoom)/(12*curzoom));
        if (selStart<0) selStart=0;
        if (selStart>=p->length) selStart=p->length-1;
        
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
        selEnd=(int)((mstate.y*dpiScale-patStartY-(3*curzoom)+patRow*12*curzoom)/(12*curzoom));
        if (selEnd<0) selEnd=0;
        if (selEnd>=p->length) selEnd=p->length-1;
        
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
        player.step-=(mstate.z-prevZ);
        if (player.step>(p->length-1)) {
          player.step-=p->length;
          player.pat++;
        }
      } else {
        patRow-=(mstate.z-prevZ)*3;
        patRow=fmin(patRow,p->length-1);
      }
      drawpatterns(true);
    }
    if ((mstate.z-prevZ)>0) {
      if (follow) {
        player.step-=(mstate.z-prevZ);
        if (player.step<0) {
          if (player.pat!=0) {
            player.pat--;
            p=song->getPattern(song->order[player.pat],true);
            player.step+=p->length;
          } else {
            player.step=-1;
          }
        }
      } else {
        patRow-=(mstate.z-prevZ)*3;
        patRow=fmax(patRow,0);
      }
      drawpatterns(true);
    }
  }
  // events only in instrument view
  if (screen==1) {
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
      if (PIR(192,60,199,72,mstate.x,mstate.y)) {CurrentIns++;}
      if (PIR(200,60,207,72,mstate.x,mstate.y)) {CurrentIns--; if (CurrentIns<1) CurrentIns=255;}
      // the right pane buttons
      if (PIR(scrW-184,84,scrW-177,96,mstate.x,mstate.y)) {song->ins[CurrentIns]->noteOffset++;}
      if (PIR(scrW-176,84,scrW-169,96,mstate.x,mstate.y)) {song->ins[CurrentIns]->noteOffset--;}
      if (PIR(scrW-168,84,scrW-161,96,mstate.x,mstate.y)) {song->ins[CurrentIns]->noteOffset+=12;}
      if (PIR(scrW-160,84,scrW-153,96,mstate.x,mstate.y)) {song->ins[CurrentIns]->noteOffset-=12;}
      // the right pane buttons
      if (PIR(scrW-216,156,scrW-192,168,mstate.x,mstate.y)) {song->ins[CurrentIns]->filterMode^=1;} // LOW
      if (PIR(scrW-184,156,scrW-152,168,mstate.x,mstate.y)) {song->ins[CurrentIns]->filterMode^=2;} // HIGH
      if (PIR(scrW-144,156,scrW-112,168,mstate.x,mstate.y)) {song->ins[CurrentIns]->filterMode^=4;} // BAND
      if (PIR(scrW-272,180,scrW-248,192,mstate.x,mstate.y)) {song->ins[CurrentIns]->filterMode^=8;} // pcm flag
      if (PIR(scrW-272,252,scrW-256,264,mstate.x,mstate.y)) {song->ins[CurrentIns]->filterMode^=16;} // rm flag
      if (PIR(scrW-272,264,scrW-256,276,mstate.x,mstate.y)) {song->ins[CurrentIns]->flags^=32;} // sync flag
      if (PIR(scrW-272,288,scrW-208,300,mstate.x,mstate.y)) {song->ins[CurrentIns]->flags^=1;} // reset osc flag
      // not this one
      if (PIR(488,60,512,72,mstate.x,mstate.y)) {hexmode=!hexmode;}
      // but yes these ones
      if (PIR(scrW-200,180,scrW-193,191,mstate.x,mstate.y)) {song->ins[CurrentIns]->filterMode^=128;} 
      if (PIR(scrW-192,180,scrW-185,191,mstate.x,mstate.y)) {song->ins[CurrentIns]->pcmPos+=4096;}
      if (PIR(scrW-184,180,scrW-177,191,mstate.x,mstate.y)) {song->ins[CurrentIns]->pcmPos+=256;}
      if (PIR(scrW-176,180,scrW-169,191,mstate.x,mstate.y)) {song->ins[CurrentIns]->pcmPos+=16;}
      if (PIR(scrW-168,180,scrW-160,191,mstate.x,mstate.y)) {song->ins[CurrentIns]->pcmPos++;}
      if (PIR(scrW-88,180,scrW-81,191,mstate.x,mstate.y)) {song->ins[CurrentIns]->pcmLen+=4096;}
      if (PIR(scrW-80,180,scrW-73,191,mstate.x,mstate.y)) {song->ins[CurrentIns]->pcmLen+=256;}
      if (PIR(scrW-72,180,scrW-63,191,mstate.x,mstate.y)) {song->ins[CurrentIns]->pcmLen+=16;}
      if (PIR(scrW-64,180,scrW-56,191,mstate.x,mstate.y)) {song->ins[CurrentIns]->pcmLen++;}

      // filter. somebody screwed up the order.
      if (PIR(scrW-(800-592),216,scrW-(800-599),228,mstate.x,mstate.y)) {song->ins[CurrentIns]->filterH-=4096;}
      if (PIR(scrW-(800-600),216,scrW-(800-607),228,mstate.x,mstate.y)) {song->ins[CurrentIns]->filterH-=256;}
      if (PIR(scrW-(800-608),216,scrW-(800-615),228,mstate.x,mstate.y)) {song->ins[CurrentIns]->filterH-=16;}
      if (PIR(scrW-(800-616),216,scrW-(800-623),228,mstate.x,mstate.y)) {song->ins[CurrentIns]->filterH--;}

      if (PIR(scrW-(800-624),252,scrW-(800-631),264,mstate.x,mstate.y)) {song->ins[CurrentIns]->LFO++;}
      if (PIR(scrW-(800-632),252,scrW-(800-640),264,mstate.x,mstate.y)) {song->ins[CurrentIns]->LFO--;}

      if (PIR(scrW-(800-600),324,scrW-(800-607),336,mstate.x,mstate.y)) {song->ins[CurrentIns]->flags+=64;}
      if (PIR(scrW-(800-608),324,scrW-(800-616),336,mstate.x,mstate.y)) {song->ins[CurrentIns]->flags-=64;}
    }
    if (rightpress) {
      if (PIR(scrW-(800-600),180,scrW-(800-607),191,mstate.x,mstate.y)) {song->ins[CurrentIns]->filterMode^=128;}
      if (PIR(scrW-(800-608),180,scrW-(800-615),191,mstate.x,mstate.y)) {song->ins[CurrentIns]->pcmPos-=4096;}
      if (PIR(scrW-(800-616),180,scrW-(800-623),191,mstate.x,mstate.y)) {song->ins[CurrentIns]->pcmPos-=256;}
      if (PIR(scrW-(800-624),180,scrW-(800-631),191,mstate.x,mstate.y)) {song->ins[CurrentIns]->pcmPos-=16;}
      if (PIR(scrW-(800-632),180,scrW-(800-640),191,mstate.x,mstate.y)) {song->ins[CurrentIns]->pcmPos--;}
      if (PIR(scrW-(800-712),180,scrW-(800-719),191,mstate.x,mstate.y)) {song->ins[CurrentIns]->pcmLen-=4096;}
      if (PIR(scrW-(800-720),180,scrW-(800-727),191,mstate.x,mstate.y)) {song->ins[CurrentIns]->pcmLen-=256;}
      if (PIR(scrW-(800-728),180,scrW-(800-735),191,mstate.x,mstate.y)) {song->ins[CurrentIns]->pcmLen-=16;}
      if (PIR(scrW-(800-736),180,scrW-(800-744),191,mstate.x,mstate.y)) {song->ins[CurrentIns]->pcmLen--;}
      
      // ditto and mimikyu
      if (PIR(scrW-(800-592),216,scrW-(800-599),228,mstate.x,mstate.y)) {song->ins[CurrentIns]->filterH+=4096;}
      if (PIR(scrW-(800-600),216,scrW-(800-607),228,mstate.x,mstate.y)) {song->ins[CurrentIns]->filterH+=256;}
      if (PIR(scrW-(800-608),216,scrW-(800-615),228,mstate.x,mstate.y)) {song->ins[CurrentIns]->filterH+=16;}
      if (PIR(scrW-(800-616),216,scrW-(800-623),228,mstate.x,mstate.y)) {song->ins[CurrentIns]->filterH++;}
    }
    if (leftclick) {
      if (PIR(0,scrH-18,8,scrH-6,mstate.x,mstate.y)) {scrollpos--; if (scrollpos<0) {scrollpos=0;};}
      if (PIR(500,scrH-18,512,scrH-6,mstate.x,mstate.y)) {scrollpos++; if (scrollpos>124) {scrollpos=124;};}
      // other stuff
      if (PIR(scrW-(800-608),252,scrW-(800-615),264,mstate.x,mstate.y)) {song->ins[CurrentIns]->LFO++;}
      if (PIR(scrW-(800-616),252,scrW-(800-623),264,mstate.x,mstate.y)) {song->ins[CurrentIns]->LFO--;}
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
        if (PIR(0,120+(ii*12)-(diskopscrollpos*12),scrW-8,133+(ii*12)-(diskopscrollpos*12),mstate.x,mstate.y)) {
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
                if (maxCTD>song->channels) maxCTD=song->channels;
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
                  if (maxCTD>song->channels) maxCTD=song->channels;
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
    if (PIR(760,60,767,72,mstate.x,mstate.y)) {song->speed--;if (song->speed<1) {song->speed=1;};player.speed=song->speed;}
    if (PIR(768,60,776,72,mstate.x,mstate.y)) {song->speed++;if (song->speed<1) {song->speed=1;};player.speed=song->speed;}
    if (PIR(456,60,463,72,mstate.x,mstate.y)) {song->detune--;}
    if (PIR(464,60,472,72,mstate.x,mstate.y)) {song->detune++;}
    if (PIR(568,60,575,72,mstate.x,mstate.y)) {song->orders--;}
    if (PIR(576,60,584,72,mstate.x,mstate.y)) {song->orders++;}
    if (PIR(32,276,64,288,mstate.x,mstate.y)) {screen=11;}
    }
    if (leftclick) {
    if (PIR(440,60,447,72,mstate.x,mstate.y)) {song->detune--;}
    if (PIR(448,60,455,72,mstate.x,mstate.y)) {song->detune++;}
    if (PIR(552,60,559,72,mstate.x,mstate.y)) {song->orders--;}
    if (PIR(560,60,567,72,mstate.x,mstate.y)) {song->orders++;}
    }
  }
  // events only in mixer view
  if (screen==4) {
    int mixerdrawoffset=(scrW/2)-chanstodisplay*48-12;
    if (leftclick) {
      for (int ii=0;ii<chanstodisplay;ii++) {
        if (PIR(56+(ii*96)+mixerdrawoffset,84,63+(ii*96)+mixerdrawoffset,95,mstate.x,mstate.y)) {song->defaultVol[ii+curedpage]++;if (song->defaultVol[ii+curedpage]>128) {song->defaultVol[ii+curedpage]=128;}}
        if (PIR(64+(ii*96)+mixerdrawoffset,84,72+(ii*96)+mixerdrawoffset,95,mstate.x,mstate.y)) {song->defaultVol[ii+curedpage]--;if (song->defaultVol[ii+curedpage]>250) {song->defaultVol[ii+curedpage]=0;}}
        if (PIR(56+(ii*96)+mixerdrawoffset,96,63+(ii*96)+mixerdrawoffset,108,mstate.x,mstate.y)) {song->defaultPan[ii+curedpage]++;}
        if (PIR(64+(ii*96)+mixerdrawoffset,96,72+(ii*96)+mixerdrawoffset,108,mstate.x,mstate.y)) {song->defaultPan[ii+curedpage]--;}
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
  Pattern* p=song->getPattern(song->order[player.pat],true);
  // FT2-like pattern editor
  // scroll code
  if (kbpressed[SDL_SCANCODE_DELETE]) {
  }
  if (curedmode==0) {
  // silences and stuff
  if (kbpressed[SDL_SCANCODE_EQUALS]) {p->data[player.step][curedpage+curedchan][0]=0x0d;EditSkip();}
  if (kbpressed[SDL_SCANCODE_1]) {p->data[player.step][curedpage+curedchan][0]=0x0f;EditSkip();}
  if (kbpressed[SDL_SCANCODE_BACKSLASH]) {p->data[player.step][curedpage+curedchan][0]=0x0e;EditSkip();}
  // notes, main octave
  if (kbpressed[SDL_SCANCODE_Z]) {p->data[player.step][curedpage+curedchan][0]=0x01+minval(curoctave<<4,0x90);EditSkip();}
  if (kbpressed[SDL_SCANCODE_X]) {p->data[player.step][curedpage+curedchan][0]=0x03+minval(curoctave<<4,0x90);EditSkip();}
  if (kbpressed[SDL_SCANCODE_C]) {p->data[player.step][curedpage+curedchan][0]=0x05+minval(curoctave<<4,0x90);EditSkip();}
  if (kbpressed[SDL_SCANCODE_V]) {p->data[player.step][curedpage+curedchan][0]=0x06+minval(curoctave<<4,0x90);EditSkip();}
  if (kbpressed[SDL_SCANCODE_B]) {p->data[player.step][curedpage+curedchan][0]=0x08+minval(curoctave<<4,0x90);EditSkip();}
  if (kbpressed[SDL_SCANCODE_N]) {p->data[player.step][curedpage+curedchan][0]=0x0a+minval(curoctave<<4,0x90);EditSkip();}
  if (kbpressed[SDL_SCANCODE_M]) {p->data[player.step][curedpage+curedchan][0]=0x0c+minval(curoctave<<4,0x90);EditSkip();}
  if (kbpressed[SDL_SCANCODE_S]) {p->data[player.step][curedpage+curedchan][0]=0x02+minval(curoctave<<4,0x90);EditSkip();}
  if (kbpressed[SDL_SCANCODE_D]) {p->data[player.step][curedpage+curedchan][0]=0x04+minval(curoctave<<4,0x90);EditSkip();}
  if (kbpressed[SDL_SCANCODE_G]) {p->data[player.step][curedpage+curedchan][0]=0x07+minval(curoctave<<4,0x90);EditSkip();}
  if (kbpressed[SDL_SCANCODE_H]) {p->data[player.step][curedpage+curedchan][0]=0x09+minval(curoctave<<4,0x90);EditSkip();}
  if (kbpressed[SDL_SCANCODE_J]) {p->data[player.step][curedpage+curedchan][0]=0x0b+minval(curoctave<<4,0x90);EditSkip();}
  // notes, 2nd octave
  if (kbpressed[SDL_SCANCODE_Q]) {p->data[player.step][curedpage+curedchan][0]=0x01+minval((curoctave+1)<<4,0x90);EditSkip();}
  if (kbpressed[SDL_SCANCODE_W]) {p->data[player.step][curedpage+curedchan][0]=0x03+minval((curoctave+1)<<4,0x90);EditSkip();}
  if (kbpressed[SDL_SCANCODE_E]) {p->data[player.step][curedpage+curedchan][0]=0x05+minval((curoctave+1)<<4,0x90);EditSkip();}
  if (kbpressed[SDL_SCANCODE_R]) {p->data[player.step][curedpage+curedchan][0]=0x06+minval((curoctave+1)<<4,0x90);EditSkip();}
  if (kbpressed[SDL_SCANCODE_T]) {p->data[player.step][curedpage+curedchan][0]=0x08+minval((curoctave+1)<<4,0x90);EditSkip();}
  if (kbpressed[SDL_SCANCODE_Y]) {p->data[player.step][curedpage+curedchan][0]=0x0a+minval((curoctave+1)<<4,0x90);EditSkip();}
  if (kbpressed[SDL_SCANCODE_U]) {p->data[player.step][curedpage+curedchan][0]=0x0c+minval((curoctave+1)<<4,0x90);EditSkip();}
  if (kbpressed[SDL_SCANCODE_2]) {p->data[player.step][curedpage+curedchan][0]=0x02+minval((curoctave+1)<<4,0x90);EditSkip();}
  if (kbpressed[SDL_SCANCODE_3]) {p->data[player.step][curedpage+curedchan][0]=0x04+minval((curoctave+1)<<4,0x90);EditSkip();}
  if (kbpressed[SDL_SCANCODE_5]) {p->data[player.step][curedpage+curedchan][0]=0x07+minval((curoctave+1)<<4,0x90);EditSkip();}
  if (kbpressed[SDL_SCANCODE_6]) {p->data[player.step][curedpage+curedchan][0]=0x09+minval((curoctave+1)<<4,0x90);EditSkip();}
  if (kbpressed[SDL_SCANCODE_7]) {p->data[player.step][curedpage+curedchan][0]=0x0b+minval((curoctave+1)<<4,0x90);EditSkip();}
  if (kbpressed[SDL_SCANCODE_I]) {p->data[player.step][curedpage+curedchan][0]=0x01+minval((curoctave+2)<<4,0x90);EditSkip();}
  if (kbpressed[SDL_SCANCODE_9]) {p->data[player.step][curedpage+curedchan][0]=0x02+minval((curoctave+2)<<4,0x90);EditSkip();}
  if (kbpressed[SDL_SCANCODE_O]) {p->data[player.step][curedpage+curedchan][0]=0x03+minval((curoctave+2)<<4,0x90);EditSkip();}
  if (kbpressed[SDL_SCANCODE_0]) {p->data[player.step][curedpage+curedchan][0]=0x04+minval((curoctave+2)<<4,0x90);EditSkip();}
  if (kbpressed[SDL_SCANCODE_P]) {p->data[player.step][curedpage+curedchan][0]=0x05+minval((curoctave+2)<<4,0x90);EditSkip();}
  /*
  if (kbpressed[66]) {p->data[player.step][curedpage+curedchan][0]=0x06+minval((curoctave+2)<<4,0x90);EditSkip();}
  if (kbpressed[65]) {p->data[player.step][curedpage+curedchan][0]=0x07+minval((curoctave+2)<<4,0x90);EditSkip();}
  */
  }
  if (curedmode==1) {
  if (kbpressed[SDL_SCANCODE_0]) {p->data[player.step][curedpage+curedchan][1]=(p->data[player.step][curedpage+curedchan][1]<<4);drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_1]) {p->data[player.step][curedpage+curedchan][1]=(p->data[player.step][curedpage+curedchan][1]<<4)+1;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_2]) {p->data[player.step][curedpage+curedchan][1]=(p->data[player.step][curedpage+curedchan][1]<<4)+2;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_3]) {p->data[player.step][curedpage+curedchan][1]=(p->data[player.step][curedpage+curedchan][1]<<4)+3;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_4]) {p->data[player.step][curedpage+curedchan][1]=(p->data[player.step][curedpage+curedchan][1]<<4)+4;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_5]) {p->data[player.step][curedpage+curedchan][1]=(p->data[player.step][curedpage+curedchan][1]<<4)+5;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_6]) {p->data[player.step][curedpage+curedchan][1]=(p->data[player.step][curedpage+curedchan][1]<<4)+6;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_7]) {p->data[player.step][curedpage+curedchan][1]=(p->data[player.step][curedpage+curedchan][1]<<4)+7;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_8]) {p->data[player.step][curedpage+curedchan][1]=(p->data[player.step][curedpage+curedchan][1]<<4)+8;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_9]) {p->data[player.step][curedpage+curedchan][1]=(p->data[player.step][curedpage+curedchan][1]<<4)+9;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_A]) {p->data[player.step][curedpage+curedchan][1]=(p->data[player.step][curedpage+curedchan][1]<<4)+10;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_B]) {p->data[player.step][curedpage+curedchan][1]=(p->data[player.step][curedpage+curedchan][1]<<4)+11;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_C]) {p->data[player.step][curedpage+curedchan][1]=(p->data[player.step][curedpage+curedchan][1]<<4)+12;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_D]) {p->data[player.step][curedpage+curedchan][1]=(p->data[player.step][curedpage+curedchan][1]<<4)+13;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_E]) {p->data[player.step][curedpage+curedchan][1]=(p->data[player.step][curedpage+curedchan][1]<<4)+14;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_F]) {p->data[player.step][curedpage+curedchan][1]=(p->data[player.step][curedpage+curedchan][1]<<4)+15;drawpatterns(true);}
  }
  if (curedmode==2) {
  if (kbpressed[SDL_SCANCODE_0]) {p->data[player.step][curedpage+curedchan][2]=(p->data[player.step][curedpage+curedchan][2]<<4);drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_1]) {p->data[player.step][curedpage+curedchan][2]=(p->data[player.step][curedpage+curedchan][2]<<4)+1;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_2]) {p->data[player.step][curedpage+curedchan][2]=(p->data[player.step][curedpage+curedchan][2]<<4)+2;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_3]) {p->data[player.step][curedpage+curedchan][2]=(p->data[player.step][curedpage+curedchan][2]<<4)+3;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_4]) {p->data[player.step][curedpage+curedchan][2]=(p->data[player.step][curedpage+curedchan][2]<<4)+4;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_5]) {p->data[player.step][curedpage+curedchan][2]=(p->data[player.step][curedpage+curedchan][2]<<4)+5;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_6]) {p->data[player.step][curedpage+curedchan][2]=(p->data[player.step][curedpage+curedchan][2]<<4)+6;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_7]) {p->data[player.step][curedpage+curedchan][2]=(p->data[player.step][curedpage+curedchan][2]<<4)+7;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_8]) {p->data[player.step][curedpage+curedchan][2]=(p->data[player.step][curedpage+curedchan][2]<<4)+8;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_9]) {p->data[player.step][curedpage+curedchan][2]=(p->data[player.step][curedpage+curedchan][2]<<4)+9;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_A]) {p->data[player.step][curedpage+curedchan][2]=(p->data[player.step][curedpage+curedchan][2]<<4)+10;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_B]) {p->data[player.step][curedpage+curedchan][2]=(p->data[player.step][curedpage+curedchan][2]<<4)+11;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_C]) {p->data[player.step][curedpage+curedchan][2]=(p->data[player.step][curedpage+curedchan][2]<<4)+12;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_D]) {p->data[player.step][curedpage+curedchan][2]=(p->data[player.step][curedpage+curedchan][2]<<4)+13;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_E]) {p->data[player.step][curedpage+curedchan][2]=(p->data[player.step][curedpage+curedchan][2]<<4)+14;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_F]) {p->data[player.step][curedpage+curedchan][2]=(p->data[player.step][curedpage+curedchan][2]<<4)+15;drawpatterns(true);}
  }
  if (curedmode==3) {
  if (kbpressed[SDL_SCANCODE_A]) {p->data[player.step][curedpage+curedchan][3]=1;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_B]) {p->data[player.step][curedpage+curedchan][3]=2;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_C]) {p->data[player.step][curedpage+curedchan][3]=3;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_D]) {p->data[player.step][curedpage+curedchan][3]=4;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_E]) {p->data[player.step][curedpage+curedchan][3]=5;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_F]) {p->data[player.step][curedpage+curedchan][3]=6;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_G]) {p->data[player.step][curedpage+curedchan][3]=7;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_H]) {p->data[player.step][curedpage+curedchan][3]=8;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_I]) {p->data[player.step][curedpage+curedchan][3]=9;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_J]) {p->data[player.step][curedpage+curedchan][3]=10;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_K]) {p->data[player.step][curedpage+curedchan][3]=11;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_L]) {p->data[player.step][curedpage+curedchan][3]=12;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_M]) {p->data[player.step][curedpage+curedchan][3]=13;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_N]) {p->data[player.step][curedpage+curedchan][3]=14;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_O]) {p->data[player.step][curedpage+curedchan][3]=15;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_P]) {p->data[player.step][curedpage+curedchan][3]=16;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_Q]) {p->data[player.step][curedpage+curedchan][3]=17;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_R]) {p->data[player.step][curedpage+curedchan][3]=18;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_S]) {p->data[player.step][curedpage+curedchan][3]=19;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_T]) {p->data[player.step][curedpage+curedchan][3]=20;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_U]) {p->data[player.step][curedpage+curedchan][3]=21;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_V]) {p->data[player.step][curedpage+curedchan][3]=22;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_W]) {p->data[player.step][curedpage+curedchan][3]=23;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_X]) {p->data[player.step][curedpage+curedchan][3]=24;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_Y]) {p->data[player.step][curedpage+curedchan][3]=25;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_Z]) {p->data[player.step][curedpage+curedchan][3]=26;drawpatterns(true);}
  }
  if (curedmode==4) {
  if (kbpressed[SDL_SCANCODE_0]) {p->data[player.step][curedpage+curedchan][4]=(p->data[player.step][curedpage+curedchan][4]<<4);drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_1]) {p->data[player.step][curedpage+curedchan][4]=(p->data[player.step][curedpage+curedchan][4]<<4)+1;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_2]) {p->data[player.step][curedpage+curedchan][4]=(p->data[player.step][curedpage+curedchan][4]<<4)+2;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_3]) {p->data[player.step][curedpage+curedchan][4]=(p->data[player.step][curedpage+curedchan][4]<<4)+3;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_4]) {p->data[player.step][curedpage+curedchan][4]=(p->data[player.step][curedpage+curedchan][4]<<4)+4;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_5]) {p->data[player.step][curedpage+curedchan][4]=(p->data[player.step][curedpage+curedchan][4]<<4)+5;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_6]) {p->data[player.step][curedpage+curedchan][4]=(p->data[player.step][curedpage+curedchan][4]<<4)+6;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_7]) {p->data[player.step][curedpage+curedchan][4]=(p->data[player.step][curedpage+curedchan][4]<<4)+7;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_8]) {p->data[player.step][curedpage+curedchan][4]=(p->data[player.step][curedpage+curedchan][4]<<4)+8;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_9]) {p->data[player.step][curedpage+curedchan][4]=(p->data[player.step][curedpage+curedchan][4]<<4)+9;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_A]) {p->data[player.step][curedpage+curedchan][4]=(p->data[player.step][curedpage+curedchan][4]<<4)+10;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_B]) {p->data[player.step][curedpage+curedchan][4]=(p->data[player.step][curedpage+curedchan][4]<<4)+11;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_C]) {p->data[player.step][curedpage+curedchan][4]=(p->data[player.step][curedpage+curedchan][4]<<4)+12;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_D]) {p->data[player.step][curedpage+curedchan][4]=(p->data[player.step][curedpage+curedchan][4]<<4)+13;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_E]) {p->data[player.step][curedpage+curedchan][4]=(p->data[player.step][curedpage+curedchan][4]<<4)+14;drawpatterns(true);}
  if (kbpressed[SDL_SCANCODE_F]) {p->data[player.step][curedpage+curedchan][4]=(p->data[player.step][curedpage+curedchan][4]<<4)+15;drawpatterns(true);}
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
  for (int su=0;su<8*(1+((song->channels-1)>>3));su++) {
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
  if (kbpressed[SDL_SCANCODE_PAGEDOWN]) {curedpage++; if (curedpage>(song->channels-chanstodisplay)) {curedpage=(song->channels-chanstodisplay);
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
  
  if (kbpressed[SDL_SCANCODE_TAB]) {MuteAllChannels();}
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

void keyEvent_pat(SDL_Event& ev) {
  int firstChan, firstMode;
  int lastChan, lastMode;
  int selTop, selBottom;
  Pattern* p=song->getPattern(song->order[player.pat],true);
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

  switch (ev.key.keysym.scancode) {
    case SDL_SCANCODE_END:
      curzoom++;
      maxCTD=(scrW*dpiScale-24*curzoom)/(96*curzoom);
      if (maxCTD<1) maxCTD=1;
      if (chanstodisplay>maxCTD) {
        chanstodisplay=maxCTD;
      }
      if (curedpage>(song->channels-chanstodisplay)) {
        curedpage=(song->channels-chanstodisplay);
      }
      drawpatterns(true);
      drawmixerlayer();
      break;
    case SDL_SCANCODE_HOME:
      curzoom--;
      if (curzoom<1) {
        triggerfx(1);
        curzoom=1;
      } else {
        maxCTD=(scrW*dpiScale-24*curzoom)/(96*curzoom);
        if (maxCTD<1) maxCTD=1;
        if (maxCTD>=song->channels) maxCTD=song->channels;
        chanstodisplay=maxCTD;
        if (curedpage>(song->channels-chanstodisplay)) {
          curedpage=(song->channels-chanstodisplay);
        }
        drawpatterns(true);
        drawmixerlayer();
      }
      break;
    case SDL_SCANCODE_UP:
      // go back
      if (player.playMode==0) {
        player.tick=1;
        player.step--;
        if (player.step<0) {
          player.step=0;
        }
        selEnd=player.step;
        if (!(ev.key.keysym.mod&KMOD_SHIFT)) {
          selStart=player.step;
          curselchan=curedchan;
          curselmode=curedmode;
        }
      }
      drawpatterns(true);
      break;
    case SDL_SCANCODE_DOWN:
      // skipping
      if (player.playMode==0) {
        player.tick=1;
        player.step++;
        if (player.step>(p->length-1)) {
          player.step=0;
          player.pat++;
        }
        selEnd=player.step;
        if (!(ev.key.keysym.mod&KMOD_SHIFT)) {
          selStart=player.step;
          curselchan=curedchan;
          curselmode=curedmode;
        }
      }
      drawpatterns(true);
      break;
    case SDL_SCANCODE_LEFT:
      curedmode--;
      if (curedmode<0) {
        curedchan--;
        curedmode=4;
        if (curedchan<0) {
          curedchan=0;
          curedmode=0;
        }
      }
      if (!(ev.key.keysym.mod&KMOD_SHIFT)) {
        curselchan=curedchan;
        curselmode=curedmode;
        selEnd=selStart;
      }
      drawpatterns(true);
      break;
    case SDL_SCANCODE_RIGHT:
      curedmode++;
      if (curedmode>4) {
        curedchan++;
        curedmode=0;
        if (curedchan>=chanstodisplay) {
          curedchan=chanstodisplay-1;
          curedmode=4;
        }
      }
      if (!(ev.key.keysym.mod&KMOD_SHIFT)) {
        curselchan=curedchan;
        curselmode=curedmode;
        selEnd=selStart;
      }
      drawpatterns(true);
      break;
    case SDL_SCANCODE_DELETE:
      // delete from selection start to end
      for (int i=firstChan; i<=lastChan; i++) {
        for (int j=(i==firstChan)?firstMode:0; (j<5 && (i<lastChan || j<=lastMode)); j++) {
          for (int k=selTop; k<=selBottom; k++) {
            p->data[k][curedpage+i][j]=0x00;
          }
        }
      }
      drawpatterns(true);
      selStart=player.step;
      selEnd=player.step;
      curselchan=curedchan;
      curselmode=curedmode;
      break;
    default:
      break;
  }
  
  // note input.
  int inNote=-1;
  int inIns=-1;
  int inVolEffect=-1;
  int inVol=-1;
  int inEffect=-1;
  int inEffectVal=-1;
  
  switch (curedmode) {
    case 0: // note
      switch (ev.key.keysym.scancode) {
        // octave 1
        case SDL_SCANCODE_Z:
          inNote=1;
          break;
        case SDL_SCANCODE_S:
          inNote=2;
          break;
        case SDL_SCANCODE_X:
          inNote=3;
          break;
        case SDL_SCANCODE_D:
          inNote=4;
          break;
        case SDL_SCANCODE_C:
          inNote=5;
          break;
        case SDL_SCANCODE_V:
          inNote=6;
          break;
        case SDL_SCANCODE_G:
          inNote=7;
          break;
        case SDL_SCANCODE_B:
          inNote=8;
          break;
        case SDL_SCANCODE_H:
          inNote=9;
          break;
        case SDL_SCANCODE_N:
          inNote=10;
          break;
        case SDL_SCANCODE_J:
          inNote=11;
          break;
        case SDL_SCANCODE_M:
          inNote=12;
          break;
        // octave 2
        case SDL_SCANCODE_Q:
          inNote=17;
          break;
        case SDL_SCANCODE_2:
          inNote=18;
          break;
        case SDL_SCANCODE_W:
          inNote=19;
          break;
        case SDL_SCANCODE_3:
          inNote=20;
          break;
        case SDL_SCANCODE_E:
          inNote=21;
          break;
        case SDL_SCANCODE_R:
          inNote=22;
          break;
        case SDL_SCANCODE_5:
          inNote=23;
          break;
        case SDL_SCANCODE_T:
          inNote=24;
          break;
        case SDL_SCANCODE_6:
          inNote=25;
          break;
        case SDL_SCANCODE_Y:
          inNote=26;
          break;
        case SDL_SCANCODE_7:
          inNote=27;
          break;
        case SDL_SCANCODE_U:
          inNote=28;
          break;
        // octave 3
        case SDL_SCANCODE_I:
          inNote=33;
          break;
        case SDL_SCANCODE_9:
          inNote=34;
          break;
        case SDL_SCANCODE_O:
          inNote=35;
          break;
        case SDL_SCANCODE_0:
          inNote=36;
          break;
        case SDL_SCANCODE_P:
          inNote=37;
          break;
        case SDL_SCANCODE_EQUALS:
          inNote=13;
          break;
        case SDL_SCANCODE_BACKSLASH:
          inNote=14;
          break;
        case SDL_SCANCODE_1:
          inNote=15;
          break;
        default:
          break;
      }
      break;
    case 1: // instrument
      switch (ev.key.keysym.scancode) {
        case SDL_SCANCODE_0:
          inIns=0;
          break;
        case SDL_SCANCODE_1:
          inIns=1;
          break;
        case SDL_SCANCODE_2:
          inIns=2;
          break;
        case SDL_SCANCODE_3:
          inIns=3;
          break;
        case SDL_SCANCODE_4:
          inIns=4;
          break;
        case SDL_SCANCODE_5:
          inIns=5;
          break;
        case SDL_SCANCODE_6:
          inIns=6;
          break;
        case SDL_SCANCODE_7:
          inIns=7;
          break;
        case SDL_SCANCODE_8:
          inIns=8;
          break;
        case SDL_SCANCODE_9:
          inIns=9;
          break;
        case SDL_SCANCODE_A:
          inIns=10;
          break;
        case SDL_SCANCODE_B:
          inIns=11;
          break;
        case SDL_SCANCODE_C:
          inIns=12;
          break;
        case SDL_SCANCODE_D:
          inIns=13;
          break;
        case SDL_SCANCODE_E:
          inIns=14;
          break;
        case SDL_SCANCODE_F:
          inIns=15;
          break;
        default:
          break;
      }
      break;
    case 2: // volume/volume effect
      if (ev.key.keysym.mod&KMOD_SHIFT) {
        switch (ev.key.keysym.scancode) {
          case SDL_SCANCODE_A:
            inVolEffect=0;
            break;
          case SDL_SCANCODE_B:
            inVolEffect=1;
            break;
          case SDL_SCANCODE_C:
            inVolEffect=2;
            break;
          case SDL_SCANCODE_D:
            inVolEffect=3;
            break;
          case SDL_SCANCODE_V:
            inVolEffect=4;
            break;
          case SDL_SCANCODE_P:
            inVolEffect=8;
            break;
          case SDL_SCANCODE_E:
            inVolEffect=12;
            break;
          case SDL_SCANCODE_F:
            inVolEffect=13;
            break;
          case SDL_SCANCODE_G:
            inVolEffect=14;
            break;
          case SDL_SCANCODE_H:
            inVolEffect=15;
            break;
          case SDL_SCANCODE_L:
            inVolEffect=28;
            break;
          case SDL_SCANCODE_R:
            inVolEffect=29;
            break;
          case SDL_SCANCODE_O:
            inVolEffect=30;
            break;
          case SDL_SCANCODE_U:
            inVolEffect=31;
            break;
          default:
            break;
        }
      } else {
        switch (ev.key.keysym.scancode) {
          case SDL_SCANCODE_0:
            inVol=0;
            break;
          case SDL_SCANCODE_1:
            inVol=1;
            break;
          case SDL_SCANCODE_2:
            inVol=2;
            break;
          case SDL_SCANCODE_3:
            inVol=3;
            break;
          case SDL_SCANCODE_4:
            inVol=4;
            break;
          case SDL_SCANCODE_5:
            inVol=5;
            break;
          case SDL_SCANCODE_6:
            inVol=6;
            break;
          case SDL_SCANCODE_7:
            inVol=7;
            break;
          case SDL_SCANCODE_8:
            inVol=8;
            break;
          case SDL_SCANCODE_9:
            inVol=9;
            break;
          case SDL_SCANCODE_A:
            inVol=10;
            break;
          case SDL_SCANCODE_B:
            inVol=11;
            break;
          case SDL_SCANCODE_C:
            inVol=12;
            break;
          case SDL_SCANCODE_D:
            inVol=13;
            break;
          case SDL_SCANCODE_E:
            inVol=14;
            break;
          case SDL_SCANCODE_F:
            inVol=15;
            break;
          default:
            break;
        }
      }
      break;
    case 3: // effect
      switch (ev.key.keysym.scancode) {
        case SDL_SCANCODE_A:
          inEffect=1;
          break;
        case SDL_SCANCODE_B:
          inEffect=2;
          break;
        case SDL_SCANCODE_C:
          inEffect=3;
          break;
        case SDL_SCANCODE_D:
          inEffect=4;
          break;
        case SDL_SCANCODE_E:
          inEffect=5;
          break;
        case SDL_SCANCODE_F:
          inEffect=6;
          break;
        case SDL_SCANCODE_G:
          inEffect=7;
          break;
        case SDL_SCANCODE_H:
          inEffect=8;
          break;
        case SDL_SCANCODE_I:
          inEffect=9;
          break;
        case SDL_SCANCODE_J:
          inEffect=10;
          break;
        case SDL_SCANCODE_K:
          inEffect=11;
          break;
        case SDL_SCANCODE_L:
          inEffect=12;
          break;
        case SDL_SCANCODE_M:
          inEffect=13;
          break;
        case SDL_SCANCODE_N:
          inEffect=14;
          break;
        case SDL_SCANCODE_O:
          inEffect=15;
          break;
        case SDL_SCANCODE_P:
          inEffect=16;
          break;
        case SDL_SCANCODE_Q:
          inEffect=17;
          break;
        case SDL_SCANCODE_R:
          inEffect=18;
          break;
        case SDL_SCANCODE_S:
          inEffect=19;
          break;
        case SDL_SCANCODE_T:
          inEffect=20;
          break;
        case SDL_SCANCODE_U:
          inEffect=21;
          break;
        case SDL_SCANCODE_V:
          inEffect=22;
          break;
        case SDL_SCANCODE_W:
          inEffect=23;
          break;
        case SDL_SCANCODE_X:
          inEffect=24;
          break;
        case SDL_SCANCODE_Y:
          inEffect=25;
          break;
        case SDL_SCANCODE_Z:
          inEffect=26;
          break;
        default:
          break;
      }
      break;
    case 4: // effect value
      switch (ev.key.keysym.scancode) {
        case SDL_SCANCODE_0:
          inEffectVal=0;
          break;
        case SDL_SCANCODE_1:
          inEffectVal=1;
          break;
        case SDL_SCANCODE_2:
          inEffectVal=2;
          break;
        case SDL_SCANCODE_3:
          inEffectVal=3;
          break;
        case SDL_SCANCODE_4:
          inEffectVal=4;
          break;
        case SDL_SCANCODE_5:
          inEffectVal=5;
          break;
        case SDL_SCANCODE_6:
          inEffectVal=6;
          break;
        case SDL_SCANCODE_7:
          inEffectVal=7;
          break;
        case SDL_SCANCODE_8:
          inEffectVal=8;
          break;
        case SDL_SCANCODE_9:
          inEffectVal=9;
          break;
        case SDL_SCANCODE_A:
          inEffectVal=10;
          break;
        case SDL_SCANCODE_B:
          inEffectVal=11;
          break;
        case SDL_SCANCODE_C:
          inEffectVal=12;
          break;
        case SDL_SCANCODE_D:
          inEffectVal=13;
          break;
        case SDL_SCANCODE_E:
          inEffectVal=14;
          break;
        case SDL_SCANCODE_F:
          inEffectVal=15;
          break;
        default:
          break;
      }
      break;
  }
  // note set
  if (inNote>=0) {
    player.step=selTop;
    if (inNote==13 || inNote==14 || inNote==15) {
      p->data[selTop][curedpage+curedchan][0]=inNote;
    } else {
      p->data[selTop][curedpage+curedchan][0]=minval(inNote+(curoctave<<4),0x9C);
    }
    EditSkip();
  }
  // instrument set
  if (inIns>=0) {
    p->data[selTop][curedpage+curedchan][1]<<=4;
    p->data[selTop][curedpage+curedchan][1]|=inIns;
    drawpatterns(true);
  }
  // volume effect set
  if (inVolEffect>=0) {
    // set volume effect
    p->data[selTop][curedpage+curedchan][2]&=0x0f;
    p->data[selTop][curedpage+curedchan][2]|=(inVolEffect&0x0f)<<4;
    p->data[selTop][curedpage+curedchan][3]&=0x7f;
    p->data[selTop][curedpage+curedchan][3]|=(inVolEffect&0x10)<<3;
    drawpatterns(true);
  }
  // volume set
  // TODO: other vol effects
  if (inVol>=0) {
    // check type
    if (p->data[selTop][curedpage+curedchan][2]==0 ||
        (p->data[selTop][curedpage+curedchan][2]>=0x40 &&
         p->data[selTop][curedpage+curedchan][2]<0x80)) {
      // normal volume
      p->data[selTop][curedpage+curedchan][2]=
        0x40+(((p->data[selTop][curedpage+curedchan][2]&0x3f)<<4)|inVol);
        if (p->data[selTop][curedpage+curedchan][2]>0x7f ||
            p->data[selTop][curedpage+curedchan][2]<0x40) {
          p->data[selTop][curedpage+curedchan][2]=
           0x40+(p->data[selTop][curedpage+curedchan][2]&0x0f);
        }
    } else if (p->data[selTop][curedpage+curedchan][2]>=0x80 &&
               p->data[selTop][curedpage+curedchan][2]<0xc0) {
      // panning
      p->data[selTop][curedpage+curedchan][2]=
        0x80+(((p->data[selTop][curedpage+curedchan][2]&0x3f)<<4)|inVol);
        if (p->data[selTop][curedpage+curedchan][2]>0xbf ||
            p->data[selTop][curedpage+curedchan][2]<0x80) {
          p->data[selTop][curedpage+curedchan][2]=
           0x80+(p->data[selTop][curedpage+curedchan][2]&0x0f);
        }
    } else {
      // other
      p->data[selTop][curedpage+curedchan][2]&=0xf0;
      p->data[selTop][curedpage+curedchan][2]|=inVol;
    }
    drawpatterns(true);
  }
  // effect set
  if (inEffect>=0) {
    p->data[selTop][curedpage+curedchan][3]&=0x80;
    p->data[selTop][curedpage+curedchan][3]|=inEffect;
    drawpatterns(true);
  }
  // effect value set
  if (inEffectVal>=0) {
    p->data[selTop][curedpage+curedchan][4]<<=4;
    p->data[selTop][curedpage+curedchan][4]|=inEffectVal;
    drawpatterns(true);
  }
}

void keyEvent(SDL_Event& ev) {
  // global keys
  if (!ev.key.repeat) switch (ev.key.keysym.scancode) {
    case SDL_SCANCODE_F5: // play/pause
      if (player.tick==0) {
        Play();
      } else {
        // TODO this
      }
      break;
    case SDL_SCANCODE_F6: // play
      Play();
      break;
    case SDL_SCANCODE_F7: // play pattern
      Play();
      break;
    case SDL_SCANCODE_F8: // stop
      player.stop();
      break;
    case SDL_SCANCODE_APPLICATION:
    case SDL_SCANCODE_GRAVE:
      ntsc=!ntsc;
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
      break;
    default:
      break;
  }
  
  switch (screen) {
    case 0: keyEvent_pat(ev); break;
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
  if (screen==0) {
    patStartX=(scrW*((float)dpiScale)-(24+chanstodisplay*96)*curzoom)/2;
    patStartY=(60+((scrH*dpiScale)-60)/2);
    patOffY=(patRow*12)*curzoom;
    Color barcol;
    
    // top bar
    int rectX;
    for (int i=0; i<chanstodisplay; i++) {
      soundchip::channel c=chip[(i+curedpage)>>3].chan[(i+curedpage)&7];

      if (c.flags.pcm) {
        barcol=g._WRAP_map_rgb(255,0,0);
      } else if (c.flags.shape==0) {
        barcol=mapHSV(250-(c.duty),0.67,1);
      } else {
        barcol=g._WRAP_map_rgb(
          (c.flags.shape==4 || c.flags.shape==1 || c.flags.shape==5)?(255):(0),
          (c.flags.shape!=5)?(255):(0),
          (c.flags.shape!=1 && c.flags.shape!=2)?(255):(0)
        );
      }
      barcol.a=0.1+float(c.vol)/200.0f;
      rectX=patStartX/dpiScale+(3+5.5+(12*i))*8*(float(curzoom)/float(dpiScale));
      g._WRAP_draw_filled_rectangle(rectX+6,67,rectX+6+38*(float(curzoom)/float(dpiScale))*(float((c.vol*(127-maxval(0,-c.pan)))>>7)/127.0f),73,barcol);
      g._WRAP_draw_filled_rectangle(rectX-6,67,rectX-6-38*(float(curzoom)/float(dpiScale))*(float((c.vol*(127-maxval(0,c.pan)))>>7)/127.0f),73,barcol);
      
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
                          0,//maxval(0,patRow-16)*12,
                          g._WRAP_get_bitmap_width(patternbitmap),
                          scrH*dpiScale-maxval(60,252-(patRow*12)),
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
                               (patStartY+(3*curzoom)-(patRow-selTop)*12*curzoom)/dpiScale,
                               (patStartX+(modeOff[j+1]+(i*96))*curzoom)/dpiScale,
                               (patStartY+(15*curzoom)-(patRow-selBottom)*12*curzoom)/dpiScale,
                               g._WRAP_map_rgba(128,128,128,128));
      }
    }
    g._WRAP_draw_filled_rectangle(
      0,
      ((patStartY+(3*curzoom)+((follow)?(0):(12*(maxval(0,player.step)-patRow)*curzoom)))/dpiScale),
      scrW+1,
      ((patStartY+(15*curzoom)+((follow)?(0):(12*(maxval(0,player.step)-patRow)*curzoom)))/dpiScale),
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
    patseek+=(player.pat-patseek)/4;
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
    g.tPos(18,1); g.printf("%.2X",player.speed);
    g.tPos(17,2); g.printf("%d",player.tempo);
    g.tPos(18,3); g.printf("%.2X",player.pat);
    g.tPos(32,1); g.printf("%.2X",song->order[player.pat]);
    g.tPos(32,2); g.printf("%.2X",curoctave);
    Pattern* p=song->getPattern(song->order[player.pat],true);
    g.tPos(32,3); g.printf("%.2X",p->length);
    
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
    g.printf(" %.2x/%.2x\n",player.tick,player.speed);
    g.printf(" %.2x/%.2x\n",maxval(0,player.step),p->length);
    g.printf(" %.2x:%.2x",song->order[player.pat],player.pat,song->orders);
    // draw orders
    // -128, 192, 255, +191, 128
    g._WRAP_set_clipping_rectangle(184,16,16,36);
    delta=(6*fmod(patseek,1));
    g.tNLPos(23);
    g.tPos(-fmod(patseek,1));
    if (((int)patseek-1)>0) {
      g.tColor(244-delta); g.printf("%s\n",getVisPat(song->order[maxval((int)patseek-2,0)]).c_str());
    } else {
      g.printf("\n");
    }
    if (((int)patseek)>0) {
      g.tColor(250-delta); g.printf("%s\n",getVisPat(song->order[maxval((int)patseek-1,0)]).c_str());
    } else {
      g.printf("\n");
    }
    g.tColor(255-delta); g.printf("%s\n",getVisPat(song->order[(int)patseek]).c_str());
    if (((int)patseek)<song->orders) {
      g.tColor(249+delta); g.printf("%s\n",getVisPat(song->order[minval((int)patseek+1,255)]).c_str());
    }
    if (((int)patseek+1)<song->orders) {
      g.tColor(244+delta); g.printf("%s",getVisPat(song->order[maxval((int)patseek+2,0)]).c_str());
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
        if (maxCTD>song->channels) maxCTD=song->channels;
        if (maxCTD<1) maxCTD=1;
        chanstodisplay=maxCTD;
        drawmixerlayer();
        drawpatterns(true);
      }
    } else if (ev.type==SDL_KEYDOWN) {
      if (inputvar!=NULL) {
        if (!imeActive) {
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
      } else {
        // pass event to keyboard input
        keyEvent(ev);
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
          g.printf("%.1fHz CLOCK: 6.18MHz i: %x t: %.5x NTSC",player.tempo/2.5,ASC::interval,ASC::currentclock);
        } else {
          g.printf("%.1fHz CLOCK: 5.95MHz i: %x t: %.5x PAL",player.tempo/2.5,ASC::interval,ASC::currentclock);
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
    scrW=800;
    scrH=450;
  }

  CleanupPatterns();

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
  printf("finished\n");
  return 0;
}
