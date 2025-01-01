#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "faults.h"
#include "wrap.h"

#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

// 共有メモリのサイズ
#define SHM_NAME "/shared_faults_memory"

#define ENTRY_SIZE 300
#define SHM_SIZE sizeof(InputFaults) + sizeof(Fault) * ENTRY_SIZE // 30個のFaultを格納するサイズ

#define SHM_NAME_MUTATED "/shared_mutated_faults"
#define ENTRY_SIZE_MUTATED 300
#define SHM_SIZE_MUTATED sizeof(InputFaults) + sizeof(Fault) * ENTRY_SIZE_MUTATED

/* SFIのmode選択 */
#define SHM_NAME_MODE "/shared_mode"
#define SHM_SIZE_MODE sizeof(enum Mode)

enum Mode* SFI_mode = NULL; // SHMから取得するモード

InputFaults *input_faults = NULL; // 入力ごとのFaults管理リスト

InputFaults *input_faults_mutated; // mutate後の入力. テスト対象の変異後の入力での実行時の参照用

bool record_mode = true;

// has_faults関数のINJECTION_mode使用するグローバル変数
static bool* flags = NULL;  // flags配列を初期化
static int prev_size = 0;   // 前回のサイズを記録

unsigned int response_code = 0;

/* AFLNetにおける状態(レスポンスコード)をSFIに対応付けるための抽出 */
void set_response_code_ftp(const void *buf, size_t len) {
    // データ長が短い場合はスキップ
    if (len < 4) {
        return;
    }
    
    // バッファ解析
    const char terminator[2] = {0x0D, 0x0A};  // CRLF
    const unsigned char *buffer = (const unsigned char *)buf;
    
   size_t mem_count = 0;  // 処理済みバッファのカウント
    char mem[1024] = {0};  // 一時的なバッファ

    for (size_t byte_count = 0; byte_count < len; byte_count++) {
        // バッファに現在の文字を追加
        mem[mem_count++] = buffer[byte_count];

        // CRLF を検出
        if ((mem_count > 1) && (memcmp(&mem[mem_count - 2], terminator, 2) == 0)) {
            // レスポンスコード形式を確認
            if ((mem_count > 4) && 
                (mem[3] == ' ' || 
                 (isdigit(buffer[byte_count]) && memcmp(&mem[0], &buffer[byte_count], 3) != 0))) {
                
                unsigned int new_response_code = 
                    (mem[0] - '0') * 100 + 
                    (mem[1] - '0') * 10 + 
                    (mem[2] - '0');

                if (new_response_code != response_code) {
                    response_code = new_response_code;
                }
                return;  // 最初のレスポンスコードのみ処理
            }
        }
    }
}

enum Mode* setup_mode_shared_memory() {
  int shm_fd = shm_open(SHM_NAME_MODE, O_CREAT | O_RDWR, 0666);
  if (shm_fd == -1) {
    perror("shm_open failed for Mode");
    return NULL;
  }

  if (ftruncate(shm_fd, SHM_SIZE_MODE) == -1) {
    perror("ftruncate failed for Mode");
    close(shm_fd);
    return NULL;
  }

  enum Mode* mode_ptr = (enum Mode*)mmap(NULL, SHM_SIZE_MODE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
  if (mode_ptr == MAP_FAILED) {
    perror("mmap failed for Mode");
    close(shm_fd);
    return NULL;
  }

  close(shm_fd);  // ファイルディスクリプタを閉じる
  fprintf(stderr, "Shared memory for Mode initialized.\n");

  return mode_ptr;
}

void initialize_sfi_mode() {
    SFI_mode = setup_mode_shared_memory();
    if (SFI_mode == NULL) {
        fprintf(stderr, "Failed to initialize SFI_mode shared memory.\n");
        exit(1);
    }
}

void RFList_set(int FID) {

  if (input_faults->current_size < ENTRY_SIZE) {  // 配列の最大サイズに達していない場合
    input_faults->faults[input_faults->current_size].fid = FID;
    input_faults->faults[input_faults->current_size].probability = 0.5;
    input_faults->faults[input_faults->current_size].is_fi = false;
    input_faults->faults[input_faults->current_size].response_code = response_code;
    input_faults->current_size++;
    printf("Added FID: %d, Current size: %d\n", FID, input_faults->current_size);  // デバッグログ

    // メモリの同期
    if (msync(input_faults, SHM_SIZE, MS_SYNC) == -1) {
      perror("msync failed");
      exit(1);
    }

  } else {
    // 最大数に達している場合は警告を表示
    fprintf(stderr, "Fault list is full.\n");
  }
}

void setup_shared_memory() {

  fprintf(stderr, "Entering setup_shared_memory...\n");
    // 共有メモリの作成
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
      perror("shm_open failed");
      exit(1);
  }
  
