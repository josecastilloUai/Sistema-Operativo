#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

#define NUM_FILOSOFOS 5
#define IZQUIERDA (id + NUM_FILOSOFOS - 1) % NUM_FILOSOFOS
#define DERECHA (id + 1) % NUM_FILOSOFOS

sem_t tenedores[NUM_FILOSOFOS];
sem_t mutex;

int comidas[NUM_FILOSOFOS] = {0};
int suma_total_comidas = 0;
int limite_comidas = 10000;

void* filosofo(void* num) {
    int id = *(int*)num;
    while (1) {
        printf("Filosofo %d pensando...\n", id);
        
        sem_wait(&mutex);
        sem_wait(&tenedores[IZQUIERDA]);
        sem_wait(&tenedores[DERECHA]);
        sem_post(&mutex);
        
        printf("Filosofo %d comiendo...\n", id);
        
        comidas[id]++;
        suma_total_comidas++;
        
         // Libera los dos tenedores
        sem_post(&tenedores[IZQUIERDA]);
        sem_post(&tenedores[DERECHA]);
         printf("Filosofo %d ha terminado de comer y ahora está pensando...\n", id);

        if (suma_total_comidas >= limite_comidas) {
            return NULL; // Termina el hilo del filósofo
        }
    }
}

int main() {
    pthread_t filosofos[NUM_FILOSOFOS];
    int id[NUM_FILOSOFOS];

    for (int i = 0; i < NUM_FILOSOFOS; i++) {
        sem_init(&tenedores[i], 0, 1);
    }
    sem_init(&mutex, 0, 1);

    for (int i = 0; i < NUM_FILOSOFOS; i++) {
        id[i] = i;
        pthread_create(&filosofos[i], NULL, filosofo, &id[i]);
    }

    for (int i = 0; i < NUM_FILOSOFOS; i++) {
        pthread_join(filosofos[i], NULL);
    }

    for (int i = 0; i < NUM_FILOSOFOS; i++) {
        printf("Filosofo %d comio %d veces.\n", i, comidas[i]);
    }

    return 0;
}
