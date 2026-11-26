#ifndef PLATFORM_THREAD_H
#define PLATFORM_THREAD_H
#include <stdatomic.h>

    #ifdef _WIN32
       #include <windows.h>
       
       typedef struct {
           HANDLE h;
           
       } mine_thread_t;
       
       typedef struct {
           CRITICAL_SECTION cs;
       } mine_mutex_t;
       
       typedef mine_mutex_t pt_mutex_t;
    
    #else
       #include <pthread.h>
    
       typedef struct {
           pthread_t id_posix;
       } mine_thread_t;
       
       typedef struct {
           pthread_mutex_t mtx;
       } mine_mutex_t;
       
       typedef mine_mutex_t pt_mutex_t;
   
    #endif
    
    typedef void *(*custom_thread_fn)(void *arg);
    int create_mine_thread(mine_thread_t *target, custom_thread_fn custom, void *arg);
    int wait_mine_thread(mine_thread_t *target);
    int init_mine_mutex(mine_mutex_t *m);
    int lock_mine_mutex(mine_mutex_t *m);
    int unlock_mine_mutex(mine_mutex_t *m);
    int destroy_mine_mutex(mine_mutex_t *m);
    
    typedef struct {
        atomic_flag flag;
    } mine_spinlock_t;
    
    typedef mine_spinlock_t pt_spinlock_t;
    
    int init_mine_spinlock(mine_spinlock_t *s);
    int lock_mine_spinlock(mine_spinlock_t *s);
    int unlock_mine_spinlock(mine_spinlock_t *s);
    int destroy_mine_spinlock(mine_spinlock_t *s);
    
    #endif



