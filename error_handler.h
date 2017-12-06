#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

void EH_signal(int signum, sighandler_t handler);
pid_t EH_fork();
FILE* EH_fopen(const char* path, const char *desc);
int EH_shmget(key_t,size_t size, int flags);
void* EH_shmat(int shmid, const void *shmaddr, int shmflg);

#endif
