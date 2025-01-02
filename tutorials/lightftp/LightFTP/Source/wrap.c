#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "faults.h"  // has_fault 関数を使用するためにインクルード1:
#include "wrap.h" // __real_malloc の宣言を追加


extern void *__real_malloc (size_t);
static char buf [1024];

extern ssize_t __real_recv(int sockfd, void *buf, size_t len, int flags);
static char log_buf[1024];

// send() の実際の関数ポインタ
extern ssize_t __real_send(int sockfd, const void *buf, size_t len, int flags);

void *
__wrap_malloc (size_t size)
{
    void *ptr = __real_malloc (size);
    
    if (has_fault(1)) { // mallocのFIDは1と設定する。
      return NULL;
    }
    return ptr;
}

ssize_t __wrap_recv(int sockfd, void *buf, size_t len, int flags) {
    // 本来の recv を呼び出す
    ssize_t ret = __real_recv(sockfd, buf, len, flags);

    if (has_fault(2)){
    // printf は内部で malloc を呼び出す可能性があるため使用しない
    return -1;
  }
    return ret;
}

ssize_t __wrap_send(int sockfd, const void *buf, size_t len, int flags) {
    // 送信データのレスポンスコードを解析
    set_response_code_ftp(buf, len);

    // 実際の send() をロード
    ssize_t ret = __real_send(sockfd, buf, len, flags);
    if (has_fault(3)){
    return -1; 
  }
    // 実際の send() を呼び出す
    return ret;
}
