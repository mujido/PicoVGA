
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#ifndef _MAIN_H
#define _MAIN_H

#define IMG_LAYER	1	// overlapped layer with RLE image

constexpr int DEFAULT_MODE_INDEX = 0;

extern const u16 Pattern1_rows[241];
extern const u8 Pattern1[3376] __attribute__ ((aligned(4)));

extern const u16 Pattern2_rows[241];
extern const u8 Pattern2[34888] __attribute__ ((aligned(4)));

extern const u16 Pattern3_rows[241];
extern const u8 Pattern3[23512] __attribute__ ((aligned(4))); 

extern const u16 Pattern4_rows[241];
extern const u8 Pattern4[5280] __attribute__ ((aligned(4)));

#endif // _MAIN_H
