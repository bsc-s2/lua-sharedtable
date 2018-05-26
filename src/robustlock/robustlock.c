#include "robustlock.h"

int st_robustlock_init(pthread_mutex_t *lock) {

    st_assert_nonull(lock);

    int ret = ST_ERR;
    pthread_mutexattr_t attr;

    memset(&attr, 0, sizeof(attr));

    /* possible error: ENOMEM */
    ret = pthread_mutexattr_init(&attr);
    if (ret != ST_OK) {
        derr("pthread_mutexattr_init ret: %d, err: %s\n", ret, strerror(ret));
        return ret;
    }

    /* possible error: EINVAL */
    st_assert_ok(pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK), "");
    st_assert_ok(pthread_mutexattr_setrobust(&attr, PTHREAD_MUTEX_ROBUST), "");
    st_assert_ok(pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED), "");

    /* possible error: EBUSY, EINVAL */
    st_assert_ok(pthread_mutex_init(lock, &attr), "");

    pthread_mutexattr_destroy(&attr);
    return ret;
}

void st_robustlock_lock(pthread_mutex_t *lock) {

    st_assert_nonull(lock);

    /* Lock recovery process ref:
     * http://man7.org/linux/man-pages/man3/pthread_mutex_lock.3p.html
     */

    int ret;

    ret = pthread_mutex_lock(lock);
    dd("pthread_mutex_lock ret:%d, pid:%d, address:%p", ret, getpid(), lock);

    if (ret == ST_OK) {
        return;
    }

    /* Only one thread could receive a EOWNERDEAD return value.
     *
     * EOWNERDEAD:
     *      The mutex is a robust mutex and the process containing the
     *      previous owning thread terminated while holding the mutex lock.
     *      The mutex lock shall be **ACQUIRED** by the calling thread and
     *      it is up to the new owner to make the state consistent.
     */

    /* EOWNERDEAD is the only non-0 return value we can deal with
     */
    st_assert(ret == EOWNERDEAD,
              "pthread_mutex_lock ret: %d, err: %s",
              ret, strerror(ret));

    dd("lock owner dead, pid:%d", getpid());

    /* recover lock held by a dead process */

    /* There is only one possible return value for pthread_mutex_consistent:
     * EINVAL:
     *      The mutex is either not robust or is not in an inconsistent
     *      state.
     * And we can be sure it must be inconsistent since this function is the
     * only one that get the EOWNERDEAD.
     * Thus if pthread_mutex_consistent fails, just raise.
     */
    ret = pthread_mutex_consistent(lock);
    dd("pthread_mutex_consistent ret: %d, pid:%d, address: %p",
       ret, getpid(), lock);

    st_assert(ret == ST_OK,
              "pthread_mutex_consistent: non-0 return:"
              " ret:%d, err:%s, pid:%d, lock:%p",
              ret, strerror(ret), getpid(), lock);

    /* After calling pthread_mutex_consistent,
     * it is not required to unlock-relock it.
     *
     * Since receiving a [EOWNERDEAD] means this thread has locked the lock
     * except the lock is marked as **INCONSISTENT**.
     *
     * The only thing we need to do is to mark it as **CONSISTENT**.
     *
     * pthread_mutex_consistent is only used to let the calling thread have
     * a chance to squre things about shared resource before any other can
     * acquire the lock.
     *
     * From man of pthread_mutex_lock:
     *      If mutex is a robust mutex and the owning thread terminated
     *      while holding the mutex lock, a call to pthread_mutex_lock() may
     *      return the error value [EOWNERDEAD] even if the process in which
     *      the owning thread resides has not terminated.
     *      In these cases, the mutex is **LOCKED BY THE THREAD** but the
     *      state it protects is marked as inconsistent.
     */
}


/** please refer to comments of st_robustlock_lock */
int st_robustlock_trylock(pthread_mutex_t *lock) {
    st_assert_nonull(lock);

    int ret = pthread_mutex_trylock(lock);
    if (ret == ST_OK || ret == EBUSY) {
        return ret;
    }

    st_assert(ret == EOWNERDEAD,
              "pthread_mutex_unlock ret: %d, err: %s",
              ret, strerror(ret));

    st_assert_ok(pthread_mutex_consistent(lock),
                 "pthyread_mutex_consistent: non-0 return: %p",
                 lock);

    return ST_OK;
}


void st_robustlock_unlock(pthread_mutex_t *lock) {

    st_assert(lock != NULL, "lock is NULL");

    int ret = pthread_mutex_unlock(lock);
    st_assert(ret == ST_OK,
              "pthread_mutex_unlock ret:%d, err:%s, pid:%d",
              ret, strerror(ret), getpid());
}


int st_robustlock_destroy(pthread_mutex_t *lock) {

    st_assert_nonull(lock);

    return pthread_mutex_destroy(lock);
}
