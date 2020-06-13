#include <stdio.h>
#include <string.h>

FILE* f;
FILE* out;
FILE* sout;

char str[256];

unsigned char glyph[64];
int glyphSize;

int glyphIndex;

unsigned char glyphSizes[8192];

char char2hex(char c) {
  if (c>='0' && c<='9') {
    return c-'0';
  }
  if (c>='A' && c<='F') {
    return 10+c-'A';
  }
  if (c>='a' && c<='f') {
    return 10+c-'a';
  }
  return -1;
}

int main(int argc, char** argv) {
  if (argc<4) {
    printf("usage: %s in out sizeout\n",argv[0]);
    return 1;
  }
  f=fopen(argv[1],"r");
  if (f==NULL) {
    perror("in");
    return 1;
  }

  out=fopen(argv[2],"wb");
  if (out==NULL) {
    perror("out");
    return 1;
  }

  sout=fopen(argv[3],"wb");
  if (sout==NULL) {
    perror("out");
    return 1;
  }

  memset(glyphSizes,0,8192);

  int h=0;
  while (!feof(f)) {
    fgets(str,255,f);
    glyphIndex=0;
    glyphSize=0;
    h=0;
    do {
      if (str[h]==':') break;
      glyphIndex<<=4;
      glyphIndex|=char2hex(str[h]);
    } while (++h);
    if (glyphIndex>0xffff) {
      break;
    }
    memset(glyph,0,32);
    do {
      if (str[h]=='\n' || str[h]==0) break;
      glyph[glyphSize>>1]<<=4;
      glyph[glyphSize>>1]|=char2hex(str[h]);
      glyphSize++;
    } while (++h);
    glyphSize>>=1;
    // write a 16x16 glyph regardless
    fseek(out,glyphIndex*32,SEEK_SET);
    if (glyphSize==32) {
      fwrite(glyph,1,32,out);
    } else {
      for (int i=0; i<16; i++) {
        fputc(glyph[i],out);
        fputc(0,out);
      }
    }

    if (glyphSize>16) {
      glyphSizes[glyphIndex>>3]|=(1<<(glyphIndex&7));
    } else {
      glyphSizes[glyphIndex>>3]&=~(1<<(glyphIndex&7));
    }
  }
  fseek(out,2097151,SEEK_SET);
  fputc(0,out);

  fwrite(glyphSizes,1,8192,sout);

  fclose(f);
  fclose(out);
  fclose(sout);
  return 0;
}
