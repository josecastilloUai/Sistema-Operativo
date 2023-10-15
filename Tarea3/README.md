# Tarea 3

## Objetivo
 En esta tarea, se le pide que resuelva el clásico problema de
sincronización de los Filósofos Pensantes utilizando semáforos en el
lenguaje de programación C.

## Contexto:
Cinco filósofos se sientan alrededor de una mesa y pasan su vida
pensando y comiendo. Entre cada par de filósofos hay un tenedor. Un
filósofo necesita dos tenedores para comer. Un filósofo puede tomar el
tenedor que está a su izquierda o a su derecha, pero no puede comer
hasta que tenga ambos tenedores en su mano. Después de comer, el
filósofo deja ambos tenedores en la mesa y continúa pensando

### Instrucciones:
Usted recibirá un código base (filosofos.c) que ya implementa la
estructura básica del problema.
Debe completar el código para garantizar que los filósofos no sufran de
hambre y que no haya condiciones de carrera al acceder a los
tenedores.
Utilice semáforos para lograr la sincronización adecuada.
Asegúrese de evitar el interbloqueo (deadlock) y de que ningún filósofo
quede esperando indefinidamente (inanición).

## Código Base
```c
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

#define NUM_FILOSOFOS 5

sem_t tenedores[NUM_FILOSOFOS];

void* filosofo(void* num) {
    int id = *(int*)num;
    while(1) {
        printf("Filosofo %d pensando...\n", id);
        
        // Adquiere tenedores
        // TODO: Complete este código
        
        printf("Filosofo %d comiendo...\n", id);
        
        // Libera tenedores
        // TODO: Complete este código

        printf("Filosofo %d ha terminado de comer y ahora está pensando...\n", id);
    }
}

int main() {
    pthread_t filosofos[NUM_FILOSOFOS];
    int id[NUM_FILOSOFOS];

    for (int i = 0; i < NUM_FILOSOFOS; i++) {
        sem_init(&tenedores[i], 0, 1);
    }

    for (int i = 0; i < NUM_FILOSOFOS; i++) {
        id[i] = i;
        pthread_create(&filosofos[i], NULL, filosofo, &id[i]);
    }

    for (int i = 0; i < NUM_FILOSOFOS; i++) {
        pthread_join(filosofos[i], NULL);
    }

    return 0;
}

```
## Solución

### Definiciones  Constantes:
- **NUM_FILOSOFOS**: Esta constante define la cantidad de filósofos en la cena. En este caso, se ha establecido en 5, lo que significa que hay 5 filósofos en la mesa.

- **IZQUIERDA y DERECHA**: Estas son macros que representan los índices de los tenedores a la izquierda y a la derecha de un filósofo.

   - `IZQUIERDA` se calcula como `(id + NUM_FILOSOFOS - 1) % NUM_FILOSOFOS`. Esta fórmula toma el número de filósofo `id` y calcula el índice del tenedor a la izquierda, teniendo en cuenta que los filósofos están sentados en un círculo. Si `id` es 0 (el primer filósofo), el tenedor a la izquierda es el filósofo con el número 4, y si `id` es 1, el tenedor a la izquierda es el filósofo con el número 0, y así sucesivamente.

   - `DERECHA` se calcula como `(id + 1) % NUM_FILOSOFOS`. Esta fórmula toma el número de filósofo `id` y calcula el índice del tenedor a la derecha. Si `id` es 4 (el último filósofo), el tenedor a la derecha es el filósofo con el número 0, y si `id` es 0, el tenedor a la derecha es el filósofo con el número 1, y así sucesivamente.

   -   Estas macros se utilizan en el código para determinar los índices de los tenedores a la izquierda y a la derecha de un filósofo en función de su número de filósofo, lo que permite a los filósofos acceder a los tenedores de manera lógica y sincronizada.
- **sem_t tenedores[NUM_FILOSOFOS]**
- **sem_t mutex;**

```c
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

#define NUM_FILOSOFOS 5
#define IZQUIERDA (id + NUM_FILOSOFOS - 1) % NUM_FILOSOFOS
#define DERECHA (id + 1) % NUM_FILOSOFOS

sem_t tenedores[NUM_FILOSOFOS]; // Semáforos para representar los tenedores
sem_t mutex; // Semáforo para garantizar acceso exclusivo a los tenedores

```
### Función filosofo:

**Explicación del Código:**

1. `void* filosofo(void* num)`: Esta función es una función que se ejecutará en un hilo (thread) y recibe un puntero a un número (`num`) como argumento. La función devuelve un puntero `void*`, que generalmente se usa para indicar el resultado de la ejecución del hilo (aunque en este caso no se utiliza).

2. `int id = *(int*)num;`: En esta línea, se convierte el puntero `num` a un puntero a un entero (`int*`) y luego se accede al valor apuntado por ese puntero. Esto se hace para obtener el número de filósofo (`id`) al que se asocia este hilo. Cada hilo de filósofo tiene un número único que se pasa como argumento cuando se crea el hilo.

3. `printf("Filosofo %d pensando...\n", id);`: Esta línea imprime un mensaje en la consola indicando que el filósofo con el número `id` está pensando. Esto representa la fase de pensamiento del filósofo.

4. A continuación, el filósofo intenta adquirir tenedores para comer. Esto se hace mediante el uso de semáforos para garantizar la sincronización:

   - `sem_wait(&mutex);`: El filósofo adquiere acceso exclusivo al área crítica, que es el proceso de adquirir tenedores. El semáforo `mutex` se utiliza para garantizar que solo un filósofo a la vez puede intentar adquirir tenedores.

   - `sem_wait(&tenedores[IZQUIERDA]);`: El filósofo espera a que el tenedor izquierdo esté disponible. Si otro filósofo lo está utilizando, este se quedará bloqueado hasta que el tenedor esté disponible.

   - `sem_wait(&tenedores[DERECHA]);`: Similar al paso anterior, el filósofo espera a que el tenedor derecho esté disponible.

   - `sem_post(&mutex);`: Una vez que el filósofo ha adquirido ambos tenedores, libera el acceso exclusivo al área crítica para permitir que otros filósofos accedan a los tenedores.

