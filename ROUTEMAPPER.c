#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
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
    int bloqueada; // 0 = libre, 1 = bloqueada
} Conexion; //calle entre dos puntos

typedef struct {
    Lugar lugar; //informacion con respecto al lugar
    List *conexiones; //lista de rutas con respecto al lugar
} Vertice; //nodo del grafo o lugar del mapa

typedef struct {
    List *vertices; //lista de todos los lugares
    HashMap *indiceCoordenadas; // Para guardar sin repeticion
    HashMap *IndiceNombre; // Para guardar nombres sin repeticion
    Vertice **porId; //arreglo de punteros a vertice

    int cantidadVertices; //cantidad de lugares
    int cantidadAristas; //cantidad de rutas
    int capacidadPorId; 


    List *ultimaruta;
    double ultimaDistanciaKm;
    double ultimoTiempoAuto;
    double ultimoTiempoBici;
    double ultimoTiempoPie;
    double ultimoCombustible;
    int rutaCalculada;
} Grafo;

Grafo *generarGrafo(){
    Grafo *grafo = malloc(sizeof(Grafo));
    if (!grafo) return NULL;
    
    grafo -> vertices = list_create();
    grafo -> indiceCoordenadas = hashmap_create(2000003);
    grafo -> IndiceNombre = hashmap_create(200003);
    
    grafo->capacidadPorId = 1100000;
    grafo->porId = calloc(grafo->capacidadPorId, sizeof(Vertice *));

    if (grafo->vertices == NULL || grafo->indiceCoordenadas == NULL || grafo->porId == NULL) {
        printf("Error al crear estructuras del grafo\n");
        free(grafo);
        return NULL;
    }
    
    grafo -> cantidadVertices = 0;
    grafo -> cantidadAristas = 0;
    
    grafo->ultimaruta = list_create(); 
    grafo->ultimaDistanciaKm = 0;
    grafo->ultimoTiempoAuto = 0;
    grafo->ultimoTiempoBici = 0;
    grafo->ultimoTiempoPie = 0;
    grafo->ultimoCombustible = 0;
    grafo->rutaCalculada = 0;   

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


int asegurarCapacidadPorId(Grafo *grafo){
    if (grafo->cantidadVertices < grafo->capacidadPorId) {
        return 1;
    }

    int nuevaCapacidad = grafo->capacidadPorId * 2;

    Vertice **nuevoPorId = realloc(grafo->porId, nuevaCapacidad * sizeof(Vertice *));

    if (nuevoPorId == NULL) {
        printf("Error: no hay memoria suficiente para ampliar porId\n");
        return 0;
    }

    for (int i = grafo->capacidadPorId; i < nuevaCapacidad; i++) {
        nuevoPorId[i] = NULL;
    }

    grafo->porId = nuevoPorId;
    grafo->capacidadPorId = nuevaCapacidad;

    printf("porId ampliado a %d posiciones\n", nuevaCapacidad);

    return 1;
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

    if (!asegurarCapacidadPorId(grafo)) 
    {
    
    free(nuevo);
    free(key);
    
    return NULL;
    }

    nuevo->lugar.id = grafo->cantidadVertices;
    grafo->porId[grafo->cantidadVertices] = nuevo;  

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

void limpiarBufferEntrada()
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

char *copiarString(const char *texto)
{
    char *copia = malloc(strlen(texto) + 1);
    if(copia == NULL) return NULL; 
    
    strcpy(copia, texto); 
    return copia; 
}

void normalizarUnNombre(const char *origen, char *destino)
{
    int k = 0;

    if(origen == NULL)
    {
        destino[0] = '\0';
        return;
    }
    
    for(int i = 0; origen[i] != '\0'; i++)
    {
        unsigned char c = (unsigned char)origen[i]; 

        if(isspace(c) || c == '-' || c == '.' || c == ',')
        {
            continue;
        }
        destino[k++] = tolower(c);
    }
    destino[k] = '\0';
}

int listaVertices(List *lista, int ID)
{
    Vertice *v = list_first(lista);

    while(v != NULL)
    {
        if(v->lugar.id == ID) return 1;

        v = list_next(lista);
    }
    return 0;
}

void AgregarVerticeNombreHash(Grafo *grafo, Vertice *vertice, const char *nombre)
{
    if(grafo == NULL || vertice == NULL || nombre == NULL) return; 

    if(strcmp(nombre, "Sin nombre") == 0) return; 

    char nombreNormalizado[MAX]; 
    normalizarUnNombre(nombre, nombreNormalizado); 

    if(strlen(nombreNormalizado) == 0) return;

    List *listaCoincidentes = hashmap_search(grafo->IndiceNombre, nombreNormalizado);

    if(listaCoincidentes == NULL)
    {
        listaCoincidentes = list_create();

        char *key = copiarString(nombreNormalizado);
        hashmap_insert(grafo->IndiceNombre, key, listaCoincidentes); 
    }

    if(!listaVertices(listaCoincidentes, vertice->lugar.id)) list_pushBack(listaCoincidentes, vertice);
}


Conexion *crearConexion(int idOrigen, int idDestino, double distancia, const char *clase, const char *oneway)
{
    Conexion *conexion = malloc(sizeof(Conexion));

    if (conexion == NULL){
        puts("Error al crear conexion.");
        return NULL;
    }

    if (clase == NULL) {
        clase = "Desconocido";
    }

    conexion->origen = idOrigen;
    conexion->destino = idDestino;
    conexion->distanciaKm = distancia;
    conexion->bloqueada = 0; 

    if (strcmp(clase, "residential") == 0) {
        conexion->tiempoAuto = distancia / 30.0;
        conexion->tiempoBici = distancia / 15.0;
        conexion->tiempoPie = distancia / 5.0;
        conexion->combustibleAuto = distancia * 0.08;
    } 
    else if (strcmp(clase, "primary") == 0) {
        conexion->tiempoAuto = distancia / 60.0;
        conexion->tiempoBici = distancia / 20.0;
        conexion->tiempoPie = distancia / 5.0;
        conexion->combustibleAuto = distancia * 0.1;
    } 
    else if (strcmp(clase, "secondary") == 0) {
        conexion->tiempoAuto = distancia / 50.0;
        conexion->tiempoBici = distancia / 18.0;
        conexion->tiempoPie = distancia / 5.0;
        conexion->combustibleAuto = distancia * 0.09;
    } 
    else {
        conexion->tiempoAuto = distancia / 40.0;
        conexion->tiempoBici = distancia / 12.0;
        conexion->tiempoPie = distancia / 5.0;
        conexion->combustibleAuto = distancia * 0.07;
    }

    return conexion;
}

void cargarDatosAlGrafo(Grafo *grafo, const char *nombre, const char *clase, const char *oneway, keyCordenadas keyCor1, keyCordenadas keyCor2){
    //filtrar datos repetidos a traves de el hashmap del grafo, mediante las coordenadas
    Vertice *verticeOrigen = Obtener_CrearVertice(grafo, keyCor1.keyLongitud, keyCor1.keyLattitud, nombre);
    Vertice *verticeDestino = Obtener_CrearVertice(grafo, keyCor2.keyLongitud, keyCor2.keyLattitud, nombre);
    
    if (verticeOrigen == NULL || verticeDestino == NULL) return;

    AgregarVerticeNombreHash(grafo, verticeOrigen, nombre);
    AgregarVerticeNombreHash(grafo, verticeDestino, nombre);
    
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

    int rc = sqlite3_open("data/chile.gpkg", &db);

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
    const char *comando = "SELECT fid, geom, name, fclass, oneway ""FROM gis_osm_roads_free ";
    
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

void CalcularRutasExistentes(Grafo *grafo, int idOrigen, int *alcanzable, int accidentes)
{
    if(grafo == NULL || alcanzable == NULL) return;

    for(int i=0; i < grafo->cantidadVertices; i++)
    {
        alcanzable[i] = 0;
    }

    Stack *pila =stack_create(NULL);

    int  *inicio = malloc(sizeof(int));
    *inicio = idOrigen;
    stack_push(pila, inicio); 
    
    alcanzable[idOrigen] = 1;

    while(!pilaEstaVacia(pila))
    {
        int *idactualptr = (int *) stack_pop(pila);
        int idactual = *idactualptr;
        free(idactualptr); 

        Vertice *verticeActual = grafo->porId[idactual];

        if(verticeActual == NULL) continue;

        Conexion *conexion = list_first(verticeActual->conexiones);

        while (conexion != NULL)
        {
            int vecino = conexion->destino;

            if(vecino >= 0 && vecino < grafo->cantidadVertices)
            {
                if(accidentes == 1 && conexion->bloqueada == 1)
                {
                    conexion = list_next(verticeActual->conexiones);
                    continue;
                }

                if(alcanzable[vecino] == 0)
                {
                    alcanzable[vecino] = 1;

                    int *nuevoID = malloc(sizeof(int));
                    *nuevoID = vecino;
                    stack_push(pila, nuevoID);
                }
            }
            conexion = list_next(verticeActual->conexiones);
        }
        
    }
    free(pila);
}


Vertice *SeleccionarVerticePorNombre(Grafo *grafo, const char *tipoPunto, int *alcanzableSinAccidente, int *alcanzableConAccidente)
{
    char nombreBuscado[MAX];

    printf("\nIngrese Nombre de calle para %s: ", tipoPunto);
    limpiarBufferEntrada();
    fgets(nombreBuscado, MAX, stdin);

    nombreBuscado[strcspn(nombreBuscado, "\n")] = '\0'; 

    char nombreNormalizado[MAX];
    normalizarUnNombre(nombreBuscado, nombreNormalizado);

    List *coincidencia = hashmap_search(grafo->IndiceNombre, nombreNormalizado);

    if(coincidencia == NULL)
    {
        printf("No se encontraron coincidencias para: %s\n", nombreBuscado);
        return NULL;
    }

    printf("\nCoincidencias encontradas:\n");
    printf("====================================================================================================================\n");
    printf("%-8s | %-30s | %-12s | %-12s | %-12s | %-18s\n","ID", "Nombre", "Latitud", "Longitud", "Ruta", "Accidentes");
    printf("====================================================================================================================\n");

    int contador = 0;
    Vertice *v = list_first(coincidencia); 

    while(v != NULL)
    {
        char Ruta[20];
        char Accidente[25];

        if(alcanzableSinAccidente == NULL || alcanzableConAccidente == NULL)
        {
            strcpy(Ruta, "ORIGEN");
            strcpy(Accidente, "-");
        }
        else if(alcanzableConAccidente[v->lugar.id] == 1)
        {
            strcpy(Ruta, "Hay Ruta");
            strcpy(Accidente, "No Hay Accidente");
        }
        else if(alcanzableSinAccidente[v->lugar.id] == 1 && alcanzableConAccidente[v->lugar.id] == 0)
        {
            strcpy(Ruta, "No Hay Ruta");
            strcpy(Accidente, "Si Hay Accidente");
        }
        else
        {
            strcpy(Ruta, "No Hay Ruta");
            strcpy(Accidente, "No Hay Accidente");
        }

        printf("%-8d | %-30s | %-12.6f | %-12.6f | %-12s | %-18s\n", v->lugar.id, v->lugar.nombre, v->lugar.latitud, v->lugar.longitud, Ruta, Accidente); 

        contador ++;

        v = list_next(coincidencia);
    }

    printf("====================================================================================================================\n");
    printf("Coincidencias en total: %d\n", contador); 

    int opcion;

    printf("\nSeleccione el punto:\n");
    printf("1) Seleccionar por ID\n");
    printf("2) Seleccionar por Coordenadas exactas\n");
    printf("Ingrese Opcion: ");
    scanf("%d", &opcion);


    if(opcion == 1)
    {
        int id;
        printf("Ingrese ID: ");
        scanf("%d", &id);

        if(id < 0 || id >= grafo->cantidadVertices || grafo->porId[id] == NULL)
        {
            printf("ID no valido\n");
            return NULL; 
        }

        if(alcanzableConAccidente != NULL && alcanzableConAccidente[id] == 0)
        {
            if(alcanzableSinAccidente != NULL && alcanzableSinAccidente[id] == 1)
            {
                printf("\n La ruta no se recomienda ya que existe un accidente bloqueando el camino"); 
            }
            else
            {
                printf("\n No existe una ruta entre ambos puntos"); 
            }
            
            return NULL; 
        }

        return grafo->porId[id];
    }
    if(opcion == 2)
    {
        double lon, lat; 
        

        printf("Ingrese longitud: ");
        scanf("%lf", &lon);

        printf("\nIngrese latitud: ");
        scanf("%lf", &lat);

        char *key = UnionCoordenada(lon, lat);
        Vertice *seleccionado = hashmap_search(grafo->indiceCoordenadas, key);
        free(key); 

        if(seleccionado == NULL)
        {
            printf("No se encontro un vertice con esas coordenadas exactas\n");
            return NULL;
        }

        int id = seleccionado->lugar.id;

        if(alcanzableConAccidente != NULL && alcanzableConAccidente[id] == 0)
        {
            if(alcanzableSinAccidente != NULL && alcanzableSinAccidente[id] == 1)
            {
                printf("\nLa ruta no se recomienda ya que existe un accidente bloqueando el camino\n"); 
            }
            else
            {
                printf("\nNo existe una ruta entre ambos puntos\n"); 
            }

            return NULL; 
        }

        return seleccionado; 
    }
    printf("Opcion no valida\n");
    return NULL;
}

Conexion *buscarConexionEntre(Grafo *grafo, int idInicio, int idFinal)
{
    if(grafo == NULL || idInicio < 0 || idInicio >= grafo->cantidadVertices) return NULL;

    Vertice *origen = grafo->porId[idInicio];

    if(origen == NULL) return NULL;

    Conexion *conexion = list_first(origen->conexiones);

    while(conexion != NULL)
    {
        if(conexion->destino == idFinal) return conexion; 

        conexion = list_next(origen->conexiones);
    }
    return NULL;
}

void CalcularRuta(Grafo *grafo)
{
    if (grafo == NULL || grafo->cantidadVertices == 0){
        printf("Primero debes cargar los datos\n");
        return;
    }

    printf("\nSeleccionar Origen\n");
    Vertice *origen = SeleccionarVerticePorNombre(grafo, "Origen", NULL, NULL);

    if(origen == NULL)
    {
        printf("Error al seleccionar origen intentelo nuevamente");
        return; 
    }

    int *alcanzableSinAccidente = malloc(grafo->cantidadVertices * sizeof(int));
    int *alcanzableConAccidente = malloc(grafo->cantidadVertices * sizeof(int));

    if(alcanzableSinAccidente == NULL || alcanzableConAccidente == NULL)
    {
        free(alcanzableConAccidente);
        free(alcanzableSinAccidente);
        return;
    }

    CalcularRutasExistentes(grafo, origen->lugar.id, alcanzableSinAccidente, 0);
    CalcularRutasExistentes(grafo, origen->lugar.id, alcanzableConAccidente, 1);

    printf("Seleccionar Destino\n");
    Vertice *destino = SeleccionarVerticePorNombre(grafo, "Destino", alcanzableSinAccidente, alcanzableConAccidente);

    if(destino == NULL)
    {
        printf("Error al seleccionar destino intentelo nuevamente");
        free(alcanzableConAccidente);
        free(alcanzableSinAccidente);
        return; 
    }

    int n = grafo->cantidadVertices;
    int idOrigen = origen->lugar.id;
    int idDestino = destino->lugar.id;

    double *dist = malloc(n * sizeof(double));
    int *anterior = malloc(n * sizeof(int));
    int *visitado = malloc(n * sizeof(int));

    if (dist == NULL || anterior == NULL || visitado == NULL){
        printf("Error al reservar memoria para Dijkstra\n");
        free(dist);
        free(anterior);
        free(visitado);
        free(alcanzableConAccidente);
        free(alcanzableSinAccidente);
        return;
    }

    for(int i = 0; i < n; i++){
        dist[i] = 1e18;
        anterior[i] = -1;
        visitado[i] = 0;
    }

    dist[idOrigen] = 0;

    Heap *heap = heap_create();

    nodoDijkstra *nodoInicio = malloc(sizeof(nodoDijkstra));
    nodoInicio->idVertice = idOrigen;
    nodoInicio->distancia = 0.0;

    heap_push(heap, nodoInicio, 0);

    printf("\nCalculando ruta...\n");

    while(!heapEstaVacia(heap)){
        nodoDijkstra *actual = (nodoDijkstra *) heap_top(heap);
        heap_pop(heap);

        int u = actual->idVertice;
        free(actual);

        if (visitado[u] == 1){
            continue;
        }

        visitado[u] = 1;

        if (u == idDestino){
            break;
        }

        Vertice *verticeActual = grafo->porId[u];

        if(verticeActual == NULL){
            continue;
        }

        Conexion *conexion = list_first(verticeActual->conexiones);

        while (conexion != NULL) {
            int w = conexion->destino;
            double nuevaDist = dist[u] + conexion->distanciaKm;

            if(nuevaDist < dist[w] && conexion -> bloqueada == 0){
                dist[w] = nuevaDist;
                anterior[w] = u;

                nodoDijkstra *nodoVecino = malloc(sizeof(nodoDijkstra));
                nodoVecino->idVertice = w;
                nodoVecino->distancia = nuevaDist;

                heap_push(heap, nodoVecino, (int)(-nuevaDist * 1000));
            }

            conexion = list_next(verticeActual->conexiones);
        }
    }

    if(dist[idDestino] >= 1e18){
        printf("\nNo existe ruta entre los puntos dados\n");

        grafo->rutaCalculada = 0;

        if(grafo->ultimaruta != NULL) list_clean(grafo->ultimaruta);
    }

    else
    
    {
        Stack *pila = stack_create(NULL);

        int nodoActual = idDestino;

        while (nodoActual != -1)
        {
            int *id = malloc(sizeof(int));
            *id = nodoActual;
            stack_push(pila, id);
            nodoActual = anterior[nodoActual];
        }

        // Limpiar ruta anterior
        list_clean(grafo->ultimaruta);

        grafo->ultimaDistanciaKm = dist[idDestino];
        grafo->ultimoTiempoAuto = 0;
        grafo->ultimoTiempoBici = 0;
        grafo->ultimoTiempoPie = 0;
        grafo->ultimoCombustible = 0;

        int idAnteriorRuta = -1;

        while(!pilaEstaVacia(pila)) 
        {
            int *id = (int *) stack_pop(pila);
            Vertice *v = grafo->porId[*id];

            if (v != NULL) list_pushBack(grafo->ultimaruta, v);

            if (idAnteriorRuta != -1)
            {
                Conexion *conexion = buscarConexionEntre(grafo, idAnteriorRuta, *id);

                if (conexion != NULL)
                {
                    grafo->ultimoTiempoAuto += conexion->tiempoAuto;
                    grafo->ultimoTiempoBici += conexion->tiempoBici;
                    grafo->ultimoTiempoPie += conexion->tiempoPie;
                    grafo->ultimoCombustible += conexion->combustibleAuto;
                }
            }

            idAnteriorRuta = *id;
            free(id);
        }

        grafo->rutaCalculada = 1;

        printf("\nRuta calculada correctamente\n");
        printf("Ir a opcion 3 para ver informacion completa\n");
    }

    free(dist);
    free(anterior);
    free(visitado);
    free(heap);
    free(alcanzableConAccidente);
    free(alcanzableSinAccidente);
}

void mostrarInformacion(Grafo *grafo)
{
    if(grafo == NULL)
    {
        printf("Error Grafo no inicializado");
        return;
    }
    if(grafo->rutaCalculada == 0 || grafo->ultimaruta == NULL)
    {
        printf("Primero Calcula una ruta en la Opcion 2.\n");
        return;
    }

    printf("\n========== INFORMACION DE LA ULTIMA RUTA ==========\n");
    printf("\nCamino calculado:\n");
    printf("---------------------------------------------------\n");

    int contador = 1; 
    Vertice *v = list_first(grafo->ultimaruta);
    
    while(v != NULL)
    {
        printf("%d) ID: %d | %s | Lat: %.6f | Lon: %.6f\n", contador, v->lugar.id, v->lugar.nombre, v->lugar.latitud, v->lugar.longitud);

        Vertice *siguiente = list_next(grafo->ultimaruta);

        if(siguiente != NULL) printf("                         %s|%s\n", AMARILLO, RESET);

        contador ++;
        v = siguiente; 
    }

     printf("\n========== RESUMEN DE LA RUTA ==========\n");
     printf("Distancia Total: %.3fKm \n", grafo->ultimaDistanciaKm);
     printf("Tiempo estimado en auto: %.1fMin \n", grafo->ultimoTiempoAuto * 60);
     printf("Tiempo estimado en bicicleta: %.1fMin \n", grafo->ultimoTiempoBici * 60); 
     printf("Tiempo estimado caminando: %.1fMin \n", grafo->ultimoTiempoPie * 60);
     printf("Combustible aproximado en auto: %.3fL \n", grafo->ultimoCombustible);
     printf("==========================================\n");
    
}

void reportarAccidente(Grafo *grafo)
{
    if (grafo == NULL || grafo -> cantidadVertices == 0)
    {
        printf("Primero debes cargar los datos\n");
        return;
    }

    //se ingresan los ids de origen y destino de la conexion a bloquear
    int idOrigen, idDestino;

    printf("\n========== REPORTAR ACCIDENTE ==========\n");
    
    printf("Ingrese el ID de la calle afectada Origen: ");
    scanf("%d", &idOrigen);

     if(idOrigen < 0 || idOrigen >= grafo->cantidadVertices || grafo->porId[idOrigen] == NULL)
    {
        printf("ID de origen invalido\n");
        return; 
    }

    Vertice *verticeOrigen = grafo -> porId[idOrigen];
    
    printf("\n Calle origen seleccionado\n");
    printf("ID: %d | %s | Lat: %.6f | Lon: %.6f\n", verticeOrigen->lugar.id, verticeOrigen->lugar.nombre, verticeOrigen->lugar.latitud, verticeOrigen->lugar.longitud);

    printf("\nConexiones cercanas desde este origen\n");
    printf("=====================================================================================\n");
    printf("%-10s | %-30s | %-12s | %-12s | %-10s\n", "Destino", "Nombre", "Latitud", "Longitud", "Estado");
    printf("=====================================================================================\n");


    int cantidad_Conexiones = 0;

    Conexion *conexion = list_first(verticeOrigen -> conexiones);
    
    while(conexion != NULL)
    {
        int idVecino = conexion->destino;

        if(idVecino >= 0 && idVecino < grafo->cantidadVertices && grafo->porId[idVecino] != NULL)
        {
            Vertice *vecino = grafo->porId[idVecino];

            printf("%-10d | %-30s | %-12.6f | %-12.6f | %-10s\n", vecino->lugar.id, vecino->lugar.nombre, vecino->lugar.latitud, vecino->lugar.longitud, conexion->bloqueada == 1 ? "ACCIDENTE" : "LIBRE");

            cantidad_Conexiones ++; 
        }

        conexion = list_next(verticeOrigen->conexiones);
    
    }

    printf("=====================================================================================\n");

    if(cantidad_Conexiones == 0) 
    {
        printf("Esta calle no tiene conexiones salientes\n");
        return;
    }

    printf("\n Ingrese el ID de la calle afectada Destino: ");
    scanf("%d", &idDestino); 

    if(idDestino < 0 || idDestino >= grafo->cantidadVertices || grafo->porId[idDestino] == NULL)
    {
        printf("ID de destino invalido\n");
        return; 
    }

    Conexion *conexionAccidente = buscarConexionEntre(grafo, idOrigen, idDestino);

    if(conexionAccidente == NULL)
    {
        printf("\nNo existe una conexion directa entre %d y %d\n", idOrigen, idDestino);
        printf("Porfavor selecciona un destino de la tabla");    
    }

    if(conexionAccidente->bloqueada == 1)
    {
        printf("\nEste segmento ya tiene un accidente reportado");
        return;
    }

    conexionAccidente->bloqueada = 1;
    int bloqueodevuelta = 0;
    Conexion *conexionVuelta = buscarConexionEntre(grafo, idDestino, idOrigen);

    if(conexionVuelta != NULL)
    {
        conexionVuelta->bloqueada = 1;
        bloqueodevuelta = 1;
    }

    grafo->rutaCalculada = 0;

    if(grafo->ultimaruta != NULL) list_clean(grafo->ultimaruta); 

    printf("\nAccidente reportado correctamente\n");
    printf("Segmento de calle bloqueado %d -> %d\n", idOrigen, idDestino);

    if(bloqueodevuelta == 1) printf("Tambien se bloqueo la vuelta: %d -> %d\n", idDestino, idOrigen);
    else printf("No existia vuelta, solo se bloqueo una sola direccion");

    printf("Ultima ruta fue innvalidad porfavor vuelva a calcular la ruta"); 
}

/*
flujo general del programa: cargar datos -> cargar grafo -> programar funciones -> probar funciones -> mostrar resultados
antes de cerrar programa, liberar memoria y cerrar conexiones a la base de datos
*/

int main() 
{
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
