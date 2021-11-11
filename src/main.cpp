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

#include <deque>
#include <stdexcept>

#ifdef HAVE_GUI
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_sdlrenderer.h"
#include "ImGuiFileDialog.h"
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

int curins=1; // selected instrument
int curmacro=-1; // selected macro
int curoctave=2;
int curedchan=0; // cureditingchannel
int curedmode=0; // current editing mode, 0=note, 1=instrument number 2=volume 3=effect name 4=effect value
int curedpage=0; // current page, 0-3
int curselchan=0;
int curselmode=0;
int curzoom=1;
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
string curfname;
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

const char* noteNamesPat[256]={
  "...", "C-0", "C#0", "D-0", "D#0", "E-0", "F-0", "F#0", "G-0", "G#0", "A-0", "A#0", "B-0", "===", "~~~", "^^^",
  "...", "C-1", "C#1", "D-1", "D#1", "E-1", "F-1", "F#1", "G-1", "G#1", "A-1", "A#1", "B-1", "===", "~~~", "^^^",
  "...", "C-2", "C#2", "D-2", "D#2", "E-2", "F-2", "F#2", "G-2", "G#2", "A-2", "A#2", "B-2", "===", "~~~", "^^^",
  "...", "C-3", "C#3", "D-3", "D#3", "E-3", "F-3", "F#3", "G-3", "G#3", "A-3", "A#3", "B-3", "===", "~~~", "^^^",
  "...", "C-4", "C#4", "D-4", "D#4", "E-4", "F-4", "F#4", "G-4", "G#4", "A-4", "A#4", "B-4", "===", "~~~", "^^^",
  "...", "C-5", "C#5", "D-5", "D#5", "E-5", "F-5", "F#5", "G-5", "G#5", "A-5", "A#5", "B-5", "===", "~~~", "^^^",
  "...", "C-6", "C#6", "D-6", "D#6", "E-6", "F-6", "F#6", "G-6", "G#6", "A-6", "A#6", "B-6", "===", "~~~", "^^^",
  "...", "C-7", "C#7", "D-7", "D#7", "E-7", "F-7", "F#7", "G-7", "G#7", "A-7", "A#7", "B-7", "===", "~~~", "^^^",
  "...", "C-8", "C#8", "D-8", "D#8", "E-8", "F-8", "F#8", "G-8", "G#8", "A-8", "A#8", "B-8", "===", "~~~", "^^^",
  "...", "C-9", "C#9", "D-9", "D#9", "E-9", "F-9", "F#9", "G-9", "G#9", "A-9", "A#9", "B-9", "===", "~~~", "^^^",
  "...", "???", "???", "???", "???", "???", "???", "???", "???", "???", "???", "???", "???", "===", "~~~", "^^^",
  "...", "???", "???", "???", "???", "???", "???", "???", "???", "???", "???", "???", "???", "===", "~~~", "^^^",
  "...", "???", "???", "???", "???", "???", "???", "???", "???", "???", "???", "???", "???", "===", "~~~", "^^^",
  "...", "???", "???", "???", "???", "???", "???", "???", "???", "???", "???", "???", "???", "===", "~~~", "^^^",
  "...", "???", "???", "???", "???", "???", "???", "???", "???", "???", "???", "???", "???", "===", "~~~", "^^^",
  "...", "???", "???", "???", "???", "???", "???", "???", "???", "???", "???", "???", "???", "===", "~~~", "^^^"
};

const char* noteNames[256]={
  "C-0", "C#0", "D-0", "D#0", "E-0", "F-0", "F#0", "G-0", "G#0", "A-0", "A#0", "B-0",
  "C-1", "C#1", "D-1", "D#1", "E-1", "F-1", "F#1", "G-1", "G#1", "A-1", "A#1", "B-1",
  "C-2", "C#2", "D-2", "D#2", "E-2", "F-2", "F#2", "G-2", "G#2", "A-2", "A#2", "B-2",
  "C-3", "C#3", "D-3", "D#3", "E-3", "F-3", "F#3", "G-3", "G#3", "A-3", "A#3", "B-3",
  "C-4", "C#4", "D-4", "D#4", "E-4", "F-4", "F#4", "G-4", "G#4", "A-4", "A#4", "B-4",
  "C-5", "C#5", "D-5", "D#5", "E-5", "F-5", "F#5", "G-5", "G#5", "A-5", "A#5", "B-5",
  "C-6", "C#6", "D-6", "D#6", "E-6", "F-6", "F#6", "G-6", "G#6", "A-6", "A#6", "B-6",
  "C-7", "C#7", "D-7", "D#7", "E-7", "F-7", "F#7", "G-7", "G#7", "A-7", "A#7", "B-7",
  "C-8", "C#8", "D-8", "D#8", "E-8", "F-8", "F#8", "G-8", "G#8", "A-8", "A#8", "B-8",
  "C-9", "C#9", "D-9", "D#9", "E-9", "F-9", "F#9", "G-9", "G#9", "A-9", "A#9", "B-9",
  "C-10", "C#10", "D-10", "D#10", "E-10", "F-10", "F#10", "G-10",
  "???", "???", "???", "???", "???", "???", "???", "???", "???", "???", "???", "???", "???", "???", "???", "???",
  "???", "???", "???", "???", "???", "???", "???", "???", "???", "???", "???", "???", "???", "???", "???", "???",
  "???", "???", "???", "???", "???", "???", "???", "???", "???", "???", "???", "???", "???", "???", "???", "???",
  "???", "???", "???", "???", "???", "???", "???", "???", "???", "???", "???", "???", "???", "???", "???", "???",
  "???", "???", "???", "???", "???", "???", "???", "???", "???", "???", "???", "???", "???", "???", "???", "???",
  "???", "???", "???", "???", "???", "???", "???", "???", "???", "???", "???", "???", "???", "???", "???", "???",
  "???", "???", "???", "???", "???", "???", "???", "???", "???", "???", "???", "???", "???", "???", "???", "???",
  "???", "???", "???", "???", "???", "???", "???", "???", "???", "???", "???", "???", "???", "???", "???", "???"
};

const char* hexValues[256]={
  "00", "01", "02", "03", "04", "05", "06", "07", "08", "09", "0A", "0B", "0C", "0D", "0E", "0F",
  "10", "11", "12", "13", "14", "15", "16", "17", "18", "19", "1A", "1B", "1C", "1D", "1E", "1F",
  "20", "21", "22", "23", "24", "25", "26", "27", "28", "29", "2A", "2B", "2C", "2D", "2E", "2F",
  "30", "31", "32", "33", "34", "35", "36", "37", "38", "39", "3A", "3B", "3C", "3D", "3E", "3F",
  "40", "41", "42", "43", "44", "45", "46", "47", "48", "49", "4A", "4B", "4C", "4D", "4E", "4F",
  "50", "51", "52", "53", "54", "55", "56", "57", "58", "59", "5A", "5B", "5C", "5D", "5E", "5F",
  "60", "61", "62", "63", "64", "65", "66", "67", "68", "69", "6A", "6B", "6C", "6D", "6E", "6F",
  "70", "71", "72", "73", "74", "75", "76", "77", "78", "79", "7A", "7B", "7C", "7D", "7E", "7F",
  "80", "81", "82", "83", "84", "85", "86", "87", "88", "89", "8A", "8B", "8C", "8D", "8E", "8F",
  "90", "91", "92", "93", "94", "95", "96", "97", "98", "99", "9A", "9B", "9C", "9D", "9E", "9F",
  "A0", "A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8", "A9", "AA", "AB", "AC", "AD", "AE", "AF",
  "B0", "B1", "B2", "B3", "B4", "B5", "B6", "B7", "B8", "B9", "BA", "BB", "BC", "BD", "BE", "BF",
  "C0", "C1", "C2", "C3", "C4", "C5", "C6", "C7", "C8", "C9", "CA", "CB", "CC", "CD", "CE", "CF",
  "D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7", "D8", "D9", "DA", "DB", "DC", "DD", "DE", "DF",
  "E0", "E1", "E2", "E3", "E4", "E5", "E6", "E7", "E8", "E9", "EA", "EB", "EC", "ED", "EE", "EF",
  "F0", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "FA", "FB", "FC", "FD", "FE", "FF"
};

