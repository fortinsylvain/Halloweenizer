//------------------------------------------------------------------------------------
//-- (c) Copyright Matrox Graphics Inc.                            __  __   ____  ___
//--                                                              |  \/  | / ___||_ _|
//-- This documents contains confidential proprietary information | |\/| || |  _  | |
//-- that may not be disclosed without prior written permission   | |  | || |_| | | |
//-- from Matrox Graphics Inc.                                    |_|  |_| \____||___|
//--
//-- $Id: mystring.c 92339 2022-05-29 18:03:42Z sfortin $
//-- $HeadURL: http://rex/mgi_svn/Users/sfortin/DdcSink_RP2040/mystring.c $
//-- $Revision: 92339 $
//-- $Date: 2022-05-29 14:03:42 -0400 (Sun, 29 May 2022) $
//-- $Author: sfortin $
//-- Author: Sylvain Fortin
//-- Project: HDMI HDCP certification
//-- Documentation:
//-- Description: 
//-- Note:
//------------------------------------------------------------------------------------
#include "mystring.h"


char ToUppercase( char Character )
{
   if( (Character >= 'a') & (Character <= 'z') )
      return (Character - ('a'-'A'));
   else
      return Character;
}

bool IsHexAsciiChar(char Character)
{
   if( Character < '0' )
      return false;
   else if( (Character > '9') && (Character < 'A') )
      return false;
   else if( (Character > 'F') && (Character < 'a') )
      return false;
   else if( Character > 'f' )
      return false;
   else
      return true;
}

/**
 * hex2int
 * take a hex string and convert it to a 32bit number (max 8 hex digits)
 */
unsigned long hex2int(char *hex) 
{
   unsigned long val = 0;
   while (*hex) {
      // get current character then increment
      unsigned char byte = *hex++;
      // transform hex character to the 4bit equivalent number, using the ascii table indexes
      if (byte >= '0' && byte <= '9') 
         byte = byte - '0';
      else if (byte >= 'a' && byte <='f') 
         byte = byte - 'a' + 10;
      else if (byte >= 'A' && byte <='F') 
         byte = byte - 'A' + 10;
      // shift 4 to make space for new digit, and add the 4 bits of the new digit 
      val = (val << 4) | (byte & 0xF);
   }
   return val;
}

// Blindly copy until 0 is uncountered
void MyCpyStrToArray(char* DstArray, char* SrcArray)
{
   while (*SrcArray) {
      // get current character then increment
      unsigned char byte = *SrcArray++;
      *DstArray++ = byte;
   }
}



