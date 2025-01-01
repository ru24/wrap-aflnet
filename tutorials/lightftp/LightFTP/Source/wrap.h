#ifndef WRAP_H
#define WRAP_H

#include <sys/types.h>  // ssize_t のために追加

extern void *__real_malloc(size_t size);
extern ssize_t __real_recv(int sockfd, void *buf, size_t len, int flags);  // ssize_t に合わせて修正
ssize_t __wrap_send(int sockfd, const void *buf, size_t len, int flags);

#endif // WRAP_H

