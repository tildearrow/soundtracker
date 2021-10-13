#include "tracker.h"

void MacroStatus::next() {
  hasChanged=false;
  if (macro==NULL) {
    pos=0;
    return;
  }
  if (pos>=(int)macro->cmds.size()) return;
  if (--waitTime>0) return;

  bool getOut=false;
  while (!getOut) {
    pos++;
    if (pos>=macro->cmds.size()) break;
    MacroCommand cmd=macro->cmds[pos];
    switch (cmd.type&127) {
      case cmdEnd:
        pos=macro->cmds.size();
        break;
      case cmdSet:
        hasChanged=true;
        value=cmd.value;
        break;
      case cmdWait:
        waitTime=cmd.value;
        getOut=true;
        break;
      case cmdWaitRel:
        if (!released) {
          getOut=true;
          pos--;
        }
        break;
      case cmdLoop:
        pos=cmd.value-1;
        break;
      case cmdLoopRel:
        if (!released) pos=cmd.value-1;
        break;
      case cmdAdd:
        hasChanged=true;
        value+=cmd.value;
        break;
      case cmdSub:
        hasChanged=true;
        value-=cmd.value;
        break;
    }
    if (cmd.type&128) getOut=true;
  }
}

void MacroStatus::release() {
  released=true;

  if (macro!=NULL) {
    if (macro->jumpRelease>-1) {
      pos=macro->jumpRelease-1;
    }
  }
}

void MacroStatus::load(Macro* m) {
  macro=m;
  pos=-1;
  waitTime=0;
  released=false;
  hasChanged=false;
  value=0;
}

MacroStatus::MacroStatus() {
  load(NULL);
}