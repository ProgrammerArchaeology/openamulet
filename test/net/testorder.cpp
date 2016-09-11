#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main (void)
{
  int big;
  long h,n;
  char *hp,*np;
  h=65536;
  n=htonl(h);
  hp=(char *)&h;
  np=(char *)&n;
  printf ("Your system represents 2^16 as:   %x:%x:%x:%x\n",
	  hp[0],hp[1],hp[2],hp[3]);
  printf ("Network Order represents 2^16 as: %x:%x:%x:%x\n",
	  hp[0],hp[1],hp[2],hp[3]);
  big=(h=n);
  printf ("\nYou are using %s Endian.\n", (big?"Big":"Little"));
  return big;
}
