#include <stdio.h>
#include <stdlib.h>
#include "tdas/list.h"
#include "tdas/stack.h"
#include "tdas/queue.h"
#include "tdas/heap.h"
#include "tdas/extra.h"
#include <string.h>
#include <time.h>

//#define FILA x
//#define COL y
//#define OBSTACULO 0
//#define CAMINO 1

// Definición de la estructura para el estado del puzzle
typedef struct {
    int maze[N][N]; // Matriz NxN que representa el tablero
    int x;    // Posición x del agente
    int y;    // Posición x del agente
    int steps; // Pasos realizados hasta la posición actual
    List* actions; //Secuencia de movimientos para llegar al estado
} State;

typedef struct { int accionX, accionY; } accion;

int distancia_L1(State* state) {
    return abs(state->x - (N-1)) + abs(state->y - (N-1));
}

int esSolucionFinal(State *actual){
    return actual -> x == N-1 && actual -> y == N-1;
}

int pilaEstaVacia(Stack *pila){
    return stack_top(pila) == NULL;
}

int colaEstaVacia(Queue *cola){
    return queue_front(cola) == NULL;
}

int heapEstaVacia(Heap *heap){
    return heap_top(heap) == NULL;
}

// Función para imprimir el estado actual
void imprimirEstado(const State *estado) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (estado->x==i && estado->y==j) printf(" A ");
            else if (i == 0 && j == 0) printf(" I ");
            else if (i == N-1 && j == N-1) printf(" M ");
            else if (estado->maze[i][j] == 0)
                printf(" . "); // Imprime un espacio en blanco para el espacio vacío
            else
                printf("[X]");
        }
        printf("\n");
    }
}

void imprimirCamino(State *camino){
    accion *movimiento = list_first(camino -> actions);
      /*{-1,0},//arriba
        {1,0},//abajo
        {0,-1},//izq
        {0,1}//der*/
    printf("\nPasos totales a realizar -> %i\n", camino -> steps);

    while (movimiento != NULL){
        if (movimiento -> accionX == -1 && movimiento -> accionY == 0)printf("Arriba\n");
        if (movimiento -> accionX == 1 && movimiento -> accionY == 0)printf("Abajo\n");
        if (movimiento -> accionX == 0 && movimiento -> accionY == -1)printf("Izquierda\n");
        if (movimiento -> accionX == 0 && movimiento -> accionY == 1)printf("Derecha\n");

        movimiento = list_next(camino -> actions);
    }
}

State crearEstadoInicial(int maze[N][N], int dificultad){
    State estado;
     // Copiar el laberinto generado al estado
    generate_maze(estado.maze,  dificultad);
    estado.x = 0;
    estado.y = 0;
    estado.steps = 0;
    estado.actions = list_create();
    return estado;
}

State *transition(State *actual, accion movimiento){
    State *nuevo = malloc(sizeof(State));
    if (nuevo == NULL) exit(EXIT_FAILURE);

    *nuevo = *actual;

    nuevo -> x += movimiento.accionX;
    nuevo -> y += movimiento.accionY;
    nuevo -> steps += 1;//actual -> steps +1;
    nuevo -> actions = list_create();

    accion *aux = list_first(actual -> actions);

    while (aux != NULL){
        accion *copia = malloc(sizeof(accion));
        if (copia == NULL) exit(EXIT_FAILURE);

        *copia = *aux;

        list_pushBack(nuevo -> actions, copia);

        aux = list_next(actual -> actions);
    }

    accion *nuevaAccion = malloc(sizeof(accion));
    if (nuevaAccion == NULL) exit(EXIT_FAILURE);

    *nuevaAccion = movimiento;

    list_pushBack(nuevo -> actions, nuevaAccion);

    return nuevo;
}

List *getAdjNodes(State *actual){
    List *vecinos = list_create();

    accion acciones[] = {
        {-1,0},//arriba
        {1,0},//abajo
        {0,-1},//izq
        {0,1}//der
    };

    for (unsigned short i = 0; i < 4; i++){
        accion movimiento = acciones[i];

        int nuevaFila = actual -> x + movimiento.accionX;
        int nuevaCol = actual -> y + movimiento.accionY;

        //verificar si se sale de los limites
        if (nuevaFila < 0 || nuevaFila >= N ||
            nuevaCol < 0 || nuevaCol >= N) continue;

        //verificar si estamos en un obstaculo
        if (actual -> maze[nuevaFila][nuevaCol] == 1) continue;

        State *nuevo = transition(actual, movimiento);
        list_pushBack(vecinos, nuevo);
    }
    return vecinos;
}

void dfs(State *estado_inicial){

    Stack *pila = stack_create(NULL);

    int visitado[N][N] = {0};

    stack_push(pila, estado_inicial);

    while (!pilaEstaVacia(pila)){

        State *actual = stack_top(pila);
        stack_pop(pila);

        if (visitado[actual -> x][actual -> y]) continue;

        visitado[actual -> x][actual -> y] = 1;

        if (esSolucionFinal(actual)){
            printf("\nCamino encontrado B)\n");
            imprimirCamino(actual);
            return;
        }

        List *vecinos = getAdjNodes(actual);

        State *vecino = list_first(vecinos);

        while (vecino != NULL){
            stack_push(pila, vecino);
            vecino = list_next(vecinos);
        }
    }
    printf("\nCamino NO encontrado B(\n");
}

