#include <Cocoa/Cocoa.h>
#include "nsstub.h"

// this may need to be replaced
double nsStubDPI() {
  CGFloat val;
  val=[[NSScreen mainScreen] backingScaleFactor];
  return (double)val;
}

char* nsStubFontPath(const char* name) {
  return "";
};
