#include "tracker.h"
#include "unifont.h"

extern const unsigned char unifont_bin[2097152];
extern const unsigned char unifont_siz[8192];

#define CHAR_WIDTH(x) ((unifont_siz[x>>3]&(1<<(x&7)))?2:1)

float intens[6]={
  0, 0.37, 0.53, 0.68, 0.84, 1
};

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

Point Graphics::getTPos() {
  return textPos;
}

void Graphics::tPos(float x, float y) {
  textPos.x=x; textPos.y=y;
  //fprintf(stderr,"\x1b[%d;%dH",(int)y+1,(int)x+1);
}

void Graphics::tPos(float y) {
  textPos.x=nlPos; textPos.y=y;
  //fprintf(stderr,"\x1b[%d;%dH",(int)y+1,(int)nlPos+1);
}

void Graphics::tNLPos(float x) {
  nlPos=x;
}

void Graphics::tAlign(float x) {
  align=x;
}

void Graphics::tColor(unsigned char color) {
  textCol.a=1;
  textCol.r=0;
  textCol.g=0;
  textCol.b=0;
  if (color<16) {
    if (color==8) {
      textCol.r=0.5;
      textCol.g=0.5;
      textCol.b=0.5;
    } else {
      if (color<8) {
        textCol.r=(color&1)?0.75:0;
        textCol.g=(color&2)?0.75:0;
        textCol.b=(color&4)?0.75:0;
      } else {
        textCol.r=(color&1)?1:0;
        textCol.g=(color&2)?1:0;
        textCol.b=(color&4)?1:0;
      }
    }
  } else {
    if (color>231) {
      // shade of gray/grey
      textCol.r=(float)(color-232)/24.0;
      textCol.g=(float)(color-232)/24.0;
      textCol.b=(float)(color-232)/24.0;
    } else {
      textCol.r=intens[(color-16)/36];
      textCol.g=intens[((color-16)/6)%6];
      textCol.b=intens[(color-16)%6];
    }
  }
  //fprintf(stderr,"\x1b[38;5;%dm",color);
}

void Graphics::loadPage(int num) {
  ::printf("loading page %d\n",num);
  uCache=SDL_CreateRGBSurfaceWithFormat(0,32*16,32*16,32,SDL_PIXELFORMAT_RGBA32);
  for (int h=0; h<32; h++) {
    for (int i=0; i<32; i++) {
      for (int j=0; j<16; j++) {
        for (int k=0; k<16; k++) {
          ((unsigned char*)uCache->pixels)[(h<<15)+(i<<6)+(j<<11)+(k<<2)]=255;
          ((unsigned char*)uCache->pixels)[(h<<15)+(i<<6)+(j<<11)+(k<<2)+1]=255;
          ((unsigned char*)uCache->pixels)[(h<<15)+(i<<6)+(j<<11)+(k<<2)+2]=255;
          ((unsigned char*)uCache->pixels)[(h<<15)+(i<<6)+(j<<11)+(k<<2)+3]=
            (unifont_bin[(num<<15)+(h<<10)+(i<<5)+(j<<1)+(k>>3)]&(1<<((15-k)&7)))?255:0;
        }
      }
    }
  }
  
  uText[num]=SDL_CreateTextureFromSurface(sdlRend,uCache);
  loadedPage[num]=true;
  
  SDL_FreeSurface(uCache);
}

