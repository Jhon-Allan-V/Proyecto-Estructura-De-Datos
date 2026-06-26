#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "tdas/list.h"
#include "tdas/hashmap.h"
//#include "tdas/map.h"
#include "tdas/stack.h"
#include "tdas/queue.h"
#include "tdas/heap.h"
#include "tdas/extra.h"
#include "libSqlite3/sqlite3.h" //libreria para consultar info en data/chile/chile.gpkg

#define PI 3.14159265358979323846
#define MAX 256

// colores para la interfaz o menu
#define ROJO    "\033[31m"
#define VERDE   "\033[32m"
#define AZUL    "\033[34m"
#define MORADO "\033[35m"
#define AMARILLO "\033[33m"
#define RESET   "\033[0m"

//se usara para almaxenar una key temporal, para validar que no se repitan calles en el grafo
typedef struct{
    double keyLongitud;
    double keyLattitud;
}keyCordenadas;

typedef struct{
    int idVertice;
    double distancia;
}nodoDijkstra;

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
    Vertice **porId; //arreglo de punteros a vertice

    List *ultimaRuta; //lista de ids de la ultima ruta
    int hayRuta; //hayRtua = 0 si no hay ruta aun

    int cantidadVertices; //cantidad de lugares
    int cantidadAristas; //cantidad de rutas
} Grafo;

Grafo *generarGrafo(){
    Grafo *grafo = malloc(sizeof(Grafo));
    if (!grafo) return NULL;
    
    grafo -> vertices = list_create();
    grafo -> indiceCoordenadas = hashmap_create(2000003);
    grafo -> porId = malloc(1100000 * sizeof(Vertice *));
    grafo -> ultimaRuta = NULL;
    grafo -> hayRuta = 0;
    grafo -> cantidadVertices = 0;
    grafo -> cantidadAristas = 0;

    return grafo;
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
        //return NULL;
        return existente;
    }

    Vertice *nuevo = malloc(sizeof(Vertice));
    if(nuevo == NULL)
    {
        free(key);
        return NULL;
    }

    nuevo->lugar.id = grafo->cantidadVertices;
    grafo -> porId[grafo -> cantidadVertices] = nuevo;

    strncpy(nuevo->lugar.nombre, nombre ? nombre : "Sin nombre", MAX); // Copia nombre de la calle dentro del vertice si no existe queda sin nombre
    nuevo->lugar.nombre[MAX - 1] = '\0'; // Asegura que el string termine bien a pesar de superar el limite

    nuevo->lugar.latitud = lat;
    nuevo->lugar.longitud = lon; 

    nuevo->conexiones = list_create();

    list_pushBack(grafo->vertices, nuevo);
    hashmap_insert(grafo->indiceCoordenadas, key, nuevo);

    grafo->cantidadVertices ++; 

    return nuevo;
}

Conexion *crearConexion(int idOrigen, int idDestino, double distancia, const char *clase, const char *oneway){
    Conexion *conexion = malloc(sizeof(Conexion));
    if (conexion == NULL){
        puts("Error al crear conexion.");
        presioneTeclaParaContinuar();
        limpiarPantalla();
        exit(EXIT_FAILURE);
    }

    conexion -> origen = idOrigen;
    conexion -> destino = idDestino;
    conexion -> distanciaKm = distancia;

    // Asignar tiempos y combustible según la clase de calle
    if (strcmp(clase, "residential") == 0) {
        // velocidad = distancia / tiempo -> tiempo = distancia / velocidad MAX de la calle

        conexion -> tiempoAuto = distancia / 30.0; // 30 km/h
        conexion -> tiempoBici = distancia / 15.0; // 15 km/h
        conexion -> tiempoPie = distancia / 5.0; // 5 km/h
        conexion -> combustibleAuto = distancia * 0.08; // 0.08 litros/km
    } else if (strcmp(clase, "primary") == 0) {
        conexion -> tiempoAuto = distancia / 60.0; // 60 km/h
        conexion -> tiempoBici = distancia / 20.0; // 20 km/h
        conexion -> tiempoPie = distancia / 5.0; // 5 km/h
        conexion -> combustibleAuto = distancia * 0.1; // 0.1 litros/km
    } else if (strcmp(clase, "secondary") == 0) {
        conexion -> tiempoAuto = distancia / 50.0; // 50 km/h
        conexion -> tiempoBici = distancia / 18.0; // 18 km/h
        conexion -> tiempoPie = distancia / 5.0; // 5 km/h
        conexion -> combustibleAuto = distancia * 0.09; // 0.09 litros/km
    } else {
        // Valores por defecto para otras clases de calles
        conexion -> tiempoAuto = distancia / 40.0; // 40 km/h
        conexion -> tiempoBici = distancia / 12.0; // 12 km/h
        conexion -> tiempoPie = distancia / 5.0; // 5 km/h
        conexion -> combustibleAuto = distancia * 0.07; // 0.07 litros/km
    }

    return conexion;
}

