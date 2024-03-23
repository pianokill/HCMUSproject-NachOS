#include "syscall.h"
int main()
{
    //int result = Sub(40, 23); 
    //int result = ReadInt(); d
    //PrintInt(4567810); e
    //char c = ReadChar(); h
    //PrintChar('a'); i

    char buffer[100];
    //ReadString(buffer, 100);
    buffer[0] = 'h';
    buffer[1] = 'i';

    PrintString(buffer);
    Halt();
}