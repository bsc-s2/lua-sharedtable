#include "btree.h"
#include <assert.h>

int
st_btu64_init(st_btu64_t *bt) {
    if (bt == NULL || bt->root != NULL) {
        return ST_ARG_INVALID;
    }

    bt->level = 0;
    bt->origin = bt;

    bt->st = st_malloc(sizeof(st_bt_stat_t));
    if (bt->st == NULL) {
        return ST_OUT_OF_MEMORY;
    }
    memset(bt->st, 0, sizeof(*bt->st));

    /* initially, root is a leaf node */
    bt->root = st_btu64_allocnode(bt, 0);

    if (bt->root == NULL) {
        st_free(bt->st);
        return ST_OUT_OF_MEMORY;
    }

    return ST_OK;
}

int
st_btu64_new_root(st_btu64_t *bt) {
    st_btu64_node_t *newroot;

    dd("create new root for spliting root");

    newroot = st_btu64_allocnode(bt, bt->level + 1);
    if (newroot == NULL) {
        return ST_OUT_OF_MEMORY;
    }

    newroot->children[ 0 ] = bt->root;

    bt->root = newroot;
    bt->level++;
    return ST_OK;
}

int
st_btu64_add(st_btu64_t *bt, st_btu64_key_t k, st_btu64_val_t v) {
    st_btu64_t       b;
    st_btu64_node_t *child;
    int               rc;
    int               l;
    int               ichild;

    dd("BTREE-ADD: root.n_keys=%d", bt->root->n_keys);

    if (bt->root->n_keys == ST_BT_MAX_N_KEYS) {
        /* root is full, need to split */
        rc = st_btu64_new_root(bt);
        if (rc < 0) {
            return rc;
        }
    }

    for (b = *bt; b.level > 0; b.level--) {

        l = st_btu64_binsearch_last(&b, k, v);

        child = b.root->children[ l + 1 ];

        if (child->n_keys == ST_BT_MAX_N_KEYS) {

            ichild = l + 1;

            rc = st_btu64_split_child(&b, ichild);
            if (rc < 0) {
                return rc;
            }

            if (k < b.root->keys[ ichild ]
                    || (k == b.root->keys[ ichild ]
                        && v < b.root->vals[ ichild ])) {
                b.root = child;
            } else {
                b.root = b.root->children[ ichild + 1 ];
            }
        } else {
            b.root = child;
        }
    }

    /* at leaf node */
    l = st_btu64_binsearch_last(&b, k, v);

    rc = st_btu64_add_to_node(&b, l + 1, k, v, NULL);
    bt->origin->st->n_entry++;
    return rc;
}

int
st_btu64_pop(st_btu64_t *bt, st_btu64_flag_t flag) {
    return ST_OK;
}

