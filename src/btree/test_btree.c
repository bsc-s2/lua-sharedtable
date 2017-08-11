#include "btree.h"
#include "hash/fibonacci.h"
#include "unittest/unittest.h"

#define MAX_KEYS 100
#define mes_len 10240
char mes[ mes_len ];
char *dump_mes = NULL;

#define V( k ) ( k+1 )

#define bt_declare( name )                                                    \
        st_btu64_t name = {0};                                               \
        do {                                                                  \
            int rc = st_btu64_init( &name );                                 \
            st_ut_eq(ST_OK, rc, "init bt" );                               \
        } while( 0 )

#define bt_add_ks_u64( bt, ks )                                               \
        bt_add_ks_n_u64( bt, st_nelts( ks ), ks, dump_mes )

#define bt_add_ks_n_u64( bt, n, ks, mes )                                     \
        do {                                                                  \
            int __i;                                                          \
            int rc;                                                           \
            for ( __i = 0; __i < (n); __i++ ) {                               \
                                                                              \
                /* val for test is key + 1 */                                 \
                rc = st_btu64_add( (bt), (ks)[ __i ], (ks)[ __i ] + 1 );     \
                st_ut_eq(ST_OK, rc, "add to btree: %llu", (ks)[ __i ] );   \
                                                                              \
                if ( (mes) != NULL ) {                                        \
                    (mes) += snprintf( (mes), 10240, "%llu, ", (ks)[ __i ] ); \
                }                                                             \
            }                                                                 \
        } while ( 0 )

