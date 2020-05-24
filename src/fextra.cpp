#include "fextra.h"

size_t fsize(FILE* f) {
  size_t tell, ret;
  tell=ftell(f);
  fseek(f,0,SEEK_END);
  ret=ftell(f);
  fseek(f,tell,SEEK_SET);
  return ret;
}

short fgetsh(FILE* f) {
  short ret;
  fread(&ret,sizeof(short),1,f);
  return ret;
}

int fgeti(FILE* f) {
  int ret;
  fread(&ret,sizeof(int),1,f);
  return ret;
}