int
st_btu64_del(st_btu64_t *bt, st_btu64_key_t k, st_btu64_val_t v,
             st_btu64_flag_t flag, st_btu64_kv_t *deleted) {
    int               rc;
    st_btu64_t       b;
    st_btu64_rst_t   r;
    st_btu64_rst_t  *rst;
    st_btu64_kv_t    minkv;
    st_btu64_node_t *node;
    st_btu64_node_t *child;
    int               ifound;

    dd("BTREE-DEL: %llu", k);

    node = bt->root;
    if (node->n_keys == 0) {

        if (bt->root->children[ 0 ] != NULL) {
            dd("BTREE-SHRINK: level %d to %d", bt->level, bt->level - 1);
            bt->root = bt->root->children[ 0 ];
            bt->level--;
            st_free(node);
        } else {
            return ST_NOT_FOUND;
        }
    }

    rst = &r;

    st_btu64_dnode(bt, bt->root);

    ifound = -1;
    for (b = *bt; b.level > 0; b.level--) {

        if (ifound == -1) {
            st_btu64_binsearch(&b, k, v, flag, rst);
            ifound = rst->i;
        }

        dlog("search k, v to del in node: [");
        st_btu64_dkvs_inl(&b, b.root);
        ddv("] ifound is: %d\n", ifound);

        if (ifound == -1) {   /* not found */

            child = b.root->children[ rst->left + 1 ];

            assert(child->n_keys >= ST_BT_MIN_N_KEYS);

            if (child->n_keys == ST_BT_MIN_N_KEYS) {

                if (st_btu64_canmerge(&b, rst->left)) {
                    dd("not found, can merge left");
                    st_btu64_merge(&b, rst->left);
                    child = b.root->children[ rst->left ];
                } else if (st_btu64_canmerge(&b, rst->right)) {
                    dd("not found, can merge right");
                    st_btu64_merge(&b, rst->right);
                } else {
                    dd("not found, rotate");
                    if (st_btu64_rotateleft(&b, rst->right) == ST_ARG_INVALID) {
                        rc = st_btu64_rotateright(&b, rst->left);
                        assert(rc == ST_OK);
                    }
                    st_btu64_dd(bt, 0);
                }
            }

            b.root = child;
            ifound = -1;
        } else { /* found */

            if (st_btu64_canmerge(&b, ifound)) {
                /* merging push the k, v found one level down */

                dd("found, merge and push down");

                int ii;
                ii = b.root->children[ ifound ]->n_keys;

                st_btu64_merge(&b, ifound);
                b.root = b.root->children[ ifound ];
                ifound = ii;

            } else if (st_btu64_rotateleft(&b, ifound) == ST_OK) {
                dd("after rotate left:");
                st_btu64_dd(bt, 0);

                b.root = b.root->children[ ifound ];
                ifound = b.root->n_keys - 1;
                dd("after rotate left, ifound=%d", ifound);
            } else if (st_btu64_rotateright(&b, ifound) == ST_OK) {
                b.root = b.root->children[ ifound + 1 ];
                ifound = 0;
            } else {
                /* both left and right child is full */

                deleted->k = b.root->keys[ ifound ];
                deleted->v = b.root->vals[ ifound ];

                rc = st_btu64_delmin(&b, ifound, &minkv);
                if (rc != ST_OK) {
                    return rc;
                }

                b.root->keys[ ifound ] = minkv.k;
                b.root->vals[ ifound ] = minkv.v;

                bt->origin->st->n_entry--;

                return ST_OK;
            }
        }
    }

    dd("at leaf: ");
    st_btu64_dd(&b, 0);
    dd(" ifound is: %d", ifound);

    /* at leaf node */
    if (ifound == -1) {
        st_btu64_binsearch(&b, k, v, flag, rst);
        ifound = rst->i;
    }

    if (ifound == -1) {
        return ST_NOT_FOUND;
    }

    deleted->k = b.root->keys[ ifound ];
    deleted->v = b.root->vals[ ifound ];
    st_btu64_node_shiftleft(&b, ifound, 1);

    bt->origin->st->n_entry--;

    return ST_OK;
}

int
st_btu64_search(st_btu64_t *bt, st_btu64_key_t k, st_btu64_val_t v,
                st_btu64_flag_t flag, st_btu64_rst_t *rst) {
    st_btu64_rst_t r = { 0 };
    st_btu64_t     b;

    for (b = *bt; b.level > 0; b.level--) {

        st_btu64_binsearch(&b, k, v, flag, rst);
        if (rst->i != -1) {
            r = *rst;

            if (flag & ST_BT_FIRST) {
                b.root = b.root->children[ rst->i ];
            } else {
                b.root = b.root->children[ rst->i + 1 ];
            }
        } else {
            b.root = b.root->children[ rst->left + 1 ];
        }
    }

    /* search inside leaf node. */
    st_btu64_binsearch(&b, k, v, flag, rst);
    if (rst->i != -1) {
        r = *rst;
    }

    if (r.node != NULL) {
        *rst = r;
        return ST_OK;
    } else {
        return ST_NOT_FOUND;
    }
}

int
st_btiter_init(st_btu64_t *bt, st_btu64_iter_t *it) {
    assert(bt->level <= ST_BTITER_MAX_LEVEL);

    it->bt = bt;
    it->level = bt->level;
    it->nodes[ it->level ] = bt->root;
    it->indexes[ it->level ] = -1;

    return ST_OK;
}

