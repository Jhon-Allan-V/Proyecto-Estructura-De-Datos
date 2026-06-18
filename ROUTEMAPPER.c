#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "tdas/list.h"
#include "tdas/map.h"
#include "tdas/stack.h"
#include "tdas/queue.h"
#include "tdas/heap.h"
#include "tdas/extra.h"
#include "libSqlite3/sqlite3.h" //libreria para consultar info en data/chile/chile.gpkg

#define MAX 256

//se usara para almaxenar una key temporal, para validar que no se repitan calles en el grafo
typedef struct{
    double keyLongitud;
    double keyLattitud;
}keyParaMapaTemporal;

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

Grafo *generarGrafo(){
    Grafo *grafo = malloc(sizeof(Grafo));
    if (!grafo) return NULL;
    grafo -> vertices = list_create();
    grafo -> cantidadVertices = 0;
    grafo -> cantidadAristas = 0;

    return grafo;
}

int callback(void *dato, int col, char **valores, char **nombres){
    for (int i = 0; i < col; i++){
        printf("%s\n", valores[i]);
    }

    return 0;
}

void consultarTablasDB(sqlite3 *db){
    char *error = NULL;

    const char *sql = "SELECT name FROM sqlite_master WHERE type='table';";

    printf("\nTablas en la base de datos:\n");
    int rc = sqlite3_exec(db, sql, callback, NULL, &error);

    if (rc != SQLITE_OK){
        printf("ERROR: %s\n", error);
        sqlite3_free(error);
    }
}

void cargarDatosAlGrafo(Grafo *grafo, const char *nombre,
                               const char *clase, const char *oneway,
                               double lon1, double lat1, double lon2, double lat2) {

}

//primero se verifica si la base de datos puede ser abrida o leida por el programa
void obtenerInformacionDB(Grafo *grafo){
    if (grafo == NULL) {
        printf("Error: grafo no inicializado.\n");
        return;
    }

    if (grafo->vertices != NULL && list_size(grafo->vertices) > 0) {
        printf("El grafo ya tiene datos cargados. No se pueden cargar nuevamente.\n");
        return;
    }

    sqlite3 *db;

    int rc = sqlite3_open("data/chile-260615-free.gpkg/chile.gpkg", &db);

    if (rc != SQLITE_OK) { //si la base de datos no se puede abrir. el programa cierra
        printf("ERROR: %s\n", sqlite3_errmsg(db));

        presioneTeclaParaContinuar();
        limpiarPantalla();

        sqlite3_close(db);
        exit(EXIT_FAILURE);
    }
    //consultarTablasDB(db); //mostrar las tablas de la base de datos para verificar lectura de este

    sqlite3_stmt *stmt;
    // leer todos los segmentos de la tabla gis_osm_roads_free, que contiene las calles y caminos de chile
    const char *comando = "SELECT fid, geom, name, fclass, oneway FROM gis_osm_roads_free LIMIT 100;";
    rc = sqlite3_prepare_v2(db, comando, -1, &stmt, NULL);

    if (rc != SQLITE_OK){
        printf("ERROR: %s\n", sqlite3_errmsg(db));

        presioneTeclaParaContinuar();
        limpiarPantalla();

        sqlite3_finalize(stmt);
        sqlite3_close(db);
        exit(EXIT_FAILURE);
    }

    //se iteran las filas
    while (sqlite3_step(stmt) == SQLITE_ROW){
        int fid = sqlite3_column_int(stmt, 0);
        const unsigned char *geom = sqlite3_column_blob(stmt, 1); //dato binario
        int geom_size = sqlite3_column_bytes(stmt, 1);
        const char *nombre = (const char *)sqlite3_column_text(stmt, 2); //nombre de la calle
        const char *clase = (const char *)sqlite3_column_text(stmt, 3); //tipo de calle (residential, primary, secondary, etc)
        const char *oneway = (const char *)sqlite3_column_text(stmt, 4); //si es un solo sentido o no

        double lon1, lat1, lon2, lat2;
        int resultadoParseo = parse_gpkg_linestring(geom, geom_size, &lon1, &lat1, &lon2, &lat2);

        if (resultadoParseo != 1){
            printf("Error al parsear la geometria en fila %d.\n", fid);
            continue; //se salta esta fila y se sigue con la siguiente
        }

        printf("\nFila %d: %s, %s, %s, (%lf, %lf) -> (%lf, %lf)\n", fid, nombre, clase, oneway, lon1, lat1, lon2, lat2);

        //crear mapa temporal (tabla hash) para verificar que no se repitan calles, la key seria : (lon, lat)
        //Map *mapaTemporal = sorted_map_create(compararKeysMapaTemporal););
        //cargarDatosAlGrafo(grafo, nombre, clase, oneway, lon1, lat1, lon2, lat2);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
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

    Grafo *grafo = generarGrafo(); // se genera o inicializa el grafo

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
            obtenerInformacionDB(grafo); // se lee la base de datos y se llena el grafo con los lugares y conexiones
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