#define bt_case_init( name, cases, icase, mes_format, ... )                   \
        do {                                                                  \
            char *p;                                                          \
            p = mes;                                                          \
            p += snprintf( p, mes_len,                                        \
                           mes_format ", case %d of %lu in [",                \
                           ##__VA_ARGS__, icase, st_nelts( cases )-1 );      \
            bt_add_ks_n_u64( &name,                                           \
                             (cases)[ icase ].n, (cases)[ icase ].ks, p );    \
            p += snprintf( p, mes_len - ( p-mes ), "]" );                     \
                                                                              \
            dd( "btree created:" );                                           \
            st_btu64_dd( &name, 0 );                                            \
        } while ( 0 )

#define ass_bt_empty( bt )                                                    \
        do {                                                                  \
            st_ut_eq(0, (bt)->level,                                      \
                    "btree level is 0 %s", mes );                             \
            st_ut_eq(0, (bt)->root->n_keys,                               \
                    "no elts on root %s", mes );                              \
        } while ( 0 )

#define ass_iter_all( bt, cs, idel, case_mes )                                \
        do {                                                                  \
            st_btu64_iter_t  it;                                                 \
            st_btu64_kv_t   *kv;                                                 \
            st_btu64_key_t   k;                                                  \
            int           ichk;                                               \
            uint64_t __ks[ MAX_KEYS ];                                        \
            int         __i;                                                  \
            int __idel = (idel);                                              \
                                                                              \
            for ( __i = 0; __i < (cs)->n; __i++ ) {                           \
                __ks[ __i ] = (cs)->ks[__i];                                  \
            }                                                                 \
            qsort( __ks, (cs)->n, sizeof( uint64_t ), cmp_u64 );              \
                                                                              \
            st_btiter_init( (bt), &it );                                     \
            for ( ichk = 0; ichk < (cs)->n; ichk++ ) {                        \
                                                                              \
                k = __ks[ ichk ];                                             \
                if ( __idel >= 0 && (cs)->ks[ __idel ] == k ) {               \
                    /* skip only one */                                       \
                    __idel = -1;                                              \
                    continue;                                                 \
                }                                                             \
                kv = st_btiter_next( &it );                                  \
                st_ut_ne( NULL, kv, "not NULL %s", (case_mes) );   \
                                                                              \
                st_ut_eq( k, kv->k,                                   \
                        "%d-th key iterator %s", ichk, (case_mes) );          \
            }                                                                 \
            kv = st_btiter_next( &it );                                      \
            st_ut_eq( NULL, kv, "NULL after iter %s", (case_mes) );\
        } while ( 0 )

#define ass_node_ks( bt, nd, ks )                                             \
        do {                                                                  \
            int n = sizeof( (ks) ) / sizeof( (ks)[ 0 ] );                     \
            int i;                                                            \
            st_ut_eq( n, (nd)->n_keys, "nr of keys" );                     \
            for ( i = 0; i < n; i++ ) {                                       \
                st_ut_eq( (ks)[ i ],                                  \
                        (nd)->keys[ i ],                                      \
                        "check key: %d", i );                                 \
            }                                                                 \
        } while ( 0 )

#define ass_search_kv_notfound( bt, key, val, flag, mes )                     \
        do {                                                                  \
            int             rc;                                               \
            st_btu64_rst_t rst;                                              \
                                                                              \
            rc = st_btu64_search( (bt), (key), (val), (flag), &rst );        \
            st_ut_eq( ST_NOT_FOUND, rc,                                      \
                    "not found search %s for (%llu, %llu); %s",               \
                    st_bt_flagstr( flag ),                                   \
                    (st_btu64_key_t)(key), (st_btu64_val_t)(val), (mes) );  \
        } while ( 0 )

#define ass_search_k_notfound( bt, key, mes )                                 \
        do {                                                                  \
            int i;                                                            \
            st_btu64_flag_t flags[] = {                                      \
                ST_BT_FIRST,                                                 \
                ST_BT_FIRST | ST_BT_CMP_VAL,                                \
                ST_BT_LAST,                                                  \
                ST_BT_LAST | ST_BT_CMP_VAL,                                 \
            };                                                                \
            for ( i = 0; i < st_nelts( flags ); i++ ) {                      \
                ass_search_kv_notfound( (bt), (key), 0,                       \
                                        flags[ i ], (mes) );                  \
                ass_search_kv_notfound( (bt), (key), (key)+1,                 \
                                        flags[ i ], (mes) );                  \
            }                                                                 \
        } while ( 0 )

#define ass_search_kv_found( bt, key, val, flag, mes )                             \
        do {                                                                       \
            st_btu64_rst_t rst;                                                   \
            int             rc;                                                    \
                                                                                   \
            rc = st_btu64_search( (bt), (key), (val), (flag), &rst );             \
                                                                                   \
            st_ut_eq( ST_OK, rc,                                                 \
                    "found, search %s (%llu, %llu); %s",                           \
                    st_bt_flagstr( flag ), (key), (st_btu64_val_t)(val), (mes) );\
            st_ut_ne( -1, rst.i,                                                \
                    "i should not be -1, %s", (mes) );                             \
                                                                                   \
            st_ut_eq( (key), rst.node->keys[ rst.i ],                      \
                    "found key %s", (mes) );                                       \
                                                                                   \
            if ( (flag) & ST_BT_CMP_VAL ) {                                       \
                 st_ut_eq( (val), rst.node->vals[ rst.i ],                 \
                         "found val %s", (mes)  );                                 \
            }                                                                      \
        } while ( 0 )

#define ass_search_k_found( bt, key, mes )                                    \
        do {                                                                  \
            int i;                                                            \
            int flags[] = {                                                   \
                ST_BT_FIRST,                                                 \
                ST_BT_LAST                                                   \
            };                                                                \
                                                                              \
            for ( i = 0; i < st_nelts( flags ); i++ ) {                      \
                                                                              \
                ass_search_kv_found( (bt), (key), 0,                          \
                                     flags[i], (mes) );                       \
                                                                              \
                ass_search_kv_notfound( (bt), (key), 0,                       \
                                        flags[i] | ST_BT_CMP_VAL, (mes) );   \
                                                                              \
                ass_search_kv_found( (bt), (key), (key)+1,                    \
                                     flags[i] | ST_BT_CMP_VAL, (mes) );      \
                                                                              \
                ass_search_kv_notfound( (bt), (key), (key)+2,                 \
                                        flags[i] | ST_BT_CMP_VAL, (mes) );   \
                                                                              \
            }                                                                 \
        } while ( 0 )

int
cmp_u64(const void *a, const void *b) {
    uint64_t x = *(uint64_t *)a;
    uint64_t y = *(uint64_t *)b;
    return x > y ? 1 : (x < y ? -1 : 0);
}

st_test(btree, align) {
    struct case_s {
        int v;
        int align;
        int rst;
    };

    int i;
    int n;
    int rst;
    struct case_s *cs;
    struct case_s cases[] = {
        { .v = 0, .align = 0, .rst = 0 },
        { .v = 1, .align = 0, .rst = 1 },
        { .v = 2, .align = 0, .rst = 2 },
        { .v = 3, .align = 0, .rst = 3 },

        { .v = 0, .align = 1, .rst = 0 },
        { .v = 1, .align = 1, .rst = 1 },
        { .v = 2, .align = 1, .rst = 2 },
        { .v = 3, .align = 1, .rst = 3 },

        { .v = 0, .align = 2, .rst = 0 },
        { .v = 1, .align = 2, .rst = 2 },
        { .v = 2, .align = 2, .rst = 2 },
        { .v = 3, .align = 2, .rst = 4 },
        { .v = 4, .align = 2, .rst = 4 },

        { .v = 0, .align = 512, .rst = 0 },
        { .v = 1, .align = 512, .rst = 512 },
        { .v = 2, .align = 512, .rst = 512 },
        { .v = 511, .align = 512, .rst = 512 },
        { .v = 512, .align = 512, .rst = 512 },
        { .v = 513, .align = 512, .rst = 1024 },
        { .v = 514, .align = 512, .rst = 1024 },
        { .v = 1023, .align = 512, .rst = 1024 },
        { .v = 1024, .align = 512, .rst = 1024 },
    };

    n = sizeof(cases) / sizeof(cases[ 0 ]);

    for (i = 0; i < n; i++) {
        cs = &cases[ i ];
        rst = st_align(cs->v, cs->align);
        st_ut_eq(cs->rst, rst, "case %d of %d", i, n - 1);
    }
}

st_test(btree, node_alloc) {
    st_btu64_node_t *nd;
    int i;

    bt_declare(bt);

    nd = st_btu64_allocnode(&bt, 0);
    st_ut_ne(NULL, nd, "alloc");
    st_ut_eq(0, nd->n_keys, "n_keys is 0");

    for (i = 0; i < nd->n_keys; i++) {
        st_ut_eq(0, nd->keys[ i ], "key is zero");
        st_ut_eq(0, nd->vals[ i ], "val is zero");
    }
    st_free(nd);

    nd = st_btu64_allocnode(&bt, 1);
    for (i = 0; i < nd->n_keys; i++) {
        st_ut_eq(0, nd->keys[ i ], "key is zero");
        st_ut_eq(0, nd->vals[ i ], "val is zero");
        st_ut_eq(NULL, nd->children[ i ], "child is NULL");
    }
    st_ut_eq(NULL, nd->children[ i ], "child is NULL");

    st_free(nd);

    st_btu64_free(&bt);
}

st_test(btree, init_free) {
    int             rc;
    st_btu64_node_t *root;
    st_btu64_t        bt   = { 0 };

    rc = st_btu64_init(NULL);
    st_ut_eq(ST_ARG_INVALID, rc, "init NULL");

    rc = st_btu64_init(&bt);
    st_ut_eq(ST_OK, rc, "valid init");

    rc = st_btu64_init(&bt);
    st_ut_eq(ST_ARG_INVALID, rc, "re-inited");

    st_ut_eq(15, ST_BT_MAX_N_KEYS, "max n");
    st_ut_eq(7, ST_BT_MIN_N_KEYS, "min n");
    st_ut_eq(0, bt.level, "bt.level");

    st_ut_eq(&bt, bt.origin, "bt.origin");

    root = bt.root;
    st_ut_ne(NULL, root, "root is not NULL");
    st_ut_eq(0, root->n_keys, "root nr keys");

    st_btu64_free(&bt);
    st_ut_eq(NULL, bt.root, "root is NULL after free");

    /* TODO test st_btu64_free() frees sub nodes */
}

st_test(btree, free_deep) {
}

st_test(btree, binsearch) {
    int               i;
    int               j;
    int               n;
    char              mes[ 10240 ];
    char             *p;
    st_btu64_node_t *root;
    st_btu64_rst_t   rst;
    st_btu64_key_t   k;

    /* large enough for all cases */
    bt_declare(bt);

    struct case_s {
        int                n;
        uint64_t           ks[ MAX_KEYS ];
        uint64_t           tosearch;
        int                flag;
        st_btu64_rst_t rst;
    };

    struct case_s *cs;
    struct case_s cases[] = {

        /* search in empty array */
        { .n = 0, .ks = {}, .tosearch = 0, .flag = ST_BT_FIRST, .rst = { .i = -1, .left = -1, .right = 0 } },
        { .n = 0, .ks = {}, .tosearch = 1, .flag = ST_BT_FIRST, .rst = { .i = -1, .left = -1, .right = 0 } },

        { .n = 0, .ks = {}, .tosearch = 0, .flag = ST_BT_LAST, .rst = { .i = -1, .left = -1, .right = 0 } },
        { .n = 0, .ks = {}, .tosearch = 1, .flag = ST_BT_LAST, .rst = { .i = -1, .left = -1, .right = 0 } },

        /* exactly one elt, found */
        { .n = 1, .ks = {0}, .tosearch = 0, .flag = ST_BT_FIRST, .rst = { .i = 0, .left = -1, .right = 0 } },

        { .n = 1, .ks = {0}, .tosearch = 0, .flag = ST_BT_LAST, .rst = { .i = 0, .left = 0, .right = 1 } },
        { .n = 1, .ks = {103}, .tosearch = 103, .flag = ST_BT_LAST, .rst = { .i = 0, .left = 0, .right = 1 } },

        /* exactly one elt, not found */
        { .n = 1, .ks = {1}, .tosearch = 0, .flag = ST_BT_FIRST, .rst = { .i = -1, .left = -1, .right = 0 } },
        { .n = 1, .ks = {1}, .tosearch = 2, .flag = ST_BT_FIRST, .rst = { .i = -1, .left = 0, .right = 1 } },

        { .n = 1, .ks = {1}, .tosearch = 0, .flag = ST_BT_LAST, .rst = { .i = -1, .left = -1, .right = 0 } },
        { .n = 1, .ks = {1}, .tosearch = 2, .flag = ST_BT_LAST, .rst = { .i = -1, .left = 0, .right = 1 } },

        /* 2 elts, found */
        { .n = 2, .ks = {1, 3}, .tosearch = 1, .flag = ST_BT_FIRST, .rst = { .i = 0, .left = -1, .right = 0 } },
        { .n = 2, .ks = {1, 3}, .tosearch = 3, .flag = ST_BT_FIRST, .rst = { .i = 1, .left = 0, .right = 1 } },

        { .n = 2, .ks = {1, 3}, .tosearch = 1, .flag = ST_BT_LAST, .rst = { .i = 0, .left = 0, .right = 1 } },
        { .n = 2, .ks = {1, 3}, .tosearch = 3, .flag = ST_BT_LAST, .rst = { .i = 1, .left = 1, .right = 2 } },

        /* 2 elts, not found */
        { .n = 2, .ks = {1, 3}, .tosearch = 0, .flag = ST_BT_FIRST, .rst = { .i = -1, .left = -1, .right = 0 } },
        { .n = 2, .ks = {1, 3}, .tosearch = 2, .flag = ST_BT_FIRST, .rst = { .i = -1, .left = 0, .right = 1 } },
        { .n = 2, .ks = {1, 3}, .tosearch = 4, .flag = ST_BT_FIRST, .rst = { .i = -1, .left = 1, .right = 2 } },

        { .n = 2, .ks = {1, 3}, .tosearch = 0, .flag = ST_BT_LAST, .rst = { .i = -1, .left = -1, .right = 0 } },
        { .n = 2, .ks = {1, 3}, .tosearch = 2, .flag = ST_BT_LAST, .rst = { .i = -1, .left = 0, .right = 1 } },
        { .n = 2, .ks = {1, 3}, .tosearch = 4, .flag = ST_BT_LAST, .rst = { .i = -1, .left = 1, .right = 2 } },

        /* 3 elts, found */
        { .n = 3, .ks = {1, 3, 5}, .tosearch = 1, .flag = ST_BT_FIRST, .rst = { .i = 0, .left = -1, .right = 0 } },
        { .n = 3, .ks = {1, 3, 5}, .tosearch = 3, .flag = ST_BT_FIRST, .rst = { .i = 1, .left = 0, .right = 1 } },
        { .n = 3, .ks = {1, 3, 5}, .tosearch = 5, .flag = ST_BT_FIRST, .rst = { .i = 2, .left = 1, .right = 2 } },

        { .n = 3, .ks = {1, 3, 5}, .tosearch = 1, .flag = ST_BT_LAST, .rst = { .i = 0, .left = 0, .right = 1 } },
        { .n = 3, .ks = {1, 3, 5}, .tosearch = 3, .flag = ST_BT_LAST, .rst = { .i = 1, .left = 1, .right = 2 } },
        { .n = 3, .ks = {1, 3, 5}, .tosearch = 5, .flag = ST_BT_LAST, .rst = { .i = 2, .left = 2, .right = 3 } },

        /* all same 3 elts, found */
        { .n = 3, .ks = {1, 1, 1}, .tosearch = 1, .flag = ST_BT_FIRST, .rst = { .i = 0, .left = -1, .right = 0 } },

        { .n = 3, .ks = {1, 1, 1}, .tosearch = 1, .flag = ST_BT_LAST, .rst = { .i = 2, .left = 2, .right = 3 } },

        /* all same 3 elts, not found */
        { .n = 3, .ks = {1, 1, 1}, .tosearch = 0, .flag = ST_BT_FIRST, .rst = { .i = -1, .left = -1, .right = 0 } },
        { .n = 3, .ks = {1, 1, 1}, .tosearch = 2, .flag = ST_BT_FIRST, .rst = { .i = -1, .left = 2, .right = 3 } },

        { .n = 3, .ks = {1, 1, 1}, .tosearch = 0, .flag = ST_BT_LAST, .rst = { .i = -1, .left = -1, .right = 0 } },
        { .n = 3, .ks = {1, 1, 1}, .tosearch = 2, .flag = ST_BT_LAST, .rst = { .i = -1, .left = 2, .right = 3 } },

        /* has 3 same elts, found */
        { .n = 5, .ks = {1, 3, 3, 3, 5}, .tosearch = 1, .flag = ST_BT_FIRST, .rst = { .i = 0, .left = -1, .right = 0 } },
        { .n = 5, .ks = {1, 3, 3, 3, 5}, .tosearch = 3, .flag = ST_BT_FIRST, .rst = { .i = 1, .left = 0, .right = 1 } },
        { .n = 5, .ks = {1, 3, 3, 3, 5}, .tosearch = 5, .flag = ST_BT_FIRST, .rst = { .i = 4, .left = 3, .right = 4 } },

        { .n = 5, .ks = {1, 3, 3, 3, 5}, .tosearch = 1, .flag = ST_BT_LAST, .rst = { .i = 0, .left = 0, .right = 1 } },
        { .n = 5, .ks = {1, 3, 3, 3, 5}, .tosearch = 3, .flag = ST_BT_LAST, .rst = { .i = 3, .left = 3, .right = 4 } },
        { .n = 5, .ks = {1, 3, 3, 3, 5}, .tosearch = 5, .flag = ST_BT_LAST, .rst = { .i = 4, .left = 4, .right = 5 } },

        /* has 3 same elts, not found */
        { .n = 5, .ks = {1, 3, 3, 3, 5}, .tosearch = 0, .flag = ST_BT_FIRST, .rst = { .i = -1, .left = -1, .right = 0 } },
        { .n = 5, .ks = {1, 3, 3, 3, 5}, .tosearch = 2, .flag = ST_BT_FIRST, .rst = { .i = -1, .left = 0, .right = 1 } },
        { .n = 5, .ks = {1, 3, 3, 3, 5}, .tosearch = 4, .flag = ST_BT_FIRST, .rst = { .i = -1, .left = 3, .right = 4 } },
        { .n = 5, .ks = {1, 3, 3, 3, 5}, .tosearch = 6, .flag = ST_BT_FIRST, .rst = { .i = -1, .left = 4, .right = 5 } },

        { .n = 5, .ks = {1, 3, 3, 3, 5}, .tosearch = 0, .flag = ST_BT_LAST, .rst = { .i = -1, .left = -1, .right = 0 } },
        { .n = 5, .ks = {1, 3, 3, 3, 5}, .tosearch = 2, .flag = ST_BT_LAST, .rst = { .i = -1, .left = 0, .right = 1 } },
        { .n = 5, .ks = {1, 3, 3, 3, 5}, .tosearch = 4, .flag = ST_BT_LAST, .rst = { .i = -1, .left = 3, .right = 4 } },
        { .n = 5, .ks = {1, 3, 3, 3, 5}, .tosearch = 6, .flag = ST_BT_LAST, .rst = { .i = -1, .left = 4, .right = 5 } },

    };

    root = bt.root;

    n = sizeof(cases) / sizeof(struct case_s);
    for (i = 0; i < n; i++) {

        cs = &cases[ i ];
        root->n_keys = cs->n;

        p = mes;
        p += snprintf(p, 10240, "case %d of %d,"
                      " search %s for %llu in [",
                      i, n - 1, st_bt_flagstr(cs->flag), cs->tosearch);

        for (j = 0; j < cs->n; j++) {

            bt.root->keys[ j ] = cs->ks[ j ];
            bt.root->vals[ j ] = cs->ks[ j ] + 1;

            p += snprintf(p, 10240, "%llu, ", cs->ks[ j ]);
        }
        p += snprintf(p, 10240, "]");

        k = cs->tosearch;

        st_btu64_binsearch(&bt, k, 0, cs->flag, &rst);
        st_ut_eq(cs->rst.i, rst.i, "rst.i, without val %s", mes);

        st_btu64_binsearch(&bt, k, k + 1, cs->flag | ST_BT_CMP_VAL, &rst);
        st_ut_eq(cs->rst.i, rst.i, "rst.i, with val %s", mes);

        st_btu64_binsearch(&bt, k, k + 2, cs->flag | ST_BT_CMP_VAL, &rst);
        st_ut_eq(-1, rst.i, "rst.i, incorrect val %s", mes);

        st_btu64_binsearch(&bt, k, k + 1, cs->flag, &rst);
        st_ut_eq(cs->rst.i, rst.i, "rst.i, %s", mes);
        st_ut_eq(cs->rst.left, rst.left, "rst.left, %s", mes);
        st_ut_eq(cs->rst.right, rst.right, "rst.right, %s", mes);

    }

    st_btu64_free(&bt);
}

st_test(btree, search) {

    struct case_s {
        int      n;
        uint64_t ks[ MAX_KEYS ];
    };

    int            i;
    int            j;
    struct case_s *cs;
    struct case_s  cases[] = {

        /* search in empty array */
        { .n = 0, .ks = {}, },

        { .n = 1, .ks = {1}, },
        { .n = 3, .ks = {1, 3, 5}, },
        { .n = 10, .ks = {1, 3, 5, 7, 9, 11, 13, 15, 17, 19}, },
        { .n = 10, .ks = {1, 3, 3, 3, 3, 11, 13, 13, 13, 13}, },

    };

    for (i = 0; i < st_nelts(cases); i++) {

        cs = &cases[ i ];

        bt_declare(bt);
        bt_case_init(bt, cases, i, "search");

        /* test searching for key less than first */
        ass_search_k_notfound(&bt, 0, mes);

        for (j = 0; j < cs->n; j++) {
            ass_search_k_notfound(&bt, cs->ks[ j ] - 1, mes);
            ass_search_k_found(&bt, cs->ks[ j ], mes);
        }

        /* test searching for key greater than last */
        if (cs->n > 0) {
            ass_search_k_notfound(&bt, cs->ks[ cs->n - 1 ] + 1, mes);
        }

        st_btu64_free(&bt);
    }
}

st_test(btree, xx) {
    int           i;
    int           n;
    st_btu64_key_t k;

    bt_declare(bt);

    n = 1024;

    for (i = 0; i < n; i++) {
        k = fib_hash16(i);
        dd("add: %llu", k);
        st_btu64_add(&bt, k, 0);
    }
    st_ut_ne(0, bt.level, "level is not 0");
    st_btu64_dd(&bt, 0);

    st_btu64_free(&bt);
}

st_test(btree, add_ascent) {
    int           i;
    int           n;
    st_btu64_kv_t   *kvp;
    st_btu64_iter_t  it;

    bt_declare(bt);

    n = 64;

    for (i = 0; i < n; i++) {
        st_btu64_add(&bt, i, i);
    }
    st_ut_ne(0, bt.level, "level is not 0");
    st_btu64_dd(&bt, 0);

    i = 0;
    st_btiter_init(&bt, &it);
    while ((kvp = st_btiter_next(&it)) != NULL) {
        st_ut_eq(i, kvp->k, "%d-th key", i);
        i++;
    }
    st_ut_eq(n, i, "all kv iterated");

    st_btu64_free(&bt);
}

st_test(btree, add_random) {
    int            i;
    int            n;
    st_btu64_kv_t    *kvp;
    st_btu64_key_t  k;
    st_btu64_key_t  prev         = 0;
    st_btu64_iter_t   it;
    int            rset[ 4096 ] = { 0 };

    bt_declare(bt);

    n = 64;

    for (i = 0; i < n; i++) {
        k = fib_hash64(i) % 4096;
        st_btu64_add(&bt, k, i);
        rset[ k ] = 1;
    }
    st_ut_ne(0, bt.level, "level is not 0");
    st_btu64_dd(&bt, 0);

    st_btiter_init(&bt, &it);
    while ((kvp = st_btiter_next(&it)) != NULL) {
        st_ut_eq(1, rset[ kvp->k ] > 0 ? 1 : 0,
               "iter get only key added %llu", kvp->k);
        rset[ kvp->k ] = 2;

        if (prev > 0) {
            st_ut_eq(1, prev <= kvp->k ? 1 : 0,
                   "prev: %llu <= cur: %llu", prev, kvp->k);
        }

        prev = kvp->k;
    }
    for (i = 0; i < 4096; i++) {
        st_ut_ne(1, rset[ i ], "every key added iterated");
    }

    st_btu64_free(&bt);
}

st_test(btree, add_same_k) {
    uint64_t ks[] = {
        1, 2, 3, 3, 3, 5, 5,
        5,
        5, 5, 5, 5, 5, 5, 7,
        9
    };

    bt_declare(bt);

    bt_add_ks_n_u64(&bt, st_nelts(ks), ks, dump_mes);
    st_btu64_dd(&bt, 0);

    {
        uint64_t rks[] = { 5 };
        ass_node_ks(&bt, bt.root, rks);
    }
    {
        uint64_t rks[] = {
            1, 2, 3, 3, 3, 5, 5,
        };
        ass_node_ks(&bt, bt.root->children[ 0 ], rks);
    }
    {
        uint64_t rks[] = {
            5, 5, 5, 5, 5, 5, 7,
            9
        };
        ass_node_ks(&bt, bt.root->children[ 1 ], rks);
    }

    st_btu64_free(&bt);
}

st_test(btree, add_same_k_iter_all) {
    struct case_s {
        int n;
        uint64_t ks[ MAX_KEYS ];
    };

    int            n;
    int            i;
    int            j;
    st_btu64_key_t    k;
    st_btu64_kv_t    *kv;
    st_btu64_iter_t   it;
    char           mes[ 10240 ];
    char          *p;
    struct case_s *cs;
    struct case_s cases[] = {
        { .n = 10, .ks = { 1, 3, 3, 3, 3, 11, 13, 13, 13, 13 }, },
        { .n = 10, .ks = { 1, 1, 1, 1, 1, 11, 11, 11, 11, 11 }, },
    };

    n = sizeof(cases) / sizeof(cases[ 0 ]);
    for (i = 0; i < n; i++) {

        cs = &cases[ i ];

        bt_declare(bt);

        p = mes;
        p += snprintf(p, 10240, "case %d of %d in [", i, n - 1);
        bt_add_ks_n_u64(&bt, cs->n, cs->ks, p);
        p += snprintf(p, 10240, "]");

        st_btu64_dd(&bt, 0);

        st_btiter_init(&bt, &it);
        for (j = 0; j < cs->n; j++) {
            k = cs->ks[ j ];
            kv = st_btiter_next(&it);
            st_ut_ne(NULL, kv, "not NULL");

            st_ut_eq(k, kv->k, "%d-th key iterator %s", j, mes);
        }

        st_btu64_free(&bt);
    }

}

st_test(btree, iter) {
    struct case_s {
        int n;
        uint64_t ks[ MAX_KEYS ];
    };

    int            i;
    struct case_s *cs;
    struct case_s  cases[] = {
        { .n = 0,  .ks = {}, },
        { .n = 1,  .ks = { 1 }, },
        { .n = 10, .ks = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }, },
        { .n = 10, .ks = { 1, 3, 3, 3, 3, 11, 13, 13, 13, 13 }, },
        { .n = 10, .ks = { 1, 1, 1, 1, 1, 11, 11, 11, 11, 11 }, },
        { .n = 10, .ks = { 1, 3, 5, 7, 9, 11, 13, 15, 17, 19 }, },
    };

    for (i = 0; i < st_nelts(cases); i++) {

        cs = &cases[ i ];

        bt_declare(bt);
        bt_case_init(bt, cases, i, "iter all");

        ass_iter_all(&bt, cs, -1 /* no deleted */, mes);

        st_btu64_free(&bt);
    }

}