st_btu64_kv_t *
st_btiter_next(st_btu64_iter_t *it) {

    /*               /` here everytime we enter this function.
     *              v
     *          node
     *       /        \
     *  l-child         r-child
     */
    st_btu64_node_t *child;

    while (1) {
        /* dd( "level:%d, node:%p, i:%d", it->level, ST_BTITER_NODE( it ), ST_BTITER_I( it ) ); */

        while (ST_BTITER_I(it) == ST_BTITER_NODE(it)->n_keys) {
            it->level++;
            if (it->level > it->bt->level) {
                return NULL;
            }

            ST_BTITER_I(it)++;
            if (ST_BTITER_I(it) == ST_BTITER_NODE(it)->n_keys) {
                continue;
            } else {
                it->kv.k = ST_BTITER_CUR_KEY(it);
                it->kv.v = ST_BTITER_CUR_VAL(it);
                return &it->kv;
            }
        }

        if (it->level == 0) {
            ST_BTITER_I(it)++;
            if (ST_BTITER_I(it) == ST_BTITER_NODE(it)->n_keys) {
                continue;
            } else {
                it->kv.k = ST_BTITER_CUR_KEY(it);
                it->kv.v = ST_BTITER_CUR_VAL(it);
                return &it->kv;
            }
        } else {

            child = ST_BTITER_NODE(it)->children[ ST_BTITER_I(it) + 1 ];

            it->level--;
            ST_BTITER_NODE(it) = child;
            ST_BTITER_I(it) = -1;
        }
    }
}

int
st_btu64_add_to_node(st_btu64_t *b, int at,
                     st_btu64_key_t k, st_btu64_val_t v, st_btu64_node_t *child) {
    /*
     * add a node to an inner node, at position <at>. echo key, value,
     * children at or postcede moves 1 forward.
     */

    st_btu64_node_shiftright(b, at, 1);
    st_btu64_setentry(b, at, k, v, child);
    return ST_OK;
}

/* set k, v and its right child if it is non-leaf node */
void
st_btu64_setentry(st_btu64_t *b, int at,
                  st_btu64_key_t k, st_btu64_val_t v, st_btu64_node_t *child) {
    b->root->keys[ at ] = k;
    b->root->vals[ at ] = v;

    if (b->level > 0) {
        assert(child != NULL);
        b->root->children[ at + 1 ] = child;
    }
}

int
st_btu64_split_child(st_btu64_t *b, int32_t ichild) {
    st_btu64_node_t *nd;
    st_btu64_node_t *child;
    int             j;
    int             mid;

    assert(b->level >= 1);

    dd("split child, level=%d at:%d", b->level, ichild);
    st_btu64_dd(b, 0);
    child = b->root->children[ ichild ];

    assert(child->n_keys == ST_BT_MAX_N_KEYS);

    nd = st_btu64_allocnode(b, b->level - 1);
    if (nd == NULL) {
        return ST_OUT_OF_MEMORY;
    }

    mid = ST_BT_MIN_N_KEYS;
    dd("split mid: %d", mid);

    nd->n_keys = child->n_keys - mid - 1;
    dd("new node n_keys:%d", nd->n_keys);

    for (j = 0; j < nd->n_keys; j++) {
        nd->keys[ j ] = child->keys[ mid + 1 + j ];
        nd->vals[ j ] = child->vals[ mid + 1 + j ];
        dd("moved k, v: k=%llu", child->keys[ mid + 1 + j ]);
    }
    if (b->level - 1 > 0) {
        for (j = 0; j < nd->n_keys + 1; j++) {
            nd->children[ j ] = child->children[ mid + 1 + j ];
        }
    }

    /* add spliter key-value to its parent */

    st_btu64_add_to_node(b, ichild,
                         child->keys[ mid ],
                         child->vals[ mid ],
                         nd);

    child->n_keys = mid;

    dd("after split:");
    st_btu64_dd(b, 0);

    return ST_OK;
}