const char* volValues[256]={
  "...", "a01", "a02", "a03", "a04", "a05", "a06", "a07", "a08", "a09", "a0A", "a0B", "a0C", "a0D", "a0E", "a0F",
  "b00", "b01", "b02", "b03", "b04", "b05", "b06", "b07", "b08", "b09", "b0A", "b0B", "b0C", "b0D", "b0E", "b0F",
  "c00", "c01", "c02", "c03", "c04", "c05", "c06", "c07", "c08", "c09", "c0A", "c0B", "c0C", "c0D", "c0E", "c0F",
  "d00", "d01", "d02", "d03", "d04", "d05", "d06", "d07", "d08", "d09", "d0A", "d0B", "d0C", "d0D", "d0E", "d0F",
  "v00", "v01", "v02", "v03", "v04", "v05", "v06", "v07", "v08", "v09", "v0A", "v0B", "v0C", "v0D", "v0E", "v0F",
  "v10", "v11", "v12", "v13", "v14", "v15", "v16", "v17", "v18", "v19", "v1A", "v1B", "v1C", "v1D", "v1E", "v1F",
  "v20", "v21", "v22", "v23", "v24", "v25", "v26", "v27", "v28", "v29", "v2A", "v2B", "v2C", "v2D", "v2E", "v2F",
  "v30", "v31", "v32", "v33", "v34", "v35", "v36", "v37", "v38", "v39", "v3A", "v3B", "v3C", "v3D", "v3E", "v3F",
  "p00", "p01", "p02", "p03", "p04", "p05", "p06", "p07", "p08", "p09", "p0A", "p0B", "p0C", "p0D", "p0E", "p0F",
  "p10", "p11", "p12", "p13", "p14", "p15", "p16", "p17", "p18", "p19", "p1A", "p1B", "p1C", "p1D", "p1E", "p1F",
  "p20", "p21", "p22", "p23", "p24", "p25", "p26", "p27", "p28", "p29", "p2A", "p2B", "p2C", "p2D", "p2E", "p2F",
  "p30", "p31", "p32", "p33", "p34", "p35", "p36", "p37", "p38", "p39", "p3A", "p3B", "p3C", "p3D", "p3E", "p3F",
  "e00", "e01", "e02", "e03", "e04", "e05", "e06", "e07", "e08", "e09", "e0A", "e0B", "e0C", "e0D", "e0E", "e0F",
  "f00", "f01", "f02", "f03", "f04", "f05", "f06", "f07", "f08", "f09", "f0A", "f0B", "f0C", "f0D", "f0E", "f0F",
  "g00", "g01", "g02", "g03", "g04", "g05", "g06", "g07", "g08", "g09", "g0A", "g0B", "g0C", "g0D", "g0E", "g0F",
  "h00", "h01", "h02", "h03", "h04", "h05", "h06", "h07", "h08", "h09", "h0A", "h0B", "h0C", "h0D", "h0E", "h0F",  
};

