#include "syscall.h"

int main() {
    PrintString("Before sleep\n");
    Sleep(3);
    PrintString("After sleep\n");
    Exit(0);
}