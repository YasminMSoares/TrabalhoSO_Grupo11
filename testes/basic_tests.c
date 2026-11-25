#include <assert.h>
#include <stdio.h>
#include "../code/platform_thread.h"

static mine_mutex_t g_mtx;
static mine_spinlock_t g_spin;
static long g_count_mtx = 0;
static long g_count_spin = 0;
static int g_arg_received = 0;

static void *thread_arg_test(void *arg) {
    int *val = (int *)arg;
    if (val != NULL) {
        g_arg_received = *val;
    }
    return NULL;
}

static void *worker_mutex(void *arg) {
    int n = *(int *)arg;
    for (int i = 0; i < n; i++) {
        assert(lock_mine_mutex(&g_mtx) == 0);
        g_count_mtx++;
        assert(unlock_mine_mutex(&g_mtx) == 0);
    }
    return NULL;
}

static void *worker_spin(void *arg) {
    int n = *(int *)arg;
    for (int i = 0; i < n; i++) {
        assert(lock_mine_spinlock(&g_spin) == 0);
        g_count_spin++;
        assert(unlock_mine_spinlock(&g_spin) == 0);
    }
    return NULL;
}

static void test_null_errors(void) {
    mine_thread_t t;
    int arg = 1;
    assert(create_mine_thread(NULL, thread_arg_test, &arg) == -1);
    assert(create_mine_thread(&t, NULL, &arg) == -1);
    assert(wait_mine_thread(NULL) == -1);

    assert(init_mine_mutex(NULL) == -1);
    assert(lock_mine_mutex(NULL) == -1);
    assert(unlock_mine_mutex(NULL) == -1);
    assert(destroy_mine_mutex(NULL) == -1);

    assert(init_mine_spinlock(NULL) == -1);
    assert(lock_mine_spinlock(NULL) == -1);
    assert(unlock_mine_spinlock(NULL) == -1);
    assert(destroy_mine_spinlock(NULL) == -1);
    printf("[OK] Validation of null arguments!\n");
}

static void test_thread_isolated(void) {
    mine_thread_t t;
    int value = 42;
    assert(create_mine_thread(&t, thread_arg_test, &value) == 0);
    assert(wait_mine_thread(&t) == 0);
    assert(g_arg_received == 42);
    printf("[OK] Thread creation and waiting!\n");
}

static void test_mutex_isolated(void) {
    mine_thread_t t1, t2;
    int n = 50000;
    assert(init_mine_mutex(&g_mtx) == 0);
    assert(create_mine_thread(&t1, worker_mutex, &n) == 0);
    assert(create_mine_thread(&t2, worker_mutex, &n) == 0);
    assert(wait_mine_thread(&t1) == 0);
    assert(wait_mine_thread(&t2) == 0);
    assert(destroy_mine_mutex(&g_mtx) == 0);
    assert(g_count_mtx == 100000);
    printf("[OK] Operations from mutex(%ld)\n", g_count_mtx);
}

static void test_spinlock_isolated(void) {
    mine_thread_t t1, t2;
    int n = 50000;
    assert(init_mine_spinlock(&g_spin) == 0);
    assert(create_mine_thread(&t1, worker_spin, &n) == 0);
    assert(create_mine_thread(&t2, worker_spin, &n) == 0);
    assert(wait_mine_thread(&t1) == 0);
    assert(wait_mine_thread(&t2) == 0);
    assert(destroy_mine_spinlock(&g_spin) == 0);
    assert(g_count_spin == 100000);
    printf("[OK] Operations from spinlock(%ld)\n", g_count_spin);
}

int main(void) {
    test_null_errors();
    test_thread_isolated();
    test_mutex_isolated();
    test_spinlock_isolated();
    printf("All basic tests have passed!\n");
    return 0;
}