static int
index_of(int *a, int n, int tosearch) {
    int i;
    for (i = 0; i < n; i++) {
        if (a[ i ] == tosearch) {
            return i;
        }
    }
    return -1;
}

/* TODO st_btu64_del accept NULL as its 4th argument */

st_test(btree, delmin) {
    struct case_s {
        int      n;
        uint64_t ks[ MAX_KEYS ];
        int      ndel;
        uint64_t todel[ MAX_KEYS ];
    };

    int            i;
    int            idel;
    int            rc;
    st_btu64_kv_t     deleted;
    struct case_s *cs;
    struct case_s cases[] = {
        { .n = 0,  .ks = {},          .ndel = 0, .todel = {}, },
        { .n = 1,  .ks = { 1 },       .ndel = 1, .todel = { 1 } },

        /* 1 3 5
         */
        { .n = 3,  .ks = { 1, 3, 5 }, .ndel = 3, .todel = { 1, 3, 5 } },

        {
            .n = 16,  .ks = {
                1, 1, 1, 1, 1, 1, 1,
                3,
                5, 5, 5, 5, 5, 5, 5, 7
            },
            .ndel = 2, .todel = { 1, 5 }
        },

        /* both children full.
         *
         * | `-1, 3, 3,
         * |-5
         *   `-9, 11, 13,
         */
        {
            .n = 31,  .ks = {
                /* init btree */
                1, 1, 1, 1, 1, 1, 1,
                5,
                9, 9, 9, 9, 9, 9, 9,

                /* to first child */
                3, 3, 3, 3, 3, 3, 3, 3,

                /* to second child */
                11, 11, 11, 11, 11, 11, 11, 11,

            }, .ndel = 2, .todel = { 1, 9 }
        },

        /* both children has only min_n_keys.
         * rotateable.
         *
         * | `-1..
         * |-3
         * | `-3..
         * |-5
         * | `-9..
         * |-11
         *   `-13, 15..
         */
        {
            .n = 33,  .ks = {
                /* init btree */
                1, 1, 1, 1, 1, 1, 1,
                5,
                9, 9, 9, 9, 9, 9, 9,

                /* to first child */
                3, 3, 3, 3, 3, 3, 3, 3, 3, /* 9 */

                /* to second child */
                11, 13, 15, 15, 15, 15, 15, 15, 15, /* 9 */
            }, .ndel = 4, .todel = { 1, 3, 9, 13 }
        },
    };

    for (i = 0; i < st_nelts(cases); i++) {

        cs = &cases[ i ];

        /* delete one */
        for (idel = -1; idel < cs->ndel; idel++) {

            bt_declare(bt);
            bt_case_init(bt, cases, i, "delmin after (%d)-th key", idel);

            rc = st_btu64_delmin(&bt, idel, &deleted);
            if (idel == cs->ndel - 1) {
                st_ut_eq(ST_NOT_FOUND, rc,
                       "delmin not found %s", mes);
            } else {
                st_ut_eq(ST_OK, rc,
                       "delmin ok %s", mes);
                st_ut_eq(cs->todel[ idel + 1 ], deleted.k,
                       "deleted key %s", mes);
            }

            st_btu64_dd(&bt, 0);
            st_btu64_free(&bt);
        }
    }

}

