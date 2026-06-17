#include <stdio.h>
#include <stdlib.h>
#include "tdas/list.h"
#include "tdas/stack.h"
#include "tdas/queue.h"
#include "tdas/heap.h"
#include "tdas/extra.h"
#include <string.h>
#include <time.h>
#include "libSqlite3/sqlite3.h" //libreria para consultar info en data/chile/chile.gpkg

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

    float distanciaKm;

    float tiempoAuto;
    float tiempoBici;
    float tiempoPie;

    float combustibleAuto;
} Conexion; //calle entre dos puntos

typedef struct {
    Lugar lugar; //informacion con respecto al lugar
    List *conexiones; //lista de rutas con respecto al lugar
} Vertice; //nodo del grafo o lugar del mapa

typedef struct {
    List *vertices; //lista de todos los lugares

    int cantidadVertices; //cantidad de lugares
    int cantidadAristas; //cantidad de rutas
} Grafo;

Grafo *generarGrafo() {
    Grafo *grafo = malloc(sizeof(Grafo));
    grafo->vertices = NULL;
    grafo->cantidadVertices = 0;
    grafo->cantidadAristas = 0;

    return grafo;
}

void cargarDatosAlGrafo(Grafo *grafo) {
    if (grafo -> vertices != NULL) {
        printf("El grafo ya tiene datos cargados. No se pueden cargar nuevamente.\n");
        return;
    }

    printf("Funcion cargarDatosAlGrafo() por implementar.\n");
    
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

/*
flujo general del programa: cargar datos -> cargar grafo -> programar funciones -> probar funciones -> mostrar resultados
antes de cerrar programa, liberar memoria y cerrar conexiones a la base de datos
*/

int main() {

    Grafo *grafo = generarGrafo(); // se generavo inicializa el grafo

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
            cargarDatosAlGrafo(grafo); // se lee la base de datos y se llena el grafo con los lugares y conexiones
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

        // Evitamos pausar y limpiar pantalla si el usuario eligió salir
        if (opcion != '6') {
            presioneTeclaParaContinuar();
            limpiarPantalla();
        }

  } while (opcion != '6');

  return 0;
}
