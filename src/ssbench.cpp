// specs2 soundchip sequence interpreter... benchmark!
#include "soundchip.h"
#include "ssinter.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <unistd.h>

bool quit;

soundchip sc;

double targetSR;

double noProc;

double procPos;
int ticks, speed;

FILE* f;
int frame;

SSInter s;

size_t fsize;

float resa0[2], resa1[2];

#define resaf 0.33631372025095791864295318996109

std::string str;

int main(int argc, char** argv) {
  int which;
  sc.Init();
  procPos=0;
  frame=0;
  ticks=0;
  resa0[0]=0; resa0[1]=0;
  resa1[0]=0; resa1[1]=0;

  quit=false;
  
  if (argc<2) {
    printf("usage: %s [-n] file\n",argv[0]);
    return 1;
  }
  targetSR=297500; // PAL.
  speed=119000; // PAL.
  which=1;

  s.init(&sc);

  if (strcmp(argv[1],"-n")==0) {
    which=2;
    targetSR=309000; // NTSC.
    speed=103103; // NTSC. 103000 for no colorburst compensation
  }
  
  if ((f=fopen(argv[which],"r"))==NULL) {
    printf("not exist\n");
    return 1;
  }

  fseek(f,0,SEEK_END);
  fsize=ftell(f);
  fseek(f,0,SEEK_SET);

  printf("start!\n");
  
  for (int i=0; i<8; i++) {
    sc.chan[i].pan=0;
    sc.chan[i].duty=0x3f;
  }

  float temp[2];
  int wc;
  int writable;
  while (!quit) {
    ticks-=20; // 20 CPU cycles per sound output cycle
    if (ticks<=0) {
      str="";
      while ((wc=fgetc(f))!=EOF) {
        str+=wc;
        if (wc=='R') break;
      }
      if (feof(f)) quit=true;
      writable=0;
      s.next(str.c_str(),writable,str.size());
      ticks+=speed;
      frame++;
    }
    sc.NextSample(&temp[0],&temp[1]);
    resa0[0]=resa0[0]+resaf*(temp[0]-resa0[0]);
    resa1[0]=resa1[0]+resaf*(resa0[0]-resa1[0]);
    resa1[0]=resa1[0]+resaf*(resa0[0]-resa1[0]);
    resa1[0]=resa1[0]+resaf*(resa0[0]-resa1[0]);
    
    resa0[1]=resa0[1]+resaf*(temp[1]-resa0[1]);
    resa1[1]=resa1[1]+resaf*(resa0[1]-resa1[1]);
    resa1[1]=resa1[1]+resaf*(resa0[1]-resa1[1]);
    resa1[1]=resa1[1]+resaf*(resa0[1]-resa1[1]);
  }

  printf("end!\n");
  return 0;
}
