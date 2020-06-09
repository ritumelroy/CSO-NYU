#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>
#include <assert.h>
#include "rwlock.h"

/* rwl implements a reader-writer lock.
 * A reader-write lock can be acquired in two different modes, 
 * the "read" (also referred to as "shared") mode,
 * and the "write" (also referred to as "exclusive") mode.
 * Many threads can grab the lock in the "read" mode.  
 * By contrast, if one thread has acquired the lock in "write" mode, no other 
 * threads can acquire the lock in either "read" or "write" mode.
 */

//helper function
static inline int
cond_timedwait(pthread_cond_t *c, pthread_mutex_t *m, const struct timespec *expire)
{
	int r; 
	if (expire != NULL)  {
		r = pthread_cond_timedwait(c, m, expire);
	} else
		r = pthread_cond_wait(c, m);
	assert(r == 0 || r == ETIMEDOUT);
       	return r;
}

//rwl_init initializes the reader-writer lock 
void
rwl_init(rwl *l)
{
    l->readers=l->writer=l->r_waiters=l->w_waiters=0;
    pthread_cond_init(&(l->w_cond),NULL);
    pthread_cond_init(&(l->r_cond),NULL);
    pthread_mutex_init(&(l->lock),NULL);


}

//rwl_nwaiters returns the number of threads *waiting* to acquire the lock
//Note: it should not include any thread who has already grabbed the lock
int
rwl_nwaiters(rwl *l) 
{
int total_wait = l->w_waiters + l->r_waiters;

	return total_wait;

}

//rwl_rlock attempts to grab the lock in "read" mode
//if lock is not grabbed before absolute time "expire", it returns ETIMEDOUT
//else it returns 0 (when successfully grabbing the lock)
int
rwl_rlock(rwl *l, const struct timespec *expire)
{
    pthread_mutex_lock(&l->lock);//LOCK
    l->r_waiters+=1;
    if(l->writer !=0 || l->w_waiters !=0){
           int t = cond_timedwait(&l->r_cond,&l->lock,expire);//log time
            if(t == ETIMEDOUT){//
               //one less reader waiting
               l->r_waiters-=1;
               pthread_mutex_unlock(&l->lock);//UNLOCK
               return t;
            }else if(t ==0){//lock is not grabbed before time "expire"
                l->r_waiters-=1;//subtract one from reader that is waiting
                l->readers+=1;//add one to reader ready to go
                pthread_mutex_unlock(&l->lock);//UNLOCK
                return 0;
            }            
    }
    l->r_waiters-=1; //one less read read in the wait
    l->readers+=1; //one more ready to go reader
    
    pthread_mutex_unlock(&l->lock);//UNLOCK
  
    return 0;
}

//rwl_runlock unlocks the lock held in the "read" mode
void
rwl_runlock(rwl *l)
{
    pthread_mutex_lock(&l->lock); //LOCK
    //lock is held in the read mode so now we can decrement the active readers 
    l->readers -=1;
    if(l->readers==0){
        pthread_cond_broadcast(&l->w_cond);
    }
    pthread_mutex_unlock(&l->lock);//UNLOCK
}



//rwl_wlock attempts to grab the lock in "write" mode
//if lock is not grabbed before absolute time "expire", it returns ETIMEDOUT
//else it returns 0 (when successfully grabbing the lock)
int
rwl_wlock(rwl *l, const struct timespec *expire)
{
        pthread_mutex_lock(&l->lock);//LOCK
        l->w_waiters+=1;
        if(l->writer!=0 || l->readers>0){
            int t = cond_timedwait(&l->w_cond,&l->lock,expire);//log time
            if(t == ETIMEDOUT){
                //one less writer waiting
                l->w_waiters -=1;
                pthread_mutex_unlock(&l->lock);
                return t;
            }else if(t==0){
                l->w_waiters-=1;
                l->writer+=1;
                pthread_mutex_unlock(&l->lock);
                return 0;
            }
        }
        l->writer+=1;
        l->w_waiters-=1;
        pthread_mutex_unlock(&l->lock);
	return 0;
}

//rwl_wunlock unlocks the lock held in the "write" mode
void
rwl_wunlock(rwl *l)
{
    pthread_mutex_lock(&l->lock);
    l->writer-=1;
    pthread_cond_broadcast(&l->r_cond);
    pthread_mutex_unlock(&l->lock);
}
