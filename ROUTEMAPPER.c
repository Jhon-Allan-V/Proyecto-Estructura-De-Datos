#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "tdas/list.h"
#include "tdas/hashmap.h"
#include "tdas/map.h"
#include "tdas/stack.h"
#include "tdas/queue.h"
#include "tdas/heap.h"
#include "tdas/extra.h"
#include "libSqlite3/sqlite3.h" //libreria para consultar info en data/chile/chile.gpkg
#define PI 3.14159265358979323846


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
    HashMap *indiceCoordenadas; // Para guardar sin repeticion

    int cantidadVertices; //cantidad de lugares
    int cantidadAristas; //cantidad de rutas
} Grafo;

Grafo *generarGrafo(){
    Grafo *grafo = malloc(sizeof(Grafo));
    if (!grafo) return NULL;
    
    grafo -> vertices = list_create();
    grafo -> indiceCoordenadas = hashmap_create(2000003);
    
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
    const char *comando = "SELECT fid, geom, name, fclass, oneway FROM gis_osm_roads_free LIMIT 50;";
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

/*

Crear una key unida unica por cord.

Para que en el hashmap compruebe si ese punto existe
dentro del mapa.

Ejemplo :
lon = -71.612345
lat = -33.056765

la key sera ("-71.612345, -33.056765")

*/

char *UnionCoordenada(double lon, double lat)
{
    char limite[100];
    sprintf(limite, "%.6lf,%.6lf", lon, lat);

    char *key = malloc(strlen(limite) + 1);
    strcpy(key, limite);

    return key;
}

/* 
Convertir los grados a radianes 

las coordenadas vienen en grados, pero nuestras funciones matematicas
para calcular distancia trabajan con sin(), cos() trabajan con radianes

*/

double grados_Radianes(double grados)
{
    return grados * PI / 180.0;
}

/* Calcula la distancia entre 2 coordenadas 

Usamos la formula de Haversine 
Esta formuala le otorga un peso a la arista del grafo 

*/

double calcularDistancia(double lat1, double lon1, double lat2, double lon2)
{
    double R = 6371; // Radio aprox de la tierra en kilometros.

    double dlat = grados_Radianes(lat2 - lat1); // Cambio de latitud
    double dlon = grados_Radianes(lon2 - lon1); // Cambio de longitud


    // Se pasan las latitudes originales a radianes 
    lat1 = grados_Radianes(lat1); 
    lat2 = grados_Radianes(lat2); 

    // Formula de Haversine

    double H = sin(dlat / 2) * sin(dlat / 2) + cos(lat1) * cos(lat2) * sin(dlon / 2) * sin(dlon / 2);

    double C = 2 * atan2((sqrt(H)), sqrt(1 - H)); // Angulo central entre dos puntos

    return R * C; // obtenemos la diferencia en kilometros

}

/*

Buscar un vertice por la coordenada si este ya existe retorna el vertice existente
pero en caso contrario crea un nuevo vertice y lo guarda en la lista de vertices del grafo
y lo guarda en el registro de HashMap

*/

Vertice *Obtener_CrearVertice(Grafo *grafo, double lon, double lat, const char *nombre)
{
    char *key = UnionCoordenada(lon, lat);

    Vertice *existente = hashmap_search(grafo->indiceCoordenadas, key);

    if(existente != NULL)
    {
        free(key);
        return NULL;
    }

    Vertice *nuevo = malloc(sizeof(Vertice));
    if(nuevo == NULL)
    {
        free(key);
        return NULL;
    }

    nuevo->lugar.id = grafo->cantidadVertices;

    strncpy(nuevo->lugar.nombre, nombre ? nombre : "Sin nombre", MAX); // Copia nombre de la calle dentro del vertice si no existe queda sin nombre
    nuevo->lugar.nombre[MAX - 1] = "\0"; // Asegura que el string termine bien a pesar de superar el limite

    nuevo->lugar.latitud = lat;
    nuevo->lugar.longitud = lon; 

    nuevo->conexiones = list_create();

    list_pushBack(grafo->vertices, nuevo);
    hashmap_insert(grafo->indiceCoordenadas, key, nuevo);

    grafo->cantidadVertices ++; 

    return nuevo;
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
        puts("2) Configuracion Inicial"); //busqueda por nombre
        puts("3) Calcular Ruta"); //busqueda por coordenadas
        puts("4) Mostrar Ruta"); // busqueda por id
        puts("5) Reportar Accidente"); // se reporta que calle esta bloqueada, para que al momento de busqueda se tenga en cuenta eso
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
