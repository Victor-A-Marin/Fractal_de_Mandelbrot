// Define as estruturas de cor, estruturas de tarefa/resultado
// adaptadas e as globais necessárias.

#ifndef MANDELBROT_SDL_H
#define MANDELBROT_SDL_H

#include <pthread.h>
#include <SDL2/SDL.h>

// Configurações da Janela
#define WIDTH 1280
#define HEIGHT 720
#define MAX_TASKS 500

// Estruturas
typedef struct {
    int start_x, end_x;
    int start_y, end_y;
} Task;

// Contém pixels RGB reais (uint32_t)
typedef struct {
    int start_x, end_x;
    int start_y, end_y;
    uint32_t *pixels; // Alocado dinamicamente baseado na altura da tarefa
} Result;

// Variáveis Globais (definidas na main, lidas pelas threads)
extern int num_threads;
extern int max_iter;
extern int task_size;
extern uint32_t screen_buffer[HEIGHT * WIDTH]; // Buffer total em memória

// Controle de Workers
extern int active_workers;
extern pthread_mutex_t worker_mutex;

// Objetos SDL compartilhados (para a thread print)
extern SDL_Texture* g_texture;
extern SDL_Renderer* g_renderer;
extern pthread_mutex_t sdl_mutex; // Garante acesso seguro à textura

// API dos Buffers
void task_buffer_push(Task t);
int task_buffer_pop(Task *t); 
void task_buffer_finish();

void result_buffer_push(Result r);
int result_buffer_pop(Result *r); 
void result_buffer_wake_printer();

// API das Threads
void* producer_thread(void* arg);
void* worker_thread(void* arg);
void* print_thread(void* arg);

#endif