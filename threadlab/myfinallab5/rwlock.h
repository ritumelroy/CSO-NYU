#ifndef RWLOCK_H
#define RWLOCK_H

typedef struct {
    int writer, w_waiters,readers,r_waiters;
    pthread_mutex_t lock;
    pthread_cond_t r_cond,w_cond;
}rwl;

void rwl_init(rwl *l);
int rwl_nwaiters(rwl *l);
int rwl_rlock(rwl *l, const struct timespec *expire);
void rwl_runlock(rwl *l);
int rwl_wlock(rwl *l, const struct timespec *expire);
void rwl_wunlock(rwl *l);

#endif