st_btu64_node_t *
st_btu64_allocnode(st_btu64_t *bt, uint8_t level) {
    st_btu64_node_t *nd;
    size_t          size;

    if (level == 0) {
        size = sizeof(st_btu64_leaf_t);
    } else {
        size = sizeof(st_btu64_node_t);
    }

    nd = (st_btu64_node_t *)st_malloc(size);

    if (nd == NULL) {
        return NULL;
    }

    memset(nd, 0, size);
    bt->origin->st->n_node_create++;
    return nd;
}

void
st_btu64_free(st_btu64_t *b) {
    int      i;
    st_btu64_t bchild;

    if (b->level > 0) {

        bchild = *b;
        bchild.level--;

        for (i = 0; i < b->root->n_keys + 1; i++) {
            bchild.root = b->root->children[ i ];
            st_btu64_free(&bchild);
        }
    }

    st_free(b->root);
    b->root = NULL;

    if (b == b->origin) {
        st_free(b->st);
    }
}

int
st_btu64_binsearch(st_btu64_t *b, st_btu64_key_t k, st_btu64_val_t v,
                   st_btu64_flag_t flag, st_btu64_rst_t *rst) {
    /*
     * stop condition:
     * keys[ rst.l ] <= key <= key[ rst.r ]
     *
     * search first:
     *  keys[rst.l] < key <= keys[rst.r]
     *
     * search last:
     *  keys[rst.l] <= key < keys[rst.r]
     */

    int32_t           l;
    int32_t           i;
    st_btu64_node_t *node;

    node = b->root;
    rst->node = b->root;
    rst->i = -1;

    if (!(flag & ST_BT_CMP_VAL)) {
        v = ST_BT_VAL_INVALID;
    }

    if (flag & ST_BT_FIRST) {
        l = st_btu64_binsearch_first(b, k, v);
        i = l + 1;
    } else {
        l = st_btu64_binsearch_last(b, k, v);
        i = l;
    }

    if (i >= 0 && i < node->n_keys
            && b->root->keys[ i ] == k
            && (v == ST_BT_VAL_INVALID || v == b->root->vals[ i ])) {
        rst->i = i;
    }

    rst->left = l;
    rst->right = l + 1;
    return ST_OK;
}

int32_t
st_btu64_binsearch_first(st_btu64_t *b,
                         st_btu64_key_t k, st_btu64_val_t v) {
    int32_t l;
    int32_t mid;
    int32_t r;

    r = b->root->n_keys;
    l = -1;

    while (r - l > 1) {

        mid = (l + r) >> 1;

        if (k > b->root->keys[ mid ]
                || (k == b->root->keys[ mid ]
                    && v != ST_BT_VAL_INVALID && v > b->root->vals[ mid ])) {
            l = mid;
        } else {
            r = mid;
        }
    }
    return l;
}

int32_t
st_btu64_binsearch_last(st_btu64_t *b,
                        st_btu64_key_t k, st_btu64_val_t v) {
    int32_t l;
    int32_t mid;
    int32_t r;

    r = b->root->n_keys;
    l = -1;

    dd("k=%llu, v=%llu", k, v);
    while (r - l > 1) {

        mid = (l + r) >> 1;
        dd("%d, %d, %d", l, mid, r);
        dd("%llu", b->root->vals[ mid ]);

        if (k < b->root->keys[ mid ]
                || (k == b->root->keys[ mid ]
                    && v != ST_BT_VAL_INVALID && v < b->root->vals[ mid ])) {
            r = mid;
        } else {
            l = mid;
        }
    }
    dd("%d, %d", l, r);
    return l;
}

