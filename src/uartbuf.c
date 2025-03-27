#include "uartbuf.h"

char UartBuf[UartBufSize];

uint8_t UartBufWrAdd = 0;
uint8_t UartBufRdAdd = 0;

uint8_t UartAddDiff;

void UartBufWr(char Data)
{
   UartAddDiff = UartBufRdAdd - UartBufWrAdd;
   // Check buffer is already not overflow
   if( ((UartAddDiff == 1) || ((UartBufWrAdd == (UartBufSize-1)) && (UartBufRdAdd == 0))) == false )
   {
      // Check buffer is almost FULL
      if( (UartAddDiff == 2) ||
       ((UartBufWrAdd == (UartBufSize-2)) && (UartBufRdAdd == 0)) ||
       ((UartBufWrAdd == (UartBufSize-1)) && (UartBufRdAdd == 1)) )
      { 
          //UartBuf[UartBufWrAdd] = BuffCode_OVERFLOW;
      }
      else
         UartBuf[UartBufWrAdd] = Data;
      
      if( UartBufWrAdd == (UartBufSize-1) )
         UartBufWrAdd = 0;
      else
         UartBufWrAdd += 1;
   }
}

char UartBufRd()
{
   char Data;
   Data = UartBuf[UartBufRdAdd];
   if( UartBufRdAdd == (UartBufSize-1) )
      UartBufRdAdd = 0;
   else
      UartBufRdAdd += 1;
   return Data;
}

////inline bool UartBufIsEmpty()  // inline does not appear to speed up still use venner?
//static inline bool UartBufIsEmpty()  // run in ram to try to speed up
bool UartBufIsEmpty()
{
   if( UartBufWrAdd != UartBufRdAdd )
      return false;
   else
      return true;
}

void UartBufClear()
{
   UartBufWrAdd = 0;
   UartBufRdAdd = 0;
}

