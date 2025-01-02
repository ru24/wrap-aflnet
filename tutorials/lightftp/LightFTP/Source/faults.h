// faults.h
#ifndef FAULTS_H
#define FAULTS_H

#include <stdbool.h>
#include "../../../../shared_defs.h"

extern InputFaults *input_faults; // 入力ごとのFaults管理リスト
extern Fault *faults;

extern bool record_mode; 

void RFList_set(int fid);
bool has_fault(int FID);
void cleanup(void);
void set_response_code_ftp(const void *buf, size_t len);
void setup_shared_memory();

#endif // FAULTS_H
