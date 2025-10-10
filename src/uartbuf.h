
#include <stdint.h>
#include <stdbool.h>
#define UartBufSize 32

void UartBufWr(char);
char UartBufRd();
bool UartBufIsEmpty();
void UartBufClear();


