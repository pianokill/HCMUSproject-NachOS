#include "syscall.h"

int main(){
    char buffer[100];
    ReadString(buffer, 100);
    PrintString(buffer);
    Halt();
}