const char* chanValueNames[32]={
  "<FREQ", ">FREQ", "VOL", "PAN", "<CNT", ">CNT", "<CUT", ">CUT", "DUTY", "RES", "<PCMP", ">PCMP", "<PCMB", ">PCMB", "<PCMR", ">PCMR",
  "<SWFS", ">SWFS", "SWFA", "SWFB", "<SWVS", ">SWVS", "SWVA", "SWVB", "<SWCS", ">SWCS", "SWCA", "SWCB", "<PCMC", ">PCMC", "<RESET", ">RESET"
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
Swiper diskopSwiper;
float doScroll;

// new things
Song* song=NULL;
Player player;
std::mutex canUseSong;

bool insEditOpen=false;
bool macroEditOpen=false;
bool memViewOpen=false;
bool songEditOpen=false;
bool macroGraph=false;

struct SelectionPoint {
  int x, y;
  SelectionPoint():
    x(0), y(0) {}
} selStart, selEnd;

bool selecting=false;
bool isSaving=false;

int delayedStep=0;
int delayedPat=0;
int mvChip=0;

float nextScroll=-1.0f;
float amp=1.0f;

enum WindowTypes {
  wPattern,
  wInstrument,
  wMacro,
  wHeaderBar
};

int curWindow=wPattern;

#ifdef HAVE_GUI
std::map<SDL_Keycode,int> noteKeys;
std::map<SDL_Keycode,int> valueKeys;
std::map<SDL_Keycode,int> effectKeys;
#endif

enum UndoAction {
  undoPatternNote,
  undoPatternInsert,
  undoPatternDelete,
  undoPatternPullDelete,
  undoPatternCut,
  undoPatternPaste
};

struct UndoData {
  unsigned char row, column, oldData, newData;
  unsigned char reserved[3];
  UndoData(unsigned char r, unsigned char c, unsigned char o, unsigned char n):
    row(r),
    column(c),
    oldData(o),
    newData(n) {}
};

struct UndoStep {
  UndoAction action;
  unsigned char pattern;
  std::vector<UndoData> data;
};

std::deque<UndoStep> undoHist;
std::deque<UndoStep> redoHist;

unsigned char oldPat[256][32][8];

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
    buf[0][i]=amp*float(bbOut[0][i])/16384.0f;
    buf[1][i]=amp*float(bbOut[1][i])/16384.0f;
#else
    switch (ar.format) {
        case AUDIO_F32:
          buf[0][i*ar.channels]=amp*float(bbOut[0][i])/16384.0f;
          buf[0][1+(i*ar.channels)]=amp*float(bbOut[1][i])/16384.0f;
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

string getVisPat(unsigned char p) {
  if (p==254) return "--";
  if (p==255) return "==";
  return fmt::sprintf("%.2X",p);
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
  origin="";
  undoHist.clear();
  redoHist.clear();
  canUseSong.unlock();
  curfname="";
}

void updateWindowTitle() {
  string name=song->name;
  if (name=="") {
    SDL_SetWindowTitle(sdlWin,PROGRAM_NAME);
  } else {
    SDL_SetWindowTitle(sdlWin,(name+S(" - ")+S(PROGRAM_NAME)).c_str());
  }
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
    memset(song->name,0,32);
    for (sk=4;sk<30;sk++) {
      if (memblock[sk]==0) break;
      song->name[sk-4]=memblock[sk];
    }
    printf("module name is %s\n",song->name);
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
  updateWindowTitle();
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
        origin="early ProTracker/NoiseTracker";
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
    memset(song->name,0,32);
    for (sk=0;sk<20;sk++) {
      if (h.name[sk]==0) break;
      song->name[sk]=h.name[sk];
    }
    printf("module name is %s\n",song->name);
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
  updateWindowTitle();
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
  memset(song->name,0,32);
  for (sk=0;sk<28;sk++) {
    if (memblock[sk]==0) break;
    song->name[sk]=memblock[sk];
  }
  printf("module name is %s\n",song->name);
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
  memset(song->name,0,32);
  for (int i=0; i<20; i++) {
    if (h.name[i+17]==0) break;
    song->name[i]=h.name[i+17];
  }
  
  origin="";
  for (int i=0; i<20; i++) {
    if (h.program[i]==0) break;
    origin+=h.program[i];
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
      popbox=PopupBox("Error",fmt::sprintf("pattern %d header size mismatch! %d != 9",i,ph.size));
      triggerfx(1);
      fclose(xm);
      return 1;
    }
    Pattern* p=song->getPattern(i,true);
    p->length=ph.rows[0]+(ph.rows[1]<<8);
    if (p->length>256) p->length=256;
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
  updateWindowTitle();
  return 0;
}

int SaveFile(const char* filename) {
  // save file
  FILE *sfile;
  //printf("\nplease write filename? ");
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
  int oldseek=0;
  
  // temporary, gonna get replaced by a better thing soon
  // just for the sake of linux
  sfile=ps_fopen(filename,"wb");
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
      fputc(m->intendedUse,sfile); // write intendedUse
      fwrite("\0\0\0\0\0\0\0",1,7,sfile); // reserved
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
    curfname=filename;
    origin=fmt::sprintf("soundtracker (r%d)\n",song->version);
    return 0;
  }
  curfname="";
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
        curfname="";
        triggerfx(1);
        popbox=PopupBox("Error","todo");
        return 1;
        break;
      case FormatMOD:
        curfname="";
        return ImportMOD(sfile);
        break;
      case FormatS3M:
        curfname="";
        return ImportS3M(sfile);
        break;
      case FormatIT:
        curfname="";
        return ImportIT(sfile);
        break;
      case FormatXM:
        curfname="";
        return ImportXM(sfile);
        break;
      default:
        printf("error: not a compatible file!\n");fclose(sfile);
        triggerfx(1);
        curfname="";
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
      curfname="";
      return 1;
    }

    printf("module version %d\n",song->version);
    origin=fmt::sprintf("soundtracker (r%d)\n",song->version);
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
          switch (j) {
            case 4: // shape
              m->intendedUse=iuShape;
              break;
            case 5: // pitch
              m->intendedUse=iuPitch;
              break;
            case 6: case 7: // fine pitch and panning
              m->intendedUse=iuPan;
              break;
            default:
              m->intendedUse=iuGeneric;
              break;
          }
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
        int intendedUse=fgeti(sfile);
        fgeti(sfile);
        m->intendedUse=intendedUse;
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
    updateWindowTitle();
    printf("setting filename to %s\n",filename);
    curfname=filename;
    return 0;
  } else {
    perror("can't open file");
    popbox=PopupBox("Error","can't open file!");
    curfname="";
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

short newMacro(MacroIntendedUse intendedUse=iuGeneric) {
  if (song->macros.size()>=8192) return -1;
  Macro* m=new Macro();
  m->intendedUse=intendedUse;
  song->macros.push_back(m);
  return short(song->macros.size()-1);
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

void startSelection(int x, int y) {
  selStart.x=x;
  selStart.y=y;
  selEnd.x=x;
  selEnd.y=y;
  selecting=true;
}

void updateSelection(int x, int y) {
  if (!selecting) return;
  selEnd.x=x;
  selEnd.y=y;
}

void finishSelection() {
  // swap points if required
  if (selEnd.x<selStart.x) {
    selEnd.x^=selStart.x;
    selStart.x^=selEnd.x;
    selEnd.x^=selStart.x;
  }
  if (selEnd.y<selStart.y) {
    selEnd.y^=selStart.y;
    selStart.y^=selEnd.y;
    selEnd.y^=selStart.y;
  }
  selecting=false;
}

void updateScroll(int amount) {
  float lineHeight=(ImGui::GetTextLineHeight()+2*dpiScale);
  nextScroll=lineHeight*amount;
}

void drawPatterns(float ypos) {
  char id[16];
  SelectionPoint visStart=selStart;
  SelectionPoint visEnd=selEnd;
  // swap points if required
  if (visEnd.x<visStart.x) {
    visEnd.x^=visStart.x;
    visStart.x^=visEnd.x;
    visEnd.x^=visStart.x;
  }
  if (visEnd.y<visStart.y) {
    visEnd.y^=visStart.y;
    visStart.y^=visEnd.y;
    visEnd.y^=visStart.y;
  }
  float oneCharSize=ImGui::CalcTextSize("A").x;

  ImGui::Begin("Pattern View",NULL,ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoBringToFrontOnFocus|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoScrollWithMouse|ImGuiWindowFlags_NoScrollbar);
  ImGui::SetWindowPos("Pattern View",ImVec2(0.0f,ypos));
  ImGui::SetWindowSize("Pattern View",ImVec2(scrW*dpiScale,scrH*dpiScale-ypos),ImGuiCond_Always);
  ImGui::Columns(2);
  ImGui::SetColumnWidth(0,oneCharSize*2+ImGui::GetStyle().ItemSpacing.x*2+ImGui::GetStyle().ItemInnerSpacing.x*2+ImGui::GetStyle().ScrollbarSize);

  // order view
  if (ImGui::BeginTable("OrdersList",1,ImGuiTableFlags_BordersInnerH|ImGuiTableFlags_BordersInnerV|ImGuiTableFlags_BordersOuterH|ImGuiTableFlags_BordersOuterV|ImGuiTableFlags_ScrollY)) {
    char orderName[10];
    for (int i=0; i<=song->orders; i++) {
      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      sprintf(orderName,"%.2X##OL%d",song->order[i],i);
      ImGui::Selectable(orderName,delayedPat==i);
      if (ImGui::IsItemClicked()) {
        player.pat=i;
        if (player.playMode==1) Play();
      }
    }
    ImGui::EndTable();
  }
  ImGui::NextColumn();

  // pattern view
  Pattern* p=song->getPattern(song->order[delayedPat],false);
  int playerStep=player.step;
  float lineHeight=(ImGui::GetTextLineHeight()+2*dpiScale);
  ImGui::PushStyleVar(ImGuiStyleVar_CellPadding,ImVec2(0.0f,0.0f));
  if (ImGui::BeginTable("Pattern",song->channels+1,ImGuiTableFlags_BordersInnerV|ImGuiTableFlags_ScrollX|ImGuiTableFlags_ScrollY|ImGuiTableFlags_NoPadInnerX)) {
    ImGui::TableSetupColumn("pos",ImGuiTableColumnFlags_WidthFixed);
    if (player.playMode==1) updateScroll(playerStep);
    if (nextScroll>-0.5f) {
      ImGui::SetScrollY(nextScroll);
      nextScroll=-1.0f;
    }
    ImGui::TableSetupScrollFreeze(1,1);
    for (int i=0; i<song->channels; i++) {
      ImGui::TableSetupColumn(fmt::sprintf("c%d",i).c_str(),ImGuiTableColumnFlags_WidthFixed);
    }
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::Text("%.2X ",song->order[delayedPat]);
    for (int i=0; i<song->channels; i++) {
      soundchip::channel& c=chip[i>>3].chan[i&7];
      ImGui::TableNextColumn();
      if (c.flags.pcm) {
        ImGui::PushStyleColor(ImGuiCol_Header,ImVec4(
          0.6f, 0.1f, 0.1f,
          (float)c.vol/127.0f
        ));
      } else {
        ImGui::PushStyleColor(ImGuiCol_Header,ImVec4(
          (c.flags.shape==4 || c.flags.shape==1 || c.flags.shape==5)?0.6f:0.1f,
          (c.flags.shape!=5)?0.6f:0.1f,
          (c.flags.shape!=1 && c.flags.shape!=2)?0.6f:0.1f,
          (float)c.vol/127.0f
        ));
      }
      if (player.channelMask[i]) {
        ImGui::PushStyleColor(ImGuiCol_HeaderActive,ImVec4(0.8f,0.6f,0.1f,1.0f));
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered,ImVec4(0.4f,0.1f,0.1f,1.0f));
        ImGui::PushStyleColor(ImGuiCol_Text,ImVec4(0.4f,0.4f,0.4f,1.0f));
      } else {
        ImGui::PushStyleColor(ImGuiCol_HeaderActive,ImVec4(0.8f,0.6f,0.1f,1.0f));
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered,ImVec4(0.1f,0.3f,0.1f,1.0f));
        ImGui::PushStyleColor(ImGuiCol_Text,ImVec4(1.0f,1.0f,1.0f,1.0f));
      }
      ImGui::Selectable(fmt::sprintf(" %d##_CH%d",i,i).c_str(),!player.channelMask[i],ImGuiSelectableFlags_NoPadWithHalfSpacing,ImVec2(0.0f,lineHeight+2.0f*dpiScale));
      ImGui::PopStyleColor();
      ImGui::PopStyleColor();
      ImGui::PopStyleColor();
      ImGui::PopStyleColor();
      if (ImGui::IsItemClicked()) {
        player.toggleChannel(i);
      }
      if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
        bool isSolo=false;
        for (int j=0; j<song->channels; j++) {
          if (j==i) {
            isSolo=true;
            continue;
          }
          if (!player.channelMask[j]) {
            isSolo=false;
            break;
          }
        }
        if (isSolo) {
          for (int j=0; j<song->channels; j++) {
            player.maskChannel(j,false);
          }
        } else {
          for (int j=0; j<song->channels; j++) {
            player.maskChannel(j,true);
          }
          player.maskChannel(i,false);
        }
      }
    }
    int drawStart=int((scrH*dpiScale-ypos)/(lineHeight*2));
    int drawEnd=p->length;
    ImVec2 threeChars=ImVec2(oneCharSize*3.0f,lineHeight);
    ImVec2 twoChars=ImVec2(oneCharSize*2.0f,lineHeight);
    ImVec2 oneChar=ImVec2(oneCharSize,lineHeight);
    for (int i=1-drawStart; i<drawEnd; i++) {
      ImGui::TableNextRow(0,lineHeight);
      if (i<0) continue;
      ImGui::TableNextColumn();
      ImGui::TextColored(colors[colRowNumber],"%.2X ",i);
      if (player.playMode!=0 && i==delayedStep) ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0,0x40ffffff);
      bool selectedRow=(i>=visStart.y && i<=visEnd.y);
      for (int j=0; j<song->channels; j++) {
        int selX=j*5;
        bool selectedNote=(selectedRow && selX>=visStart.x && selX<=visEnd.x);
        selX++;
        bool selectedIns=(selectedRow && selX>=visStart.x && selX<=visEnd.x);
        selX++;
        bool selectedVol=(selectedRow && selX>=visStart.x && selX<=visEnd.x);
        selX++;
        bool selectedEffect=(selectedRow && selX>=visStart.x && selX<=visEnd.x);
        selX++;
        bool selectedEffectVal=(selectedRow && selX>=visStart.x && selX<=visEnd.x);

        ImGui::TableNextColumn();

        // note
        sprintf(id,"%s##PN%d_%.2X",noteNamesPat[p->data[i][j][0]],i,j);
        if (p->data[i][j][0]==0) {
          ImGui::PushStyleColor(ImGuiCol_Text,colors[colInvalidEffect]);
        }
        ImGui::Selectable(id,selectedNote,ImGuiSelectableFlags_NoPadWithHalfSpacing,threeChars);
        if (ImGui::IsItemClicked()) {
          startSelection(j*5,i);
        }
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem)) {
          updateSelection(j*5,i);
        }
        if (p->data[i][j][0]==0) {
          ImGui::PopStyleColor();
        }
        ImGui::SameLine(0.0f,0.0f);

        // instrument
        if (p->data[i][j][1]==0) {
          sprintf(id,"..##PI%d_%.2X",i,j);
          ImGui::PushStyleColor(ImGuiCol_Text,colors[colInvalidEffect]);
        } else {
          sprintf(id,"%s##PI%d_%.2X",hexValues[p->data[i][j][1]],i,j);
          ImGui::PushStyleColor(ImGuiCol_Text,colors[colInstrument]);
        }
        ImGui::Selectable(id,selectedIns,ImGuiSelectableFlags_NoPadWithHalfSpacing,twoChars);
        if (ImGui::IsItemClicked()) {
          startSelection(1+j*5,i);
        }
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem)) {
          updateSelection(1+j*5,i);
        }
        ImGui::PopStyleColor();
        ImGui::SameLine(0.0f,0.0f);

        // volume
        sprintf(id,"%s##PV%d_%.2X",volValues[p->data[i][j][2]],i,j);
        ImGui::PushStyleColor(ImGuiCol_Text,colors[(p->data[i][j][2]==0)?colInvalidEffect:colVolume]);
        ImGui::Selectable(id,selectedVol,ImGuiSelectableFlags_NoPadWithHalfSpacing,threeChars);
        if (ImGui::IsItemClicked()) {
          startSelection(2+j*5,i);
        }
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem)) {
          updateSelection(2+j*5,i);
        }
        ImGui::PopStyleColor();
        ImGui::SameLine(0.0f,0.0f);

        // effect
        sprintf(id,"%s##PE%d_%.2X",getFX(p->data[i][j][3]),i,j);
        ImGui::PushStyleColor(ImGuiCol_Text,colors[getFXColor(p->data[i][j][3])]);
        ImGui::Selectable(id,selectedEffect,ImGuiSelectableFlags_NoPadWithHalfSpacing,oneChar);
        if (ImGui::IsItemClicked()) {
          startSelection(3+j*5,i);
        }
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem)) {
          updateSelection(3+j*5,i);
        }
        ImGui::SameLine(0.0f,0.0f);

        // effect value
        if (p->data[i][j][4]==0) {
          sprintf(id,"..##PF%d_%.2X",i,j);
        } else {
          sprintf(id,"%s##PF%d_%.2X",hexValues[p->data[i][j][4]],i,j);
        }
        ImGui::Selectable(id,selectedEffectVal,ImGuiSelectableFlags_NoPadWithHalfSpacing,twoChars);
        if (ImGui::IsItemClicked()) {
          startSelection(4+j*5,i);
        }
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem)) {
          updateSelection(4+j*5,i);
        }
        ImGui::PopStyleColor();
      }
    }
    for (int i=0; i<=drawStart; i++) {
      ImGui::TableNextRow(0,lineHeight);
    }
    ImGui::EndTable();
  }
  delayedStep=playerStep;
  delayedPat=player.pat;
  ImGui::PopStyleVar();
  if (ImGui::IsWindowFocused()) curWindow=wPattern;
  ImGui::End();
}