void
st_btu64_merge(st_btu64_t *b, int at) {
    int             i;
    st_btu64_node_t *l;
    st_btu64_node_t *r;
    st_btu64_node_t *parent;

    parent = b->root;

    assert(at >= 0);
    assert(at < parent->n_keys);
    assert(b->level >= 1);

    l = b->root->children[ at ];
    r = b->root->children[ at + 1 ];

    dd("merge: %d-th kv", at);
    dd("merge parent:");
    st_btu64_dnode(b, parent);
    dd("merge left child:");
    st_btu64_dnode(b, l);
    dd("merge right child:");
    st_btu64_dnode(b, r);


    assert(l->n_keys == ST_BT_MIN_N_KEYS);
    assert(r->n_keys == ST_BT_MIN_N_KEYS);


    l->keys[ l->n_keys ] = parent->keys[ at ];
    l->vals[ l->n_keys ] = parent->vals[ at ];
    l->n_keys++;

    for (i = 0; i < r->n_keys; i++) {
        l->keys[ l->n_keys + i ] = r->keys[ i ];
        l->vals[ l->n_keys + i ] = r->vals[ i ];
    }

    if (b->level - 1 > 0) {
        for (i = 0; i <= r->n_keys; i++) {
            l->children[ l->n_keys + i ] = r->children[ i ];
        }
    }
    l->n_keys += r->n_keys;

    r->n_keys = 0;
    st_free(r);

    st_btu64_node_shiftleft(b, at, 1);

    b->origin->st->n_node_merge++;

    dd("merge finished:");
    st_btu64_dd(b->origin, 0);
}

int
st_btu64_delmin(st_btu64_t *b, int i_after_which, st_btu64_kv_t *deleted) {
    /* nd->kvs[ i_after_which ] must not be modified */

    int           i;
    int           rc;
    st_btu64_node_t *child;
    st_btu64_t      b2;

    assert(b->origin->root == b->root || b->root->n_keys > ST_BT_MIN_N_KEYS);

    i = i_after_which;
    if (i >= b->root->n_keys) {
        return ST_NOT_FOUND;
    }

    for (b2 = *b; b2.level > 0; b2.level--) {

        child = b2.root->children[ i + 1 ];
        assert(child->n_keys >= ST_BT_MIN_N_KEYS);

        if (child->n_keys == ST_BT_MIN_N_KEYS) {

            if (st_btu64_canmerge(&b2, i + 1)) {
                st_btu64_merge(&b2, i + 1);
            } else {
                rc = st_btu64_rotateleft(&b2, i + 1);
                assert(rc == ST_OK && "rotate left must success");
            }
        }

        b2.root = child;
        i = -1;
    }

    i++;
    if (i < b2.root->n_keys) {

        deleted->k = b2.root->keys[ i ];
        deleted->v = b2.root->vals[ i ];
        st_btu64_node_shiftleft(&b2, i, 1);

        return ST_OK;
    } else {
        return ST_NOT_FOUND;
    }
}

int
st_btu64_canmerge(st_btu64_t *b, int32_t at) {
    dd("check if can merge:");
    st_btu64_dd(b, 0);

    if (at < 0 || at >= b->root->n_keys) {
        return 0;
    }

    return b->root->children[ at ]->n_keys == ST_BT_MIN_N_KEYS
           && b->root->children[ at + 1 ]->n_keys == ST_BT_MIN_N_KEYS;
}

int
st_btu64_rotateleft(st_btu64_t *b, int32_t i) {
    st_btu64_t       br;
    st_btu64_node_t *nd;
    st_btu64_node_t *l;
    st_btu64_node_t *r;

    assert(b->level > 0 && "b->root must be a non-leaf node");

    br = *b;
    br.level--;

    nd = b->root;

    dd("BT-ROTATE-LEFT");

    if (i < 0 || i >= nd->n_keys) {
        return ST_ARG_INVALID;
    }


    l = b->root->children[ i ];
    r = b->root->children[ i + 1 ];

    if (l->n_keys < ST_BT_MAX_N_KEYS
            && r->n_keys > ST_BT_MIN_N_KEYS) {

        dd("start rotate left");

        br.root = l;
        if (br.level == 0) {
            st_btu64_add_to_node(&br, l->n_keys,
                                 nd->keys[ i ], nd->vals[ i ], NULL);
        } else {
            st_btu64_add_to_node(&br, l->n_keys,
                                 nd->keys[ i ], nd->vals[ i ],
                                 r->children[ 0 ]);
        }

        dd("after add to left:");
        st_btu64_dd(b, 0);

        nd->keys[ i ] = r->keys[ 0 ];
        nd->vals[ i ] = r->vals[ 0 ];

        if (b->level > 1) {
            r->children[ 0 ] = r->children[ 1 ];
        }

        dd("after set parent:");
        st_btu64_dd(b, 0);

        br.root = r;
        st_btu64_node_shiftleft(&br, 0, 1);

        dd("after rotate left:");
        st_btu64_dd(b, 0);

        return ST_OK;

    } else {
        /* not enough node on right or full on left */
        return ST_ARG_INVALID;
    }
}

