Trabalho do GB

Trabalhar com fractal de Mandelbrot: fórmula matématica que faz um desenho na tela.

1. Main cria tarefas.
2. Tarefas com buffer compartilhado.
3. Threads trabalhadoras.
    * Pegam uma tarefa.
    * Computam a tarefa.
    * Gravam no buffer o resultado.
    * Pegam a próxima tarefa.
4. Resultados no buffer compartilhado.
5. Thread print na tela.

Parâmetros do programa: n° de threads/processos, complexidade do modelo, tamanho das tarefas

Fazer em pthreads ou MPI

Entrega e Apresentação Aula 16