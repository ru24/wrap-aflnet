#ifndef SHARED_DEFS_H
#define SHARED_DEFS_H

#include <stdint.h> // 標準的な固定幅整数型を使用する

// u8 型の定義
typedef uint8_t u8;

extern const char *SEM_MODE_NAME;
extern const char *SEM_FAULTS_NAME;
extern const char *SEM_MUTATED_NAME;

extern const char *SHM_NAME;
extern const char *SHM_NAME_MUTATED;
extern const char *SHM_NAME_MODE;

// サイズ定義
#define ENTRY_SIZE 300
#define ENTRY_SIZE_MUTATED 300
#define SHM_SIZE sizeof(InputFaults)
#define SHM_SIZE_MUTATED sizeof(InputFaults)
#define SHM_SIZE_MODE sizeof(enum Mode)

// 構造体定義
enum Mode {
  RECORD_mode,
  INJECTION_mode
};

typedef struct {
  int fid;
  float probability;
  u8 is_fi; // 修正後もこのまま使用可能
  unsigned int response_code;
} Fault;

typedef struct {
  int current_size;    // faults配列の現在のサイズ
  enum Mode current_mode; // モードの切り替え
  Fault faults[ENTRY_SIZE_MUTATED];      // 入力に関連するFault配列
} InputFaults;

// 共有メモリを初期化, リセット
void initialize_input_faults(InputFaults* input_faults);

#endif // SHARED_DEFS_H

