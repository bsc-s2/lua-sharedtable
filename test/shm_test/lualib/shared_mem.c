#define _BSD_SOURCE

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <unistd.h>
#include <sys/types.h>

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#define META_KEY               "shred_mem_meta_key"
#define BASE_ADDR_LEN          (24)

#define SHARED_MEM_PATH        "/shared_dict"
#define SHARED_MEM_REGION_SIZE (10 *1024 * 1024)  /** 10 MB, 2560 pages */
#define SHARED_MEM_REGION_NUM  (10)


typedef struct shared_mem_ud {
    uint64_t shm_fd;
    uint64_t region_num;
    uint64_t region_size;
    uint8_t  *shm_base_ptr;
    char     base_addr_str[BASE_ADDR_LEN];
} shared_mem_ud_s;


static int
make_shared_memory(shared_mem_ud_s *sm_ptr)
{
    sm_ptr->shm_fd = shm_open(SHARED_MEM_PATH, O_CREAT|O_RDWR|O_TRUNC, 0666);
    if (-1 == sm_ptr->shm_fd) {
        return -1;
    }

    sm_ptr->region_num = SHARED_MEM_REGION_NUM;
    sm_ptr->region_size = SHARED_MEM_REGION_SIZE;

    uint64_t nbytes = sm_ptr->region_num * sm_ptr->region_size;
    if (0 != ftruncate(sm_ptr->shm_fd, nbytes)) {
        goto err_quit;
    }

    sm_ptr->shm_base_ptr = mmap(NULL,
                                nbytes,
                                PROT_READ | PROT_WRITE,
                                MAP_SHARED,
                                sm_ptr->shm_fd,
                                0);
    if (MAP_FAILED == sm_ptr->shm_base_ptr) {
        goto err_quit;
    }

    return 0;

err_quit:
    close(sm_ptr->shm_fd);
    shm_unlink(SHARED_MEM_PATH);

    return -1;
}


static uintptr_t
get_uaddr(lua_State *L, const char *uaddr_str)
{
    char *endptr = NULL;

    uintptr_t addr = strtoull(uaddr_str, &endptr, 10);
    luaL_argcheck(L, 2, '\0' != *endptr, "'addr' is not number string");

    return addr;
}


static int
test_new(lua_State *L)
{
    uint32_t nbytes = sizeof(shared_mem_ud_s);
    shared_mem_ud_s *sm_ptr = (shared_mem_ud_s *)lua_newuserdata(L, nbytes);

    luaL_getmetatable(L, META_KEY);
    lua_setmetatable(L, -2);

    const char *uaddr_str = luaL_checkstring(L, 1);
    uintptr_t addr = get_uaddr(L, uaddr_str);

    if (0 == addr) {
        /** create shared memory  */
        printf("make shared memory from scrash\n");
        if (-1 == make_shared_memory(sm_ptr)) {
            luaL_error(L, "failed to make_shared_memory: %s", strerror(errno));
        }

    } else {
        printf("make shared memory from initialized one\n");
        sm_ptr->shm_fd = -1;
        sm_ptr->shm_base_ptr = (uint8_t *)addr;
        sm_ptr->region_num = SHARED_MEM_REGION_NUM;
        sm_ptr->region_size = SHARED_MEM_REGION_SIZE;
    }

    memset(sm_ptr->base_addr_str, 0, BASE_ADDR_LEN);
    int ret = snprintf(sm_ptr->base_addr_str,
                       BASE_ADDR_LEN,
                       "%lu",
                       (uintptr_t)sm_ptr->shm_base_ptr);
    if (0 > ret || ret > BASE_ADDR_LEN) {
        luaL_error(L, "failed to make base addr string");
    }

    return 1;
}


static shared_mem_ud_s *
test_check_ud(lua_State *L)
{
    void *ud = luaL_checkudata(L, 1, META_KEY);
    luaL_argcheck(L, ud != NULL, 1, "type 'shared_mem_ud_s' expected");

    return (shared_mem_ud_s *)ud;
}


static int
test_get_base_addr_str(lua_State *L)
{
    shared_mem_ud_s *sm_ptr = test_check_ud(L);
    lua_pushstring(L, sm_ptr->base_addr_str);

    return 1;
}


// index start from 0
// test_str:release_region(0)
static int
test_release_region(lua_State *L)
{
    shared_mem_ud_s *sm_ptr = test_check_ud(L);

    int index = luaL_checknumber(L, 2);
    luaL_argcheck(L,
                  2,
                  index >= 0 && index <= SHARED_MEM_REGION_NUM,
                  "index out of range");

    uint8_t *base = sm_ptr->shm_base_ptr + index * SHARED_MEM_REGION_SIZE;

    int ret = madvise(base, SHARED_MEM_REGION_SIZE, MADV_REMOVE);
    if (ret != 0) {
        luaL_error(L, "failed to release shared memory: %s", strerror(errno));
    }

    return 0;
}


static int
get_page_size(void)
{
    return sysconf(_SC_PAGESIZE);
}


static int
test_force_load_pages(lua_State *L)
{
    shared_mem_ud_s *sm_ptr = test_check_ud(L);

    uint32_t page_size = (uint32_t)get_page_size();
    int page_per_region = sm_ptr->region_size / page_size ;
    int page_num = sm_ptr->region_num * page_per_region;
    // printf("lsl-debug: page_num: %d\n", page_num);

    uint8_t *start = sm_ptr->shm_base_ptr - page_size;
    for (int idx = 0; idx < page_num; idx++) {
        start += page_size;

        start[0] = 1;
    }

    return 0;
}


static int
test_gc(lua_State *L)
{
    printf("test_gc is called\n");
    shared_mem_ud_s *sm_ptr = test_check_ud(L);
    munmap(sm_ptr->shm_base_ptr, sm_ptr->region_num*sm_ptr->region_size);

    close(sm_ptr->shm_fd);
    shm_unlink(SHARED_MEM_PATH);

    return 0;
}

static const luaL_Reg userdata_func[] = {
    {"new", test_new},
    {NULL, NULL},
};


static const luaL_Reg meta_func[] = {
    {"load_pages",        test_force_load_pages},
    {"release_region",    test_release_region},
    {"get_base_addr_str", test_get_base_addr_str},
    {"__gc",              test_gc},
    {"destory",           test_gc},
    {NULL,                NULL},
};


int
luaopen_libshared_dict(lua_State *L)
{
    luaL_newmetatable(L, META_KEY);
    lua_pushstring(L, "__index");
    lua_pushvalue(L, -2);
    lua_settable(L, -3);

    luaL_register(L, NULL, meta_func);

    lua_newtable(L);
    luaL_register(L, "libshared_dict", userdata_func);

    return 1;
}


