//---------------------------------------------------------------------------------
#ifndef _font_h_
#define _font_h_
//---------------------------------------------------------------------------------
#if defined FONT_ORIGINAL
#include "font_orig.h"
#elif defined FONT_6X10
#include "font_6x10.h"
#elif defined FONT_ACORN
#include "font_acorn_8x8.h"
#else
#include "font_6x10.h" // standard font
#endif 
//---------------------------------------------------------------------------------
#endif //_font_h_
//---------------------------------------------------------------------------------
