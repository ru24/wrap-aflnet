// faults.h
#ifndef FAULTS_H
#define FAULTS_H

#include <stdbool.h>

enum Mode { 
  RECORD_mode, 
  INJECTION_mode
};

typedef struct {
    int fid;
    float probability;
    bool is_fi;
    unsigned int response_code;
} Fault;

// 各入力に対して関連するFaultを管理する構造体
typedef struct {
    int current_size; // faults配列の現在のサイズ
    enum Mode current_mode; // has_faultsのmode切り替え
    Fault faults[];   // 入力に関連するFault配列
} InputFaults;

extern InputFaults *input_faults; // 入力ごとのFaults管理リスト
extern Fault *faults;

extern bool record_mode; 

void RFList_set(int fid);
bool has_fault(int FID);
void cleanup(void);
void set_response_code_ftp(const void *buf, size_t len);
void setup_shared_memory();

#endif // FAULTS_H