int
st_btu64_rotateright(st_btu64_t *b, int i) {
    st_btu64_node_t *l;
    st_btu64_node_t *r;
    st_btu64_node_t *nd;
    st_btu64_t      br;

    dd("BT-ROTATE-RIGHT");

    nd = b->root;

    if (i < 0 || i >= nd->n_keys) {
        return ST_ARG_INVALID;
    }

    l = b->root->children[ i ];
    r = b->root->children[ i + 1 ];

    if (l->n_keys > ST_BT_MIN_N_KEYS && r->n_keys < ST_BT_MAX_N_KEYS) {

        dd("start rotate right");

        br = *b;
        br.root = r;
        br.level--;

        st_btu64_node_shiftright(&br, 0, 1);

        r->keys[ 0 ] = nd->keys[ i ];
        r->vals[ 0 ] = nd->vals[ i ];
        if (b->level > 1) {
            r->children[ 1 ] = r->children[ 0 ];
            r->children[ 0 ] = l->children[ l->n_keys ];
        }

        nd->keys[ i ] = l->keys[ l->n_keys - 1 ];
        nd->vals[ i ] = l->vals[ l->n_keys - 1 ];

        l->n_keys--;
        return ST_OK;

    } else {
        /* not enough node on left or full on right */
        return ST_ARG_INVALID;
    }
}

/* move kvs left without the left-most child */
void
st_btu64_node_shiftleft(st_btu64_t *b, int from, int shift) {
    int32_t         i;
    st_btu64_node_t *nd;

    nd = b->root;

    assert(b->origin->root == nd || nd->n_keys - shift >= ST_BT_MIN_N_KEYS);

    for (i = from; i < nd->n_keys - shift; i++) {
        nd->keys[ i ] = nd->keys[ i + shift ];
        nd->vals[ i ] = nd->vals[ i + shift ];
    }
    if (b->level > 0) {
        for (i = from + 1; i < nd->n_keys - shift + 1; i++) {
            dd("child %d now is child %d", i, i + shift);
            nd->children[ i ] = nd->children[ i + shift ];
        }
        for (i = nd->n_keys - shift + 1; i < nd->n_keys + 1; i++) {
            nd->children[ i ] = NULL;
        }
    }
    nd->n_keys -= shift;
}

/* move (n_keys - from) kvs rght without the left-most child */
void
st_btu64_node_shiftright(st_btu64_t *b, int from, int shift) {
    int32_t       i;
    st_btu64_node_t *nd;

    nd = b->root;

    assert(b->root->n_keys + shift <= ST_BT_MAX_N_KEYS);

    for (i = nd->n_keys - 1; i >= from; i--) {
        nd->keys[ i + shift ] = nd->keys[ i ];
        nd->vals[ i + shift ] = nd->vals[ i ];
    }
    if (b->level > 0) {
        for (i = nd->n_keys; i > from; i--) {
            nd->children[ i + shift ] = nd->children[ i ];
        }
    }
    nd->n_keys += shift;
}

char *
st_bt_flagstr(st_btu64_flag_t flag) {
    if (flag & ST_BT_FIRST) {
        if (flag & ST_BT_CMP_VAL) {
            return "first.key_val";
        } else {
            return "first.key";
        }
    } else {
        if (flag & ST_BT_CMP_VAL) {
            return "last.key_val";
        } else {
            return "last.key";
        }
    }
}

#ifdef ST_DEBUG

void
st_btu64_dnodeinfo_ln(struct st_btnode_dbg_s *dbg) {
    if (dbg->verbose) {
        dd("%s" "btnode %p lvl=%d, n=%d", dbg->pad, dbg->node, dbg->level, dbg->node->n_keys);
    }
}

