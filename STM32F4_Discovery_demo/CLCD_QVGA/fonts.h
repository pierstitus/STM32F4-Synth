/*
 * fonts.h
 *
 * File containing prototype of the fonts for display
 *
 *
 */

#include <stdint.h>

#ifndef _FONT_
#define _FONT_

#define FONT_TABLE_HEIGHT_IDX 				0
#define FONT_TABLE_PAD_AFTER_CHAR_IDX 		1
#define FONT_TABLE_LINE_SPACING_IDX 		2
#define FONT_TABLE_DECENDERS_HEIGHT_IDX 	3
#define FONT_TABLE_UNUSED_IDX				4
#define FONT_TABLE_CHAR_LOOKUP_IDX			5

extern const uint8_t font_Small[];
extern const uint8_t font_Larger[];
//extern const uint8_t font_Medium[];
extern const uint8_t font_MediumBold[];
extern const uint8_t font_LargeNumbers[];

#endif
