#include "serialprocess.h"

char CmdHistory[CMD_HIST_SIZE][CMD_LENGTH];
unsigned char CmdHistSel;
bool CmdStringFlag;
unsigned char AnsiEscapeState;
char VtSequ[2];
unsigned char CmdCharCnt;
char CmdString[CMD_LENGTH];

// Process control character and build the command coming from the terminal 
void ProcessSerialChar(char recd)
{
    if (recd == 0x00) // [NULL]
    {                 // Do nothing, USB appear to send this periodically!
    }
    else if (recd == 0x03) // [Ctrl] C ?
    {
    }                      // Do not store...
    else if (recd == 0x08) // Backspace ?
    {
        if (CmdCharCnt > 0)
        {
            // Move characters one space left from CmdCharCnt
            CmdCharCnt = CmdCharCnt - 1;
            for (unsigned char i = CmdCharCnt; i < sizeof(CmdString) - 1; i++)
                CmdString[i] = CmdString[i + 1];
            printf("\x1b\x5b\x32\x4B"); // erase curent line
            printf("\r");               // return cursor to line begin
            printf(">");
            printf(CmdString);
            printf("\x0D\033["); // Put cursor to location
            // UART_SendInt(CmdCharCnt+1);
            printf("%x", CmdCharCnt + 1);
            printf("C");
        }
    }
    else if (recd == 0x7F) // Backspace Key? Cause reception of a single DEL character 127 0x7F
    {
        if( CmdCharCnt > 0 )    // Character deletion possible only if Cmd not empty
        {
            CmdCharCnt = CmdCharCnt - 1;
            for (unsigned char i = CmdCharCnt; i < sizeof(CmdString) - 1; i++)
            {
                CmdString[i] = CmdString[i + 1];
            }
        }
        printf("\x1b\x5b\x32\x4B"); // erase curent line
        printf("\r");               // return cursor to line begin
        printf(">");
        printf(CmdString);
        GoToCol(CmdCharCnt+1);      // Put cursor to current command character position
    }
    else if (recd == 0x1B) // ESC ?
    {
        if (AnsiEscapeState == 0)
            AnsiEscapeState = 1;
    }
    else if ((recd == 0x5b) && (AnsiEscapeState == 1)) // Got ESC + '['
    {
        AnsiEscapeState = 2;
    }
    else if (AnsiEscapeState == 2)
    {

        if (recd == 0x44) // Got ESC + '[' + 'D'   (LEFT ARROW) ?
        {
            AnsiEscapeState = 0;
            if (CmdCharCnt > 0)
            {
                printf("\x1b\x5b\x44");
                CmdCharCnt = CmdCharCnt - 1;
            }
        }
        else if (recd == 0x43) // Got ESC + '[' + 'C'   (RIGHT ARROW) ?
        {
            AnsiEscapeState = 0;
            if (!((CmdString[CmdCharCnt] == 0) && (CmdString[CmdCharCnt + 1] == 0)))
            {
                printf("\x1b\x5b\x43");
                CmdCharCnt = CmdCharCnt + 1;
            }
        }
        else if (recd == 0x41) // Got ESC + '[' + 'A'  (UP ARROW) ?
        {
            AnsiEscapeState = 0;
            printf("\x1b\x5b\x32\x4B"); // erase curent line
            printf("\r");               // return cursor to line begin
            printf(">");
            CmdCharCnt = 0;
            if( CmdHistSel == CMD_HIST_SIZE)    // Already at the top?
            {
                CpHistToCmd(CmdHistSel-1);      // Yes, then only show this command
            }
            else                                // No, then increase position and show corresponding command
            {
                CmdHistSel++;
                CpHistToCmd(CmdHistSel-1);
            }
        }
        else if (recd == 0x42) // Got ESC + '[' + 'B'  (DOWN ARROW) ?
        {
            AnsiEscapeState = 0;
            printf("\x1b\x5b\x32\x4B"); // erase curent line
            printf("\r");               // return cursor to line begin
            printf(">");
            CmdCharCnt = 0;
            if( CmdHistSel == 0)        // Oustide bottom? Value 0 mean reach outside buttom of history
            {                           // Yes, show the prompt without command
            }
            else if( CmdHistSel == 1)   // Last item of history bottom?
            {
                CmdHistSel--;           // Yes, then decrease position and show prompt without command
                ClearCmd();             // Clear command
            }
            else
            {
                CmdHistSel--;                   // Decrease position and show corresponding command
                CpHistToCmd(CmdHistSel-1);
            }
        }
        else if ((recd >= '1') & (recd <= '9')) // Got ESC + '[' + digit bewteen 1-9 ?
        {
            if (VtSequ[0] == 0) // first vt sequence character ?
            {
                VtSequ[0] = recd;
            }
            else // else this is the second
            {
                VtSequ[1] = recd;
            }
        }
        else if (recd == '~') // end of vt sequ ?
        {
            if (VtSequ[0] == '3') // Delete ?
            {
                if (CmdCharCnt > 0)
                {
                    for (unsigned char i = CmdCharCnt; i < sizeof(CmdString) - 1; i++)
                        CmdString[i] = CmdString[i + 1];
                    printf("\x1b\x5b\x32\x4B"); // erase curent line
                    printf("\r");               // return cursor to line begin
                    printf(">");
                    printf(CmdString);
                    printf("\x0D\033["); // Put cursor to location
                    printf("%x", CmdCharCnt + 1);
                    printf("C");
                }
            }
            AnsiEscapeState = 0;
        }
    }
    else if (recd == 0x0D) // Enter ?
    {
        if (CmdString[0] != 0) // Check if command is not empty
        {
            if(CmdString[0]!='!')   // If not an history recall
            {
                if (!CmpStrEqu(CmdHistory[0], CmdString)) // Different from first history entry ?
                {
                    for (unsigned char HistLine = CMD_HIST_SIZE - 1; HistLine > 0; HistLine--) // Push history
                        for (unsigned char i = 0; i < CMD_LENGTH; i++)
                            CmdHistory[HistLine][i] = CmdHistory[HistLine - 1][i];
                    // CmdString[CmdCharCnt] = 0;  // Mark end of string
                    for (unsigned char i = 0; i < CMD_LENGTH; i++) // Save to history
                        CmdHistory[0][i] = CmdString[i];
                }
            }
        }
        CmdHistSel = 0;
        CmdStringFlag = true;
    }
    else
    {
        CmdString[CmdCharCnt] = recd;
        // UART_SendChar(recd);
        printf("%c", recd);
        CmdCharCnt += 1;
    }
}

