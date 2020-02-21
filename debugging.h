#ifndef DEBUGGING_H
#define DEBUGGING_H

#include "config.h"

#ifdef ENABLE_DEBUGGING
  #define debugf(msg) Serial.print(F(msg))
  #define debug(msg) Serial.print(msg)
  #define debugfln(msg) Serial.println(F(msg))
  #define debugln(msg) Serial.println(msg)
  #define debugfmt(msg, fmt) Serial.print(msg, fmt)
  #define debuglnfmt(msg, fmt) Serial.println(msg, fmt)
#else
  #define debugf(msg)
  #define debug(msg)
  #define debugfmt(msg, fmt)
  #define debugln(msg)
  #define debugfln(msg)
  #define debuglnfmt(msg, fmt)
#endif

#endif
