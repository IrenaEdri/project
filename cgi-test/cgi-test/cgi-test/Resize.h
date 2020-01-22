#ifndef RESIZE_H
#define RESIZE_H

#include <stdlib.h>
#include <string.h>

void DSP_Resize_3x3_to_9x9(	const	unsigned char *input_image,
									unsigned char *output_image);

int RescaleLine(unsigned short *ay, const	unsigned char *s,	unsigned short wd,
				unsigned short pws,			unsigned short pwd,	unsigned short fh);

int Rescale(unsigned char *d,  const 	unsigned char *s,
			unsigned short wd,			unsigned short hd,
			unsigned short ws,			unsigned short hs);

#endif
