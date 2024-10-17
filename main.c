#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <float.h>
#include <locale.h>

typedef struct {
    int pid;
    float burst_time;
    float tempo_espera;
    float tempo_retorno;
    float tempo_restante; // apenas para escalonamento preemptivo
} Process;

// Função para redimensionar o array de processos
Process* resize_array(Process* processes, int* n) {
    *n += 1;
    processes = realloc(processes, (*n) * sizeof(Process));
    if (!processes) {
        printf("Erro ao alocar memória.\n");
        exit(EXIT_FAILURE);
    }
    return processes;
}

// Função do SJF não preemptivo - bubble sort
void sort_by_burst_time(Process processes[], int n) {
    for (int i = 0; i < n - 1; i++) { // controla o número de passagens pelo array
        for (int j = 0; j < n - i - 1; j++) {
            if (processes[j].burst_time > processes[j + 1].burst_time) { 
                // verifica se o tempo de burst é maior que o próximo e já ajusta
                Process temp = processes[j];
                processes[j] = processes[j + 1];
                processes[j + 1] = temp;
            }
        }
    }
}

// FCFS
void fcfs(Process processes[], int n) {
    processes[0].tempo_espera = 0; // seta tempo como zero
    for (int i = 1; i < n; i++) {
        processes[i].tempo_espera = processes[i - 1].tempo_espera + processes[i - 1].burst_time;
    } // calcula o tempo de espera pro próximo processo
    for (int i = 0; i < n; i++) {
        processes[i].tempo_retorno = processes[i].tempo_espera + processes[i].burst_time;
    } // vai somando o tempo atual com o anterior
    float total_tempo_espera = 0.0;
    printf("Ordem de execução (FCFS): "); // imprime e calcula
    for (int i = 0; i < n; i++) {
        printf("P%d ", processes[i].pid);
        total_tempo_espera += processes[i].tempo_espera;
    }
    printf("\n");
    printf("Tempo médio de espera (FCFS): %.2f\n", total_tempo_espera / n);
}

// SJF não preemptivo
void sjf_non_preemptive(Process processes[], int n) {
    sort_by_burst_time(processes, n); // vai ordenar o tempo de burst
    fcfs(processes, n);
}

// SJF preemptivo (SRTF)
void sjf_preemptivo(Process** processes, int* n) {
    int completed = 0, t = 0, shortest = 0;
    float min_remaining_time = FLT_MAX;
    int* is_completed = (int*) malloc(*n * sizeof(int));

    for (int i = 0; i < *n; i++) {
        (*processes)[i].tempo_restante = (*processes)[i].burst_time; 
        // para cada unidade de tempo ele seleciona o menor tempo que ainda não foi completado
        is_completed[i] = 0;
    }

    while (completed != *n) { // vai rodar até completar o tempo setado
        for (int i = 0; i < *n; i++) { 
            // seleção do processo com menor tempo de burst de tempo restante
            if ((processes[0][i].tempo_restante < min_remaining_time) && !is_completed[i]) {
                min_remaining_time = processes[0][i].tempo_restante;
                shortest = i;
            }
        }
        // aqui ele vai atualizar o tempo com o menor burst até o processo selecionado ser concluído
        (*processes)[shortest].tempo_restante--;
        min_remaining_time = ((*processes)[shortest].tempo_restante > 0) ? (*processes)[shortest].tempo_restante : FLT_MAX;

        // verifica se ele está concluído
        if ((*processes)[shortest].tempo_restante == 0) {
            completed++;
            is_completed[shortest] = 1;
            (*processes)[shortest].tempo_espera = t + 1 - (*processes)[shortest].burst_time;
            (*processes)[shortest].tempo_retorno = t + 1;
        }
        t++;

        // Solicita ao usuário se deseja adicionar um novo processo
        if (t % 5 == 0) {
            char option;
            printf("Deseja adicionar um novo processo? (s/n): ");
            scanf(" %c", &option);
            if (option == 's' || option == 'S') {
                *processes = resize_array(*processes, n); 
                // vai realocar o tamanho do array process
                printf("Digite o PID do novo processo: ");
                scanf("%d", &(*processes)[*n - 1].pid);
                printf("Digite o tempo de burst do novo processo: ");
                scanf("%f", &(*processes)[*n - 1].burst_time);
                (*processes)[*n - 1].tempo_restante = (*processes)[*n - 1].burst_time;
                is_completed = (int*) realloc(is_completed, (*n) * sizeof(int));
                is_completed[*n - 1] = 0;
            } else if (option == 'n' || option == 'N') {
                break; // Se o usuário não quiser adicionar mais processos, sai do loop
            }
        }
    }

    free(is_completed); // Libera a memória alocada para o array is_completed

    float total_tempo_espera = 0.0;
    printf("Ordem de execução (SJF preemptivo): ");
    for (int i = 0; i < *n; i++) {
        printf("P%d ", (*processes)[i].pid);
        total_tempo_espera += (*processes)[i].tempo_espera;
    }
    printf("\n");
    printf("Tempo médio de espera (SJF preemptivo): %.2f\n", total_tempo_espera / *n);
}