st_test(btree, del) {
    struct case_s {
        int n;
        uint64_t ks[ MAX_KEYS ];
    };

    int              i;
    int              idel;
    int              rc;
    st_btu64_flag_t flag;
    st_btu64_key_t  k;
    st_btu64_kv_t   deleted;
    struct case_s   *cs;
    struct case_s cases[] = {
        { .n = 0,  .ks = {}, },
        { .n = 1,  .ks = { 1 }, },
        { .n = 3,  .ks = { 1, 3, 5 }, },
        { .n = 4,  .ks = { 1, 3, 5, 7 }, },
        { .n = 10, .ks = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }, },
        { .n = 10, .ks = { 1, 3, 3, 3, 3, 11, 13, 13, 13, 13 }, },
        { .n = 10, .ks = { 1, 1, 1, 1, 1, 11, 11, 11, 11, 11 }, },

        /*
         * test delete with both children full.
         * to formulate btree with both children full like this:
         *
         * | `-1, 3, 3,
         * |-5
         *   `-9, 11, 11,
         */
        {
            .n = 7,  .ks = {
                1, 5, 9,
                3, 3,
                11, 11
            },
        },
    };

    /* TODO test deletion key-value */

    for (i = 0; i < st_nelts(cases); i++) {

        cs = &cases[ i ];

        /* delete one with key */
        for (idel = 0; idel < cs->n; idel++) {

            bt_declare(bt);
            bt_case_init(bt, cases, i, "del %d-th key %llu", idel, cs->ks[ idel ]);

            k = cs->ks[ idel ] - 1;

            rc = st_btu64_del(&bt, k, 0, ST_BT_FIRST, &deleted);
            st_ut_eq(ST_NOT_FOUND, rc,
                   "delete first not found %s", mes);

            rc = st_btu64_del(&bt, k, 0, ST_BT_LAST, &deleted);
            st_ut_eq(ST_NOT_FOUND, rc,
                   "delete last not found %s", mes);

            k = cs->ks[ idel ];

            rc = st_btu64_del(&bt, k, 0, ST_BT_FIRST, &deleted);
            st_ut_eq(ST_OK, rc,
                   "delete ok %s", mes);

            st_btu64_dd(&bt, 0);
            ass_iter_all(&bt, cs, idel, mes);

            st_btu64_free(&bt);
        }

        /* delete one with key-value */
        for (idel = 0; idel < cs->n; idel++) {

            bt_declare(bt);
            bt_case_init(bt, cases, i, "del %d-th key-value %llu", idel, cs->ks[ idel ]);

            k = cs->ks[ idel ];

            flag = ST_BT_FIRST | ST_BT_CMP_VAL;
            rc = st_btu64_del(&bt, k, V(k) - 1, flag, &deleted);
            st_ut_eq(ST_NOT_FOUND, rc,
                   "delete with incorrect val not found: %s %s", st_bt_flagstr(flag), mes);

            flag = ST_BT_LAST | ST_BT_CMP_VAL;
            rc = st_btu64_del(&bt, k, V(k) - 1, flag, &deleted);
            st_ut_eq(ST_NOT_FOUND, rc,
                   "delete incorrect val not found: %s %s", st_bt_flagstr(flag), mes);

            flag = ST_BT_FIRST | ST_BT_CMP_VAL;
            rc = st_btu64_del(&bt, k, V(k), flag, &deleted);
            st_ut_eq(ST_OK, rc,
                   "delete ok %s", mes);

            st_btu64_dd(&bt, 0);
            ass_iter_all(&bt, cs, idel, mes);

            st_btu64_free(&bt);
        }

        {
            /* delete all */
            bt_declare(bt);
            bt_case_init(bt, cases, i, "del all");

            for (idel = 0; idel < cs->n; idel++) {

                k = cs->ks[ idel ];

                rc = st_btu64_del(&bt, k, 0, ST_BT_FIRST, &deleted);
                st_ut_eq(ST_OK, rc,
                       "delete ok %s", mes);

                dd("after deleting %llu:", k);
                st_btu64_dd(&bt, 0);
            }

            ass_bt_empty(&bt);
            st_btu64_free(&bt);
        }

        {
            /* delete all in random order */
            bt_declare(bt);
            bt_case_init(bt, cases, i, "del all random order");

            int ideleted[ MAX_KEYS ];
            int ndeleted = 0;
            int ii;

            for (idel = 0; idel < cs->n; idel++) {

                ii = fib_hash32(idel) % cs->n;
                while (index_of(ideleted, ndeleted, ii) >= 0) {
                    ii = (ii + 1) % cs->n;
                }
                ideleted[ ndeleted ] = ii;
                ndeleted++;

                k = cs->ks[ ii ];

                rc = st_btu64_del(&bt, k, 0, ST_BT_FIRST, &deleted);
                st_ut_eq(ST_OK, rc,
                       "delete ok %s", mes);

                dd("after deleting %llu:", k);
                st_btu64_dd(&bt, 0);
            }

            ass_bt_empty(&bt);
            st_btu64_free(&bt);
        }
    }

}

