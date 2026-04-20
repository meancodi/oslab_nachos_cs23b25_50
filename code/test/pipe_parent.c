#include "syscall.h"
#define stdout 1

int main() {
    int parentFd, childFd;
    int pid;
    char msg[] = "Hello from parent!";

    Pipe(&parentFd, &childFd);

    pid = ExecP("../test/pipe_child", childFd);
    if (pid < 0) {
        PrintString("ExecP failed\n");
        Exit(-1);
    }

    pipeWrite(parentFd, msg, 18);

    Join(pid);
    Exit(0);
}