// Round Robin
void round_robin(Process processes[], int n, int quantum, int ttc) {
    int time = 0, completed = 0; // inicializa com o tempo de burst
    for (int i = 0; i < n; i++) {
        processes[i].tempo_restante = processes[i].burst_time;
    }
    printf("Ordem de execução (Round Robin): ");
    while (completed < n) {
        for (int i = 0; i < n; i++) {
            if (processes[i].tempo_restante > 0) { // faz a verificação pra ver se o tempo acabou
                if (processes[i].tempo_restante > quantum) {
                    // se tempo atual > quantum, se sim incrementa valor e o tempo restante decrementa do valor
                    time += quantum;
                    processes[i].tempo_restante -= quantum;
                    printf("P%d ", processes[i].pid);
                    time += ttc; // Simula a troca de contexto
                } else {
                    // se não, tempo total é incrementado, tempo de espera calculado
                    time += processes[i].tempo_restante;
                    processes[i].tempo_espera = time - processes[i].burst_time;
                    processes[i].tempo_restante = 0;
                    completed++;
                    printf("P%d ", processes[i].pid);
                    time += ttc; // Tempo de troca de contexto é adicionado
                }
            }
        }
    }

    for (int i = 0; i < n; i++) {
        processes[i].tempo_retorno = processes[i].tempo_espera + processes[i].burst_time;
    } // cálculo do tempo de retorno

    float total_tempo_espera = 0.0;
    for (int i = 0; i < n; i++) {
        total_tempo_espera += processes[i].tempo_espera;
    } // calcula o tempo total de espera
    printf("\n");
    printf("Tempo médio de espera (Round Robin): %.2f\n", total_tempo_espera / n);
}

int main() {
    setlocale(LC_ALL, "Portuguese");
    int n, scheduling_policy, quantum, ttc;
    printf("Digite o número de processos: ");
    scanf("%d", &n);
    Process* processes = (Process*) malloc(n * sizeof(Process));
    for (int i = 0; i < n; i++) {
        printf("Digite o PID do processo %d: ", i + 1);
        scanf("%d", &processes[i].pid);
        printf("Digite o tempo de burst do processo %d: ", processes[i].pid);
        scanf("%f", &processes[i].burst_time);
    }
    printf("Escolha a política de escalonamento (1: FCFS, 2: SJF não preemptivo, 3: SJF preemptivo, 4: Round Robin): ");
    scanf("%d", &scheduling_policy);
    if (scheduling_policy == 4) {
        printf("Digite o quantum: ");
        scanf("%d", &quantum);
        printf("Digite o tempo de troca de contexto: ");
        scanf("%d", &ttc);
    }
    switch (scheduling_policy) {
        case 1:
            fcfs(processes, n);
            break;
        case 2:
            sjf_non_preemptive(processes, n);
            break;
        case 3:
            sjf_preemptivo(&processes, &n);
            break;
        case 4:
            round_robin(processes, n, quantum, ttc);
            break;
        default:
            printf("Política inválida!\n");
            break;
    }
    free(processes);
    return 0;
}
