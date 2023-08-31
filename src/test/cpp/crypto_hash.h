#pragma once
#define ASCON_PRINTSTATE
#include "api.h"
#include "ascon.h"
#include "crypto_hash.h"
#include "permutations.h"
#include "printstate.h"

#include <inttypes.h>
#include <stdio.h>

void printword(const char* text, const word_t x) {
  printf("%s=%016" PRIx64 "\n", text, WORDTOU64(x));
}

void printstate(const char* text, const state_t* s) {
  printf("%s:\n", text);
  printword("  x0", s->x0);
  printword("  x1", s->x1);
  printword("  x2", s->x2);
  printword("  x3", s->x3);
  printword("  x4", s->x4);
}

int crypto_xof(uint64_t* out, const uint64_t* in,
                unsigned long long len) {
  /* initialize */
  state_t s;
  s.x0 = ASCON_XOF_IV;
  s.x1 = 0;
  s.x2 = 0;
  s.x3 = 0;
  s.x4 = 0;
  P12(&s);
  printstate("initialization", &s);

  /* absorb full plaintext blocks */
  while (len >= ASCON_HASH_RATE) {
    s.x0 ^= in[0];
    P12(&s);
    in++;
    len -= ASCON_HASH_RATE;
  }
  /* absorb final plaintext block */
  s.x0 ^= in[0];
  s.x0 ^= PAD(len);
  P12(&s);
  printstate("absorb plaintext", &s);

  /* squeeze full output blocks */
  len = CRYPTO_BYTES;
  while (len > ASCON_HASH_RATE) {
    out[0] = s.x0;
    P12(&s);
    out++;
    len -= ASCON_HASH_RATE;
  }
  /* squeeze final output block */
  out[0] = s.x0;
  printstate("squeeze output", &s);

  return 0;
}