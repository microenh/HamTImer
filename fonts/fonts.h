#pragma once

#include <stdint.h>

typedef struct _tFont
{    
  const unsigned char *table;
  uint16_t width;
  uint16_t height;
  
} sFONT;

// extern const sFONT Liberation24;
// extern const sFONT Liberation26;
// extern const sFONT Liberation28;
extern const sFONT Liberation36;
extern const sFONT Liberation48;
// extern const sFONT Liberation72;

// extern const unsigned char liberation_24pt[];
// extern const unsigned char liberation_26pt[];
// extern const unsigned char liberation_28pt[];
extern const unsigned char liberation_36pt[];
extern const unsigned char liberation_48pt[];
// extern const unsigned char liberation_72pt[];
