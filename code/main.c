// Instancia as configurações, cria as threads, injeta as tarefas e espera.

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "mandelbrot_sdl.h"

// Definição das globais
int num_threads = 4;
int max_iter = 500; // Complexidade menor para ver desenhando
int task_size = 4;   // Tarefas menores para desenho mais progressivo
uint32_t screen_buffer[HEIGHT * WIDTH] = {0};

int active_workers = 0;
pthread_mutex_t worker_mutex = PTHREAD_MUTEX_INITIALIZER;

// SDL Globais
SDL_Texture* g_texture = NULL;
SDL_Renderer* g_renderer = NULL;
pthread_mutex_t sdl_mutex = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char* argv[]) {
    // Parâmetros do programa: [threads] [complexidade] [tamanho_tarefa]
    if (argc > 1) num_threads = atoi(argv[1]);
    if (argc > 2) max_iter = atoi(argv[2]);
    if (argc > 3) task_size = atoi(argv[3]);

    // --- INICIALIZAÇÃO SDL2 ---
    if (SDL_Init(SDL_INIT_VIDEO) < 0) return 1;
    
    SDL_Window* window = SDL_CreateWindow("Mandelbrot Progressivo Pthreads",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) return 1;

    // Criamos o renderer com aceleração por hardware e VSync ligado
    g_renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!g_renderer) return 1;

    // Criamos uma textura "Streaming" (pode ser atualizada frequentemente pela CPU)
    g_texture = SDL_CreateTexture(g_renderer, SDL_PIXELFORMAT_ARGB8888, 
                                    SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);
    if (!g_texture) return 1;

    printf("Iniciando Mandelbrot Grafico: %d Threads | %d Iteracoes | Tarefas de %d linhas\n", 
            num_threads, max_iter, task_size);

    active_workers = num_threads;

    pthread_t workers[num_threads];
    pthread_t printer;

    // Cria as Threads Trabalhadoras e a Thread Print
    for (int i = 0; i < num_threads; i++) pthread_create(&workers[i], NULL, worker_thread, NULL);
    pthread_create(&printer, NULL, print_thread, NULL);

    // Main Produz Tarefas
    for (int y = 0; y < HEIGHT; y += task_size) {
        for (int x = 0; x < WIDTH; x += task_size) {
            Task t;
            t.start_x = x;
            t.end_x = (x + task_size > WIDTH) ? WIDTH : x + task_size;
            t.start_y = y;
            t.end_y = (y + task_size > HEIGHT) ? HEIGHT : y + task_size;
            
            task_buffer_push(t);
        }
    }
    task_buffer_finish();

    // --- LOOP DE EVENTOS (MAIN THREAD) ---
    int quit = 0;
    SDL_Event e;
    while (!quit) {
        // Trata eventos da janela (fechar)
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) quit = 1;
        }

        // Desenha na tela (copia a textura atual para o renderer e apresenta)
        pthread_mutex_lock(&sdl_mutex);
        SDL_RenderClear(g_renderer);
        SDL_RenderCopy(g_renderer, g_texture, NULL, NULL);
        SDL_RenderPresent(g_renderer);
        pthread_mutex_unlock(&sdl_mutex);
        
        // Pequeno delay para não consumir 100% da CPU na main thread só desenhando
        SDL_Delay(16); // ~60 FPS
    }

    // --- FINALIZAÇÃO ---
    // Aguarda conclusão das threads
    for (int i = 0; i < num_threads; i++) pthread_join(workers[i], NULL);
    pthread_join(printer, NULL);

    SDL_DestroyTexture(g_texture);
    SDL_DestroyRenderer(g_renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    printf("Processamento finalizado!\n");
    return 0;
}