void bfs(State *estado_inicial){
    Queue *cola = queue_create(NULL);

    int visitado[N][N] = {0};

    queue_insert(cola, estado_inicial);

    while (!colaEstaVacia(cola)){

        State *actual = queue_front(cola);
        queue_remove(cola);

        if (visitado[actual -> x][actual -> y]) continue;

        visitado[actual -> x][actual -> y] = 1;

        if (esSolucionFinal(actual)){
            printf("\nCamino encontrado B)\n");
            imprimirCamino(actual);
            return;
        }

        List *vecinos = getAdjNodes(actual);

        State *vecino = list_first(vecinos);

        while (vecino != NULL){
            queue_insert(cola, vecino);
            vecino = list_next(vecinos);
        }
    }
    printf("\nCamino NO encontrado B(\n");
}

void best_first(State *estado_inicial){

    Heap* heap = heap_create();

    int visitado[N][N] = {0};

    //heap_push(Heap* pq, void* data, int priority);
    heap_push(heap, estado_inicial, -1 * distancia_L1(estado_inicial));

    while (!heapEstaVacia(heap)){

        State *actual = heap_top(heap);
        heap_pop(heap);

        if (visitado[actual -> x][actual -> y]) continue;

        visitado[actual -> x][actual -> y] = 1;

        if (esSolucionFinal(actual)){
            printf("\nCamino encontrado B)\n");
            imprimirCamino(actual);
            return;
        }
        //obtener soluciones
        List *vecinos = getAdjNodes(actual);

        State *vecino = list_first(vecinos);

        //recorrer estados de la lista vecinos
        while (vecino != NULL){

            int prioridad = -1 * distancia_L1(vecino);

            heap_push(heap, vecino, prioridad);

            vecino = list_next(vecinos);
        }
    }
    printf("\nCamino NO encontrado B(\n");
}

int main() {
    // Inicializar la semilla de aleatoriedad
    srand(time(NULL));

    int maze[N][N];
    int dificultad;

    // Solicitar la dificultad al usuario con validación
    do {
        printf("Ingrese la dificultad del laberinto (porcentaje de obstáculos, 0 a 100): ");
        scanf("%d", &dificultad);
        if (dificultad < 0 || dificultad > 100) {
            printf("Error: Por favor ingrese un valor válido entre 0 y 100.\n");
        }
    } while (dificultad < 0 || dificultad > 100);

    // Estado inicial del puzzle
    // Laberinto generado con la dificultad ingresada por el usuario
    State estado_inicial = crearEstadoInicial(maze, dificultad);

    // Imprime el estado inicial
    printf("\nEstado inicial del puzzle:\n");
    imprimirEstado(&estado_inicial);

    printf("Distancia L1: %d\n", distancia_L1(&estado_inicial));
/*
    // Ejemplo de heap (cola con prioridad)
    printf("\n***** EJEMPLO USO DE HEAP ******\nCreamos un Heap e insertamos 3 elementos con distinta prioridad\n");
    Heap* heap = heap_create();
    char* data = strdup("Cinco");
    printf("Insertamos el elemento %s con prioridad -5\n", data);
    heap_push(heap, data, -5 );
    data = strdup("Seis");
    printf("Insertamos el elemento %s con prioridad -6\n", data);
    heap_push(heap, data, -6 );
    data = strdup("Siete");
    printf("Insertamos el elemento %s con prioridad -7\n", data);
    heap_push(heap, data, -7 );

    printf("\nLos elementos salen del Heap ordenados de mayor a menor prioridad\n");
    while (heap_top(heap) != NULL){
        printf("Top: %s\n", (char*) heap_top(heap));
        heap_pop(heap);
    }
    printf("No hay más elementos en el Heap\n");
*/
    char opcion;
    do {
        //printf("\npuzzle:\n");
        //imprimirEstado(&estado_inicial);

        printf("\n\n***** EJEMPLO MENU ******\n");
        puts("========================================");
        puts("     Escoge método de búsqueda");
        puts("========================================");

        puts("1) Búsqueda en Profundidad");
        puts("2) Buscar en Anchura");
        puts("3) Buscar Mejor Primero");
        puts("4) Salir");

        printf("Ingrese su opción: ");
        scanf(" %c", &opcion);

        switch (opcion) {
        case '1':
            dfs(&estado_inicial);
            break;
        case '2':
            bfs(&estado_inicial);
            break;
        case '3':
            best_first(&estado_inicial);
            break;
        }

        // Evitamos pausar y limpiar pantalla si el usuario eligió salir
        if (opcion != '4') {
            presioneTeclaParaContinuar();
            limpiarPantalla();
        }

  } while (opcion != '4');

  return 0;
}
