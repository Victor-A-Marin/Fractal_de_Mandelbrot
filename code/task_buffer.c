// Encapsula toda a lógica de fila e locks das tarefas.

#include "mandelbrot_sdl.h"

static Task buffer[MAX_TASKS];
static int count = 0, head = 0, tail = 0;
static int finished = 0; 

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond_not_empty = PTHREAD_COND_INITIALIZER; // Antigo 'cond'
static pthread_cond_t cond_not_full  = PTHREAD_COND_INITIALIZER; // NOVO

void task_buffer_push(Task t) {
    pthread_mutex_lock(&mutex);
    
    // Se o buffer estiver cheio, a Main bloqueia aqui e espera os workers aliviarem
    while (count == MAX_TASKS) {
        pthread_cond_wait(&cond_not_full, &mutex);
    }
    
    buffer[tail] = t;
    tail = (tail + 1) % MAX_TASKS;
    count++;
    
    pthread_cond_signal(&cond_not_empty); // Avisa os workers que há tarefas
    pthread_mutex_unlock(&mutex);
}

int task_buffer_pop(Task *t) {
    pthread_mutex_lock(&mutex);
    while (count == 0 && !finished) {
        pthread_cond_wait(&cond_not_empty, &mutex);
    }
    
    if (count == 0 && finished) {
        pthread_mutex_unlock(&mutex);
        return 0; 
    }
    
    *t = buffer[head];
    head = (head + 1) % MAX_TASKS;
    count--;
    
    pthread_cond_signal(&cond_not_full); // Avisa a Main que abriu uma vaga no buffer
    pthread_mutex_unlock(&mutex);
    return 1;
}

void task_buffer_finish() {
    pthread_mutex_lock(&mutex);
    finished = 1;
    pthread_cond_broadcast(&cond_not_empty); 
    pthread_mutex_unlock(&mutex);
}