st_test(btree, pop) {
    int n;
    int i;
    int j;
    int rc;
    st_btu64_kv_t kv;
    bt_declare(bt);

    struct case_s {
        int      n;
        uint64_t nums[ MAX_KEYS ];
    };

    struct case_s *cs;
    struct case_s cases[] = {

        /* search in empty array */
        { .n = 0, .nums = {}, },
        { .n = 0, .nums = {}, },
    };

#   define add_all()                                                          \
    do {                                                                      \
        for ( j = 0; j < cs->n; j++ ) {                                       \
            kv.k = cs->nums[ j ];                                             \
                    kv.v = cs->nums[ j ];                                     \
                    rc = st_btu64_add( &bt, cs->nums[ j ], 1 );                 \
                    st_ut_eq( ST_OK, rc, "add %llu", kv.k );                \
        }                                                                     \
    } while ( 0 )

    n = sizeof(cases) / sizeof(struct case_s);
    for (i = 0; i < n; i++) {

        cs = &cases[ i ];
        add_all();

        for (j = 0; j < cs->n; j++) {
            /* st_btu64_pop */
        }
    }

#   undef add_all
}

st_ben(btree, binsearch, 2) {
    int         i;
    st_btu64_key_t k;
    st_btu64_node_t ns[ 100 ];
    st_btu64_node_t *old;

    bt_declare(bt);

    /* make room for 64 keys */
    bt.root->n_keys = 64;
    ns[ 0 ] = *bt.root;
    old = bt.root;
    bt.root = &ns[ 0 ];

    for (i = 0; i < 64; i++) {
        bt.root->keys[ i ] = i;
    }

    k = 37;
    for (i = 0; i < n; i++) {
        st_btu64_binsearch_last(&bt, k, ST_BT_VAL_INVALID);
    }

    bt.root = old;

    st_btu64_free(&bt);

}

