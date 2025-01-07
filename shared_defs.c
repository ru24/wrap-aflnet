#include "shared_defs.h"
#include <string.h> // memsetを使用するため

const char *SEM_MODE_NAME = "/sem_mode";
const char *SEM_FAULTS_NAME = "/sem_faults";
const char *SEM_MUTATED_NAME = "/sem_mutated";

const char *SHM_NAME = "/shared_faults_memory";
const char *SHM_NAME_MUTATED = "/shared_mutated_faults";
const char *SHM_NAME_MODE = "/shared_mode";

void initialize_input_faults(InputFaults* input_faults) {
    memset(input_faults, 0, sizeof(InputFaults));
}
