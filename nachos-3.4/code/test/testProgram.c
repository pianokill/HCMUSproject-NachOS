#include "syscall.h"

int main()
{
    OpenFileId op; int t;
    char buffer[1024];
    //int c = ReadInt();
    //PrintInt(c);
    //Create("son dep trai.txt");
    op = Open("son dep trai.txt", 2);
    t = Read(buffer, 1024, op);
    Close(op);

    Halt();
}