int decodeUTF8(unsigned char* data, char& len) {
  int ret=0xfffd;
  if (data[0]<0x80) {
    ret=data[0];
    len=1;
  } else if (data[0]<0xc0) {
    ret=0xfffd; // invalid
    len=1;
  } else if (data[0]<0xe0) {
    if (data[1]>=0x80 && data[1]<0xc0) {
      len=2;
      ret=((data[0]&31)<<6)|
          (data[1]&63);
    } else len=1;
  } else if (data[0]<0xf0) {
    if (data[1]>=0x80 && data[1]<0xc0) {
      if (data[2]>=0x80 && data[2]<0xc0) {
        len=3;
        ret=((data[0]&15)<<12)|
            ((data[1]&63)<<6)|
            (data[2]&63);
      } else len=2;
    } else len=1;
  } else if (data[0]<0xf5) {
    if (data[1]>=0x80 && data[1]<0xc0) {
      if (data[2]>=0x80 && data[2]<0xc0) {
        if (data[3]>=0x80 && data[3]<0xc0) {
          len=4;
          ret=((data[0]&7)<<18)|
              ((data[1]&63)<<12)|
              ((data[2]&63)<<6)|
              (data[3]&63);
        } else len=3;
      } else len=2;
    } else len=1;
  } else {
    len=1;
    return 0xfffd;
  }

  if ((ret>=0xd800 && ret<=0xdfff) || ret>=0x110000) return 0xfffd;
  return ret;
}

size_t utf8len(const char* s) {
  return 0;
}

int Graphics::printf(const char* format, ...) {
  va_list va;
  SDL_Rect sr, dr;
  int ret, ch;
  char clen;
  va_start(va,format);
  ret=vsnprintf(putBuf,4095,format,va);
  
  if (align!=0) {
    tPos(textPos.x-(float)ret*align,textPos.y);
  }
  
  clen=0;
  for (int i=0; i<ret; i+=clen) {
    ch=decodeUTF8((unsigned char*)&putBuf[i],clen);
    //fputc(putBuf[i],stderr);
    if (ch=='\n' || ch=='\r') {
      textPos.x=nlPos;
      textPos.y++;
      if (nlPos!=0) {
        //fprintf(stderr,"\x1b[%d;%dH",(int)textPos.y+1,(int)textPos.x+1);
      }
    } else {
      if (ch<256) {
        sr.x=(ch&15)*16;
        sr.y=((ch>>4)&15)*16;
        sr.w=16;
        sr.h=16;
        dr.x=textPos.x*8;
        dr.y=textPos.y*12;
        dr.w=16;
        dr.h=16;
        SDL_SetTextureColorMod(sdlText,textCol.r*255,textCol.g*255,textCol.b*255);
        SDL_SetTextureAlphaMod(sdlText,textCol.a*255);
        SDL_RenderCopy(sdlRend,sdlText,&sr,&dr);
      } else {
        if (!loadedPage[ch>>10]) loadPage(ch>>10);
        
        sr.x=(ch&31)*16;
        sr.y=((ch>>5)&31)*16;
        sr.w=16;
        sr.h=16;
        dr.x=textPos.x*8;
        dr.y=textPos.y*12;
        dr.w=16;
        dr.h=16;
        SDL_SetTextureColorMod(uText[ch>>10],textCol.r*255,textCol.g*255,textCol.b*255);
        SDL_SetTextureAlphaMod(uText[ch>>10],textCol.a*255);
        SDL_RenderCopy(sdlRend,uText[ch>>10],&sr,&dr);
      }
      textPos.x+=CHAR_WIDTH(ch);
    }
  }
  va_end(va);
  return ret;
}

void Graphics::setTarget(SDL_Texture* where) {
  SDL_SetRenderTarget(sdlRend,where);
}

void Graphics::setTitle(string t) {
  SDL_SetWindowTitle(sdlWin,t.c_str());
}

void Graphics::trigResize(int tx, int ty) {
  scrSize.x=tx/dpiScale;
  scrSize.y=ty/dpiScale;
}

Point Graphics::getWSize() {
  return scrSize;
}

bool Graphics::quit() {
  return true; // TODO
}

bool Graphics::preinit() {
  /*
  if (!al_init()) return false;
  
  al_init_font_addon();
  al_init_ttf_addon();
  al_init_primitives_addon();
  al_init_image_addon();
  */
  
  if (SDL_Init(SDL_INIT_EVENTS)==-1) return false;
  //if (TTF_Init()==-1) return false;
  
  return true;
}

bool Graphics::init(int width, int height) {
  tPos(0,0);
  tColor(15);
  
  for (int i=0; i<64; i++) loadedPage[i]=false;

  if (SDL_InitSubSystem(SDL_INIT_VIDEO)==-1) return false;
  
  dpiScale=getScale();

#ifdef ANDROID
  // workaround for https://bugzilla.libsdl.org/show_bug.cgi?id=2291
  SDL_GL_SetAttribute(SDL_GL_RED_SIZE,8);
  SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,8);
  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,8);
