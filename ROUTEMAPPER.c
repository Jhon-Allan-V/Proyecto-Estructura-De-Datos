#include <stdio.h>
#include <stdlib.h>
#include "tdas/list.h"
#include "tdas/stack.h"
#include "tdas/queue.h"
#include "tdas/heap.h"
#include "tdas/extra.h"
#include <string.h>
#include <time.h>

#define MAX 256

typedef struct {
    int id;
    char nombre[MAX];

    double latitud;
    double longitud;
} Lugar;

typedef struct {
    int origen; //id donde comienza la ruta
    int destino; //id donde finaliza la ruta

    float distancia_km;

    float tiempoAuto;
    float tiempoBici;
    float tiempoPie;

    float combustibleAuto;
} Conexion; //calle entre dos puntos

typedef struct {
    Lugar lugar; //informacion con respecto al lugar
    List *conexiones; //lista de rutas con respecto al lugar
} Vertice; //nodo del grafo

typedef struct {
    List *vertices; //lista de todos los lugares

    int cantidadVertices; //cantidad de lugares
    int cantidadAristas; //cantidad de rutas
} Grafo;

typedef struct {
    Grafo *grafo;

    char archivoLugares[MAX];
    char archivoRutas[MAX];
} State;

/*
typedef struct {
    int origen;
    int destino;

    float distancia;

    char calle[MAX];
} accion;
 */

 /*
int distancia_L1(State* state) {
    return abs(state->x - (N-1)) + abs(state->y - (N-1));
}
*/

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
}

void imprimirCamino(const State *estado){
}

State *crearEstadoInicial(){
}

State *transition(State *actual){
}

List *getAdjNodes(State *actual){
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

    // pedir al usuario que digite alguna opcion

    char opcion;
    do {

        printf("\n\n***** ROUTERMAPPER ******\n");
        puts("========================================");
        puts("        Escoge alguna opcion            ");
        puts("========================================");

        puts("1) Cargar Datos");
        puts("2) Configuracion Inicial");
        puts("3) Calcular Ruta");
        puts("4) Mostrar Ruta");
        puts("5) Reportar Accidente");
        puts("6) Salir");

        printf("Ingrese su opción: ");
        scanf(" %c", &opcion);

        switch (opcion) {
        case '1':
            cargarDatos();
            break;
        case '2':
            configuracionInicial();
            break;
        case '3':
            calcularRuta();
            break;
        case '4':
            mostrarRuta();
            break;
        case '5':
            reportarAccidente();
            break;
        default:
            printf("\nOpcion No Valida.\n");
            break;
        }

        // Evitamos pausar y limpiar pantalla si el usuario eligió salir
        if (opcion != '6') {
            presioneTeclaParaContinuar();
            limpiarPantalla();
        }

  } while (opcion != '4');

  return 0;
}
