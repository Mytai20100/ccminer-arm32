/*
ARM NEON implementation of Verus CLHash
*/

#include "verus_clhash_neon.h"
#include "haraka_neon.h"
#include <stdlib.h>
#include <string.h>

int __cpuverusoptimized = 0x80;

static inline uint8x16_t mulhrs_epi16_neon(uint8x16_t a, uint8x16_t b) {
    int16x8_t a_lo = vreinterpretq_s16_u8(a);
    int16x8_t b_lo = vreinterpretq_s16_u8(b);
    int32x4_t prod_lo = vmull_s16(vget_low_s16(a_lo), vget_low_s16(b_lo));
    int32x4_t prod_hi = vmull_s16(vget_high_s16(a_lo), vget_high_s16(b_lo));
    prod_lo = vaddq_s32(prod_lo, vdupq_n_s32(0x4000));
    prod_hi = vaddq_s32(prod_hi, vdupq_n_s32(0x4000));
    int16x4_t result_lo = vqshrn_n_s32(prod_lo, 15);
    int16x4_t result_hi = vqshrn_n_s32(prod_hi, 15);
    return vreinterpretq_u8_s16(vcombine_s16(result_lo, result_hi));
}

u128_neon __verusclmulwithoutreduction64alignedrepeatv2_2_neon(
    u128_neon *randomsource, const u128_neon buf[4], uint64_t keyMask,
    uint32_t *fixrand, uint32_t *fixrandex, u128_neon *g_prand, u128_neon *g_prandex)
{
    const u128_neon pbuf_copy[4] = {
        veorq_u8(buf[0], buf[2]),
        veorq_u8(buf[1], buf[3]),
        buf[2], buf[3]
    };
    const u128_neon *pbuf;
    u128_neon acc = vld1q_u8((uint8_t *)(randomsource + (keyMask + 2)));
    
    for (int64_t i = 0; i < 32; i++) {
        const uint64_t selector = vgetq_lane_u64(vreinterpretq_u64_u8(acc), 0);
        uint32_t prand_idx = (selector >> 5) & keyMask;
        uint32_t prandex_idx = (selector >> 32) & keyMask;
        u128_neon *prand = randomsource + prand_idx;
        u128_neon *prandex = randomsource + prandex_idx;
        pbuf = pbuf_copy + (selector & 3);
        vst1q_u8((uint8_t *)&g_prand[i], prand[0]);
        vst1q_u8((uint8_t *)&g_prandex[i], prandex[0]);
        fixrand[i] = prand_idx;
        fixrandex[i] = prandex_idx;
        
        switch (selector & 0x1c) {
        case 0: {
            const u128_neon temp1 = vld1q_u8((uint8_t *)prandex);
            const u128_neon temp2 = pbuf[(selector & 1) ? -1 : 1];
            const u128_neon add1 = veorq_u8(temp1, temp2);
            const uint64x2_t clprod1 = clmul_neon(vreinterpretq_u64_u8(add1), vreinterpretq_u64_u8(add1), 0x10);
            acc = veorq_u8(vreinterpretq_u8_u64(clprod1), acc);
            const u128_neon tempa1 = mulhrs_epi16_neon(acc, temp1);
            const u128_neon tempa2 = veorq_u8(tempa1, temp1);
            const u128_neon temp12 = vld1q_u8((uint8_t *)prand);
            vst1q_u8((uint8_t *)prand, tempa2);
            const u128_neon temp22 = vld1q_u8((uint8_t *)pbuf);
            const u128_neon add12 = veorq_u8(temp12, temp22);
            const uint64x2_t clprod12 = clmul_neon(vreinterpretq_u64_u8(add12), vreinterpretq_u64_u8(add12), 0x10);
            acc = veorq_u8(vreinterpretq_u8_u64(clprod12), acc);
            const u128_neon tempb1 = mulhrs_epi16_neon(acc, temp12);
            const u128_neon tempb2 = veorq_u8(tempb1, temp12);
            vst1q_u8((uint8_t *)prandex, tempb2);
            break;
        }
        case 4: {
            const u128_neon temp1 = vld1q_u8((uint8_t *)prand);
            const u128_neon temp2 = vld1q_u8((uint8_t *)pbuf);
            const u128_neon add1 = veorq_u8(temp1, temp2);
            const uint64x2_t clprod1 = clmul_neon(vreinterpretq_u64_u8(add1), vreinterpretq_u64_u8(add1), 0x10);
            acc = veorq_u8(vreinterpretq_u8_u64(clprod1), acc);
            const uint64x2_t clprod2 = clmul_neon(vreinterpretq_u64_u8(temp2), vreinterpretq_u64_u8(temp2), 0x10);
            acc = veorq_u8(vreinterpretq_u8_u64(clprod2), acc);
            const u128_neon tempa1 = mulhrs_epi16_neon(acc, temp1);
            const u128_neon tempa2 = veorq_u8(tempa1, temp1);
            const u128_neon temp12 = vld1q_u8((uint8_t *)prandex);
            vst1q_u8((uint8_t *)prandex, tempa2);
            const u128_neon temp22 = pbuf[(selector & 1) ? -1 : 1];
            const u128_neon add12 = veorq_u8(temp12, temp22);
            acc = veorq_u8(add12, acc);
            const u128_neon tempb1 = mulhrs_epi16_neon(acc, temp12);
            vst1q_u8((uint8_t *)prand, veorq_u8(tempb1, temp12));
            break;
        }
        case 8: {
            const u128_neon temp1 = vld1q_u8((uint8_t *)prandex);
            const u128_neon temp2 = vld1q_u8((uint8_t *)pbuf);
            const u128_neon add1 = veorq_u8(temp1, temp2);
            acc = veorq_u8(add1, acc);
            const u128_neon tempa1 = mulhrs_epi16_neon(acc, temp1);
            const u128_neon tempa2 = veorq_u8(tempa1, temp1);
            const u128_neon temp12 = vld1q_u8((uint8_t *)prand);
            vst1q_u8((uint8_t *)prand, tempa2);
            const u128_neon temp22 = pbuf[(selector & 1) ? -1 : 1];
            const u128_neon add12 = veorq_u8(temp12, temp22);
            const uint64x2_t clprod12 = clmul_neon(vreinterpretq_u64_u8(add12), vreinterpretq_u64_u8(add12), 0x10);
            acc = veorq_u8(vreinterpretq_u8_u64(clprod12), acc);
            const uint64x2_t clprod22 = clmul_neon(vreinterpretq_u64_u8(temp22), vreinterpretq_u64_u8(temp22), 0x10);
            acc = veorq_u8(vreinterpretq_u8_u64(clprod22), acc);
            const u128_neon tempb1 = mulhrs_epi16_neon(acc, temp12);
            const u128_neon tempb2 = veorq_u8(tempb1, temp12);
            vst1q_u8((uint8_t *)prandex, tempb2);
            break;
        }
        case 0xc: {
            const u128_neon temp1 = vld1q_u8((uint8_t *)prand);
            const u128_neon temp2 = pbuf[(selector & 1) ? -1 : 1];
            const u128_neon add1 = veorq_u8(temp1, temp2);
            const int32_t divisor = (uint32_t)selector;
            acc = veorq_u8(add1, acc);
            const int64_t dividend = vgetq_lane_u64(vreinterpretq_u64_u8(acc), 0);
            const u128_neon modulo = vreinterpretq_u8_u32(vsetq_lane_u32(dividend % divisor, vdupq_n_u32(0), 0));
            acc = veorq_u8(modulo, acc);
            const u128_neon tempa1 = mulhrs_epi16_neon(acc, temp1);
            const u128_neon tempa2 = veorq_u8(tempa1, temp1);
            if (dividend & 1) {
                const u128_neon temp12 = vld1q_u8((uint8_t *)prandex);
                vst1q_u8((uint8_t *)prandex, tempa2);
                const u128_neon temp22 = vld1q_u8((uint8_t *)pbuf);
                const u128_neon add12 = veorq_u8(temp12, temp22);
                const uint64x2_t clprod12 = clmul_neon(vreinterpretq_u64_u8(add12), vreinterpretq_u64_u8(add12), 0x10);
                acc = veorq_u8(vreinterpretq_u8_u64(clprod12), acc);
                const uint64x2_t clprod22 = clmul_neon(vreinterpretq_u64_u8(temp22), vreinterpretq_u64_u8(temp22), 0x10);
                acc = veorq_u8(vreinterpretq_u8_u64(clprod22), acc);
                const u128_neon tempb1 = mulhrs_epi16_neon(acc, temp12);
                const u128_neon tempb2 = veorq_u8(tempb1, temp12);
                vst1q_u8((uint8_t *)prand, tempb2);
            } else {
                vst1q_u8((uint8_t *)prand, vld1q_u8((uint8_t *)prandex));
                vst1q_u8((uint8_t *)prandex, tempa2);
                acc = veorq_u8(vld1q_u8((uint8_t *)pbuf), acc);
            }
            break;
        }
        case 0x10: {
            u128_neon temp1 = pbuf[(selector & 1) ? -1 : 1];
            u128_neon temp2 = vld1q_u8((uint8_t *)pbuf);
            AES2_NEON(temp1, temp2, 0);
            MIX2_NEON(temp1, temp2);
            AES2_NEON(temp1, temp2, 4);
            MIX2_NEON(temp1, temp2);
            AES2_NEON(temp1, temp2, 8);
            MIX2_NEON(temp1, temp2);
            acc = veorq_u8(temp2, veorq_u8(temp1, acc));
            const u128_neon tempa1 = vld1q_u8((uint8_t *)prand);
            const u128_neon tempa2 = mulhrs_epi16_neon(acc, tempa1);
            vst1q_u8((uint8_t *)prand, vld1q_u8((uint8_t *)prandex));
            vst1q_u8((uint8_t *)prandex, veorq_u8(tempa1, tempa2));
            break;
        }
        case 0x14: {
            const u128_neon *buftmp = &pbuf[(selector & 1) ? -1 : 1];
            uint64_t rounds = selector >> 61;
            u128_neon *rc = prand;
            uint64_t aesroundoffset = 0;
            u128_neon onekey;
            do {
                if (selector & (((uint64_t)0x10000000) << rounds)) {
                    const u128_neon temp2 = vld1q_u8((uint8_t *)(rounds & 1 ? pbuf : buftmp));
                    const u128_neon add1 = veorq_u8(rc[0], temp2);
                    rc++;
                    const uint64x2_t clprod1 = clmul_neon(vreinterpretq_u64_u8(add1), vreinterpretq_u64_u8(add1), 0x10);
                    acc = veorq_u8(vreinterpretq_u8_u64(clprod1), acc);
                } else {
                    onekey = vld1q_u8((uint8_t *)rc++);
                    u128_neon temp2 = vld1q_u8((uint8_t *)(rounds & 1 ? buftmp : pbuf));
                    AES2_NEON(onekey, temp2, aesroundoffset);
                    aesroundoffset += 4;
                    MIX2_NEON(onekey, temp2);
                    acc = veorq_u8(onekey, acc);
                    acc = veorq_u8(temp2, acc);
                }
            } while (rounds--);
            const u128_neon tempa1 = vld1q_u8((uint8_t *)prand);
            const u128_neon tempa2 = mulhrs_epi16_neon(acc, tempa1);
            const u128_neon tempa3 = veorq_u8(tempa1, tempa2);
            const u128_neon tempa4 = vld1q_u8((uint8_t *)prandex);
            vst1q_u8((uint8_t *)prandex, tempa3);
            vst1q_u8((uint8_t *)prand, tempa4);
            break;
        }
        case 0x18: {
            const u128_neon temp1 = vld1q_u8((uint8_t *)prand);
            const u128_neon temp2 = pbuf[(selector & 1) ? -1 : 1];
            const uint64x2_t add1 = vreinterpretq_u64_u8(veorq_u8(temp1, temp2));
            uint32_t divisor = (uint32_t)selector;
            const uint64x2_t clprod1 = clmul_neon(add1, add1, 0x10);
            acc = veorq_u8(vreinterpretq_u8_u64(clprod1), acc);
            const int64_t dividend = vgetq_lane_u64(vreinterpretq_u64_u8(acc), 0);
            const int64_t modulo = dividend % divisor;
            acc = veorq_u8(vreinterpretq_u8_u64(vcombine_u64(vcreate_u64(modulo), vcreate_u64(0))), acc);
            const u128_neon tempa1 = vld1q_u8((uint8_t *)prandex);
            const u128_neon tempa2 = mulhrs_epi16_neon(acc, tempa1);
            const u128_neon tempa3 = veorq_u8(tempa1, tempa2);
            const u128_neon tempa4 = vld1q_u8((uint8_t *)pbuf);
            vst1q_u8((uint8_t *)prandex, tempa3);
            vst1q_u8((uint8_t *)prand, tempa4);
            break;
        }
        case 0x1c: {
            const u128_neon temp1 = vld1q_u8((uint8_t *)prand);
            const u128_neon temp2 = vld1q_u8((uint8_t *)prandex);
            const u128_neon add1 = veorq_u8(temp1, temp2);
            const uint64x2_t clprod1 = clmul_neon(vreinterpretq_u64_u8(add1), vreinterpretq_u64_u8(add1), 0x10);
            const u128_neon clprod2 = vld1q_u8((uint8_t *)pbuf);
            acc = veorq_u8(vreinterpretq_u8_u64(clprod1), acc);
            acc = veorq_u8(clprod2, acc);
            const u128_neon tempa1 = mulhrs_epi16_neon(acc, temp1);
            const u128_neon tempa2 = veorq_u8(tempa1, temp1);
            vst1q_u8((uint8_t *)prand, tempa2);
            const u128_neon tempb1 = mulhrs_epi16_neon(acc, temp2);
            const u128_neon tempb2 = veorq_u8(tempb1, temp2);
            vst1q_u8((uint8_t *)prandex, tempb2);
            break;
        }
        }
    }
    return acc;
}
/* Helper functions */
u128_neon lazyLengthHash_neon(uint64_t keylength, uint64_t length) {
    /* Create vector with keylength and length */
    uint64x2_t lengthvector = vcombine_u64(
        vcreate_u64(length),
        vcreate_u64(keylength)
    );
    /* Carryless multiply lengthvector with itself */
    uint64x2_t clprod1 = clmul_neon(lengthvector, lengthvector, 0x10);
    return vreinterpretq_u8_u64(clprod1);
}

