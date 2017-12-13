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
        unsigned int:       __builtin_clz(i),                                 \
        unsigned long:      __builtin_clzl(i),                                \
        unsigned long long: __builtin_clzll(i)                                \
)


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

#endif /* INC_BIT_H */
