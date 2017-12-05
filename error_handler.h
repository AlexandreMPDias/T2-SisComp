#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

void EH_signal(int signum, sighandler_t handler);
pid_t EH_fork();
FILE* EH_fopen(const char* path, const char *desc);

#endif