void
st_btu64_dkv_ln(struct st_btnode_dbg_s *dbg, int i) {
    char *pad;
    st_btu64_kv_t *kv;

    UNUSED(kv);
    UNUSED(pad);

    pad = dbg->pad;

    if (dbg->verbose) {
        dd("%s" "%llu, %llu",
           pad,
           dbg->node->keys[ i ],
           dbg->node->vals[ i ]);
    } else {
        dd("%s" "%llu", pad, dbg->node->keys[ i ]);
    }
}

void
st_btu64_dkvs_inl(st_btu64_t *bt, st_btu64_node_t *nd) {
    int i;
    for (i = 0; i < nd->n_keys; i++) {
        ddv("%llu, ", nd->keys[ i ]);
    }
}

void
st_btu64_dnode(st_btu64_t *bt, st_btu64_node_t *node) {
    char                    space[ 1024 ] = { 0 };
    struct st_btnode_dbg_s dbg;

    UNUSED(dbg);

    dbg.bt = bt;
    dbg.pad = space;
    dbg.level = bt->level;
    dbg.node = node;
    dbg.verbose = 1;

    st_btu64_dnodeinfo_ln(&dbg);
    dlog("");
    st_btu64_dkvs_inl(dbg.bt, node);
    ddv("\n");
}

void
st_btu64_dtree(struct st_btnode_dbg_s *dbg) {
    int                      i;
    int                      l;
    char                    *pad;
    st_btu64_node_t        *nd;
    struct st_btnode_dbg_s  mydbg;

    mydbg = *dbg;
    nd = dbg->node;
    pad = dbg->pad;
    l = strlen(pad);

    if (dbg->level == 0) {

        if (mydbg.verbose) {
            strcpy(&pad[ l ], "|-");
            for (i = 0; i < nd->n_keys - 1; i++) {
                st_btu64_dkv_ln(&mydbg, i);
            }

            strcpy(&pad[ l ], "`-");
            st_btu64_dkv_ln(&mydbg, i);
        } else {

            strcpy(&pad[ l ], "`-");
            dlog("%s", pad);

            st_btu64_dkvs_inl(mydbg.bt, nd);
            ddv("\n");
        }

    } else {
        mydbg.level = dbg->level - 1;
        for (i = 0; i < nd->n_keys; i++) {

            mydbg.node = nd->children[ i ];

            strcpy(&pad[ l ], "|+");
            st_btu64_dnodeinfo_ln(&mydbg);

            strcpy(&pad[ l ], "| ");
            st_btu64_dtree(&mydbg);

            strcpy(&pad[ l ], "|-");
            mydbg.node = nd;
            st_btu64_dkv_ln(&mydbg, i);
        }

        strcpy(&pad[ l ], "`-");
        mydbg.node = nd->children[ i ];
        st_btu64_dnodeinfo_ln(&mydbg);

        strcpy(&pad[ l ], "  ");
        mydbg.node = nd->children[ nd->n_keys ];
        st_btu64_dtree(&mydbg);
    }
}

void
st_btu64_dd(st_btu64_t *bt, int verbose) {
    char                     pad[1024] = { 0 };
    st_bt_stat_t           *st;
    struct st_btnode_dbg_s  dbg;

    st = bt->origin->st;

    UNUSED(dbg);

    dbg.bt = bt;
    dbg.node = bt->root;
    dbg.level = bt->level;
    dbg.pad = pad;
    dbg.verbose = verbose;

    dd("st: n=%llu, node_create=%llu, node_merge=%llu, entry/node=%llu, %llu%%",
       st->n_entry,
       st->n_node_create,
       st->n_node_merge,
       st->n_entry / (st->n_node_create - st->n_node_merge),
       st->n_entry * 100 / (st->n_node_create - st->n_node_merge) / ST_BT_MAX_N_KEYS

      );

    st_btu64_dnodeinfo_ln(&dbg);
    st_btu64_dtree(&dbg);
}
#endif /* ST_DEBUG */
