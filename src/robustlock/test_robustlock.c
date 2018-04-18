#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "robustlock.h"
#include "unittest/unittest.h"

void *share_alloc(int size) {
    return mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
}

pthread_mutex_t *alloc_lock() {
    pthread_mutex_t *lock = NULL;

    lock = share_alloc(sizeof(*lock));

    st_robustlock_init(lock);

    return lock;
}

void free_lock(pthread_mutex_t *lock) {
    int ret = -1;

    ret = st_robustlock_destroy(lock);
    st_ut_eq(ST_OK, ret, "lock destroy");

    munmap(lock, sizeof(*lock));
}

void create_lock_loop_children(pthread_mutex_t *lock, int *pids,
                               int pid_num, int *stop_flag, int interval) {
    int child = 0;

    for (int i = 0; i < pid_num; i++) {

        child = fork();

        if (child == 0) {

            while (1) {
                st_robustlock_lock(lock);

                usleep(interval);

                st_robustlock_unlock(lock);

                usleep(1);

                if (*stop_flag == 1) {
                    exit(0);
                }
            }
        }

        pids[i] = child;
    }
}

void wait_children(int *pids, int pid_num) {
    int ret = -1;

    for (int i = 0; i < pid_num; i++) {
        waitpid(pids[i], &ret, 0);
        st_ut_eq(ST_OK, ret, "check children return status");
    }
}

st_test(robustlock, init) {
    int ret = -1;
    pthread_mutex_t *lock = NULL;

    lock = share_alloc(sizeof(*lock));

    ret = st_robustlock_init(lock);
    st_ut_eq(ST_OK, ret, "init with alloced lock");

    st_ut_bug(st_robustlock_init(NULL));

    free_lock(lock);
}

st_test(robustlock, destroy) {
    int ret = -1;
    pthread_mutex_t *lock = NULL;

    lock = alloc_lock();

    ret = st_robustlock_destroy(lock);
    st_ut_eq(ST_OK, ret, "destroy with alloced lock");

    ret = st_robustlock_destroy(NULL);
    st_ut_eq(ST_ARG_INVALID, ret, "destroy with unalloced lock");

    munmap(lock, sizeof(*lock));
}


st_test(robustlock, lock) {
    pthread_mutex_t *lock = NULL;

    lock = alloc_lock();

    st_ut_nobug(st_robustlock_lock(lock), "lock with alloced lock");

    st_ut_bug(st_robustlock_lock(lock), "lock twice");
    st_ut_bug(st_robustlock_lock(NULL), "lock NULL");

    free_lock(lock);
}

st_test(robustlock, unlock) {
    pthread_mutex_t *lock = NULL;

    lock = alloc_lock();

    st_ut_bug(st_robustlock_unlock(lock), "unlock with not locked lock");

    st_ut_nobug(st_robustlock_lock(lock), "lock a valid lock");

    st_ut_nobug(st_robustlock_unlock(lock), "unlock with locked lock");

    st_ut_bug(st_robustlock_unlock(NULL), "unlock with unalloced lock");

    free_lock(lock);
}

st_test(robustlock, concurrent_lock) {
    int *stop_flag = NULL;
    pthread_mutex_t *lock = NULL;
    int pids[50] = {0};

    stop_flag = share_alloc(sizeof(*stop_flag));

    lock = alloc_lock();

    create_lock_loop_children(lock, pids, 50, stop_flag, 10);

    sleep(2);

    *stop_flag = 1;

    wait_children(pids, 50);
    free_lock(lock);
}

st_test(robustlock, concurrent_deadlock) {
    int child;
    int ret = -1;
    int *stop_flag = NULL;
    pthread_mutex_t *lock = NULL;
    int pids[50] = {0};

    stop_flag = share_alloc(sizeof(*stop_flag));

    lock = alloc_lock();

    create_lock_loop_children(lock, pids, 50, stop_flag, 5);

    for (int i = 0; i < 500; i++) {
        child = fork();
        if (child == 0) {
            /* if st_bug() triggered, process receives a signal abort and quit */
            st_robustlock_lock(lock);
            exit(0);
        }
        waitpid(child, &ret, 0);
        st_ut_eq(ST_OK, ret, "concurrent deadlock");
    }

    *stop_flag = 1;

    wait_children(pids, 50);
    free_lock(lock);
}

st_ut_main;
