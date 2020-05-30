// new tracker code
#ifndef _TRACKER_H
#define _TRACKER_H
#include <stdio.h>
#include <math.h>

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
#include <string>
#include <vector>

#include "fextra.h"

typedef std::string string;

bool PIR(float x1, float y1, float x2, float y2, float checkx, float checky);

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
  bool inited;
  float nlPos;
  float align;
  float dpiScale;
  
  SDL_Window* sdlWin;
  SDL_Renderer* sdlRend;
  //TTF_Font* sdlFont;
  SDL_Surface* fontCache;
  SDL_Texture* sdlText;
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
    SDL_Texture* _WRAP_create_bitmap(int w, int h) {
      SDL_Texture* ret=SDL_CreateTexture(sdlRend,SDL_PIXELFORMAT_RGBA8888,SDL_TEXTUREACCESS_TARGET,w,h);
      SDL_SetTextureBlendMode(ret,SDL_BLENDMODE_BLEND);
      return ret;
    }
    void _WRAP_destroy_bitmap(SDL_Texture* bitmap) {
      SDL_DestroyTexture(bitmap);
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
    SDL_Texture* _WRAP_load_bitmap(const char* fn) {
      return NULL;//al_load_bitmap(fn);
    }
    void _WRAP_draw_bitmap(SDL_Texture* bitmap, float x, float y, int flags) {
      SDL_Rect dr;
      dr.x=x;
      dr.y=y;
      SDL_QueryTexture(bitmap,NULL,NULL,&dr.w,&dr.h);
      SDL_RenderCopy(sdlRend,bitmap,NULL,&dr);
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
    int _WRAP_get_bitmap_width(SDL_Texture* bitmap) {
      int retval;
      SDL_QueryTexture(bitmap,NULL,NULL,&retval,NULL);
      return retval;
    }
    void _WRAP_draw_bitmap_region(SDL_Texture* bitmap, float x, float y, float w, float h, float dx, float dy, int flags) {
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
      SDL_QueryTexture(bitmap,NULL,NULL,&tr.w,&tr.h);
      if ((sr.h+sr.y)>tr.h) {
        sr.h-=(sr.h+sr.y)-tr.h;
        dr.h=sr.h;
      }
      SDL_RenderCopy(sdlRend,bitmap,&sr,&dr);
    }
    void _WRAP_draw_scaled_bitmap(SDL_Texture* bitmap, float sx, float sy, float sw, float sh, float dx, float dy, float dw, float dh, int flags) {
      SDL_Rect sr, dr, tr;
      sr.x=sx;
      sr.y=sy;
      sr.w=sw;
      sr.h=sh;
      dr.x=dx;
      dr.y=dy;
      dr.w=dw;
      dr.h=dh;
      SDL_RenderCopy(sdlRend,bitmap,&sr,&dr);
    }
    void _WRAP_draw_rotated_bitmap(SDL_Texture* bitmap, float cx, float cy, float x, float y, float r, int flags) {
      // TODO
      ::printf("Draw Rotated Bitmap\n");
      //al_draw_rotated_bitmap(bitmap,cx,cy,x,y,r,flags);
    }
    void _WRAP_draw_circle(float x, float y, float r, Color color, float thick) {
      ::printf("Draw Circle\n");
      //al_draw_circle(x,y,r,color,thick);
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
    void setTarget(SDL_Texture* where);
    void trigResize(int tx, int ty);
    int printf(const char* format, ...);
    bool preinit();
    bool init(int width, int height);
    bool quit();
    Graphics(): inited(false), nlPos(0), align(0) {}
};

extern Graphics g;

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

class NumberPad {
  float x, y;
  Button bNum[16];
  Button bDel;
  Button bOK;
  Button bUp;
  Button bDown;
};
#endif