void cargarDatosAlGrafo(Grafo *grafo, const char *nombre, const char *clase, const char *oneway, keyCordenadas keyCor1, keyCordenadas keyCor2){
    //filtrar datos repetidos a traves de el hashmap del grafo, mediante las coordenadas
    Vertice *verticeOrigen = Obtener_CrearVertice(grafo, keyCor1.keyLongitud, keyCor1.keyLattitud, nombre);
    Vertice *verticeDestino = Obtener_CrearVertice(grafo, keyCor2.keyLongitud, keyCor2.keyLattitud, nombre);
    
    double distanciaPuntos = calcularDistancia(verticeOrigen -> lugar.latitud, verticeOrigen -> lugar.longitud, verticeDestino -> lugar.latitud, verticeDestino -> lugar.longitud);

    int esSoloIda = oneway && strcmp(oneway, "yes") == 0;
    
    if (esSoloIda){ //solo ida
        Conexion *conexionIda = crearConexion(verticeOrigen -> lugar.id, verticeDestino -> lugar.id, distanciaPuntos, clase, oneway);
        list_pushBack(verticeOrigen -> conexiones, conexionIda);
        grafo -> cantidadAristas++;
    }

    else //ida y vuelta
    {
        Conexion *conexionIda = crearConexion(verticeOrigen -> lugar.id, verticeDestino -> lugar.id, distanciaPuntos, clase, oneway);
        list_pushBack(verticeOrigen -> conexiones, conexionIda);

        Conexion *conexionVuelta = crearConexion(verticeDestino -> lugar.id, verticeOrigen -> lugar.id, distanciaPuntos, clase, oneway);
        list_pushBack(verticeDestino -> conexiones, conexionVuelta);

        grafo -> cantidadAristas += 2;
    }
}

