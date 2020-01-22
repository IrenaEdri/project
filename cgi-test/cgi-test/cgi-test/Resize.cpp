#include "stdafx.h"
#include "Resize.h"


int BicubicIndices[9*4] = {
	 0,     0,     0,     1,
     0,     0,     1,     2,
     0,     0,     1,     2,
     0,     0,     1,     2,
     0,     1,     2,     2,
     0,     1,     2,     2,
     0,     1,     2,     2,
     1,     2,     2,     2,
     1,     2,     2,     2};


double BicubicWeights[9*4] = {
	-1.0/27.0,      1.0/3.0,      7.0/9.0,     -2.0/27.0,
     0.0,            1.0,      		0.0,       0.0,
    -2.0/27.0,      7.0/9.0,      1.0/3.0,    -1.0/27.0,
    -1.0/27.0,      1.0/3.0,      7.0/9.0,    -2.0/27.0,
     0.0,            1.0,      		0.0,       0.0,
    -2.0/27.0,      7.0/9.0,      1.0/3.0,    -1.0/27.0,
    -1.0/27.0,      1.0/3.0,      7.0/9.0,    -2.0/27.0,
     0.0,            1.0,      		0.0,       0.0,
    -2.0/27.0,      7.0/9.0,      1.0/3.0,    -1.0/27.0};

//==============================================================================
// Returns the lesser of two values
#define min(a,b)            ((a)<(b)?(a):(b))

// Converts an integer value 'a' to a fixed-point value. In this implementation
// the value of 'a' may be 0 or 1.
#define int2fix(a)          ((a)<<15)

// Converts a floating-point value 'a' to a fixed-point value. In this implemen-
// tation 0 <= 'a' < 2.0.
#define float2fix(a)        ((unsigned short)((a)*(1<<15)))

// Multiplies two fixed-point values and yields a fixed-point value.
#define multfix(a,b)        (((unsigned long)(a)*(b))>>15)

// Multiplies a fixed-point value by an integer and shifts right to place the
// binary(mal) point where desired.
#define multfixint(a,b,c)   (((unsigned long)(a)*(b))>>(c))
//==============================================================================


void DSP_Resize_3x3_to_9x9(const unsigned char *input_image,
			 unsigned char *output_image)
{
	int i,j, k;
	double rows_interp[9][3];
	double cols_interp[9][9];

	memset(rows_interp, 0, sizeof(rows_interp));
	memset(cols_interp, 0, sizeof(cols_interp));

	for(i = 0; i < 9; i ++)
	{
		for(k = 0; k < 3; k++)
		{
			for(j = 0; j < 4; j++)
			{
				rows_interp[i][k] += input_image[k + 3 * BicubicIndices[i * 4 + j]] * BicubicWeights[i * 4 + j];
			}
		}
		/*
		for(k = 0; k < 3; k++)
		{
			//rows_interp[i][k] >>= 20;
		}
		*/
	}

	for(i = 0; i < 9; i ++)
	{
		for(k = 0; k < 9; k++)
		{
			for(j = 0; j < 4; j++)
			{
				cols_interp[i][k] += rows_interp[i][BicubicIndices[k * 4 + j]] * BicubicWeights[k * 4 + j];
			}

			if(cols_interp[i][k] < 0)
			{
				cols_interp[i][k] = 0;
			}
			if(cols_interp[i][k] > 255)
			{
				cols_interp[i][k] = 255;
			}
			output_image[i * 9 + k] = cols_interp[i][k];// >>= 20;
		}


	}
}

// Rescales one line of pixels.
//
// Arguments:
//      ay  - pointer to the vertical accumulator's array.
//      s   - pointer to the line of source pixels.
//      wd  - destination width.
//      pws - proportion of source width.
//      pwd - proportion of destination width.
//      fh  - height fraction for this line.
int RescaleLine(unsigned short *ay, const unsigned char *s, unsigned short wd,
                 unsigned short pws, unsigned short pwd, unsigned short fh)
{
    // The origin width fraction is
    // how much remains to finish the contribution of the origin pixels to the
    // destination pixels. If it is greater than 1.0 then one origin pixel will
    // contribute to more than one destination pixel. If it is less than 1.0 then more than one
    // origin pixel will contribute to the same destination pixel.
    unsigned short  fws;

    // The destination width fraction is
    // how much remains to finish the calculation of the destination pixel.
    unsigned short  fwd;

    // Width sub-pixel fraction. It is the fraction that the origin pixel contributes
    // to the composition of current sub-pixel (<= 1.0).
    unsigned short  fw;

    // Accumulator X.
    // Accumulates the contributions of origin pixels to the composition of each destination pixel.
    unsigned short  ax;

    // X coordinate of current origin pixel.
    unsigned short  xs;

    // X coordinate of current destination pixel.
    unsigned short  xd;

    // Iterate for all destination pixels of one line.
    for( ax = 0, xs = 0, xd = 0, fws = pws, fwd = pwd; xd < wd; )
        {
        // Obtain the width fraction that the origin pixel contributes to the composition of current destination sub-pixel.
        // It is always the lesser among the remaining width fractions of origin and destination (<= 1.0).
        fw  = min( fws, fwd );
        //----------------------------------------------------------------------
        // Calculate the intensity fraction that the origin pixel contributes to the horizontal composition of the destination sub-pixel,
        // multiplying the width fraction by the value of the source pixel, and add it to accumulator X.
        ax  += multfixint( fw, s[xs], 7 );
        //----------------------------------------------------------------------
        // Subtract the just used width-fraction from the remaining origin width-fraction, and if this reached zero...
        if(( fws -= fw ) == int2fix( 0 ))
            {
            // ... the contribution of this origin pixel is finished, reload the total value for the next origin pixel.
            fws      = pws;
            // Advance to the next origin pixel.
            xs++;
            }

        // Subtract the just used width-fraction from the remaining destination width-fraction, and if this reached zero...
        if(( fwd -= fw ) == int2fix( 0 ))
            {
            // ... the calculation of this destination pixel is finished, reload the total value for the next destination pixel.
            fwd      = pwd;
            // Calculate the intensity fraction that the origin pixels contribute to the vertical composition of the destination sub-pixel,
            // multiplying the height-fraction by the value of accumulator X, and add to accumulator Y.
            ay[xd]  += multfixint( fh, ax, 15 );
            // Zero the accumulator X because we are starting a new destination pixel.
            ax       = 0;
            // Advance to the next destination pixel.
            xd++;
            }
        }
    return 0;
}

