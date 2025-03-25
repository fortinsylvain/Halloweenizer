//------------------------------------------------------------------------------------
//-- (c) Copyright Matrox Graphics Inc.                            __  __   ____  ___
//--                                                              |  \/  | / ___||_ _|
//-- This documents contains confidential proprietary information | |\/| || |  _  | |
//-- that may not be disclosed without prior written permission   | |  | || |_| | | |
//-- from Matrox Graphics Inc.                                    |_|  |_| \____||___|
//--
//-- $Id: mystring.h 92339 2022-05-29 18:03:42Z sfortin $
//-- $HeadURL: http://rex/mgi_svn/Users/sfortin/DdcSink_RP2040/mystring.h $
//-- $Revision: 92339 $
//-- $Date: 2022-05-29 14:03:42 -0400 (Sun, 29 May 2022) $
//-- $Author: sfortin $
//-- Author: Sylvain Fortin
//-- Project: HDMI HDCP certification
//-- Documentation:
//-- Description: 
//-- Note:
//------------------------------------------------------------------------------------
//#include "type.h"
#include <stdbool.h>
char ToUppercase(char);
bool IsHexAsciiChar(char);
unsigned long hex2int(char* hex);
void MyCpyStrToArray(char*, char*);





