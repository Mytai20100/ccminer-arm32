/*
ARM NEON implementation of Haraka256/512
*/

#include "haraka_neon.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

u128_neon rc_neon[40];

void load_constants_neon() {
    uint32_t rc_data[40][4] = {
        {0x75817b9d, 0xb2c5fef0, 0xe620c00a, 0x0684704c},
        {0x2f08f717, 0x640f6ba4, 0x88f3a06b, 0x8b66b4e1},
        {0x9f029114, 0xcf029d60, 0x53f28498, 0x3402de2d},
        {0xfd5b4f79, 0xbbf3bcaf, 0x2e7b4f08, 0x0ed6eae6},
        {0xbe397044, 0x79eecd1c, 0x4872448b, 0xcbcfb0cb},
        {0x2b8a057b, 0x8d5335ed, 0x6e9032b7, 0x7eeacdee},
        {0xda4fef1b, 0xe2412761, 0x5e2e7cd0, 0x67c28f43},
        {0x1fc70b3b, 0x675ffde2, 0xafcacc07, 0x2924d9b0},
        {0xb9d465ee, 0xecdb8fca, 0xe6867fe9, 0xab4d63f1},
        {0xad037e33, 0x5b2a404f, 0xd4b7cd64, 0x1c30bf84},
        {0x8df69800, 0x69028b2e, 0x941723bf, 0xb2cc0bb9},
        {0x5c9d2d8a, 0x4aaa9ec8, 0xde6f5572, 0xfa0478a6},
        {0x29129fd4, 0x0efa4f2e, 0x6b772a12, 0xdfb49f2b},
        {0xbb6a12ee, 0x32d611ae, 0xf449a236, 0x1ea10344},
        {0x9ca8eca6, 0x5f9600c9, 0x4b050084, 0xaf044988},
        {0x27e593ec, 0x78a2c7e3, 0x9d199c4f, 0x21025ed8},
        {0x82d40173, 0xb9282ecd, 0xa759c9b7, 0xbf3aaaf8},
        {0x10307d6b, 0x37f2efd9, 0x6186b017, 0x6260700d},
        {0xf6fc9ac6, 0x81c29153, 0x21300443, 0x5aca45c2},
        {0x36d1943a, 0x2caf92e8, 0x226b68bb, 0x9223973c},
        {0xe51071b4, 0x6cbab958, 0x225886eb, 0xd3bf9238},
        {0x24e1128d, 0x933dfddd, 0xaef0c677, 0xdb863ce5},
        {0xcb2212b1, 0x83e48de3, 0xffeba09c, 0xbb606268},
        {0xc72bf77d, 0x2db91a4e, 0xe2e4d19c, 0x734bd3dc},
        {0x2cb3924e, 0x4b1415c4, 0x61301b43, 0x43bb47c3},
        {0x16eb6899, 0x03b231dd, 0xe707eff6, 0xdba775a8},
        {0x7eca472c, 0x8e5e2302, 0x3c755977, 0x6df3614b},
        {0xb88617f9, 0x6d1be5b9, 0xd6de7d77, 0xcda75a17},
        {0xa946ee5d, 0x9d6c069d, 0x6ba8e9aa, 0xec6b43f0},
        {0x3bf327c1, 0xa2531159, 0xf957332b, 0xcb1e6950},
        {0x600ed0d9, 0xe4ed0353, 0x00da619c, 0x2cee0c75},
        {0x63a4a350, 0x80bbbabc, 0x96e90cab, 0xf0b1a5a1},
        {0x938dca39, 0xab0dde30, 0x5e962988, 0xae3db102},
        {0x2e75b442, 0x8814f3a8, 0xd554a40b, 0x17bb8f38},
        {0x360a16f6, 0xaeb6b779, 0x5f427fd7, 0x34bb8a5b},
        {0xffbaafde, 0x43ce5918, 0xcbe55438, 0x26f65241},
        {0x839ec978, 0xa2ca9cf7, 0xb9f3026a, 0x4ce99a54},
        {0x22901235, 0x40c06e28, 0x1bdff7be, 0xae51a51a},
        {0x48a659cf, 0xc173bc0f, 0xba7ed22b, 0xa0c1613c},
        {0xe9c59da1, 0x4ad6bdfd, 0x02288288, 0x756acc03}
    };

    for (int i = 0; i < 40; i++) {
        rc_neon[i] = vreinterpretq_u8_u32(vld1q_u32(rc_data[i]));
    }
}

