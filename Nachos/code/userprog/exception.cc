// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "main.h"
#include "syscall.h"
#include "ksyscall.h"
#include "synchconsole.h"
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
// If you are handling a system call, don't forget to increment the pc
// before returning. (Or else you'll loop making the same system call forever!)
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	is in machine.h.
//----------------------------------------------------------------------




void
ExceptionHandler(ExceptionType which)
{
    int type = kernel->machine->ReadRegister(2);

    DEBUG(dbgSys, "Received Exception " << which << " type: " << type << "\n");

    switch (which) {
    case SyscallException:
      switch(type) {
      case SC_Halt:
	DEBUG(dbgSys, "Shutdown, initiated by user program.\n");

	SysHalt();

	ASSERTNOTREACHED();
	break;

      case SC_Add:
	DEBUG(dbgSys, "Add " << kernel->machine->ReadRegister(4) << " + " << kernel->machine->ReadRegister(5) << "\n");
	
	/* Process SysAdd Systemcall*/
	int result;
	result = SysAdd(/* int op1 */(int)kernel->machine->ReadRegister(4),
			/* int op2 */(int)kernel->machine->ReadRegister(5));

	DEBUG(dbgSys, "Add returning with " << result << "\n");
	/* Prepare Result */
	kernel->machine->WriteRegister(2, (int)result);
	
	/* Modify return point */
	{
	  /* set previous programm counter (debugging only)*/
	  kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

	  /* set programm counter to next instruction (all Instructions are 4 byte wide)*/
	  kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
	  
	  /* set next programm counter for brach execution */
	  kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
	}

	return;
	
	ASSERTNOTREACHED();

	break;
/*Exception call for Write */
  case SC_Write:
        DEBUG(dbgSys, "Write" << ", " <<kernel->machine->ReadRegister(4) << ", " << kernel->machine->ReadRegister(5) << ", " << kernel->machine->ReadRegister(6));
        char wcount; //word count
        int wsize, wbuffer, ws, wopenfileId; // word size, word buffer, word size and openfile id
        SynchConsoleOutput *p;
        wbuffer = (int)kernel->machine->ReadRegister(4); //reading from the register
        wsize = (int)kernel->machine->ReadRegister(5); //reading from the register
        wopenfileId = (int)kernel->machine->ReadRegister(6); //reading from the register
        if(wopenfileId == 1) 
                {
                p = kernel->synchConsoleOut;
                for(ws = 0; ws<wsize ; ws++) // Assigning the input value from the console to the main memory using a for loop
                        {
                                wcount = kernel->machine->mainMemory[wbuffer+ws];
                                p->PutChar(wcount);
                        }
                }
        kernel->machine->WriteRegister(2,ws); //writing to 2 register
                {
                kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));
                kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg)+4);
                kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
                }
        return;
        ASSERTNOTREACHED();
        break;  
	//Exception handling of SC_Read
	case SC_Read:
	DEBUG(dbgSys, "Read" << ", " <<kernel->machine->ReadRegister(4) << ", " << kernel->machine->ReadRegister(5) << ", " << kernel->machine->ReadRegister(6));
	char readc;
	int readsize, readbuff, reads, readopenfileId;
	SynchConsoleInput *pr;
	
	readbuff = (int)kernel->machine->ReadRegister(4); //Read from register
	readsize = (int)kernel->machine->ReadRegister(5); //Read from register
	readopenfileId = (int)kernel->machine->ReadRegister(6); //Read from register
	if(readopenfileId == 0) //if the open file id == 0 
		{
		pr = kernel->synchConsoleIn; //take input from console
		for(reads = 0; reads<readsize ; reads++) //for loop for reading from the main memory
			{
				readc = pr->GetChar();
				kernel->machine->mainMemory[readbuff+reads]=readc;
				if(readc == '\n') { break ; };
			}
		DEBUG(dbgSys, "Read: openfileid = 0, Read from console \n"); //debug statement if the syscall is handled
		}
	else
		{
		DEBUG(dbgSys, "Read: openfileid = 0, Read from non-console not handled\n"); //debug statement if the syscall is not handled
		}
	kernel->machine->WriteRegister(2,reads); //Write to 2 register
		{
		kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));
		kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg)+4);
		kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
		}
	return;
	ASSERTNOTREACHED();
	break;


// Implementing System call for Fork
	case SC_SysFork:
		
	{	
	Thread* thread; // assigning a thread element
	thread = new Thread("Fork"); 
	AddrSpace* space = new AddrSpace(); //assiging a new address space
	thread->space = space;
	kernel->machine->WriteRegister(2,0); // Writing to the 2 register
	thread->SaveUserState(); // saving the user state of the thread
	  {
         /* set previous programm counter (debugging only)*/
         kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));
         /* set programm counter to next instruction (all Instructions are 4 byte wide)*/
         kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
         /* set next programm counter for brach execution */
         kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
        }
	
	return;
	ASSERTNOTREACHED();
	}
	break;				




// Implementing the System call for Exec
case SC_Exec:
                 DEBUG(dbgSys, "Exec" << " \n");
        {
                char execBuffer[80], temp=1;
                int i;
                for (i=0; (i<80) && (temp!='\0') ;i++)
              		{
                         temp = kernel->machine->mainMemory[kernel->machine->ReadRegister(4)+i]; 
                         execBuffer[i]= temp;
                        }
               execBuffer[i] = '\0';
                delete kernel->currentThread->space;
                kernel->currentThread->space = new AddrSpace ;
                kernel->currentThread->space->Load(execBuffer);
                kernel->currentThread->space->RestoreState(); // set the kernel page table
        {
         /* set previous programm counter (debugging only)*/
         kernel->machine->WriteRegister(PrevPCReg, 0);
         /* set programm counter to next instruction (all Instructions are 4 byte wide)*/
         kernel->machine->WriteRegister(PCReg, 4);
         /* set next programm counter for brach execution */
         kernel->machine->WriteRegister(NextPCReg, 4);
        }
         kernel->currentThread->space->Execute();
        }
         return;
         ASSERTNOTREACHED();
         break;




//Exception Handling for Exit
case SC_Exit:
         DEBUG(dbgSys, "Exit Called" << " \n");
        {
         int exitstat = (int) kernel->machine->ReadRegister(4); // exit status
         kernel->stats->Print(); //printing the status of the process
         kernel->currentThread->Finish(); //Finishing the current thread to exit the process
        {
         /* set previous programm counter (debugging only)*/
         kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));
         /* set programm counter to next instruction (all Instructions are 4 byte wide)*/
         kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
         /* set next programm counter for brach execution */
         kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
        }
        kernel->machine->WriteRegister(2, exitstat);//Writing to the register to exit the process
        }
         return;
         ASSERTNOTREACHED();
         break;
		
      default:
	cerr << "Unexpected system call " << type << "\n";
	break;
      } 
      break;
    default:
      cerr << "Unexpected user mode exception" << (int)which << "\n";
      break;
    }
    ASSERTNOTREACHED();
}
