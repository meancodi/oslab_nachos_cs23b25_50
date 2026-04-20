#include "syscall.h"
#define stdout 1

int main() {
    int fd;
    char buf[32];
    int n;

    fd = GetPD();

    n = pipeRead(fd, buf, 18);
    if (n < 0) {
        PrintString("pipeRead failed\n");
        Exit(-1);
    }
    buf[n] = '\0';

    PrintString("Child received: ");
    PrintString(buf);
    PrintString("\n");

    Exit(0);
}