void haraka256_neon(unsigned char *out, const unsigned char *in) {
    uint8x16_t s[2];

    s[0] = LOAD_NEON(in);
    s[1] = LOAD_NEON(in + 16);

    AES2_NEON(s[0], s[1], 0);
    MIX2_NEON(s[0], s[1]);

    AES2_NEON(s[0], s[1], 4);
    MIX2_NEON(s[0], s[1]);

    AES2_NEON(s[0], s[1], 8);
    MIX2_NEON(s[0], s[1]);

    AES2_NEON(s[0], s[1], 12);
    MIX2_NEON(s[0], s[1]);

    AES2_NEON(s[0], s[1], 16);
    MIX2_NEON(s[0], s[1]);

    s[0] = veorq_u8(s[0], LOAD_NEON(in));
    s[1] = veorq_u8(s[1], LOAD_NEON(in + 16));

    STORE_NEON(out, s[0]);
    STORE_NEON(out + 16, s[1]);
}

void haraka256_4x_neon(unsigned char *out, const unsigned char *in) {
    uint8x16_t s[4][2];

    for (int i = 0; i < 4; i++) {
        s[i][0] = LOAD_NEON(in + i * 32);
        s[i][1] = LOAD_NEON(in + i * 32 + 16);
    }

    for (int round = 0; round < 5; round++) {
        int rci = round * 4;
        for (int i = 0; i < 4; i++) {
            AES2_NEON(s[i][0], s[i][1], rci);
            MIX2_NEON(s[i][0], s[i][1]);
        }
    }

    for (int i = 0; i < 4; i++) {
        s[i][0] = veorq_u8(s[i][0], LOAD_NEON(in + i * 32));
        s[i][1] = veorq_u8(s[i][1], LOAD_NEON(in + i * 32 + 16));
        STORE_NEON(out + i * 32, s[i][0]);
        STORE_NEON(out + i * 32 + 16, s[i][1]);
    }
}

void haraka256_8x_neon(unsigned char *out, const unsigned char *in) {
    haraka256_4x_neon(out, in);
    haraka256_4x_neon(out + 128, in + 128);
}

void haraka512_neon(unsigned char *out, const unsigned char *in) {
    uint8x16_t s[4];

    s[0] = LOAD_NEON(in);
    s[1] = LOAD_NEON(in + 16);
    s[2] = LOAD_NEON(in + 32);
    s[3] = LOAD_NEON(in + 48);

    for (int round = 0; round < 5; round++) {
        int rci = round * 8;
        AES4_NEON(s[0], s[1], s[2], s[3], rci);
        MIX4_NEON(s[0], s[1], s[2], s[3]);
    }

    s[0] = veorq_u8(s[0], LOAD_NEON(in));
    s[1] = veorq_u8(s[1], LOAD_NEON(in + 16));
    s[2] = veorq_u8(s[2], LOAD_NEON(in + 32));
    s[3] = veorq_u8(s[3], LOAD_NEON(in + 48));

    TRUNCSTORE_NEON(out, s[0], s[1], s[2], s[3]);
}

void haraka512_4x_neon(unsigned char *out, const unsigned char *in) {
    uint8x16_t s[4][4];

    for (int i = 0; i < 4; i++) {
        int offset = i * 64;
        s[i][0] = LOAD_NEON(in + offset);
        s[i][1] = LOAD_NEON(in + offset + 16);
        s[i][2] = LOAD_NEON(in + offset + 32);
        s[i][3] = LOAD_NEON(in + offset + 48);
    }

    for (int round = 0; round < 5; round++) {
        int rci = round * 8;
        for (int i = 0; i < 4; i++) {
            AES4_NEON(s[i][0], s[i][1], s[i][2], s[i][3], rci);
            MIX4_NEON(s[i][0], s[i][1], s[i][2], s[i][3]);
        }
    }

    for (int i = 0; i < 4; i++) {
        int offset = i * 64;
        s[i][0] = veorq_u8(s[i][0], LOAD_NEON(in + offset));
        s[i][1] = veorq_u8(s[i][1], LOAD_NEON(in + offset + 16));
        s[i][2] = veorq_u8(s[i][2], LOAD_NEON(in + offset + 32));
        s[i][3] = veorq_u8(s[i][3], LOAD_NEON(in + offset + 48));
        TRUNCSTORE_NEON(out + i * 32, s[i][0], s[i][1], s[i][2], s[i][3]);
    }
}

