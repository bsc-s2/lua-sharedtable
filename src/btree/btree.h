#ifndef __BTREE__BTREE_H__
#     define __BTREE__BTREE_H__

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "inc/util.h"
#include "inc/err.h"
#include "inc/log.h"

#define ST_BT_NODESIZE ( 128 * 4 )

#define ST_BT_KEY_SIZE sizeof( uint64_t )
#define ST_BT_VAL_SIZE sizeof( uint64_t )
#define ST_BT_CHILDREN_SIZE sizeof( void* )

#define _L1_SIZE 128
#define ST_BT_MAX_N_KEYS ( ( _L1_SIZE - 8 ) / ST_BT_KEY_SIZE )
/* let key array fit in 1 cache line */
/* max_n_keys = degree * 2 -1 */
#define ST_BT_DEGREE ( ( ST_BT_MAX_N_KEYS + 1 ) / 2 )
#define ST_BT_MIN_N_KEYS ( ST_BT_DEGREE - 1 )

#define ST_BT_VAL_INVALID  0xffffffffffffffff

#define ST_BT_LAST         0x00000000
#define ST_BT_FIRST        0x00000001
#define ST_BT_CMP_VAL      0x00000002
#define ST_BT_ALLOW_DUP    0x00000004

#define ST_BTITER_I( it ) (it)->indexes[ (it)->level ]
#define ST_BTITER_NODE( it ) (it)->nodes[ (it)->level ]
#define ST_BTITER_CUR_KEY( it ) ST_BTITER_NODE(it)->keys[ ST_BTITER_I(it) ]
#define ST_BTITER_CUR_VAL( it ) ST_BTITER_NODE(it)->vals[ ST_BTITER_I(it) ]

/* min degree is 2, max elts is less than 2^30, thus max level should be less
 * than 32 */
#define ST_BTITER_MAX_LEVEL 32

typedef uint64_t                st_btu64_key_t;
typedef uint64_t                st_btu64_val_t;
typedef uint32_t                st_btu64_flag_t;

typedef struct st_btu64_s      st_btu64_t;
typedef struct st_btu64_node_s st_btu64_node_t;
typedef struct st_btu64_leaf_s st_btu64_leaf_t;

typedef struct st_btu64_kv_s   st_btu64_kv_t;
typedef struct st_btu64_rst_s  st_btu64_rst_t;
typedef struct st_btu64_iter_s st_btu64_iter_t;

typedef struct st_bt_stat_s    st_bt_stat_t;

struct st_bt_stat_s {
    int64_t n_node_create;
    int64_t n_node_merge;
    int64_t n_entry;
};

struct st_btu64_s {
    uint8_t           level;    /* level of root */
    st_btu64_node_t *root;
    st_btu64_t      *origin;
    st_bt_stat_t    *st;

    /* st_btu64_node_t *min_child; */
};

struct st_btu64_leaf_s {
    int32_t           n_keys;
    st_btu64_key_t   keys[ ST_BT_MAX_N_KEYS ];
    st_btu64_val_t   vals[ ST_BT_MAX_N_KEYS ];
    st_btu64_node_t *children[ 0 ];
};

struct st_btu64_node_s {
    int32_t           n_keys;
    st_btu64_key_t   keys[ ST_BT_MAX_N_KEYS ];
    st_btu64_val_t   vals[ ST_BT_MAX_N_KEYS ];
    st_btu64_node_t *children[ ST_BT_MAX_N_KEYS + 1 ];
};

/* for debug, logging purpose only */
struct st_btnode_dbg_s {
    st_btu64_t      *bt;
    st_btu64_node_t *node;
    uint8_t           level;
    char             *pad;
    uint8_t           verbose  :1;
};

struct st_btu64_kv_s {
    st_btu64_key_t k;
    st_btu64_val_t v;
};

struct st_btu64_rst_s {
    st_btu64_node_t *node;
    int               i;
    int               left;
    int               right;
};

struct st_btu64_iter_s {
    st_btu64_kv_t    kv;                                /* result */
    uint8_t           level;
    st_btu64_t      *bt;
    st_btu64_node_t *nodes[ ST_BTITER_MAX_LEVEL ];
    int32_t           indexes[ ST_BTITER_MAX_LEVEL ];
};

/* public API */
int     st_btu64_init( st_btu64_t *bt );
void    st_btu64_free( st_btu64_t *bt );

int     st_btu64_add( st_btu64_t *bt, st_btu64_key_t k, st_btu64_val_t v );
int     st_btu64_del( st_btu64_t *bt, st_btu64_key_t k, st_btu64_val_t v,
                       st_btu64_flag_t flag, st_btu64_kv_t *deleted );

int     st_btu64_search( st_btu64_t *bt,
                          st_btu64_key_t k, st_btu64_val_t v,
                          st_btu64_flag_t flag, st_btu64_rst_t *rst );

int     st_btu64_delmin( st_btu64_t *b, int i_after_which,
                          st_btu64_kv_t *deleted );

int     st_btiter_init( st_btu64_t *bt, st_btu64_iter_t *it );
st_btu64_kv_t* st_btiter_next( st_btu64_iter_t *it );

/* internally used API */
int32_t st_btu64_binsearch( st_btu64_t *b,
                             st_btu64_key_t k, st_btu64_val_t v,
                             st_btu64_flag_t flag, st_btu64_rst_t *rst );
int32_t st_btu64_binsearch_first( st_btu64_t *b,
                                   st_btu64_key_t k, st_btu64_val_t v );
int32_t st_btu64_binsearch_last( st_btu64_t *b,
                                  st_btu64_key_t k, st_btu64_val_t v );

st_btu64_node_t* st_btu64_allocnode( st_btu64_t *bt, uint8_t level );
int     st_btu64_split_child( st_btu64_t *b, int32_t ichild );
int     st_btu64_add_to_node( st_btu64_t *b, int at, st_btu64_key_t k,
                               st_btu64_val_t v, st_btu64_node_t *child );
void    st_btu64_setentry( st_btu64_t *b, int at, st_btu64_key_t k,
                            st_btu64_val_t v, st_btu64_node_t *child );
int     st_btu64_canmerge( st_btu64_t *b, int i );
void    st_btu64_merge( st_btu64_t *b, int i );
int     st_btu64_rotateleft( st_btu64_t *b, int i );
int     st_btu64_rotateright( st_btu64_t *b, int i );

void    st_btu64_node_shiftleft( st_btu64_t *b, int from, int shift );
void    st_btu64_node_shiftright( st_btu64_t *b, int from, int shift );

char*   st_bt_flagstr( st_btu64_flag_t flag );


#ifdef ST_DEBUG
void    st_btu64_dnodeinfo_ln( struct st_btnode_dbg_s *dbg );
void    st_btu64_dkv_ln( struct st_btnode_dbg_s *dbg, int i );
void    st_btu64_dkvs_inl( st_btu64_t *bt, st_btu64_node_t *nd );
void    st_btu64_dnode( st_btu64_t *bt, st_btu64_node_t *node );
void    st_btu64_dtree( struct st_btnode_dbg_s *dbg );
void    st_btu64_dd( st_btu64_t *bt, int verbose );
#else
#   define st_btu64_dnodeinfo_ln( dbg )
#   define st_btu64_dkv_ln( dbg, i )
#   define st_btu64_dkvs_inl( bt, nd )
#   define st_btu64_dnode( bt, nd )
#   define st_btu64_dtree( dbg )
#   define st_btu64_dd( bt, verbose )
#endif


#endif /* __BTREE__BTREE_H__ */
