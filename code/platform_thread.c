#include "platform_thread.h"
#include <stdlib.h>
#ifdef _WIN32

    typedef struct {
        custom_thread_fn custom;
        void *arg;
    } package_win_t;
    
    static DWORD WINAPI bridge_win32(LPVOID p) {
        package_win_t *package = (package_win_t *)p;
        custom_thread_fn custom_local = package -> custom;
        void *arg_local = package -> arg;
        free(package);
        
        if (custom_local != NULL) {
            custom_local(arg_local);
        }
        return 0;
    }
    
    int create_mine_thread(mine_thread_t *target, custom_thread_fn custom, void *arg) {
        if (target == NULL || custom == NULL) {
            return -1;
        }
        package_win_t *package = (package_win_t *) malloc(sizeof(package_win_t));
        if (package == NULL) {
            return -1;
        }
        package -> custom = custom;
        package -> arg = arg;
        
        target -> h = CreateThread(NULL, 0, bridge_win32, package, 0, NULL);
        if (target -> h == NULL) {
            free(package);
            return -1;
        }
        return 0;
    }
    
    int wait_mine_thread(mine_thread_t *target) {
        if (target == NULL || target -> h == NULL) {
            return -1;
        }
        DWORD status = WaitForSingleObject(target -> h, INFINITE);
        CloseHandle(target -> h);
        target -> h = NULL;
        
        if (status == WAIT_OBJECT_0) {
            return 0;
        }
        return -1;
    }
    
    int init_mine_mutex(mine_mutex_t *m) {
        if (!m) return -1;
        InitializeCriticalSection(&m -> cs);
        return 0;
    }
    
    int lock_mine_mutex(mine_mutex_t *m) {
        if (!m) return -1;
        EnterCriticalSection(&m -> cs);
        return 0;
    }
    
    int unlock_mine_mutex(mine_mutex_t *m) {
        if (!m) return -1;
        LeaveCriticalSection(&m -> cs);
        return 0;
    }
    
    int destroy_mine_mutex(mine_mutex_t *m) {
        if (!m) return -1;
        DeleteCriticalSection(&m -> cs);
        return 0;
    }
    #else
    
    static int ok_or_error(int rc) {
        return (rc == 0) ? 0 : -1;
    }
    
    int create_mine_thread(mine_thread_t *target, custom_thread_fn custom, void *arg) {
        if (!target || !custom) return -1; 
            return ok_or_error(pthread_create(&target -> id_posix, NULL, custom, arg));
    }
    
    int wait_mine_thread(mine_thread_t *target) {
        if (!target) return -1; 
        return ok_or_error(pthread_join(target -> id_posix, NULL));
    }
    
    int init_mine_mutex(mine_mutex_t *m) {
        if (!m) return -1; 
        return ok_or_error(pthread_mutex_init(&m -> mtx, NULL));
    }
    
    int lock_mine_mutex(mine_mutex_t *m) {
        if (!m) return -1;
        return ok_or_error(pthread_mutex_lock(&m -> mtx));
    }
    
    int unlock_mine_mutex(mine_mutex_t *m) {
        if (!m) return -1;
        return ok_or_error(pthread_mutex_unlock(&m -> mtx));
    }
    
    int destroy_mine_mutex(mine_mutex_t *m) {
        if (!m) return -1;
        return ok_or_error(pthread_mutex_destroy(&m -> mtx));
    }
    
    #endif
    
    int init_mine_spinlock(mine_spinlock_t *s) {
        if (!s) return -1;
        atomic_flag_clear(&s -> flag);
        return 0;
    }
    
    int lock_mine_spinlock(mine_spinlock_t *s) {
        if (!s) return -1;
        while (atomic_flag_test_and_set(&s -> flag)) { 
            #ifdef _WIN32
                    SwitchToThread();
            #else
                    sched_yield();
            #endif
        }
        return 0;
    }
    
    int unlock_mine_spinlock(mine_spinlock_t *s) {
        if (!s) return -1;
        atomic_flag_clear(&s -> flag);
        return 0;
    }
    
    
    int destroy_mine_spinlock(mine_spinlock_t *s) {
        if (!s) return -1;
        atomic_flag_clear(&s -> flag);
        return 0;
    }
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