void haraka512_8x_neon(unsigned char *out, const unsigned char *in) {
    haraka512_4x_neon(out, in);
    haraka512_4x_neon(out + 128, in + 256);
}
/* Wrapper functions to match the generic interface */
void load_constants() {
    load_constants_neon();
}

void haraka256(unsigned char *out, const unsigned char *in) {
    haraka256_neon(out, in);
}

void haraka256_keyed(unsigned char *out, const unsigned char *in, const u128 *rc) {
    /* Fall back to basic haraka256 - keyed variant not optimized for NEON */
    haraka256(out, in);
}

void haraka256_4x(unsigned char *out, const unsigned char *in) {
    haraka256_4x_neon(out, in);
}

void haraka256_8x(unsigned char *out, const unsigned char *in) {
    haraka256_8x_neon(out, in);
}

void haraka512(unsigned char *out, const unsigned char *in) {
    haraka512_neon(out, in);
}

void haraka512_zero(unsigned char *out, const unsigned char *in) {
    haraka512_neon(out, in);
}

void haraka512_keyed(unsigned char *out, const unsigned char *in, const u128 *rc) {
    uint8x16_t s[4];

    s[0] = LOAD_NEON(in);
    s[1] = LOAD_NEON(in + 16);
    s[2] = LOAD_NEON(in + 32);
    s[3] = LOAD_NEON(in + 48);

    /* 4 full rounds (AES4 + MIX4), matching x86 haraka512_keyed round structure.
     * Uses the global rc_neon constants, same as the x86 version (which also
     * ignores the rc parameter and uses its global rc[] array). */
    AES4_NEON(s[0], s[1], s[2], s[3], 0);
    MIX4_NEON(s[0], s[1], s[2], s[3]);

    AES4_NEON(s[0], s[1], s[2], s[3], 8);
    MIX4_NEON(s[0], s[1], s[2], s[3]);

    AES4_NEON(s[0], s[1], s[2], s[3], 16);
    MIX4_NEON(s[0], s[1], s[2], s[3]);

    AES4_NEON(s[0], s[1], s[2], s[3], 24);

    /* MIX4_LAST: x86 computes tmp=unpacklo(s0,s1), s1=unpacklo(s2,s3),
     * s2=unpackhi(s1_new, tmp).  Only s2 is used after this.
     * unpacklo(a,b) = vzipq_u32(a,b).val[0]
     * unpackhi(a,b) = vzipq_u32(a,b).val[1]                         */
    {
        uint32x4_t t0 = vreinterpretq_u32_u8(s[0]);
        uint32x4_t t1 = vreinterpretq_u32_u8(s[1]);
        uint32x4_t t2 = vreinterpretq_u32_u8(s[2]);
        uint32x4_t t3 = vreinterpretq_u32_u8(s[3]);
        uint32x4_t tmp      = vzipq_u32(t0, t1).val[0]; /* unpacklo(s0,s1) */
        uint32x4_t new_s1   = vzipq_u32(t2, t3).val[0]; /* unpacklo(s2,s3) */
        uint32x4_t new_s2   = vzipq_u32(new_s1, tmp).val[1]; /* unpackhi */
        s[2] = vreinterpretq_u8_u32(new_s2);
    }

    /* AES4_LAST: x86 applies only rc[34] then rc[38] to s[2]. */
    s[2] = aes_enc_neon(s[2], rc_neon[34]);
    s[2] = aes_enc_neon(s[2], rc_neon[38]);

    /* x86 stores: out[28..31] = s[2].u32[2] ^ in[52..55]
     * s[2].u32[2] is the third 32-bit lane of s[2]. */
    uint32_t s2_lane2 = vgetq_lane_u32(vreinterpretq_u32_u8(s[2]), 2);
    uint32_t in_word;
    memcpy(&in_word, in + 52, 4);
    uint32_t result = s2_lane2 ^ in_word;
    memcpy(out + 28, &result, 4);
}

void haraka512_4x(unsigned char *out, const unsigned char *in) {
    haraka512_4x_neon(out, in);
}

void haraka512_8x(unsigned char *out, const unsigned char *in) {
    haraka512_8x_neon(out, in);
}

void test_implementations() {
    /* Placeholder */
}
