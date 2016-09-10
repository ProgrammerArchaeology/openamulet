// fborder.h float byte order


#define FLOAT_BYTES  4
#define DOUBLE_BYTES 8

float htonf(float host_float);
#define ntohf(f) htonf(f)
double htond(double host_double);
#define ntohd(d) htond(d)