  fprintf(stderr, "shm_open succeeded: shm_fd=%d\n", shm_fd);

    // 共有メモリのサイズを設定
  if (ftruncate(shm_fd, SHM_SIZE) == -1) {
    perror("ftruncate failed");
    exit(1);
  }
  
  fprintf(stderr, "ftruncate succeeded: size=%d\n", SHM_SIZE);

  // 共有メモリをマッピング
  input_faults = (InputFaults *)mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
  
  if (input_faults == MAP_FAILED) {
    perror("mmap failed");
    exit(1);
  }

  fprintf(stderr, "mmap succeeded: input_faults=%p\n", (void *)input_faults);

  // current_size のみを初期化
  input_faults->current_size = 0;
  input_faults->current_mode = RECORD_mode;

  // faults 配列の初期化は個別に実施
  for (int i = 0; i < ENTRY_SIZE; i++) {
    input_faults->faults[i].fid = 0;
    input_faults->faults[i].probability = 0.0;
    input_faults->faults[i].is_fi = false;
    input_faults->faults[input_faults->current_size].response_code = 0;
  }
  
  fprintf(stderr, "Shared memory initialized.\n");
}

void setup_mutated_shared_memory_target() {
    int shm_fd = shm_open("/shared_mutated_faults", O_RDWR, 0666); // 作成フラグ不要
    if (shm_fd == -1) {
        perror("shm_open failed for mutated faults");
        exit(1);
    }

    input_faults_mutated = (InputFaults *)mmap(NULL, SHM_SIZE_MUTATED, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (input_faults_mutated == MAP_FAILED) {
        perror("mmap failed for mutated faults");
        close(shm_fd);
        exit(1);
    }

    close(shm_fd);
    fprintf(stderr, "Shared memory for mutated faults attached by target.\n");
}

bool has_fault(int FID) { 

  // 共有メモリが未初期化の場合は初期化
  
  // SFI_modeが未初期化の場合、初期化
  if (SFI_mode == NULL) {
    initialize_sfi_mode();
  }

  if (input_faults == NULL) {
    fprintf(stderr, "Shared memory not initialized. Initializing now...\n");
    setup_shared_memory();
    *SFI_mode = RECORD_mode; // RECORD_modeに設定
    if (input_faults == NULL) {
      fprintf(stderr, "Failed to initialize shared memory.\n");
      exit(1);
    }
  }
  
  // 記載モード, FIDのシーケンスを収集
  if (*SFI_mode == RECORD_mode) {
    RFList_set(FID);
    return false;
  }

  // ファザー側でSFI_modeを切り替え
  // flags配列の初期化またはサイズ変更時の再初期化
  if (flags == NULL || prev_size != input_faults->current_size) {
    free(flags);  // 古いメモリを解放
    flags = (bool*)calloc(input_faults->current_size, sizeof(bool));
    if (flags == NULL) {
      fprintf(stderr, "Failed to allocate memory for flags.\n");
      exit(1);
    }
      prev_size = input_faults->current_size;
  }

  for (int i = 0; i < input_faults->current_size; i++) {
    // 一度参照したiは二度目以降は参照しないように
    // 既に参照された場合はスキップ
    if (flags[i]) {
      continue;
    }

    if (input_faults->faults[i].fid == FID) {
      // ファザーの変異後のシーケンスからSFIするか判断
      if (input_faults->faults[i].is_fi == true) {
        return true;
      } else {
        return false;
      }
    }
  }
  return false;  // 見つからない場合、故障は発生しない
}

// プログラム終了時にメモリ解放
void cleanup() {
  if (input_faults != NULL) {
    munmap(input_faults, SHM_SIZE);  // 共有メモリを解放
  }
}

