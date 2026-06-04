// Encapsula o armazenamento e sincronização dos cálculos prontos.

#include "mandelbrot_sdl.h"

static Result buffer[MAX_TASKS];
static int count = 0, head = 0, tail = 0;

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void result_buffer_push(Result r) {
    pthread_mutex_lock(&mutex);
    buffer[tail] = r;
    tail = (tail + 1) % MAX_TASKS;
    count++;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
}

int result_buffer_pop(Result *r) {
    pthread_mutex_lock(&mutex);
    while (count == 0) {
        pthread_mutex_lock(&worker_mutex);
        int workers = active_workers;
        pthread_mutex_unlock(&worker_mutex);

        if (workers == 0 && count == 0) {
            pthread_mutex_unlock(&mutex);
            return 0; // Tudo finalizado
        }
        pthread_cond_wait(&cond, &mutex);
    }
    *r = buffer[head];
    head = (head + 1) % MAX_TASKS;
    count--;
    pthread_mutex_unlock(&mutex);
    return 1;
}

void result_buffer_wake_printer() {
    pthread_mutex_lock(&mutex);
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
}