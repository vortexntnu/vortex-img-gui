#include <stdio.h>

#define ArrayCount(Array) (sizeof(Array) / sizeof(*Array))
void LogError(const char *Str) { printf("%s\n", Str); }

unsigned long CountChar(const char *Str) {
  int Result = 0;
  while (*Str++) {
    Result++;
  }
  return Result;
}

void FromCStrCopyToDest(const char *Source, char *Dest, int DestSize) {
  int NumCopied = 0;
  while (*Source) {
    if (NumCopied == DestSize) {
      LogError("FromCStrCopyToDest, Dest is too small to hold Source");
      break;
    }
    *Dest++ = *Source++;
    NumCopied++;
  }
}