u128_neon precompReduction64_si128_neon(u128_neon A) {
    /* Modulo reduction to 64-bit value */
    const u128_neon C = vreinterpretq_u8_u64(vcombine_u64(
        vcreate_u64((1U << 4) + (1U << 3) + (1U << 1) + (1U << 0)),
        vcreate_u64(0)
    ));
    
    uint64x2_t A64 = vreinterpretq_u64_u8(A);
    uint64x2_t C64 = vreinterpretq_u64_u8(C);
    uint64x2_t Q2_64 = clmul_neon(A64, C64, 0x01);
    u128_neon Q2 = vreinterpretq_u8_u64(Q2_64);
    
    /* Shuffle operation - table lookup manually */
    const uint8_t shuf_tbl[16] = {
        0, 27, 54, 45, 108, 119, 90, 65, 216, 195, 238, 245, 180, 175, 130, 153
    };
    
    /* Extract high 64 bits of Q2 */
    uint8_t Q2_high_bytes[16];
    vst1q_u8(Q2_high_bytes, Q2);
    
    /* Perform shuffle manually on upper 8 bytes */
    uint8_t Q3_bytes[16] = {0};
    for (int i = 0; i < 8; i++) {
        uint8_t idx = Q2_high_bytes[i + 8]; /* Upper 8 bytes only */
        if (idx < 16) {
            Q3_bytes[i] = shuf_tbl[idx];
        }
    }
    u128_neon Q3 = vld1q_u8(Q3_bytes);
    
    u128_neon Q4 = veorq_u8(Q2, A);
    u128_neon final = veorq_u8(Q3, Q4);
    return final;
}

uint64_t precompReduction64_neon(u128_neon A) {
    u128_neon result = precompReduction64_si128_neon(A);
    return vgetq_lane_u64(vreinterpretq_u64_u8(result), 0);
}

/* Generic interface wrapper */
uint64_t verusclhashv2_2(void * random, const unsigned char buf[64], uint64_t keyMask, uint32_t *fixrand, uint32_t *fixrandex,
	void *g_prand, void *g_prandex) {
    u128_neon acc = __verusclmulwithoutreduction64alignedrepeatv2_2_neon(
        (u128_neon *)random, (const u128_neon *)buf, keyMask, fixrand, fixrandex, 
        (u128_neon *)g_prand, (u128_neon *)g_prandex);
    acc = veorq_u8(acc, lazyLengthHash_neon(1024, 64));
    return precompReduction64_neon(acc);
}