#define rangedInput(label,var,tempvar,min,max) \
  int tempvar=var; \
  if (ImGui::InputInt(label,&tempvar,1,16)) { \
    if (tempvar<min) tempvar=min; \
    if (tempvar>max) tempvar=max; \
    var=tempvar; \
  }

#define macroSelector(label,var,intendedUse) \
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
  if (ImGui::Button("New")) { \
    var=newMacro(intendedUse); \
    curmacro=var; \
    macroEditOpen=true; \
  } \
  ImGui::SameLine(); \
  if (ImGui::Button("Go")) { \
    curmacro=var; \
    macroEditOpen=true; \
  } \
  ImGui::PopButtonRepeat(); \
  ImGui::NextColumn(); \
  ImGui::PopID();

// silly constants
const signed char S8_ONE=1;
const signed char S8_FOUR=4;
const int I_ZERO=0;
const int I_U8_MAX=255;
const int ONE=1;
const unsigned short ZERO=0;
const unsigned short SHORT_MAX=65535;
const unsigned char _CHAR_MAX=255;
const unsigned char VOL_MAX=64;
const signed char _SCHAR_MIN=-128;
const signed char _SCHAR_MAX=127;
const unsigned char THIRTY_TWO=32;

#define noteOffsetSelector(var) \
  if (var>=0x80) { \
    unsigned char theVal=var-0x80; \
    ImGui::SetNextItemWidth(120.0f*dpiScale); \
    if (ImGui::InputScalar("##OffSelector_" #var,ImGuiDataType_U8,&theVal,&S8_ONE,&S8_FOUR,noteNames[theVal])) { \
      if (theVal>127) theVal=127; \
      var=theVal+0x80; \
    } \
    ImGui::SameLine(); \
    if (ImGui::Button("Absolute")) { \
      var=0; \
    } \
  } else { \
    signed char theVal=(var>=0x40)?0x40-var:var; \
    ImGui::SetNextItemWidth(120.0f*dpiScale); \
    if (ImGui::InputScalar("##OffSelector_" #var,ImGuiDataType_S8,&theVal,&S8_ONE,&S8_FOUR)) { \
      if (theVal>63) theVal=63; \
      if (theVal<-63) theVal=-63; \
      if (theVal<0) { \
        var=0x40-theVal; \
      } else { \
        var=theVal; \
      } \
    } \
    ImGui::SameLine(); \
    if (ImGui::Button("Relative")) { \
      var=0x80+48; \
    } \
  }

void drawInsEditor() {
  if (!insEditOpen) return;
  if (ImGui::Begin("Instrument Editor",&insEditOpen)) {
    if (ImGui::InputInt("Instrument",&curins)) {
      if (curins<1) curins=1;
      if (curins>255) curins=1;
    }

    ImGui::Separator();

    Instrument* ins=song->ins[curins];

    ImGui::InputText("Name",ins->name,32);
    ImGui::SliderScalar("Volume",ImGuiDataType_U8,&ins->vol,&ZERO,&VOL_MAX);
    ImGui::SliderScalar("Pitch",ImGuiDataType_S8,&ins->pitch,&_SCHAR_MIN,&_SCHAR_MAX);

    ImGui::Text("Center Note");
    ImGui::SameLine();
    if (ImGui::BeginCombo("##CenterNote",noteNames[ins->noteOffset])) {
      for (int i=0; i<128; i++) {
        if (ImGui::Selectable(noteNames[i],i==ins->noteOffset)) {
          ins->noteOffset=i;
        }
      }
      ImGui::EndCombo();
    }

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

    ImGui::SliderScalar("Cutoff",ImGuiDataType_U16,&ins->filterH,&ZERO,&SHORT_MAX);
    ImGui::SliderScalar("Resonance",ImGuiDataType_U8,&ins->res,&ZERO,&_CHAR_MAX);

    bool resetOsc=ins->flags&1;
    bool resetFilter=ins->flags&4;
    bool syncMod=ins->flags&32;

    if (ImGui::Checkbox("Reset on new note",&resetOsc)) {
      ins->flags&=~1;
      ins->flags|=resetOsc;
    }

    if (ImGui::Checkbox("Reset filter on new note",&resetFilter)) {
      ins->flags&=~4;
      ins->flags|=resetFilter<<2;
    }

    if (ImGui::Checkbox("Sync modulation",&syncMod)) {
      ins->flags&=~32;
      ins->flags|=syncMod<<5;
    }
    ImGui::SameLine();
    noteOffsetSelector(ins->LFO);

    bool pcmOn=ins->filterMode&8;
    ImGui::BeginChild("PCM",pcmOn?ImVec2(0,120.0f*dpiScale):ImVec2(0,24.0f*dpiScale),true,ImGuiWindowFlags_MenuBar);
    ImGui::BeginMenuBar();
    if (ImGui::Checkbox("PCM",&pcmOn)) {
      ins->filterMode&=~8;
      ins->filterMode|=pcmOn<<3;
    }
    ImGui::EndMenuBar();

    if (pcmOn) {
      ImGui::InputScalar("Position",ImGuiDataType_U16,&ins->pcmPos);
      ImGui::InputScalar("Length",ImGuiDataType_U16,&ins->pcmLen);
      ImGui::InputScalar("Loop",ImGuiDataType_U16,&ins->pcmLoop);
      ImGui::SameLine();
      bool pcmLoopOn=ins->pcmMult&128;
      if (ImGui::Checkbox("##LoopOn",&pcmLoopOn)) {
        ins->pcmMult&=~128;
        ins->pcmMult|=pcmLoopOn<<7;
      }
    }
    if (ImGui::IsWindowFocused()) curWindow=wInstrument;
    ImGui::EndChild();

    ImGui::BeginChild("Macros",ImVec2(0,374.0f*dpiScale),true,ImGuiWindowFlags_MenuBar);
    ImGui::BeginMenuBar();
    ImGui::Text("Macros");
    ImGui::EndMenuBar();
    ImGui::Columns(3);
    macroSelector("Volume",ins->volMacro,iuGeneric);
    macroSelector("Cutoff",ins->cutMacro,iuGeneric);
    macroSelector("Resonance",ins->resMacro,iuGeneric);
    macroSelector("Duty",ins->dutyMacro,iuGeneric);
    macroSelector("Shape",ins->shapeMacro,iuShape);
    macroSelector("Pitch",ins->pitchMacro,iuPitch);
    macroSelector("FinePitch",ins->finePitchMacro,iuPan);
    macroSelector("Panning",ins->panMacro,iuPan);
    macroSelector("VolSweep",ins->volSweepMacro,iuVolSweep);
    macroSelector("FreqSweep",ins->freqSweepMacro,iuOtherSweep);
    macroSelector("CutSweep",ins->cutSweepMacro,iuOtherSweep);
    macroSelector("PCM Position",ins->pcmPosMacro,iuGeneric);
    if (ImGui::IsWindowFocused()) curWindow=wInstrument;
    ImGui::EndChild();

    if (ImGui::IsWindowFocused()) curWindow=wInstrument;
  }
  ImGui::End();
}

const char* intendedUseNames[iuMax]={
  "Generic",
  "Shape",
  "Pitch",
  "FinePitch/Panning",
  "VolSweep",
  "Freq/CutSweep"
};

const char* shapeNames[8]={
  "Pulse",
  "Saw",
  "Sine",
  "Triangle",
  "Noise",
  "ShNoise",
  "ElPiano",
  "PulseSaw"
};

void drawMacroEditor() {
  if (!macroEditOpen) return;
  ImGui::Begin("Macro Editor",&macroEditOpen);

  if (ImGui::InputInt("Macro",&curmacro)) {
    if (curmacro<-1) curmacro=-1;
    if (curmacro>=(int)song->macros.size()) curmacro=song->macros.size()-1;
  }

  if (curmacro<0) {
    ImGui::Text("no macro selected");
  } else {
    Macro* m=song->macros[curmacro];
    bool doJump=m->jumpRelease!=-1;
    if (ImGui::Checkbox("Jump on release",&doJump)) {
      if (doJump) {
        m->jumpRelease=0;
      } else {
        m->jumpRelease=-1;
      }
    }
    if (doJump) {
      ImGui::SameLine();
      if (ImGui::InputInt("Position",&m->jumpRelease)) {
        if (m->jumpRelease<0) m->jumpRelease=0;
        if (m->jumpRelease>(int)m->cmds.size()) m->jumpRelease=(int)m->cmds.size();
      }
    }
    unsigned char ZERO=0;
    unsigned char FIVE=5;
    unsigned char SEVEN=7;
    ImGui::SliderScalar("Type",ImGuiDataType_U8,&m->intendedUse,&ZERO,&FIVE,intendedUseNames[m->intendedUse]);

    ImGui::Checkbox("View as graph",&macroGraph);

    if (macroGraph) {
      ImGui::Text("Not done yet.");
    } else {
      if (ImGui::BeginTable("Macro",5,ImGuiTableFlags_BordersInnerV)) {
        ImGui::TableSetupColumn("pos",ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("type",ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("value",ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("endFrame",ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("pos");
        ImGui::TableNextColumn();
        ImGui::Text("type");
        ImGui::TableNextColumn();
        ImGui::Text("value");
        ImGui::TableNextColumn();
        ImGui::Text("tick");
        ImGui::TableNextColumn();
        ImGui::Text("del");

        for (size_t i=0; i<m->cmds.size(); i++) {
          ImGui::PushID(i);
          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::Text("%ld",i);
          ImGui::TableNextColumn();
          unsigned char cType=m->cmds[i].type&0x7f;
          ImGui::PushButtonRepeat(true);
          if (ImGui::ArrowButton("##down",ImGuiDir_Down)) {
            if ((m->cmds[i].type&0x7f)>0) m->cmds[i].type--;
          }
          ImGui::SameLine();
          if (ImGui::ArrowButton("##up",ImGuiDir_Up)) {
            if ((m->cmds[i].type&0x7f)<(cmdMax-1)) m->cmds[i].type++;
          }
          ImGui::PopButtonRepeat();
          ImGui::SameLine();
          switch (cType) {
            case cmdEnd:
              ImGui::Text("End");
              break;
            case cmdSet:
              ImGui::Text("Set");
              break;
            case cmdWait:
              ImGui::Text("Wait");
              break;
            case cmdWaitRel:
              ImGui::Text("Wait release");
              break;
            case cmdLoop:
              ImGui::Text("Jump");
              break;
            case cmdLoopRel:
              ImGui::Text("Jump until rel.");
              break;
            case cmdAdd:
              ImGui::Text("Add");
              break;
            case cmdSub:
              ImGui::Text("Subtract");
              break;
            default:
              ImGui::Text("???");
              break;
          }
          ImGui::TableNextColumn();
          if (cType==cmdSet) {
            switch (m->intendedUse) {
              case iuGeneric:
                ImGui::SliderScalar("##Value",ImGuiDataType_U32,&m->cmds[i].value,&I_ZERO,&I_U8_MAX);
                break;
              case iuShape: {
                unsigned char rValue=m->cmds[i].value&7;
                if (ImGui::SliderScalar("##Value",ImGuiDataType_U8,&rValue,&ZERO,&SEVEN,shapeNames[rValue])) {
                  m->cmds[i].value=rValue;
                }
                break;
              }
              case iuPitch:
                noteOffsetSelector(m->cmds[i].value);
                break;
              case iuPan:
                ImGui::InputScalar("##Value",ImGuiDataType_S8,&m->cmds[i].value);
                break;
              case iuVolSweep: {
                ImGui::InputScalar("Period",ImGuiDataType_U16,&m->cmds[i].value);
                unsigned char rValue=((unsigned char*)&m->cmds[i].value)[2]&31;
                bool dir=((unsigned char*)&m->cmds[i].value)[2]&32;
                bool loop=((unsigned char*)&m->cmds[i].value)[2]&64;
                bool loopi=((unsigned char*)&m->cmds[i].value)[2]&128;
                if (ImGui::InputScalar("Amount",ImGuiDataType_U8,&rValue)) {
                  if (rValue>31) {
                    rValue=31;
                  }
                  ((unsigned char*)&m->cmds[i].value)[2]&=~0x1f;
                  ((unsigned char*)&m->cmds[i].value)[2]|=rValue;
                }
                ImGui::InputScalar("Bound",ImGuiDataType_U8,((unsigned char*)&m->cmds[i].value)+3);
                if (ImGui::Checkbox("Up",&dir)) {
                  ((unsigned char*)&m->cmds[i].value)[2]&=~0x20;
                  ((unsigned char*)&m->cmds[i].value)[2]|=dir<<5;
                }
                ImGui::SameLine();
                if (ImGui::Checkbox("Loop",&loop)) {
                  ((unsigned char*)&m->cmds[i].value)[2]&=~0x40;
                  ((unsigned char*)&m->cmds[i].value)[2]|=loop<<6;
                }
                ImGui::SameLine();
                if (ImGui::Checkbox("Flip",&loopi)) {
                  ((unsigned char*)&m->cmds[i].value)[2]&=~0x80;
                  ((unsigned char*)&m->cmds[i].value)[2]|=loopi<<7;
                }
                ImGui::SameLine();
                break;
              }
              case iuOtherSweep: {
                ImGui::InputScalar("Period",ImGuiDataType_U16,&m->cmds[i].value);
                unsigned char rValue=((unsigned char*)&m->cmds[i].value)[2]&127;
                bool dir=((unsigned char*)&m->cmds[i].value)[2]&128;
                if (ImGui::InputScalar("Amount",ImGuiDataType_U8,&rValue)) {
                  if (rValue>127) {
                    rValue=127;
                  }
                  ((unsigned char*)&m->cmds[i].value)[2]&=~0x7f;
                  ((unsigned char*)&m->cmds[i].value)[2]|=rValue;
                }
                ImGui::InputScalar("Bound",ImGuiDataType_U8,((unsigned char*)&m->cmds[i].value)+3);
                if (ImGui::Checkbox("Up",&dir)) {
                  ((unsigned char*)&m->cmds[i].value)[2]&=~0x80;
                  ((unsigned char*)&m->cmds[i].value)[2]|=dir<<7;
                }
                break;
              }
              default:
                ImGui::Text("???");
                break;
            }
          } else if (cType!=cmdEnd && cType!=cmdWaitRel) {
            ImGui::InputScalar("##Value",ImGuiDataType_U32,&m->cmds[i].value);
          }
          ImGui::TableNextColumn();
          bool endTick=m->cmds[i].type&0x80;
          if (ImGui::Checkbox("##EndTick",&endTick)) {
            if (endTick) {
              m->cmds[i].type|=0x80;
            } else {
              m->cmds[i].type&=0x7f;
            }
          }
          ImGui::TableNextColumn();
          if (ImGui::Button("-")) {
            m->cmds.erase(m->cmds.begin()+i);
            i--;
          }
          ImGui::PopID();
        }
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("-end-");
        ImGui::TableNextColumn();
        if (ImGui::Button("+##AddOne")) {
          m->cmds.push_back(MacroCommand(cmdSet,0,true));
        }
        ImGui::EndTable();
      }
    }
  }

  if (ImGui::IsWindowFocused()) curWindow=wMacro;

  ImGui::End();
}

void drawMemoryView() {
  if (!memViewOpen) return;
  if (ImGui::Begin("Memory View",&memViewOpen)) {
    if (ImGui::SliderInt("Soundchip",&mvChip,0,3)) {
      if (mvChip<0) mvChip=0;
      if (mvChip>3) mvChip=3;
    }
    if (ImGui::BeginTable("Memory",17)) {    
      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      for (int i=0; i<16; i++) {
        ImGui::TableNextColumn();
        ImGui::Text(" %X",i);
      }
      for (int i=0; i<16; i++) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("%X0",i);
        for (int j=0; j<16; j++) {
          ImGui::TableNextColumn();
          ImGui::Text("%.2x",((unsigned char*)chip[mvChip].chan)[j+(i<<4)]);
          if (ImGui::IsItemHovered()) ImGui::SetTooltip("$%.2x: %s%d",j+(i<<4),chanValueNames[(j+(i<<4))&0x1f],i>>1);
        }
      }
      ImGui::EndTable();
    }
  }
  ImGui::End();
}

void drawSongInfo() {
  if (!songEditOpen) return;
  if (ImGui::Begin("Song Information",&songEditOpen)) {
    if (ImGui::InputText("Name",song->name,32)) {
      updateWindowTitle();
    }
    if (ImGui::InputScalar("Speed",ImGuiDataType_U8,&song->speed,&ONE,&ONE)) {
      if (song->speed<1) song->speed=1;
      player.speed=song->speed;
    }
    if (ImGui::InputScalar("Tempo",ImGuiDataType_U8,&song->tempo,&ONE,&ONE,(song->tempo==0)?"VSync":"%d")) {
      if (song->tempo==1) song->tempo=125;
      if (song->tempo<32) song->tempo=0;
      if (song->tempo==0) {
        if (ntsc) {
          player.tempo=150;
        } else {
          player.tempo=125;
        }
      } else {
        player.tempo=song->tempo;
      }
    }
    if (ImGui::InputScalar("Length",ImGuiDataType_U8,&song->orders,&ONE,&ONE)) {
    }
    ImGui::SliderScalar("Detune",ImGuiDataType_S8,&song->detune,&_SCHAR_MIN,&_SCHAR_MAX);
    if (ImGui::SliderScalar("Channels",ImGuiDataType_U8,&song->channels,&ONE,&THIRTY_TWO)) {
      if (song->channels>32) song->channels=32;
      if (song->channels<1) song->channels=1;
      if (player.playMode==1) player.play();
    }
    bool compNoise=song->flags&4;
    if (ImGui::Checkbox("compatible noise mode",&compNoise)) {
      song->flags&=~4;
      song->flags|=compNoise<<2;
    }
    if (origin!="") {
      ImGui::Text("last saved with %s",origin.c_str());
    } else {
      ImGui::Text("not saved");
    }
  }
  ImGui::End();
}

void prepareUndo() {
  Pattern* p=song->getPattern(song->order[player.pat],true);
  memcpy(oldPat,p->data,p->length*32*8);
}

bool makeUndo(UndoAction action) {
  UndoStep d;
  d.action=action;
  d.pattern=song->order[player.pat];
  Pattern* p=song->getPattern(song->order[player.pat],true);
  for (int i=0; i<p->length; i++) {
    for (int j=0; j<song->channels; j++) {
      for (int k=0; k<5; k++) {
        if (p->data[i][j][k]!=oldPat[i][j][k]) {
          d.data.push_back(UndoData(i,j*5+k,oldPat[i][j][k],p->data[i][j][k]));
        }
      }
    }
  }
  if (!d.data.empty()) {
    undoHist.push_back(d);
    redoHist.clear();
    if (undoHist.size()>100) undoHist.pop_front();
    return true;
  }
  return false;
}

void doUndo() {
  if (undoHist.empty()) {
    return;
  }
  UndoStep& us=undoHist.back();
  redoHist.push_back(us);

  switch (us.action) {
    case undoPatternNote: case undoPatternDelete: case undoPatternInsert: case undoPatternPullDelete:
    case undoPatternCut: case undoPatternPaste:
      Pattern* p=song->getPattern(us.pattern,true);
      for (UndoData& i: us.data) {
        p->data[i.row][i.column/5][i.column%5]=i.oldData;
      }
      break;
  }

  undoHist.pop_back();
}

void doRedo() {
  if (redoHist.empty()) {
    return;
  }
  UndoStep& us=redoHist.back();
  undoHist.push_back(us);

  switch (us.action) {
    case undoPatternNote: case undoPatternDelete: case undoPatternInsert: case undoPatternPullDelete:
    case undoPatternCut: case undoPatternPaste:
      Pattern* p=song->getPattern(us.pattern,true);
      for (UndoData& i: us.data) {
        p->data[i.row][i.column/5][i.column%5]=i.newData;
      }
      break;
  }

  redoHist.pop_back();
}

void doNoteInput(SDL_Event& ev) {
  Pattern* p=song->getPattern(song->order[player.pat],true);
  int type=selStart.x%5;
  int channel=selStart.x/5;
  if (selStart.x<0 || selStart.y<0 || channel>=song->channels || selStart.y>=p->length) return;
  prepareUndo();
  switch (type) {
    case 0: // note
      try {
        int note=noteKeys.at(ev.key.keysym.sym);
        if (note>250) {
          p->data[selStart.y][channel][0]=note&15;
        } else {
          p->data[selStart.y][channel][0]=hscale(curoctave*12+note);
          p->data[selStart.y][channel][1]=curins;
        }

        selStart.y++;
        if (selStart.y>=p->length) selStart.y=p->length-1;
        selEnd=selStart;
        updateScroll(selStart.y);
      } catch (std::out_of_range& e) {
      }
      break;
    case 1: case 4: // instrument or effect value
      try {
        int value=valueKeys.at(ev.key.keysym.sym);
        p->data[selStart.y][channel][type]=(p->data[selStart.y][channel][type]<<4)|value;
      } catch (std::out_of_range& e) {
      }
      break;
    case 2: // volume
      try {
        int value=valueKeys.at(ev.key.keysym.sym);
        if (p->data[selStart.y][channel][2]==0) p->data[selStart.y][channel][2]=0x40;
        if ((p->data[selStart.y][channel][2]&0xc0)==0x40 || (p->data[selStart.y][channel][2]&0xc0)==0x80) {
          unsigned char newVal=((p->data[selStart.y][channel][2]&0x3f)<<4)|value;
          if (newVal>0x3f) {
            p->data[selStart.y][channel][2]=(p->data[selStart.y][channel][2]&0xc0)|0x30|value;
          } else {
            p->data[selStart.y][channel][2]=(p->data[selStart.y][channel][2]&0xc0)|newVal;
          }
        } else {
          p->data[selStart.y][channel][2]=(p->data[selStart.y][channel][2]&0xf0)|value;
        }
      } catch (std::out_of_range& e) {
      }
      break;
    case 3: // effect
      try {
        int value=effectKeys.at(ev.key.keysym.sym);
        p->data[selStart.y][channel][3]=value;
      } catch (std::out_of_range& e) {
      }
      break;
  }
  makeUndo(undoPatternNote);
}

void doDelete() {
  finishSelection();
  prepareUndo();
  Pattern* p=song->getPattern(song->order[player.pat],true);
  for (int i=selStart.x; i<=selEnd.x; i++) {
    if (i<0 || (i/5)>=song->channels) continue;
    for (int j=selStart.y; j<=selEnd.y; j++) {
      if (j<0 || j>=p->length) continue;
      p->data[j][i/5][i%5]=0;
    }
  }
  makeUndo(undoPatternDelete);
}

void doPullDelete() {
  finishSelection();
  prepareUndo();
  Pattern* p=song->getPattern(song->order[player.pat],true);
  int distance=selEnd.y-selStart.y+1;
  for (int i=selStart.x; i<=selEnd.x; i++) {
    for (int j=selStart.y; j<p->length; j++) {
      if (j+distance>=p->length) {
        p->data[j][i/5][i%5]=0;
      } else {
        p->data[j][i/5][i%5]=p->data[j+distance][i/5][i%5];
      }
    }
  }
  makeUndo(undoPatternPullDelete);
}

void doInsert() {
  finishSelection();
  prepareUndo();
  Pattern* p=song->getPattern(song->order[player.pat],true);
  for (int i=selStart.x; i<=selEnd.x; i++) {
    for (int j=p->length-1; j>=selStart.y; j--) {
      p->data[j][i/5][i%5]=p->data[j-1][i/5][i%5];
    }
  }
  makeUndo(undoPatternInsert);
}

void doCopy() {
  finishSelection();
  string data=fmt::sprintf("org.tildearrow.soundtracker - Pattern Data (%d)\n%d",TRACKER_VER,selStart.x%5);

  Pattern* p=song->getPattern(song->order[player.pat],true);
  for (int j=selStart.y; j<=selEnd.y; j++) {
    if (j<0 || j>=p->length) continue;
    data+='\n';
    for (int i=selStart.x; i<=selEnd.x; i++) {
      if (i<0 || (i/5)>=song->channels) continue;
      data+=fmt::sprintf("%.2X",p->data[j][i/5][i%5]);
    }
  }

  SDL_SetClipboardText(data.c_str());
}

void doCut() {
  doCopy();
  prepareUndo();
  Pattern* p=song->getPattern(song->order[player.pat],true);
  for (int i=selStart.x; i<=selEnd.x; i++) {
    if (i<0 || (i/5)>=song->channels) continue;
    for (int j=selStart.y; j<=selEnd.y; j++) {
      if (j<0 || j>=p->length) continue;
      p->data[j][i/5][i%5]=0;
    }
  }
  makeUndo(undoPatternCut);
}

void doPaste() {
  finishSelection();
  char* dataPtr=SDL_GetClipboardText();
  std::vector<string> data;
  string tempS;
  for (char* i=dataPtr; *i; i++) {
    if (*i=='\r') continue;
    if (*i=='\n') {
      data.push_back(tempS);
      tempS="";
      continue;
    }
    tempS+=*i;
  }
  data.push_back(tempS);
  SDL_free(dataPtr);
  int startOff=-1;
  bool invalidData=false;

  if (data.size()<2) return;

  if (data[0]!=fmt::sprintf("org.tildearrow.soundtracker - Pattern Data (%d)",TRACKER_VER)) return;
  if (sscanf(data[1].c_str(),"%d",&startOff)!=1) return;
  if (startOff<0 || startOff>4) return;

  prepareUndo();
  Pattern* p=song->getPattern(song->order[player.pat],true);
  selStart.x=(5*(selStart.x/5))+startOff;
  if (selEnd.x<selStart.x) selEnd.x=selStart.x;
  for (size_t i=2; i<data.size(); i++) {
    unsigned char nextByte=0;
    bool pushByte=false;
    std::vector<unsigned char> udata;
    udata.clear();
    int row=selStart.y+i-2;
    if (row<0 || row>=p->length) continue;

    for (char j: data[i]) {
      if (j>='0' && j<='9') {
        nextByte=(nextByte<<4)|(j-'0');
      } else if (j>='A' && j<='F') {
        nextByte=(nextByte<<4)|(j-'A'+10);
      } else {
        invalidData=true;
        break;
      }
      if (pushByte) {
        udata.push_back(nextByte);
        nextByte=0;
      }
      pushByte=!pushByte;
    }
    if (invalidData) break;
    for (size_t j=0; j<udata.size(); j++) {
      int col=j+selStart.x;
      if (col<0 || col>=song->channels*5) continue;

      p->data[row][col/5][col%5]=udata[j];
    }
  }
  makeUndo(undoPatternPaste);
}

void doSelectAll() {
  finishSelection();
  Pattern* p=song->getPattern(song->order[player.pat],true);
  if ((selStart.x%5)==0 && (selEnd.x%5)==4) {
    if (selStart.y==0 && selEnd.y==p->length-1) { // select entire pattern
      selStart.x=0;
      selEnd.x=(song->channels*5)-1;
    } else { // select entire column
      selStart.y=0;
      selEnd.y=p->length-1;
    }
  } else {
    float aspect=float(selEnd.x-selStart.x+1)/float(selEnd.y-selStart.y+1);
    if (aspect<1.0f && !(selStart.y==0 && selEnd.y==p->length-1)) { // up-down
      selStart.y=0;
      selEnd.y=p->length-1;
    } else { // left-right
      selStart.x=5*(selStart.x/5);
      selEnd.x=5*(selEnd.x/5)+4;
    }
  }
}

void keyDown(SDL_Event& ev) {
  switch (curWindow) {
    case wPattern: {
      if (ev.key.keysym.mod&KMOD_CTRL) {
        switch (ev.key.keysym.sym) {
          case SDLK_z:
            if (ev.key.keysym.mod&KMOD_SHIFT) {
              doRedo();
            } else {
              doUndo();
            }
            break;
          case SDLK_x:
            doCut();
            break;
          case SDLK_c:
            doCopy();
            break;
          case SDLK_v:
            doPaste();
            break;
          case SDLK_a:
            doSelectAll();
            break;
        }
      } else switch (ev.key.keysym.sym) {
        case SDLK_BACKSPACE:
          doPullDelete();
          break;
        case SDLK_INSERT:
          doInsert();
          break;
        case SDLK_DELETE:
          doDelete();
          break;
        case SDLK_UP:
          if (--selStart.y<0) selStart.y=0;
          selEnd=selStart;
          updateScroll(selStart.y);
          break;
        case SDLK_DOWN: {
          Pattern* p=song->getPattern(song->order[player.pat],true);
          if (++selStart.y>=p->length) selStart.y=p->length-1;
          selEnd=selStart;
          updateScroll(selStart.y);
          break;
        }
        case SDLK_LEFT:
          if (--selStart.x<0) selStart.x=0;
          selEnd=selStart;
          break;
        case SDLK_RIGHT:
          if (++selStart.x>=5*song->channels) selStart.x=5*song->channels-1;
          selEnd=selStart;
          break;
        case SDLK_HOME:
          selStart.y=0;
          selEnd=selStart;
          updateScroll(selStart.y);
          break;
        case SDLK_END: {
          Pattern* p=song->getPattern(song->order[player.pat],true);
          selStart.y=p->length-1;
          selEnd=selStart;
          updateScroll(selStart.y);
          break;
        }
        case SDLK_PAGEUP:
          selStart.y-=16;
          if (selStart.y<0) selStart.y=0;
          selEnd=selStart;
          updateScroll(selStart.y);
          break;
        case SDLK_PAGEDOWN: {
          Pattern* p=song->getPattern(song->order[player.pat],true);
          selStart.y+=16;
          if (selStart.y>=p->length) selStart.y=p->length-1;
          selEnd=selStart;
          updateScroll(selStart.y);
          break;
        }
        default:
          doNoteInput(ev);
          break;
      }
      break;
    }
    case wInstrument:
      try {
        int note=1+noteKeys.at(ev.key.keysym.sym);
        if (note<250) {
          player.testNoteOn(0,curins,curoctave*12+note);
        }
      } catch (std::out_of_range& e) {
      }
      break;
  }
}

void keyUp(SDL_Event& ev) {

}

bool updateDisp() {
  SDL_Event ev;
  while (SDL_PollEvent(&ev)) {
    ImGui_ImplSDL2_ProcessEvent(&ev);
    switch (ev.type) {
      case SDL_KEYDOWN:
        if (!ImGui::GetIO().WantCaptureKeyboard) {
          keyDown(ev);
        }
        break;
      case SDL_KEYUP:
        if (!ImGui::GetIO().WantCaptureKeyboard) {
          keyUp(ev);
        }
        break;
      case SDL_MOUSEBUTTONUP:
        if (ev.button.button==1) {
          finishSelection();
        }
        break;
      case SDL_WINDOWEVENT:
        switch (ev.window.event) {
          case SDL_WINDOWEVENT_RESIZED:
            scrW=ev.window.data1/dpiScale;
            scrH=ev.window.data2/dpiScale;
            break;
        }
        break;
      case SDL_QUIT:
        quit=true;
        return false;
        break;
    }
  }
  curWindow=wPattern;
  ImGui_ImplSDLRenderer_NewFrame();
  ImGui_ImplSDL2_NewFrame(sdlWin);
  ImGui::NewFrame();

  ImGui::BeginMainMenuBar();
  if (ImGui::BeginMenu("file")) {
    if (ImGui::MenuItem("new")) {
      CleanupPatterns();
    }
    if (ImGui::MenuItem("open...")) {
      ImGuiFileDialog::Instance()->OpenDialog("FileDialog","Open File",".*",curdir);
      isSaving=false;
    }
    ImGui::Separator();
    if (ImGui::MenuItem("save")) {
      if (curfname=="") {
        ImGuiFileDialog::Instance()->OpenDialog("FileDialog","Save File",".*",curdir,1,NULL,ImGuiFileDialogFlags_ConfirmOverwrite);
        isSaving=true;
      } else {
        string copyofName=curfname;
        SaveFile(copyofName.c_str());
      }
    }
    if (ImGui::MenuItem("save as...")) {
      ImGuiFileDialog::Instance()->OpenDialog("FileDialog","Save File",".*",curdir,1,NULL,ImGuiFileDialogFlags_ConfirmOverwrite);
      isSaving=true;
    }
    ImGui::Separator();
    if (ImGui::MenuItem("exit")) {
      quit=true;
    }
    ImGui::EndMenu();
  }
  if (ImGui::BeginMenu("edit")) {
    if (ImGui::MenuItem("undo",NULL,false,!undoHist.empty())) {
      doUndo();
    }
    if (ImGui::MenuItem("redo",NULL,false,!redoHist.empty())) {
      doRedo();
    }
    ImGui::Separator();
    if (ImGui::MenuItem("cut")) {
      doCut();
    }
    if (ImGui::MenuItem("copy")) {
      doCopy();
    }
    if (ImGui::MenuItem("paste")) {
      doPaste();
    }
    if (ImGui::MenuItem("delete")) {
      doDelete();
    }
    if (ImGui::MenuItem("select all")) {
      doSelectAll();
    }
    ImGui::Separator();
    ImGui::MenuItem("clear...");
    ImGui::EndMenu();
  }
  if (ImGui::BeginMenu("window")) {
    if (ImGui::MenuItem("instrument editor")) {
      insEditOpen=!insEditOpen;
    }
    if (ImGui::MenuItem("macro editor")) {
      macroEditOpen=!macroEditOpen;
    }
    if (ImGui::MenuItem("song info")) {
      songEditOpen=!songEditOpen;
    }
    if (ImGui::MenuItem("memory view")) {
      memViewOpen=!memViewOpen;
    }
    ImGui::EndMenu();
  }
  if (ImGui::BeginMenu("help")) {
    ImGui::MenuItem("panic");
    ImGui::Separator();
    ImGui::MenuItem("about...");
    ImGui::EndMenu();
  }
  ImGui::EndMainMenuBar();

  ImGui::SetNextWindowSizeConstraints(ImVec2(scrW*dpiScale,32*dpiScale),ImVec2(scrW*dpiScale,160*dpiScale));
  ImGui::Begin("Controls",NULL,ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoBringToFrontOnFocus);
  ImGui::SetWindowPos("Controls",ImVec2(0.0f,ImGui::GetTextLineHeight()+ImGui::GetStyle().ItemSpacing.y*2));

  ImGui::Columns(4);
  
  ImGui::BeginChild("EditControls",ImVec2(0,0),true);

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
    if (player.pat>song->orders) player.pat=0;
    if (player.playMode==1) Play();
  }
  ImGui::NextColumn();
  ImGui::InputScalar("pattern",ImGuiDataType_U8,&(song->order[player.pat]),&ONE,&ONE);
  ImGui::NextColumn();
  if (ImGui::InputScalar("octave",ImGuiDataType_U8,&curoctave,&ONE,&ONE)) {
    if (curoctave>9) curoctave=9;
  }
  ImGui::NextColumn();
  if (ImGui::InputScalar("length",ImGuiDataType_U16,&song->getPattern(player.pat,true)->length,&ONE,&ONE)) {
    if (song->getPattern(player.pat,true)->length>256) song->getPattern(player.pat,true)->length=256;
    if (song->getPattern(player.pat,true)->length<1) song->getPattern(player.pat,true)->length=1;
  }
  ImGui::Columns(1);

  ImGui::Separator();

  if (ImGui::BeginCombo("instr",song->ins[curins]->name)) {
    for (int i=1; i<255; i++) {
      string insName=fmt::sprintf("[%d] %s",i,song->ins[i]->name);
      if (ImGui::Selectable(insName.c_str(),curins==i)) {
        curins=i;
      }
    }
    ImGui::EndCombo();
  }
  ImGui::SameLine(0.0f,4*dpiScale);
  if (ImGui::Button("edit")) {
    insEditOpen=true;
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
      player.ntsc=ntsc;
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

  float multAmp=amp*100.0f;
  if (ImGui::SliderFloat("Volume",&multAmp,0.0f,200.0f,"%.0f%%")) {
    if (multAmp>200.0f) multAmp=200.0f;
    if (multAmp<0.0f) multAmp=0.0f;
    amp=multAmp/100.0f;
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

  drawMacroEditor();

  drawMemoryView();

  drawSongInfo();

  if (ImGuiFileDialog::Instance()->Display("FileDialog")) {
    if (ImGuiFileDialog::Instance()->IsOk()) {
      curfname=ImGuiFileDialog::Instance()->GetFilePathName();
      if (curfname!="") {
        string copyOfName=curfname;
        if (isSaving) {
          printf("saving: %s\n",copyOfName.c_str());
          SaveFile(copyOfName.c_str());
          isSaving=false;
        } else {
          LoadFile(copyOfName.c_str());
        }
      }
    }
    strcpy(curdir,ImGuiFileDialog::Instance()->GetCurrentPath().c_str());
    strcat(curdir,"/");
    ImGuiFileDialog::Instance()->Close();
  }

  // end of frame
  ImGui::Render();
  ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
  SDL_RenderPresent(sdlRend);
  SDL_RenderClear(sdlRend);

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

  // octave 1
  noteKeys[SDLK_z]=0;
  noteKeys[SDLK_s]=1;
  noteKeys[SDLK_x]=2;
  noteKeys[SDLK_d]=3;
  noteKeys[SDLK_c]=4;
  noteKeys[SDLK_v]=5;
  noteKeys[SDLK_g]=6;
  noteKeys[SDLK_b]=7;
  noteKeys[SDLK_h]=8;
  noteKeys[SDLK_n]=9;
  noteKeys[SDLK_j]=10;
  noteKeys[SDLK_m]=11;

  // octave 2
  noteKeys[SDLK_q]=12;
  noteKeys[SDLK_2]=13;
  noteKeys[SDLK_w]=14;
  noteKeys[SDLK_3]=15;
  noteKeys[SDLK_e]=16;
  noteKeys[SDLK_r]=17;
  noteKeys[SDLK_5]=18;
  noteKeys[SDLK_t]=19;
  noteKeys[SDLK_6]=20;
  noteKeys[SDLK_y]=21;
  noteKeys[SDLK_7]=22;
  noteKeys[SDLK_u]=23;

  // octave 3
  noteKeys[SDLK_i]=24;
  noteKeys[SDLK_9]=25;
  noteKeys[SDLK_o]=26;
  noteKeys[SDLK_0]=27;
  noteKeys[SDLK_p]=28;
  noteKeys[SDLK_LEFTBRACKET]=29;
  noteKeys[SDLK_RIGHTBRACKET]=31;

  // special
  noteKeys[SDLK_EQUALS]=253;
  noteKeys[SDLK_1]=254;

  // value keys
  valueKeys[SDLK_0]=0;
  valueKeys[SDLK_1]=1;
  valueKeys[SDLK_2]=2;
  valueKeys[SDLK_3]=3;
  valueKeys[SDLK_4]=4;
  valueKeys[SDLK_5]=5;
  valueKeys[SDLK_6]=6;
  valueKeys[SDLK_7]=7;
  valueKeys[SDLK_8]=8;
  valueKeys[SDLK_9]=9;
  valueKeys[SDLK_a]=10;
  valueKeys[SDLK_b]=11;
  valueKeys[SDLK_c]=12;
  valueKeys[SDLK_d]=13;
  valueKeys[SDLK_e]=14;
  valueKeys[SDLK_f]=15;
  
  // effect keys
  effectKeys[SDLK_a]=1;
  effectKeys[SDLK_b]=2;
  effectKeys[SDLK_c]=3;
  effectKeys[SDLK_d]=4;
  effectKeys[SDLK_e]=5;
  effectKeys[SDLK_f]=6;
  effectKeys[SDLK_g]=7;
  effectKeys[SDLK_h]=8;
  effectKeys[SDLK_i]=9;
  effectKeys[SDLK_j]=10;
  effectKeys[SDLK_k]=11;
  effectKeys[SDLK_l]=12;
  effectKeys[SDLK_m]=13;
  effectKeys[SDLK_n]=14;
  effectKeys[SDLK_o]=15;
  effectKeys[SDLK_p]=16;
  effectKeys[SDLK_q]=17;
  effectKeys[SDLK_r]=18;
  effectKeys[SDLK_s]=19;
  effectKeys[SDLK_t]=20;
  effectKeys[SDLK_u]=21;
  effectKeys[SDLK_v]=22;
  effectKeys[SDLK_w]=23;
  effectKeys[SDLK_x]=24;
  effectKeys[SDLK_y]=25;
  effectKeys[SDLK_z]=26;


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
  strcat(curdir,"/");
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
        printf("playing: %s\n",song->name);
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
    while (!quit) {
      SDL_Event ev;
      SDL_WaitEvent(&ev);
      switch (ev.type) {
        case SDL_QUIT:
          quit=true;
          break;
      }
    }
  } else {
    SDL_RenderClear(sdlRend);
#ifdef HAVE_GUI
    while (!quit) updateDisp();
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
