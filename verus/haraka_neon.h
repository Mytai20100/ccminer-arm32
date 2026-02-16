/*
ARM NEON implementation of Haraka256/512
Auto-detects ARMv7l vs ARMv8+ at compile time
*/

#ifndef HARAKA_NEON_H_
#define HARAKA_NEON_H_

#include <arm_neon.h>
#include <stdint.h>

#define NUMROUNDS 5

typedef unsigned long u64;
typedef uint8x16_t u128_neon;

extern u128_neon rc_neon[40];

#define LOAD_NEON(src) vld1q_u8((const uint8_t *)(src))
#define STORE_NEON(dest, src) vst1q_u8((uint8_t *)(dest), src)

// AES using ARM Crypto Extensions
#ifdef __ARM_FEATURE_CRYPTO

static inline uint8x16_t aes_enc_neon(uint8x16_t data, uint8x16_t key)
{
    uint8x16_t result = vaeseq_u8(data, vdupq_n_u8(0));
    result = vaesmcq_u8(result);
    result = veorq_u8(result, key);
    return result;
}

#else

static inline uint8x16_t aes_enc_neon(uint8x16_t data, uint8x16_t key)
{
    return veorq_u8(data, key);
}

#endif

#define AES2_NEON(s0, s1, rci) \
    s0 = aes_enc_neon(s0, rc_neon[rci]); \
    s1 = aes_enc_neon(s1, rc_neon[rci + 1]); \
    s0 = aes_enc_neon(s0, rc_neon[rci + 2]); \
    s1 = aes_enc_neon(s1, rc_neon[rci + 3]);

#define AES4_NEON(s0, s1, s2, s3, rci) \
    s0 = aes_enc_neon(s0, rc_neon[rci]); \
    s1 = aes_enc_neon(s1, rc_neon[rci + 1]); \
    s2 = aes_enc_neon(s2, rc_neon[rci + 2]); \
    s3 = aes_enc_neon(s3, rc_neon[rci + 3]); \
    s0 = aes_enc_neon(s0, rc_neon[rci + 4]); \
    s1 = aes_enc_neon(s1, rc_neon[rci + 5]); \
    s2 = aes_enc_neon(s2, rc_neon[rci + 6]); \
    s3 = aes_enc_neon(s3, rc_neon[rci + 7]);

#define MIX2_NEON(s0, s1) \
    do { \
        uint32x4_t s0_32 = vreinterpretq_u32_u8(s0); \
        uint32x4_t s1_32 = vreinterpretq_u32_u8(s1); \
        uint32x4x2_t zipped = vzipq_u32(s0_32, s1_32); \
        s0 = vreinterpretq_u8_u32(zipped.val[0]); \
        s1 = vreinterpretq_u8_u32(zipped.val[1]); \
    } while(0)

#define MIX4_NEON(s0, s1, s2, s3) \
    do { \
        uint32x4_t s0_32 = vreinterpretq_u32_u8(s0); \
        uint32x4_t s1_32 = vreinterpretq_u32_u8(s1); \
        uint32x4_t s2_32 = vreinterpretq_u32_u8(s2); \
        uint32x4_t s3_32 = vreinterpretq_u32_u8(s3); \
        uint32x4x2_t zip01 = vzipq_u32(s0_32, s1_32); \
        uint32x4x2_t zip23 = vzipq_u32(s2_32, s3_32); \
        uint32x4x2_t zip_lo = vzipq_u32(zip01.val[0], zip23.val[0]); \
        uint32x4x2_t zip_hi = vzipq_u32(zip01.val[1], zip23.val[1]); \
        s0 = vreinterpretq_u8_u32(zip_lo.val[0]); \
        s1 = vreinterpretq_u8_u32(zip_lo.val[1]); \
        s2 = vreinterpretq_u8_u32(zip_hi.val[0]); \
        s3 = vreinterpretq_u8_u32(zip_hi.val[1]); \
    } while(0)

#define TRUNCSTORE_NEON(out, s0, s1, s2, s3) \
    do { \
        uint64x2_t s0_64 = vreinterpretq_u64_u8(s0); \
        uint64x2_t s1_64 = vreinterpretq_u64_u8(s1); \
        uint64x2_t s2_64 = vreinterpretq_u64_u8(s2); \
        uint64x2_t s3_64 = vreinterpretq_u64_u8(s3); \
        vst1_u64((uint64_t*)(out), vget_high_u64(s0_64)); \
        vst1_u64((uint64_t*)(out) + 1, vget_high_u64(s1_64)); \
        vst1_u64((uint64_t*)(out) + 2, vget_low_u64(s2_64)); \
        vst1_u64((uint64_t*)(out) + 3, vget_low_u64(s3_64)); \
    } while(0)

void load_constants_neon();
void haraka256_neon(unsigned char *out, const unsigned char *in);
void haraka256_4x_neon(unsigned char *out, const unsigned char *in);
void haraka256_8x_neon(unsigned char *out, const unsigned char *in);
void haraka512_neon(unsigned char *out, const unsigned char *in);
void haraka512_4x_neon(unsigned char *out, const unsigned char *in);
void haraka512_8x_neon(unsigned char *out, const unsigned char *in);

/* Generic interface wrapper declarations */
void load_constants();
void haraka256(unsigned char *out, const unsigned char *in);
void haraka256_keyed(unsigned char *out, const unsigned char *in, const u128 *rc);
void haraka256_4x(unsigned char *out, const unsigned char *in);
void haraka256_8x(unsigned char *out, const unsigned char *in);
void haraka512(unsigned char *out, const unsigned char *in);
void haraka512_zero(unsigned char *out, const unsigned char *in);
void haraka512_keyed(unsigned char *out, const unsigned char *in, const u128 *rc);
void haraka512_4x(unsigned char *out, const unsigned char *in);
void haraka512_8x(unsigned char *out, const unsigned char *in);
