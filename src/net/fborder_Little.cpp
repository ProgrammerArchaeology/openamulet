// fborder.h float byte order
#include "fborder.h"

float
htonf(float host_float)
{
  short int i;
  float net_float;
  char *phbyte, *pnbyte;
  phbyte = (char *)&host_float;
  pnbyte = (char *)&net_float;
  for (i = 0; (i < FLOAT_BYTES); i++)
    pnbyte[i] = phbyte[(FLOAT_BYTES - i - 1)];
  return net_float;
}

double
htond(double host_double)
{
  short int i;
  double net_double;
  char *phbyte, *pnbyte;
  phbyte = (char *)&host_double;
  pnbyte = (char *)&net_double;
  for (i = 0; i < DOUBLE_BYTES; i++)
    pnbyte[i] = phbyte[DOUBLE_BYTES - i - 1];
  return net_double;
}
