// Encapsula toda a lógica de fila e locks das tarefas.

#include "mandelbrot_sdl.h"

static Task buffer[MAX_TASKS];
static int count = 0, head = 0, tail = 0;
static int finished = 0; // 1 quando a Main terminar de criar tarefas

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void task_buffer_push(Task t) {
    pthread_mutex_lock(&mutex);
    buffer[tail] = t;
    tail = (tail + 1) % MAX_TASKS;
    count++;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
}

int task_buffer_pop(Task *t) {
    pthread_mutex_lock(&mutex);
    while (count == 0 && !finished) {
        pthread_cond_wait(&cond, &mutex);
    }
    
    if (count == 0 && finished) {
        pthread_mutex_unlock(&mutex);
        return 0; // Fim das tarefas
    }
    
    *t = buffer[head];
    head = (head + 1) % MAX_TASKS;
    count--;
    pthread_mutex_unlock(&mutex);
    return 1;
}

void task_buffer_finish() {
    pthread_mutex_lock(&mutex);
    finished = 1;
    pthread_cond_broadcast(&cond); // Acorda todos os workers esperando
    pthread_mutex_unlock(&mutex);
}