5. `printf("Filosofo %d comiendo...\n", id);`: Después de adquirir los tenedores, se imprime un mensaje indicando que el filósofo con el número `id` está comiendo.

6. A continuación, el filósofo libera los tenedores que ha estado utilizando para que otros filósofos puedan acceder a ellos:

   - `sem_post(&tenedores[IZQUIERDA]);`: Libera el tenedor izquierdo.
   - `sem_post(&tenedores[DERECHA]);`: Libera el tenedor derecho.

7. `printf("Filosofo %d ha terminado de comer y ahora está pensando...\n", id);`: Finalmente, se imprime un mensaje que indica que el filósofo ha terminado de comer y está pensando nuevamente.

Este ciclo se repite continuamente, lo que simula el ciclo de pensamiento y comer de los filósofos en el problema clásico de los filósofos comensales. Cada filósofo intenta adquirir los tenedores de manera sincronizada para evitar condiciones de carrera y bloqueos.

```c
void* filosofo(void* num) {
    int id = *(int*)num; // Número de filósofo
    while (1) {
        printf("Filosofo %d pensando...\n", id);
        
        // Adquiere tenedores
        sem_wait(&mutex); // Adquiere acceso exclusivo a los tenedores
        sem_wait(&tenedores[IZQUIERDA]); // Espera a que el tenedor izquierdo esté disponible
        sem_wait(&tenedores[DERECHA]); // Espera a que el tenedor derecho esté disponible
        sem_post(&mutex); // Libera el acceso exclusivo a los tenedores
        
        printf("Filosofo %d comiendo...\n", id);
        
        // Libera tenedores
        sem_post(&tenedores[IZQUIERDA]); // Libera el tenedor izquierdo
        sem_post(&tenedores[DERECHA]); // Libera el tenedor derecho

        printf("Filosofo %d ha terminado de comer y ahora está pensando...\n", id);
    }
}
```
### Función main:

1. `pthread_t filosofos[NUM_FILOSOFOS];` y `int id[NUM_FILOSOFOS];`: Se declaran dos arreglos, `filosofos` y `id`, para almacenar los hilos de los filósofos y los números de los filósofos respectivamente.

2. El bucle `for` inicializa los semáforos `tenedores` y el semáforo `mutex`:

   - `sem_init(&tenedores[i], 0, 1);`: Inicializa cada semáforo `tenedores[i]` con un valor inicial de 1, lo que significa que los tenedores están disponibles inicialmente.

   - `sem_init(&mutex, 0, 1);`: Inicializa el semáforo `mutex` con un valor inicial de 1, lo que garantiza el acceso exclusivo a los tenedores por parte de los filósofos.

3. El siguiente bucle `for` crea hilos para representar a cada filósofo y los inicia en la función `filosofo`. Para ello:

   - Se asigna un número único a cada filósofo mediante `id[i] = i;`.
   - Luego, se utiliza `pthread_create` para crear un hilo (thread) que ejecutará la función `filosofo` con el número del filósofo como argumento.

4. Después de la creación de los hilos, el código entra en otro bucle `for` que utiliza `pthread_join` para esperar a que los hilos de los filósofos terminen. Esto asegura que el programa principal no finalice antes de que todos los hilos hayan completado su ejecución.

5. Finalmente, se imprime la cantidad de comidas de cada filósofo. Esto probablemente requeriría que haya una variable `comidas` declarada y actualizada en la función `filosofo` para rastrear cuántas veces ha comido cada filósofo.

```c
int main() {
    pthread_t filosofos[NUM_FILOSOFOS];
    int id[NUM_FILOSOFOS];

    // Inicialización de los tenedores y el semáforo mutex
    for (int i = 0; i < NUM_FILOSOFOS; i++) {
        sem_init(&tenedores[i], 0, 1);
    }
    sem_init(&mutex, 0, 1);

    // Creación de hilos para los filósofos
    for (int i = 0; i < NUM_FILOSOFOS; i++) {
        id[i] = i;
        pthread_create(&filosofos[i], NULL, filosofo, &id[i]);
    }

    // Espera a que los hilos de los filósofos terminen
    for (int i = 0; i < NUM_FILOSOFOS; i++) {
        pthread_join(filosofos[i], NULL);
    }

    // Imprime la cantidad de comidas de cada filósofo
    for (int i = 0; i < NUM_FILOSOFOS; i++) {
        printf("Filosofo %d comio %d veces.\n", i, comidas[i]);
    }

    return 0;
}
```
## Ejecucción
### Para la Tarea 3
- gcc -o Tarea Tarea3.c
- ./Tarea

### Para el test
- gcc -o Test Tarea3_Test.c
- ./Test



### Referancias
- [Operating Systems: Design and Implementation" de Andrew S. Tanenbaum y Herbert Bos ](https://csc-knu.github.io/sys-prog/books/Andrew%20S%20Tanenbaum%20-%20Operating%20Systems.%20Design%20and%20Implementation.pdf) , capitulo 2.3 CLASSICAL IPC PROBLEMS , pagina 89
- [The Dining Philosophers Problem Solution in C](https://medium.com/swlh/the-dining-philosophers-problem-solution-in-c-90e2593f64e8)
- [Lecture Notes (PDF)](https://lass.cs.umass.edu/~shenoy/courses/fall13/lectures/Lec10_notes.pdf)

