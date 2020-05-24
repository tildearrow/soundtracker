// new tracker code
#ifndef _TRACKER_H
#define _TRACKER_H
#include <stdio.h>
#include <math.h>

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#ifdef __APPLE__
#include <SDL2_ttf/SDL_ttf.h>
#else
#include <SDL2/SDL_ttf.h>
#endif
#ifdef _WIN32
#include <windows.h>
#else
#ifndef __APPLE__
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
  TTF_Font* sdlFont;
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
      usleep(t*1000000);
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
    void _WRAP_set_clipping_rectangle(int x, int y, int w, int h) {
      SDL_Rect r;
      r.x=x; r.y=y; r.w=w; r.h=h;
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
      SDL_Rect sr, dr;
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

#endif
