#include "./types.h"

EXTERNC

uint32_t* wg_read_spv(const char* str, uint32_t* out_buffer_length);
void wg_free_spv(uint32_t* spv);

EXTERNC_END
