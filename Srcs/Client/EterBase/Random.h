#ifndef __INC_ETERBASE_RANDOM_H__
#define __INC_ETERBASE_RANDOM_H__
#ifdef __ANDROID__
#include <stdlib.h>
#define srandom metin2_srandom
#define random metin2_random
#endif
extern void				srandom(unsigned long seed);
extern unsigned long	random();
extern float			frandom(float flLow, float flHigh);
extern long				random_range(long from, long to);

#endif
//martysama0134's fb0d8d5d0566583187a94f9a823b2075