#endif
  
  sdlWin=SDL_CreateWindow("soundtracker",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,width*dpiScale,height*dpiScale,SDL_WINDOW_RESIZABLE);
  if (!sdlWin) return false;
  
  sdlRend=SDL_CreateRenderer(sdlWin,-1,SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC|SDL_RENDERER_TARGETTEXTURE);
  if (!sdlRend) return false;
  
  SDL_RenderSetScale(sdlRend,dpiScale,dpiScale);
  
  /*
  sdlFont=TTF_OpenFont("unifont.ttf",16);
  if (sdlFont==NULL) {
    printf("unifont.ttf wasn't found...");
    return false;
  }
  */
  scrSize.x=width;
  scrSize.y=height;

#if defined(ANDROID)
  int sizeX, sizeY;
  SDL_GetWindowSize(sdlWin,&sizeX,&sizeY);
  scrSize.x=sizeX/dpiScale;
  scrSize.y=sizeY/dpiScale;
#endif
  
  fontCache=SDL_CreateRGBSurfaceWithFormat(0,16*16,16*16,32,SDL_PIXELFORMAT_RGBA32);
  // unpack the unifont dump
  for (int i=0; i<8192; i++) {
    for (int j=0; j<8; j++) {
      ((unsigned char*)fontCache->pixels)[(i<<5)+(j<<2)]=255;
      ((unsigned char*)fontCache->pixels)[(i<<5)+(j<<2)+1]=255;
      ((unsigned char*)fontCache->pixels)[(i<<5)+(j<<2)+2]=255;
      ((unsigned char*)fontCache->pixels)[(i<<5)+(j<<2)+3]=(unifont[i]&(1<<(7-j)))?255:0;
    }
  }
  
  // uncomment for unifont.cpp generating code
  /*
  // copy font to texture
  SDL_Color c;
  SDL_Surface* s;
  SDL_Rect sr;
  c.r=255; c.g=255; c.b=255; c.a=255;
  fontCache=SDL_CreateRGBSurfaceWithFormat(0,16*16,16*16,32,SDL_PIXELFORMAT_RGBA32);
  for (int i=0; i<256; i++) {
    s=TTF_RenderGlyph_Solid(sdlFont,i,c);
    if (s==NULL) continue;
    sr=s->clip_rect;
    sr.x=(i%16)*16;
    sr.y=(i/16)*16;
    SDL_BlitSurface(s,&s->clip_rect,fontCache,&sr);
    SDL_FreeSurface(s);
  }
  FILE* st;
  st=fopen("unifont.cpp","wb");
  int packBitIndex=0;
  int writeByte=0;
  unsigned char packByte=0;
  fprintf(st,"const unsigned char unifont[]={\n  ");
  for (int i=0; i<65536; i++) {
    packByte=((((unsigned char*)fontCache->pixels)[i*4])?1:0)|(packByte<<1);
    packBitIndex++;
    if (packBitIndex>=8) {
      fprintf(st,"%3d, ",packByte);
      packBitIndex=0;
      packByte=0;
      writeByte++;
      if (writeByte>=16) {
        if (i!=65535) {
          fprintf(st,"\n  ");
        }
        writeByte=0;
      }
    };
  }
  fprintf(st,"\n};\n");
  fclose(st);
  */
  
  sdlText=SDL_CreateTextureFromSurface(sdlRend,fontCache);
  
  //fprintf(stderr,"\x1b[2J");
  return true;
}

SDL_Renderer* Graphics::_getDisplay() {
  return sdlRend;
}

float Graphics::_getScale() {
  return dpiScale;
}

bool PIR(float x1, float y1, float x2, float y2, float checkx, float checky) {
  // point-in-rectangle collision detection
  if (checkx>x1 && checkx<x2 && checky>y1 && checky<y2) {
    return true;}
  return false;
}
