#ifndef _WG_UNIFORM
#define _WG_UNIFORM

#include "./types.h"

typedef struct wg_uniform_chain_t {
  wg_sp_t* sp;
  void* chain;
  uint32_t byte_size;
} wg_uniform_chain_t;

int wg_create_uniform(wg_wingine_t* wing, wg_uniform_chain_t* chain, uint32_t byte_size);
int wg_set_current_uniform(wg_uniform_chain_t* chain, void *data);
int wg_destroy_uniform(wg_uniform_chain_t* chain);

#endif
