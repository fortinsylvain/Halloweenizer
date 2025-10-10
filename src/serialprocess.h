
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