//primero se verifica si la base de datos puede ser abrida o leida por el programa
void obtenerInformacionDB(Grafo *grafo){
    if (grafo == NULL){
        printf("Error: grafo no inicializado.\n");
        return;
    }

    if (grafo->vertices != NULL && list_size(grafo->vertices) > 0) {
        printf(ROJO"El grafo ya tiene datos cargados. No se pueden cargar nuevamente.\n"RESET);
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
    const char *comando = "SELECT fid, geom, name, fclass, oneway FROM gis_osm_roads_free;"; //LIMIT 50;
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

        //MUESTRA LA INFO DE LAS CALLES: printf("\nFila %d: %s, %s, %s, (%lf, %lf) -> (%lf, %lf)\n", fid, nombre, clase, oneway, lon1, lat1, lon2, lat2);
        keyCordenadas keyCor1 = {lon1, lat1};
        keyCordenadas keyCor2 = {lon2, lat2};
        cargarDatosAlGrafo(grafo, nombre, clase, oneway, keyCor1, keyCor2);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    puts(VERDE"\nDatos cargados exitosamente al grafo.");
    printf("* Numero de vertices cargados" AZUL"(lugares): " AMARILLO"%d\n"RESET, grafo -> cantidadVertices);
    printf(VERDE"* Numero de aristas cargados" AZUL"(conexiones entre lugares): " AMARILLO"%d\n"RESET, grafo -> cantidadAristas);
}

void CalcularRuta(Grafo *grafo){
    if (grafo == NULL || grafo -> cantidadVertices == 0){
        printf("Primero debes cargar los datos\n");
        return;
    }

    //se limpia la ultima ruta, si existe
    if(grafo -> ultimaRuta != NULL){
        list_clean(grafo -> ultimaRuta);
    }
    grafo -> ultimaRuta = list_create();
    grafo -> hayRuta = 0;
    

    //coordenadas de origen y destino (latitud y longitud)
    double lonOrigen, latOrigen, lonDestino, latDestino;

    printf("Ingrese longitud de origen: ");
    scanf("%lf", &lonOrigen);
    printf("Ingrese latitud de origen: ");
    scanf("%lf", &latOrigen);
    printf("Ingrese longitud de destino: ");
    scanf("%lf", &lonDestino);
    printf("Ingrese latitud de destino: ");
    scanf("%lf", &latDestino);

    char *keyOrigen = UnionCoordenada(lonOrigen, latOrigen);
    char *keyDestino = UnionCoordenada(lonDestino, latDestino);
    Vertice  *origen = hashmap_search(grafo -> indiceCoordenadas, keyOrigen);
    Vertice  *destino = hashmap_search(grafo -> indiceCoordenadas, keyDestino);

    free(keyOrigen);
    free(keyDestino);

    if(origen == NULL){
        printf("No se encontro el punto de ORIGEN en el grafo\n");
        return;
    }

    if(destino == NULL){
        printf("No se encontro el punto de DESTINO en el grafo\n");
        return;
    }

    int n = grafo -> cantidadVertices;
    int idOrigen = origen -> lugar.id;
    int idDestino = destino -> lugar.id;

    double *dist = malloc(n * sizeof(double));
    int *anterior = malloc(n * sizeof(int));
    int *visitado = malloc(n * sizeof(int));

    for(int i =0; i < n; i++){
        dist[i] = 1e18;
        anterior[i] = -1;
        visitado[i] = 0;
    }

    dist[idOrigen] = 0;

    Heap *heap = heap_create();

    nodoDijkstra *nodoInicio = malloc(sizeof(nodoDijkstra));
    nodoInicio -> idVertice = idOrigen;
    nodoInicio -> distancia = 0;
    heap_push(heap, nodoInicio, 0);

    printf("\nCalculando Ruta..\n");

    while(!heapEstaVacia(heap)){
        nodoDijkstra *actual = (nodoDijkstra *) heap_top(heap);
        heap_pop(heap);

        int u = actual -> idVertice;
        free(actual);

        if (visitado[u] == 1){
            continue;
        }
        visitado[u] = 1;

        //Si se llega al destino se termina
        if (u == idDestino){
            break;
        }

        //se busca el vertice u en la lista de vertices
        Vertice *verticeActual = grafo -> porId[u];
        if(verticeActual == NULL) continue;

        //se revisan vecino y se actualiza distancia si encontramos camino mas corto
        Conexion *conexion = list_first(verticeActual -> conexiones);
        while (conexion != NULL) {
            int w = conexion -> destino;
            double nuevaDist = dist[u] + conexion -> distanciaKm;

            if(nuevaDist < dist[w]){
                dist[w] = nuevaDist;
                anterior[w] = u;

                nodoDijkstra *nodoVecino = malloc(sizeof(nodoDijkstra));
                nodoVecino -> idVertice = w;
                nodoVecino -> distancia = nuevaDist;

                heap_push(heap, nodoVecino, (int)(-nuevaDist * 1000));
            }
            conexion = list_next(verticeActual -> conexiones);
        }
    }

    if(dist[idDestino] >= 1e18){
        printf("\nNo existe ruta entre los puntos dados\n");

    }
    else{
        Stack *pila = stack_create(NULL);

        int nodoActual = idDestino;
        while (nodoActual != -1){
            int *id = malloc(sizeof(int));
            *id = nodoActual;
            stack_push(pila,id);
            nodoActual = anterior[nodoActual];
        }

        printf("\nRuta encontrada\n");
        printf("=============================================\n");

        while(!pilaEstaVacia(pila)) {
            int *id = (int *) stack_pop(pila);
            Vertice *v = grafo -> porId[*id];

            //se guarda id en la ultima ruta
            int *idGuardado = malloc(sizeof(int));
            *idGuardado = *id;
            list_pushBack(grafo -> ultimaRuta, idGuardado);
            
            if (v != NULL){
                printf("-> %s (%.6f, %.6f)\n", v -> lugar.nombre, v -> lugar.latitud, v -> lugar.longitud);

            }
            free(id);
            grafo -> hayRuta = 1;
        }
        printf("=============================================\n");
        printf("Distancia total : %.3f km\n", dist[idDestino]);
    }

    free(dist);
    free(anterior);
    free(visitado);
    free(heap);

    
}

void mostrarInformacion(Grafo *grafo){
    //verificar que ya se calculo alguna ruta
    if (grafo -> hayRuta == 0 || grafo -> ultimaRuta == NULL){
        printf("Se debe calcular una ruta antes de usar esta opcion\n");
        return;
    }

    double totalDistancia = 0;
    float tiempoAuto = 0;
    float tiempoBici = 0;
    float tiempoPie = 0;
    float combustible = 0;

    printf("\nRuta calculada:\n");
    printf("========================================\n");

    int *idActual = list_first(grafo -> ultimaRuta);
    int *idSiguiente = list_next(grafo -> ultimaRuta);

    while(idActual != NULL){
        Vertice *v = grafo -> porId[*idActual];
        if (v != NULL){
            printf("-> %s (%.6f, %.6f)\n", v -> lugar.nombre, v -> lugar.latitud, v -> lugar.longitud);
        }

        //si hay un vertice siguiente se busca la conexion entre ambos y se suma al total (dist, tiempo y combustible)
        
        if(idSiguiente != NULL){
            Vertice *vActual = grafo -> porId[*idActual];
            if (vActual != NULL){
                Conexion *conexion = list_first(vActual -> conexiones);
                while(conexion != NULL){
                    if (conexion -> destino == *idSiguiente){
                        totalDistancia += conexion -> distanciaKm;
                        tiempoAuto += conexion -> tiempoAuto;
                        tiempoBici += conexion -> tiempoBici;
                        tiempoPie += conexion -> tiempoPie;
                        combustible += conexion -> combustibleAuto;
                        break;
                        
                    }
                    conexion = list_next(vActual -> conexiones);
                }
            }
        }
        idActual = idSiguiente;
        idSiguiente = list_next(grafo -> ultimaRuta);
    }
    printf("========================================\n");
    printf("Distancia total : %.3f km\n", totalDistancia);
    printf("Tiempo en auto : %.1f min\n", tiempoAuto * 60);
    printf("Tiempo en bici : %.1f min\n", tiempoBici * 60);
    printf("Tiempo a pie : %.1f min\n", tiempoPie * 60);
    printf("Combustible aprox. : %.3f L\n", combustible);
    
}

void reportarAccidente(Grafo *grafo){
    printf("Funcion reportarAccidente() por implementar.\n");
}

/*
flujo general del programa: cargar datos -> cargar grafo -> programar funciones -> probar funciones -> mostrar resultados
antes de cerrar programa, liberar memoria y cerrar conexiones a la base de datos
*/

int main() {
    limpiarPantalla();

    Grafo *grafo = generarGrafo(); // se genera o inicializa el grafo

    char opcion;
    do {

        printf(MORADO
        "\n\n"
        "  ____             _       __  __                              \n"
        " |  _ \\ ___  _   _| |_ ___|  \\/  | __ _ _ __  _ __   ___ _ __ \n"
        " | |_) / _ \\| | | | __/ _ \\ |\\/| |/ _` | '_ \\| '_ \\ / _ \\ '__|\n"
        " |  _ < (_) | |_| | ||  __/ |  | | (_| | |_) | |_) |  __/ |   \n"
        " |_| \\_\\___/ \\__,_|\\__\\___|_|  |_|\\__,_| .__/| .__/ \\___|_|   \n"
        "                                       |_|   |_|              \n"
        RESET);

        printf(ROJO"\n========================================\n"RESET);
        printf(AZUL"        Escoge alguna opcion            \n"RESET);
        printf(ROJO"========================================\n"RESET);
        puts(AZUL"1) Cargar Datos");
        puts("  2) Calcular ruta"); //busqueda por id de lugar
        puts("    3) Mostrar informacion"); //busqueda por nombre de calle
        puts("      4) Reportar accidente"); // busqueda por coordenadas
        puts(ROJO"                                5) Salir"RESET);
        printf(ROJO"========================================\n"RESET);
        printf(AMARILLO"Ingrese su opcion -> "VERDE);
        scanf(" %c"RESET, &opcion);

        switch (opcion) {
        case '1':
            obtenerInformacionDB(grafo); // se lee la base de datos y se llena el grafo con los lugares y conexiones
            break;
        case '2':
            CalcularRuta(grafo); //busqueda por id de lugar
            break;
        case '3':
            mostrarInformacion(grafo); //busqueda por nombre de el lugar
            break;
        case '4':
            reportarAccidente(grafo); //busqueda por coordenadas del lugar
            break;
        case '5':
            printf(AZUL"\nHasta luego!\n"RESET);
            break;
        default:
            printf(ROJO"\nOpcion No Valida.\n"RESET);
            break;
        }

        // Evitamos pausar y limpiar pantalla si el usuario eligió salir
        if (opcion != '5') {
            presioneTeclaParaContinuar();
            limpiarPantalla();
        }

    } while (opcion != '5');
    return 0;
}
