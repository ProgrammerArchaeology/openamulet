//
// ------------------------------------------------------------------------
// 		The OpenAmulet User Interface Development Environment
// ------------------------------------------------------------------------
// This code is based on the Amulet project of Carnegie Mellon University,
// and has been placed in the public domain.  If you are using this code
// or any part of OpenAmulet, please contact amulet@cs.cmu.edu to be put
// on the mailing list or have a look at:
// http://www.openip.org
//

#include <am_inc.h>

#include "amulet/impl/opal_misc.h"

// Color conversion routines.  RGB to HSV and HSV to RGB
// Algorithms grabbed from Foley & vanDam
void Am_RGB_To_HSV (float r, float g, float b, float& h, float& s, float& v)
{
  float min, max;
  if (r > g) {
    max = r; min = g;
  }
  else {
    max = g; min = r;
  }
  if (max < b)
    max = b;
  if (min > b)
    min = b;
  v = max;
  if (max != 0.0)
    s = (max - min) / max;
  else
    s = 0.0;
  if (s != 0.0) {
    float delta = max - min;
    if (r == max)
      h = (g - b) / delta;
    else if (g == max)
      h = 2 + (b - r) / delta;
    else if (b == max)
      h = 4 + (r - g) / delta;
    h = h * 60.0f;
    if (h < 0.0)
      h += 360.0;
  }
  else
    h = 0.0;
}

void Am_HSV_To_RGB (float h, float s, float v, float& r, float& g, float& b)
{
  if (s == 0.0) {
    r = v;
    g = v;
    b = v;
  }
  else {
    while (h >= 360.0)
      h -= 360.0;
    h = h / 60.0f;
    int region = (int)h;
    float frac = h - (float)region;
    float p = v * (1 - s);
    float q = v * (1 - (s * frac));
    float t = v * (1 - (s * (1 - frac)));
    switch (region) {
    case 0:
      r = v; g = t; b = p;
      break;
    case 1:
      r = q; g = v; b = p;
      break;
    case 2:
      r = p; g = v; b = t;
      break;
    case 3:
      r = p; g = q; b = v;
      break;
    case 4:
      r = t; g = p; b = v;
      break;
    case 5:
      r = v; g = p; b = q;
      break;
    }
  }
}

