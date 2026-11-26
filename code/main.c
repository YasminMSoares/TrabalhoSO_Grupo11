#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "platform_thread.h"

#ifdef _WIN32
    #include <windows.h>
    static double get_time_sec(void) {
        static LARGE_INTEGER freq = {0};
        LARGE_INTEGER counter;
        if (freq.QuadPart == 0) QueryPerformanceFrequency(&freq);
        QueryPerformanceCounter(&counter);
        return (double)counter.QuadPart / (double)freq.QuadPart;
    }
#else
    #include <time.h>
    static double get_time_sec(void) {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
    }
#endif

typedef enum { PRIM_MUTEX, PRIM_SPINLOCK } prim_t;

typedef struct {
    int iterations;
    double work_sec;
    prim_t prim;
} worker_arg_t;

static mine_mutex_t g_mtx;
static mine_spinlock_t g_spin;
static volatile long g_counter = 0;

static void busy_wait(double seconds) {
    if (seconds <= 0.0) return;
    double end = get_time_sec() + seconds;
    while (get_time_sec() < end) { /* trabalho ativo */ }
}

static void *worker(void *arg) {
    worker_arg_t *w = (worker_arg_t *)arg;
    for (int i = 0; i < w->iterations; i++) {
        if (w->prim == PRIM_MUTEX) {
            lock_mine_mutex(&g_mtx);
            g_counter++;
            busy_wait(w->work_sec);
            unlock_mine_mutex(&g_mtx);
        } else {
            lock_mine_spinlock(&g_spin);
            g_counter++;
            busy_wait(w->work_sec);
            unlock_mine_spinlock(&g_spin);
        }
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Uso: %s <threads> <curta|media|longa> <mutex|spinlock> [total_aquisicoes]\n", argv[0]);
        return 1;
    }

    int n_threads = atoi(argv[1]);
    double work_sec = 0.0;
    if (strcmp(argv[2], "media") == 0) work_sec = 1e-6;       // 1 us
    else if (strcmp(argv[2], "longa") == 0) work_sec = 100e-6; // 100 us

    prim_t prim = (strcmp(argv[3], "spinlock") == 0) ? PRIM_SPINLOCK : PRIM_MUTEX;
    int total_acqs = (argc >= 5) ? atoi(argv[4]) : 16000;

    mine_thread_t *threads = (mine_thread_t *)malloc(sizeof(mine_thread_t) * n_threads);
    worker_arg_t *args = (worker_arg_t *)malloc(sizeof(worker_arg_t) * n_threads);

    int base = total_acqs / n_threads;
    int rem = total_acqs % n_threads;

    init_mine_mutex(&g_mtx);
    init_mine_spinlock(&g_spin);
    g_counter = 0;

    double start = get_time_sec();
    for (int i = 0; i < n_threads; i++) {
        args[i].iterations = base + (i < rem ? 1 : 0);
        args[i].work_sec = work_sec;
        args[i].prim = prim;
        create_mine_thread(&threads[i], worker, &args[i]);
    }
    for (int i = 0; i < n_threads; i++) {
        wait_mine_thread(&threads[i]);
    }
    double elapsed = get_time_sec() - start;

    destroy_mine_mutex(&g_mtx);
    destroy_mine_spinlock(&g_spin);
    free(threads);
    free(args);

    double throughput = (elapsed > 0.0) ? ((double)total_acqs / elapsed) : 0.0;
    printf("%.6f,%.2f\n", elapsed, throughput);
    return 0;
}
