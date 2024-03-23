#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include "thread.h"
#include "synch.h"
#include "addrspace.h"
#include "utility.h"
#define MAX_SIZE 100

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------

char* User2System(int virtAddr,int limit)
{
    int i;// index
    int oneChar;
    char* kernelBuf = NULL;
    kernelBuf = new char[limit +1];//need for terminal string
    if (kernelBuf == NULL)
    return kernelBuf;
    memset(kernelBuf,0,limit+1);
    //printf("\n Filename u2s:");
    for (i = 0 ; i < limit ;i++)
    {
        machine->ReadMem(virtAddr+i,1,&oneChar);
        kernelBuf[i] = (char)oneChar;
        //printf("%c",kernelBuf[i]);
        if (oneChar == 0)
            break;
    }
    return kernelBuf;
}

int System2User(int virtAddr,int len,char* buffer)
{
    if (len < 0) return -1;
    if (len == 0)return len;
    int i = 0;
    int oneChar = 0 ;
    do{
        oneChar= (int) buffer[i];
        machine->WriteMem(virtAddr+i,1,oneChar);
        i ++;
    }while(i < len && oneChar != 0);
    return i;
}

void syscallSub(){
    int op1 = machine->ReadRegister(4);
    int op2 = machine->ReadRegister(5);
    int result = op1 - op2;
    machine->WriteRegister(2, result);
    interrupt->Halt();
}

void syscallReadInt(){
    //khai bao mang kieu char de chua
    char* buffer = new char[MAX_SIZE];
    synchconsole = new SynchConsole();
    //goi ham read cua synconsol
    int numberBytesRead = synchconsole->Read(buffer, MAX_SIZE); //55 -10 0    

    if(numberBytesRead == -1){
        printf("\n Cannot read the integer");
        delete synchconsole;
        delete[]buffer; 
        return;
    }

    //goi ham system2user de chep bien tu kernel space ve user space

    //xu ly chuoi nhap vao-->so nguyen
    int numberIndex = 1;
    char sig = buffer[0];
    int result = 0;
    for(int i = numberIndex; i < numberBytesRead; i++){
        if(buffer[i] >= '0' && buffer[i] <= '9'){
            result = result + (buffer[i] - '0') *  10; //power(10, numberBytesRead - i - 1);
        }
        else{
            printf("\nInvalid digit");
            delete[]buffer; 
            delete synchconsole;
            return;
         }
    }
    //ghi kq so nguyen vao thanh ghi so 2 tra ve
    if(sig == '-'){
        result = result * (-1);
    }
    machine->WriteRegister(2, result);
    //printf("%d\n", numberBytesRead);
    delete[]buffer; //leak memory
    delete synchconsole;
    return;
}


void syscallPrintInt(){
    synchconsole = new SynchConsole();
    int number = machine->ReadRegister(4);

    if(number == 0){
        synchconsole->Write("0", 1);
        //tang pc
        delete synchconsole;
        return;
    }
    //Xu ly chuoi so
    bool isNegative = false;
    int numberIndex = 1;
    int numberSize = 0;
    if(number < 0){
        isNegative = true;
        number = number * (-1);
    }

    int temp = number;
    while(temp > 0){
        numberSize++;
        temp = temp / 10;
    }

    char* buffer = new char[MAX_SIZE + numberSize];
    temp = number; //55 -10 100 -450
   
	if (isNegative) {
		buffer[0] = '-';
		numberSize = numberSize + 1;
	}

	for (int i = numberSize - 1; i >= 0; i--) {
		if (isNegative && i == 0) break;
		int k = temp % 10;
		buffer[i] = char(k) + '0';
		temp = temp / 10;
	}

	buffer[numberSize] = 0;

    synchconsole->Write(buffer, numberSize + 1);
    //printf("Chuoi nhap la: %s\n", buffer);
    //leak memory
    delete[]buffer;
    delete synchconsole;
    return;
    
}

