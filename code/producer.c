#include "mandelbrot_sdl.h"

void* producer_thread(void* arg) {
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
    return NULL;
}