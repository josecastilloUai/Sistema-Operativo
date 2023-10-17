#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

#define NUM_FILOSOFOS 5
#define IZQUIERDA (id + NUM_FILOSOFOS - 1) % NUM_FILOSOFOS
#define DERECHA (id + 1) % NUM_FILOSOFOS

sem_t tenedores[NUM_FILOSOFOS]; // Semáforos para representar los tenedores
sem_t mutex; // Semáforo para garantizar acceso exclusivo a los tenedores

void* filosofo(void* num) {
    int id = *(int*)num; // Número de filósofo
    while (1) {
        printf("Filosofo %d pensando...\n", id);
        
        // Adquiere tenedores
        sem_wait(&mutex); // Adquiere acceso exclusivo a los tenedores
        sem_wait(&tenedores[IZQUIERDA]); // Espera a que el tenedor izquierdo esté disponible
        printf("Filosofo %d ha tomado el tenedor izquierdo (%d)...\n", id, IZQUIERDA);
        sem_wait(&tenedores[DERECHA]); // Espera a que el tenedor derecho esté disponible
        printf("Filosofo %d ha tomado el tenedor izquierdo (%d)...\n", id, DERECHA );

        sem_post(&mutex); // Libera el acceso exclusivo a los tenedores
        printf("Filosofo %d comiendo...\n", id);
        
        // Libera tenedores
        sem_post(&tenedores[IZQUIERDA]); // Libera el tenedor izquierdo
        sem_post(&tenedores[DERECHA]); // Libera el tenedor derecho

        printf("Filosofo %d ha terminado de comer y ahora está pensando...\n", id);
    }
}

int main() {
    pthread_t filosofos[NUM_FILOSOFOS]; // Hilos que representan a los filósofos
    int id[NUM_FILOSOFOS]; // Números de los filósofos

    for (int i = 0; i < NUM_FILOSOFOS; i++) {
        sem_init(&tenedores[i], 0, 1); // Inicializa los tenedores como disponibles (1)
    }
    sem_init(&mutex, 0, 1); // Inicializa el semáforo mutex

    for (int i = 0; i < NUM_FILOSOFOS; i++) {
        id[i] = i; // Asigna un número único a cada filósofo
        pthread_create(&filosofos[i], NULL, filosofo, &id[i]); // Crea hilos para cada filósofo
    }

    for (int i = 0; i < NUM_FILOSOFOS; i++) {
        pthread_join(filosofos[i], NULL); // Espera a que los hilos de los filósofos terminen
    }

    return 0;
}