st_ben(btree, add, 2) {
    int           i;

    bt_declare(bt);

    for (i = 0; i < n; i++) {
        st_btu64_add(&bt, fib_hash64(i), i);
    }
    st_btu64_dd(&bt, 0);
    dinfo("st: n=%llu, node_create=%llu, node_merge=%llu, entry/node=%llu",
          bt.origin->st->n_entry,
          bt.origin->st->n_node_create,
          bt.origin->st->n_node_merge,
          bt.origin->st->n_entry / (bt.origin->st->n_node_create - bt.origin->st->n_node_merge)
         );

    st_btu64_free(&bt);
}

static void *
_make_bt_1m(int n) {
    int         i;
    st_btu64_t   *bt;
    int rc;

    bt = st_malloc(sizeof(st_btu64_t));
    bt->root = NULL;

    rc = st_btu64_init(bt);

    for (i = 0; i < 1024 * 1024; i++) {
        st_btu64_add(bt, i, i);
    }
    return bt;
}

static void
_free_bt(void *data) {
    st_btu64_t *bt;
    bt = data;
    st_btu64_free(bt);
    free(bt);
}


st_bench(btree, search, _make_bt_1m, _free_bt, 2) {
    int              i;
    st_btu64_key_t  k;
    st_btu64_t     *bt;
    st_btu64_rst_t  rst;

    bt = data;

    for (i = 0; i < n; i++) {
        k = i * 2654435769;
        st_btu64_search(bt, k, 0, ST_BT_FIRST, &rst);
    }

}

st_ut_main;