void syscallReadChar(){
    synchconsole = new SynchConsole();
    int maxBytes = 255;
    char* buffer = new char[maxBytes];
    int numBytesRead = 1; //synchconsole->Read(buffer, maxBytes);

    if(numBytesRead > 1){
        printf("Only 1 character input");
        DEBUG('a', "\nERROR: Only 1 character input!!!");
        machine->WriteRegister(2, 0);
    }

    else if(numBytesRead == 0){
        printf("Empty character");
        DEBUG('a', "\nERROR: Empty character!!!");
        machine->WriteRegister(2, 0);
    }

    else{
        char c = buffer[0];
        machine->WriteRegister(2, c);
    }
    delete[] buffer;
    delete synchconsole;
    return;
}


void syscallPrintChar(){
    synchconsole = new SynchConsole();
    char c = (char)machine->ReadRegister(4);
    synchconsole->Write(&c, 1);
    delete synchconsole;
    return;
}


void syscallReadString(){
    synchconsole = new SynchConsole();
    int virtAddr, len;
    char* buffer;
    virtAddr = machine->ReadRegister(4);
    len = machine->ReadRegister(5);
    buffer = new char[len + 1];

    buffer = User2System(virtAddr, len);
    int numberBytesRead = synchconsole->Read(buffer, len);
    if(numberBytesRead < 0){
        delete[] buffer;
        delete synchconsole;
        printf("Empty string");
        return;
    }

    System2User(virtAddr, len, buffer);
    delete[]buffer;
    delete synchconsole;
    return;
}


void syscallPrintString(){
    int virtAddr;
    int maxBytes = 255;
    char* buffer;
    synchconsole = new SynchConsole();

    virtAddr = machine->ReadRegister(4);
    buffer = User2System(virtAddr, maxBytes);
    int len = 0;

    while(buffer[len] != 0) len++;
    synchconsole->Write(buffer, len);
    delete[] buffer;
    delete synchconsole;

    return;
}
void AdvancePC() {
    int pcAfter = machine->registers[NextPCReg] + 4;
    machine->registers[PrevPCReg] = machine->registers[PCReg];
    machine->registers[PCReg] = machine->registers[NextPCReg];
    machine->registers[NextPCReg] = pcAfter;
}
void ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);

    switch(which) {
        case NoException:
            interrupt->Halt();
            return;
        case PageFaultException:
            DEBUG('a', "\n No valid translation found");
            printf("\n\n No valid translation found");
            interrupt->Halt();
            break;
        case ReadOnlyException:
            DEBUG('A',"\n Write attempted to page marked read-only");
            printf("\n\n Write attempted to page marke read-only");
            interrupt->Halt();
            break;
        case BusErrorException:
            DEBUG('a', "\n Translation resulted invalid physical address");
            printf("\n\n Translation resulted invalid physical address");
            interrupt->Halt();
            break;
        case AddressErrorException:
            DEBUG('a', "\n Unaligned reference or one that was beyond the end of the address space");
            printf("\n\n Unaligned reference or one that was beyond the end of the address space");
            interrupt->Halt();
            break;
        case IllegalInstrException:
            DEBUG('a', "\n Unimplemented or reserved instr.");
            printf("\n\n Unimplemented or rrvedion");
            interrupt->Halt();
            break;
        case NumExceptionTypes:
            DEBUG('a', "\n Number exception types");
            printf("\n\n Number exception types");
            interrupt->Halt();
            break;
        case SyscallException:
        {
            switch(type)
            {
                case SC_Halt:
                    DEBUG('a', "Shutdown, initiated by user program.\n");
                    interrupt->Halt();
                    return;
                case SC_Sub:
                    syscallSub();
                    break;
                case SC_ReadInt:
                    syscallReadInt();
                    break;
                case SC_PrintInt:
                    syscallPrintInt();
                    break;
                case SC_ReadChar:
                    syscallReadChar();
                    //tang pc
                    break;
                case SC_PrintChar:
                    syscallPrintChar();
                    //tang pc
                    break;
                
                case SC_ReadString:
                    syscallReadString();
                    //tang pc
                    break;

                case SC_PrintString:
                    syscallPrintString();
                    //tang pc
                    break;
            }
            AdvancePC();
            break;
        }

    }
    return;
    // if ((which == SyscallException) && (type == SC_Halt)) {
	// DEBUG('a', "Shutdown, initiated by user program.\n");
   	// interrupt->Halt();
    // }
    // else {
	// printf("Unexpected user mode exception %d %d\n", which, type);
	// ASSERT(FALSE);
    // }
}
