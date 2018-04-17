#ifndef INC_BIT_H
#define INC_BIT_H

/* count leading zero of an unsigned int */
/* __builtin_clz does not specify result of __builtin_clz(0) */
#define st_bit_clz(i) ({                                                      \
        __auto_type clz_i_ = (i);                                             \
        (clz_i_) == 0 ? sizeof(clz_i_) * 8 : st_bit_clz_positive_int_(clz_i_);\
})


#define st_bit_clz_positive_int_(i) _Generic(                                 \
        (i),                                                                  \
        unsigned int:       __builtin_clz,                                    \
        unsigned long:      __builtin_clzl,                                   \
        unsigned long long: __builtin_clzll                                   \
)(i)


/**
 * most significant bit:
 *     0b00 -- -1,
 *     0b01 -- 0,
 *     0b11 -- 1
 * for i>0, msb satisfies: 2^i == 1 << msb(i)
 */
#define st_bit_msb(i) ({                                                      \
        __auto_type msb_i_ = (i);                                             \
        (sizeof(msb_i_) * 8 - st_bit_clz(msb_i_) - 1);                        \
})


/**
 * count 1 before `ibit`, excludes the `ibit`-th bit.
 *
 *      0b00, 0 -- 0
 *      0b01, 0 -- 0
 *      0b01, 1 -- 1
 *      0b11, 1 -- 1
 *      0b11, 2 -- 2
 *
 * If try to count 1 before the 0-th bit, it should always 0.
 * If try to count 1 of all bits, do not need to shift.
 */
#define st_bit_cnt1_before(n, ibit) ({                                        \
    __auto_type cnt1_n = (n);                                                 \
    __auto_type cnt1_ibit = (ibit);                                           \
    if (cnt1_ibit <= 0) {                                                     \
        cnt1_n = 0;                                                           \
    }                                                                         \
    else {                                                                    \
        if (cnt1_ibit < sizeof(cnt1_n) * 8) {                                 \
            cnt1_n <<= sizeof(cnt1_n) * 8 - cnt1_ibit;                        \
        }                                                                     \
    }                                                                         \
    st_bit_cnt1(cnt1_n);                                                      \
})


/**
 * count all the 1 bit.
 */
#define st_bit_cnt1(n) _Generic(                                              \
    (n),                                                                      \
    unsigned int:       __builtin_popcount,                                   \
    unsigned long:      __builtin_popcountl,                                  \
    unsigned long long: __builtin_popcountll                                  \
)(n)

#endif /* INC_BIT_H */
