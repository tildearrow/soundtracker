#include <stdio.h>
#include <string.h>

class soundchip;

class SSInter {
  soundchip* out;
  int octave;
  int curChan;
  int prefChan;
  public:
    bool next(const char* buf, int& set, size_t size);
    void setChan(int ch);
    void init(soundchip* where);
    SSInter(): out(NULL), octave(4), curChan(0), prefChan(0) {}
};
