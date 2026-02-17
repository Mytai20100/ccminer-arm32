/*
ARM NEON implementation of Verus CLHash
*/

#ifndef VERUS_CLHASH_NEON_H_
#define VERUS_CLHASH_NEON_H_

#include <arm_neon.h>
#include <stdint.h>

typedef uint8x16_t u128_neon;
typedef u128_neon u128;

// Carryless multiplication using PMULL
#ifdef __ARM_FEATURE_CRYPTO

static inline uint64x2_t clmul_neon(uint64x2_t a, uint64x2_t b, int imm) {
    poly64x2_t a_poly = vreinterpretq_p64_u64(a);
    poly64x2_t b_poly = vreinterpretq_p64_u64(b);
    poly64_t a_elem = vgetq_lane_p64(a_poly, (imm & 0x01) ? 1 : 0);
    poly64_t b_elem = vgetq_lane_p64(b_poly, (imm & 0x10) ? 1 : 0);
    poly128_t result = vmull_p64(a_elem, b_elem);
    return vreinterpretq_u64_p128(result);
}

#else

static inline uint64x2_t clmul_neon(uint64x2_t a, uint64x2_t b, int imm) {
    uint64_t a_val = (imm & 0x01) ? vgetq_lane_u64(a, 1) : vgetq_lane_u64(a, 0);
    uint64_t b_val = (imm & 0x10) ? vgetq_lane_u64(b, 1) : vgetq_lane_u64(b, 0);
    /* Carryless (GF2) multiply: each bit i of a XORs b<<i into the 128-bit result.
     * When i==0, b<<0 contributes only to the low word.
     * When i>0,  b<<i may overflow into the high word: the overflow is b>>(64-i). */
    uint64_t lo = 0, hi = 0;
    for (int i = 0; i < 64; i++) {
        if (a_val & (1ULL << i)) {
            lo ^= b_val << i;
            if (i > 0) hi ^= b_val >> (64 - i);
        }
    }
    return vcombine_u64(vcreate_u64(lo), vcreate_u64(hi));
}

#endif

extern int __cpuverusoptimized;

u128_neon __verusclmulwithoutreduction64alignedrepeatv2_2_neon(
    u128_neon *randomsource, const u128_neon buf[4], uint64_t keyMask,
    uint32_t *fixrand, uint32_t *fixrandex, u128_neon *g_prand, u128_neon *g_prandex);

/* Generic interface */
uint64_t verusclhashv2_2(void * random, const unsigned char buf[64], uint64_t keyMask, uint32_t *fixrand, uint32_t *fixrandex,
	void *g_prand, void *g_prandex);

#endif /* VERUS_CLHASH_NEON_H_ */