// Copy the command at position indicated in parameter from the the history list
void CpHistToCmd(unsigned char HistPos)
{
    char OneChar;
    bool EndOfString = false;
    for (unsigned char i = 0; i < sizeof(CmdString); i++) // copy history to current cmd
    {
        OneChar = CmdHistory[HistPos][i];
        CmdString[i] = OneChar;
        if (OneChar == 0)
        {
            EndOfString = true;
        }
        if (!EndOfString)
        {
            printf("%c", OneChar);
            CmdCharCnt += 1;
        }
    }
}

// Initialize the list of command to be stored in the History buffer
void InitHistory(void)
{
    // Put some commands in history
    MyCpyStrToArray(&CmdHistory[14][0], "HF1-23 1");
    MyCpyStrToArray(&CmdHistory[13][0], "nackhpdlow 0");
    MyCpyStrToArray(&CmdHistory[12][0], "nackhpdlow 1");
    MyCpyStrToArray(&CmdHistory[11][0], "edidwritec");
    MyCpyStrToArray(&CmdHistory[10][0], "edidclear");
    MyCpyStrToArray(&CmdHistory[9][0], "cls");
    MyCpyStrToArray(&CmdHistory[8][0], "HdmiPowerCheck");
    MyCpyStrToArray(&CmdHistory[7][0], "edidquantumize");
    MyCpyStrToArray(&CmdHistory[6][0], "edidselect");
    MyCpyStrToArray(&CmdHistory[5][0], "edidtest");
    MyCpyStrToArray(&CmdHistory[4][0], "ediddump");
    MyCpyStrToArray(&CmdHistory[3][0], "hpdpulse");
    MyCpyStrToArray(&CmdHistory[2][0], "edidload 0");
    MyCpyStrToArray(&CmdHistory[1][0], "edidlist");
    MyCpyStrToArray(&CmdHistory[0][0], "edidload 13");
}

// Compare if two strings are dentical
bool CmpStrEqu(char *CmdString, char *CmdToCompare)
{
    unsigned char OneChar;
    bool Identical = true;
    for (unsigned char i = 0; i < CMD_LENGTH; i++) // Dump all character in command string
    {
        OneChar = CmdToCompare[i];
        if (OneChar == 0)
            break;
        else if (OneChar != CmdString[i])
        {
            Identical = false;
            break;
        }
    }
    return Identical;
}

// Clear Command string
void ClearCmd(void)     
{
    for (unsigned char i = 0; i < sizeof(CmdString); i++)
    {
        CmdString[i] = 0;
    }
}

// Place the cursor to the specified column position
void GoToCol(uint8_t ColumnPos)
{
    //printf("\x0D\033[20C");   // example to go to column 20
    printf("\x0D\033["); 
    printf("%d",ColumnPos);
    printf("C");

    // printf("\x1b[");     // Move cursor to location CSI n C
    // UART_SendInt(CmdCharCnt+1);
    // printf("%x", CmdCharCnt + 1);
    // putchar(CmdCharCnt+1);
    // printf("C");
}