//==============================================================================
// Rescale the entire image.
//
// Arguments:
//      d   - pointer to the buffer to receive the resulting image. It must be
//            pre-allocated.
//      s   - pointer to the buffer with the original image.
//      wd  - width of the resulting image in pixels.
//      hd  - height of the resulting image in pixels.
//      ws  - width of the original image in pixels.
//      hs  - height of the original image in pixels.
//
// Return:
//      0   - success.
//      1   - failure.
//
// Note:
//      1) This version works for raw 8-bit gray-scale images. That is, the data
//         is just a bi-dimensional array of 8-bit values representing pixels with
//         256 levels of gray.
//      2) The scaling factors may be from 0.5 to 2.0 and may be different for X
//         and Y directions.
int Rescale(unsigned char *d,  const unsigned char *s,
            unsigned short wd, unsigned short hd,
            unsigned short ws, unsigned short hs)
{
    unsigned short  pws;    // Width proportion at origin ( >= 0.5 && < 2.0 ).
    unsigned short  pwd;    // Width proportion at destination ( = 1.0 ).

    unsigned short  phs;    // Height proportion at origin ( >= 0.5 && < 2.0 ).
    unsigned short  phd;    // Height proportion at destination ( = 1.0 ).
    unsigned short  fhs;    // Height residue (fraction) at origin ( > 0.0 && <= 1.0 ).
    unsigned short  fhd;    // Height residue (fraction) at destination ( > 0.0 && <= 1.0 ).
    unsigned short  fh;     // Width residue (fraction) of current sub-pixel ( > 0.0 && <= 1.0 ).

    // Pointer to the vertical accumulator array. One entry for each destination column.
    // This is needed because each destination line may be composed of values from more
    // than one source line.
    static unsigned short  ay[256*256];

    // Current source line.
    unsigned short  ys;

    // Current destination line.
    unsigned short  yd;

    // Variable to iterate through all of the columns of a destination line.
    unsigned short  xd;

    // One of the dimensions is invalid...
    if( ws == 0 || hs == 0 || wd == 0 || hd == 0 )
        // ... return an error.
        return -1;

    // The origin and destination dimensions are identical...
    if( ws == wd && hs == hd )
        // ... nothing to do.
        return 0;

    // Special case: 3x3 to 9x9 resizing
    if(ws == 3 && hs == 3 && wd ==9 && hd == 9)
    {
    	DSP_Resize_3x3_to_9x9(s, d);
    	return 1;
    }

    // The scaling factors are outside the allowed range...
    if( (float)ws / (float)wd >= 2.0 || (float)wd / (float)ws >= 2.0 ||
        (float)hs / (float)hd >= 2.0 || (float)hd / (float)hs >= 2.0 )
        // ... return failure status.
        return -1;

    // Calculate the width proportions of origin and destination.

    // Width proportion at origin.
    pws = float2fix( (float)wd / (float)ws );
    // Width proportion at destination == 1.
    pwd = int2fix( 1 );

    // Calculate the origin and destination height proportions.

    // Height proportion at origin.
    phs = float2fix( (float)hd / (float)hs );
    // Height proportion at destination == 1.0.
    phd = int2fix( 1 );

    // Initialize all the vertical accumulators to zero.
    memset( ay, 0x00, wd * sizeof ay[0] );

    // Iterate through all lines. It may iterate more than once for each source or
    // destination line.

    // 'ys' and 'yd' advance 'staggering', sometimes one advances, sometimes
    // the other advances, sometimes both advance together.

    for( ys = 0, yd = 0, fhs = phs, fhd = phd; yd < hd; )
	{
		// The height-fraction is the lesser among the source-height-fraction
		// and the destination-height-fraction.
		fh  = min( fhs, fhd );
		//----------------------------------------------------------------------
		// Calculate the partial destination line for this combination of source
		// (ys) and destination (yd) line.
		RescaleLine( ay, s + ws * ys, wd, pws, pwd, fh );
		//----------------------------------------------------------------------
		// Subtract the just-used height-fraction from the source-height-fraction,
		// and if the last reached zero...
		if(( fhs -= fh ) == int2fix( 0 ))
		{
			// ... reload it with the total value, ...
			fhs     = phs;
			// ... and advance the source line.
			ys++;
		}

		// Subtract the just-used height-fraction from the destination-height-fraction,
		// and if the last reached zero...
		if(( fhd -= fh ) == int2fix( 0 ))
		{
			// ... reload it with the total value, ...
			fhd     = phd;
			// ... copy the accumulated resulting line to its definitive location, ...
			for( xd = 0; xd < wd; xd++ )
				d[yd*wd+xd] = ay[xd] >> 8;
			// ... zero the vertical accumulators because we are starting a new destination line, ...
			memset( ay, 0x00, wd * sizeof ay[0] );
			// ... and advance the destination line.
			yd++;
		}
	}
    
    return 1;
}
//==============================================================================

