#include <tchar.h>
#include <windows.h>
#include <dbghelp.h>
#include <stdio.h>

#pragma comment(lib, "dbghelp.lib")

void CallStackPrint(FILE* fd)
{
    HANDLE          hProcess;
    void*           stack[100];
    unsigned int    i;
    unsigned short  frames;
    char            buffer[sizeof(SYMBOL_INFO) + 256 * sizeof(TCHAR)];
    SYMBOL_INFO*    symbol = (SYMBOL_INFO*)&buffer[0];
    
    hProcess = GetCurrentProcess();
    
    SymInitialize(hProcess, NULL, TRUE);
    
    frames = CaptureStackBackTrace( 0, 100, stack, NULL );
    symbol->MaxNameLen = 255;
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
    if (NULL == fd)
        fd = stdout;
    
    for (i = 0; i < frames; ++i){
        if(SymFromAddr(hProcess, (DWORD64)stack[i], 0, symbol)){
            _ftprintf(fd, _T("%2d: %-64s - 0x%08llx\n"), i, symbol->Name, symbol->Address);
        }
    }
}