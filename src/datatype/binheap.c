#include "binheap.h"

static inline void
swap(st_binheap_node_t *a, st_binheap_node_t *b) {
    st_binheap_node_t tmp;
    tmp = *a;
    *a = *b;
    *b = tmp;
}

int
st_binheap_init_alloc(st_binheap_t *bh, uint64_t capa) {
    st_binheap_t *bh;

    if (capa < 1) {
        return st_INVALID_ARG;
    }

    bh->nodes = (st_binheap_node_t *)malloc(sizeof(st_binheap_node_t) * capa);
    if (NULL == bh->nodes) {
        return st_ERR;
    }

    bh->capa = capa;
    bh->size = 0; /* the first node is not used for convinience of operation */
    bh->mem_owned = 1;

    /* placeholder */
    bh->nodes[ 0 ]->key = 0;
    bh->nodes[ 0 ]->val = NULL;

    return 0;
}

int
st_binheap_free(st_binheap_t *bh) {
    if (bh->mem_owned) {
        free(bh->nodes);
        bh->mem_owned = 0;
    }

    return 0;
}

int
st_binheap_add(st_binheap_t *bh, uint64_t key, void *val) {
    st_binheap_node_t *node;
    st_binheap_node_t *parent;
    uint64_t             i;
    uint64_t             iparent;

    /* the first node is just a place holder */
    if (bh->size == bh->capa - 1) {
        return st_OUTOFMEM;
    }

    bh->size++;

    node = &bh->nodes[ size ];
    node->key = key;
    node->val = val;

    i = bh->size;

    while (bh->nodes[ i / 2 ].key > node->key) {
        swap(node, parent);
        i = i / 2;
    }
    return st_OK;
}

st_binheap_node_t *
st_binheap_peek(st_binheap_t *bh) {
    if (bh->size == 1) {
        return NULL;
    }

    return &bh->nodes[ 1 ];
}

int
st_binheap_remove(st_binheap_t *bh) {
    uint64_t i;
    uint64_t l;
    uint64_t r;
    uint64_t to;

    if (bh->size == 1) {
        return st_EOF;
    }

    i = 1;

    while ((i << 1) < capa) {
        l = i << 1;
        r = l | 1;

        if (r >= capa) {
            to = l;
        } else if (bh->nodes[ l ].key < bh->nodes[ r ].key) {
            to = l;
        } else {
            to = r;
        }
        swap(&bh->nodes[ i ], &bh->nodes[ to ]);
        i = to;
    }
    if (i != bh->size) {
        swap(bh->nodes[ i ], bh->nodes[ bh->size ]);
    }
    bh->size--;

    return st_OK;
}

