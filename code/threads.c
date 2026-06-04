// Contém a matemática do Mandelbrot e o comportamento das Threads.

#include <stdio.h>
#include <stdlib.h>
#include <math.h> // Necessário para log() e fmod()
#include "mandelbrot_sdl.h"

// Função matemática para converter HSV (arco-íris) em RGB (padrão da tela)
static uint32_t hsv_to_rgb(double h, double s, double v) {
    double r = 0, g = 0, b = 0;
    
    int i = (int)(h * 6.0);
    double f = h * 6.0 - i;
    double p = v * (1.0 - s);
    double q = v * (1.0 - f * s);
    double t = v * (1.0 - (1.0 - f) * s);
    
    switch (i % 6) {
        case 0: r = v; g = t; b = p; break;
        case 1: r = q; g = v; b = p; break;
        case 2: r = p; g = v; b = t; break;
        case 3: r = p; g = q; b = v; break;
        case 4: r = t; g = p; b = v; break;
        case 5: r = v; g = p; b = q; break;
    }
    
    unsigned char red   = (unsigned char)(r * 255);
    unsigned char green = (unsigned char)(g * 255);
    unsigned char blue  = (unsigned char)(b * 255);
    
    return (red << 16) | (green << 8) | blue;
}

// Mapeia a iteração contínua para o padrão de cores solicitado
static uint32_t get_dynamic_color(double nsmooth, int max_iterations) {
    // 1. Se atingiu o máximo, está DENTRO do conjunto -> Limite em PRETO
    if (nsmooth >= max_iterations) {
        return 0x000000;
    }

    // 2. Controla a velocidade do Scroll do Arco-Íris
    // Mudar o divisor altera a frequência das cores (ex: 20.0 = arco-íris mais denso)
    double hue = nsmooth / 50.0; 
    hue = hue - (int)hue; // Mantém o Hue sempre entre 0.0 e 1.0 (efeito de loop)

    // 3. Efeito de Brilho (Value) baseado na distância
    double saturation = 1.0;
    double value = 1.0;

    // Pontos muito distantes (iterações muito baixas) -> Tendem ao BRANCO
    if (nsmooth < 5.0) {
        // Reduz a saturação gradativamente até 0 (que vira branco puro)
        saturation = nsmooth / 5.0; 
    }
    
    // Pontos muito perto de tender ao infinito (iterações altas) -> Suaviza para o PRETO
    // Isso evita um corte seco de cor na borda do fractal
    if (nsmooth > (max_iterations * 0.9)) {
        value = (max_iterations - nsmooth) / (max_iterations * 0.1);
    }

    return hsv_to_rgb(hue, saturation, value);
}

static uint32_t compute_mandelbrot_pixel(int px, int py, int max_iterations) {
    // Mapeamento do plano complexo
    double x0 = ((double)px / WIDTH) * 3.5 - 2.5;
    double y0 = ((double)py / HEIGHT) * 2.0 - 1.0;
    double x = 0.0, y = 0.0;
    int iter = 0;

    // Para suavização matemática, precisamos expandir o limite de escape para 4.0 de raio (ou 16.0 de módulo quadrado)
    while (x*x + y*y <= 16.0 && iter < max_iterations) {
        double xtemp = x*x - y*y + x0;
        y = 2*x*y + y0;
        x = xtemp;
        iter++;
    }

    // --- ALGORITMO DE SUAVIZAÇÃO (Contagem de iteração normalizada) ---
    // Remove o efeito "quadradão/pixelado" das faixas de cores
    double nsmooth = iter;
    if (iter < max_iterations) {
        double log_zn = log(x*x + y*y) / 2.0;
        double nu = log(log_zn / log(2.0)) / log(2.0);
        nsmooth = iter + 1.0 - nu;
    }

    return get_dynamic_color(nsmooth, max_iterations);
}

void* worker_thread(void* arg) {
    Task t;
    while (task_buffer_pop(&t)) {
        Result r;
        r.start_x = t.start_x;
        r.end_x = t.end_x;
        r.start_y = t.start_y;
        r.end_y = t.end_y;
        
        int w_task = r.end_x - r.start_x;
        int h_task = r.end_y - r.start_y;
        
        // Aloca espaço para a matriz do bloco (Largura x Altura)
        r.pixels = malloc(w_task * h_task * sizeof(uint32_t));
        
        // Loop bidimensional dentro do bloco recebido
        for (int i = 0; i < h_task; i++) {
            int global_y = t.start_y + i;
            for (int j = 0; j < w_task; j++) {
                int global_x = t.start_x + j;
                r.pixels[i * w_task + j] = compute_mandelbrot_pixel(global_x, global_y, max_iter);
            }
        }
        result_buffer_push(r);
    }

    pthread_mutex_lock(&worker_mutex);
    active_workers--;
    pthread_mutex_unlock(&worker_mutex);
    result_buffer_wake_printer(); 
    return NULL;
}

void* print_thread(void* arg) {
    Result r;
    while (result_buffer_pop(&r)) {
        int w_task = r.end_x - r.start_x;
        int h_task = r.end_y - r.start_y;

        // 1. Atualiza o buffer global de memória
        for (int i = 0; i < h_task; i++) {
            int global_y = r.start_y + i;
            for (int j = 0; j < w_task; j++) {
                int global_x = r.start_x + j;
                screen_buffer[global_y * WIDTH + global_x] = r.pixels[i * w_task + j];
            }
        }

        // 2. Atualiza a região quadrada exata na textura do SDL
        pthread_mutex_lock(&sdl_mutex);
        SDL_Rect rect = { .x = r.start_x, .y = r.start_y, .w = w_task, .h = h_task };
        // O "pitch" (último parâmetro) agora é baseado na largura do bloco (w_task)
        SDL_UpdateTexture(g_texture, &rect, r.pixels, w_task * sizeof(uint32_t));
        pthread_mutex_unlock(&sdl_mutex);

        // Delay para dar tempo de assistir o mosaico renderizando
        SDL_Delay(5); 

        free(r.pixels);
    }
    return NULL;
}