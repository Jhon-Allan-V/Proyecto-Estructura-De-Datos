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
    int origen;
    int destino;

    float distancia_km;

    float tiempoAuto;
    float tiempoBici;
    float tiempoPie;

    float combustibleAuto;
} Conexion;

typedef struct {
    Lugar lugar;
    List *conexiones;
} Vertice;

typedef struct {
    List *vertices;

    int cantidadVertices;
    int cantidadAristas;
} Grafo;

typedef struct {
    Grafo *grafo;

    char archivoLugares[MAX];
    char archivoRutas[MAX];
} State;

int pilaEstaVacia(Stack *pila){
    return stack_top(pila) == NULL;
}

int colaEstaVacia(Queue *cola){
    return queue_front(cola) == NULL;
}

int heapEstaVacia(Heap *heap){
    return heap_top(heap) == NULL;
}

void imprimirEstado(const State *estado) {
    if (estado && estado->grafo)
        printf("Grafo con %d vertices y %d aristas\n",
               estado->grafo->cantidadVertices,
               estado->grafo->cantidadAristas);
}

void imprimirCamino(List *camino){
    if (!camino) return;
    Vertice *v = list_first(camino);
    while (v != NULL) {
        printf("  -> %s\n", v->lugar.nombre);
        v = list_next(camino);
    }
}

void cargarDatos(){
    printf("Funcion cargarDatos() por implementar.\n");
}

void configuracionInicial(){
    printf("Funcion configuracionInicial() por implementar.\n");
}

void calcularRuta(){
    printf("Funcion calcularRuta() por implementar.\n");
}

void mostrarRuta(){
    printf("Funcion mostrarRuta() por implementar.\n");
}

void reportarAccidente(){
    printf("Funcion reportarAccidente() por implementar.\n");
}

int main() {

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

        printf("Ingrese su opcion: ");
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
        case '6':
            printf("\nHasta luego!\n");
            break;
        default:
            printf("\nOpcion No Valida.\n");
            break;
        }

        if (opcion != '6') {
            presioneTeclaParaContinuar();
            limpiarPantalla();
        }

  } while (opcion != '6');

  return 0;
}
