//------------------------------------------------------------------------------------
//-- (c) Copyright Matrox Graphics Inc.                            __  __   ____  ___
//--                                                              |  \/  | / ___||_ _|
//-- This documents contains confidential proprietary information | |\/| || |  _  | |
//-- that may not be disclosed without prior written permission   | |  | || |_| | | |
//-- from Matrox Graphics Inc.                                    |_|  |_| \____||___|
//--
//-- $Id: serialprocess.h 97901 2022-09-30 22:23:04Z sfortin $
//-- $HeadURL: http://rex/mgi_svn/Users/sfortin/DdcSink_RP2040/serialprocess.h $
//-- $Revision: 97901 $
//-- $Date: 2022-09-30 18:23:04 -0400 (Fri, 30 Sep 2022) $
//-- $Author: sfortin $
//-- Author: Sylvain Fortin
//-- Project: HDMI HDCP certification
//-- Documentation:
//-- Description: 
//-- Note:
//------------------------------------------------------------------------------------
#include <stdio.h>
#include <stdbool.h>

#include "mystring.h" // my string manipulations

#define CMD_LENGTH 50       // Number of characters to store history lines
#define CMD_HIST_SIZE 20    // Number of lines in history

extern char CmdHistory[CMD_HIST_SIZE][CMD_LENGTH];
extern unsigned char CmdHistSel;
extern bool CmdStringFlag;
extern unsigned char AnsiEscapeState;
extern char VtSequ[2];
extern unsigned char CmdCharCnt;
extern char CmdString[CMD_LENGTH];

void ProcessSerialChar(char);
void CpHistToCmd(unsigned char);
void InitHistory(void);
void ClearScreen(void);
bool CmpStrEqu(char*, char*);
void ClearCmd(void);
void GoToCol(uint8_t);

