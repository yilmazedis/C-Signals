#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main (int argc, char **argv)
{
  int aflag = 0;
  int bflag = 0;
  char *cvalue = NULL;
  int index;
  int c;

  opterr = 0;


  while ((c = getopt (argc, argv, ":i:j:n:")) != -1)
    switch (c)
      {
      case 'i':
        fprintf(stderr, "i\n");
        break;
      case 'j':
        fprintf(stderr, "j\n");
        break;
      case 'n':
        fprintf(stderr, "n\n");
        break;
      case ':':
        fprintf(stderr, "as\n");
      case '?':
        fprintf(stderr, "unknown option\n");
      }

  return 0;
}