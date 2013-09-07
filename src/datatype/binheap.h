#ifndef __DATATYPE__BINHEAP_H__
#     define __DATATYPE__BINHEAP_H__

#include <stdint.h>
#include <stdlib.h>


typedef struct st_binheap_node_s st_binheap_node_t;

struct st_binheap_node_s {
    uint64_t  key;
    void     *val;
};

typedef struct st_binheap_s st_binheap_t;

struct st_binheap_s {
    st_binheap_node_t *nodes;
    uint64_t            capa;
    uint64_t            size;
    uint8_t             mem_owned:1;
};

int st_binheap_init_alloc( st_binheap_t *bh, uint64_t capa );
int st_binheap_free( st_binheap_t *bh );
int st_binheap_add( st_binheap_t *bh, uint64_t key, void *val );
st_binheap_node_t* st_binheap_peek( st_binheap_t *bh );
int st_binheap_remove( st_binheap_t *bh );

#endif /* __DATATYPE__BINHEAP_H__ */
