# RouteMapper

RouteMapper es una aplicacion de consola desarrollada en C que simula un sistema basico de rutas. El programa carga datos reales de calles desde un archivo GeoPackage/SQLite de OpenStreetMap, construye un grafo ponderado y permite calcular rutas entre puntos usando el algoritmo de Dijkstra.

## Link de descargas

Archivo de OpenStreetMap.

Este no va con el programa incluido debido al peso de este que imposibilita la subidas de commits entre compañeros en github.

- https://download.geofabrik.de/south-america/chile.html Link de la pagina.

Se debe realizar instalacion del archivo chile-latest-free.gpkg.zip ultimo link de descarga en el sector Commonly Used Formats

- https://download.geofabrik.de/south-america/chile-latest-free.gpkg.zip Link directo de descarga si no encuentra el link original 

Este debe pegarse dentro de los archivos del proyecto especificamente en la carpeta data con este nombre "chile.gpkg" para el funcionamiento 
correcto del programa.

---

## Requisitos del proyecto

Para ejecutar el programa se necesita:

- Compilador GCC.
- Código fuente del proyecto.
- TDAs usados por el programa.
- Librería SQLite incluida en el proyecto o instalada en el sistema.
- Archivo `chile.gpkg`.

---

## Descripción general

RouteMapper trabaja con datos viales reales provenientes de OpenStreetMap. La información se obtiene desde una base de datos GeoPackage llamada `chile.gpkg`, la cual internamente funciona como una base SQLite.

El programa lee la tabla:

```sql
gis_osm_roads_free
```

Desde esta tabla se extraen principalmente los campos:

```sql
fid, geom, name, fclass, oneway
```

Cada registro representa un segmento de calle o camino. A partir de esos segmentos se construye un grafo.

En el grafo:

- Cada vertice representa un punto geografico.
- Cada vertice tiene un ID, nombre, latitud y longitud.
- Cada conexion representa un tramo de calle entre dos puntos.
- Cada conexion tiene distancia, tiempos estimados, combustible aproximado y estado de accidente.
- Dijkstra calcula la ruta más corta usando la distancia como peso principal.

---

## Funcionamiento general

El flujo del programa es:

```text
1. Se inicializa un grafo vacio.
2. El usuario selecciona la opcion Cargar Datos.
3. El programa abre el archivo data/chile.gpkg.
4. Se leen los segmentos de calles desde SQLite.
5. Se extraen las coordenadas de cada segmento.
6. Se crean vertices y conexiones.
7. Se evita duplicar vertice usando un HashMap de coordenadas.
8. Se crea un indice por nombre de calle usando otro HashMap.
9. El usuario puede calcular rutas por nombre de calle, ID o coordenadas.
10. El programa verifica si el destino es alcanzable.
11. Dijkstra calcula la ruta mas corta.
12. La ruta calculada se guarda para mostrarla posteriormente.
```

El programa funciona completamente por consola.

---

## Estructuras utilizadas

### Grafo

La estructura principal es `Grafo`.

```c
typedef struct {
    List *vertices;
    HashMap *indiceCoordenadas;
    HashMap *IndiceNombre;
    Vertice **porId;

    int cantidadVertices;
    int cantidadAristas;
    int capacidadPorId;

    List *ultimaruta;
    double ultimaDistanciaKm;
    double ultimoTiempoAuto;
    double ultimoTiempoBici;
    double ultimoTiempoPie;
    double ultimoCombustible;
    int rutaCalculada;
} Grafo;
```

Esta estructura almacena:

- Todos los vertice del mapa.
- Un HashMap para coordenadas.
- Un HashMap para nombres de calles.
- Un arreglo `porId` para acceder rapido a vertices por ID.
- La ultima ruta calculada.
- La distancia, tiempos y combustible de la ultima ruta.

---

### Lugar

Representa la informacion basica de un punto del mapa.

```c
typedef struct {
    int id;
    char nombre[MAX];
    double latitud;
    double longitud;
} Lugar;
```

---

### Vertice

Representa un nodo del grafo.

```c
typedef struct {
    Lugar lugar;
    List *conexiones;
} Vertice;
```

Cada vertice tiene una lista de conexiones hacia otros vértices.

---

### Conexion

Representa una arista del grafo.

```c
typedef struct {
    int origen;
    int destino;

    float distanciaKm;
    float tiempoAuto;
    float tiempoBici;
    float tiempoPie;
    float combustibleAuto;

    int bloqueada;
} Conexion;
```

El campo `bloqueada` indica si una calle tiene accidente:

```text
0 = libre
1 = bloqueada por accidente
```

---

## Algoritmos utilizados

### Formula de Haversine

Se usa para calcular la distancia aproximada entre dos coordenadas geograficos.

El programa recibe latitud y longitud, las transforma a radianes y calcula la distancia en kilometros.

Esta distancia se usa como peso principal de la conexion.

---

### HashMap de coordenadas

Para evitar duplicar vertices, el programa genera una clave con longitud y latitud.

Ejemplo:

```text
-71.612345,-33.056765
```

Si la coordenada ya existe, se reutiliza el vertice existente.  
Si no existe, se crea un nuevo vertice.

Esto permite que la carga del grafo sea mucho más eficiente.

---

### HashMap de nombres

El programa tambien crea un indice por nombre de calle.

Antes de guardar el nombre, este se normaliza:

- Se eliminan espacios.
- Se eliminan guiones.
- Se eliminan puntos.
- Se eliminan comas.
- Se convierte a minúsculas.

Ejemplo:

```text
"Avenida Argentina" -> "avenidaargentina"
```

Esto permite buscar una calle de forma mas rapida sin recorrer todos los vertices.

---

### Verificacion de alcanzabilidad

Antes de ejecutar Dijkstra, el programa calcula que vertices son alcanzables desde el origen.

Se calcula en dos formas:

```text
1. Alcanzabilidad sin considerar accidentes.
2. Alcanzabilidad considerando accidentes.
```

Esto permite mostrar al usuario una tabla con columnas como:

```text
Ruta | Accidentes
```

Ejemplo:

```text
Hay Ruta    | No Hay Accidente
No Hay Ruta | Si Hay Accidente
No Hay Ruta | No Hay Accidente
```

Esta verificación no reemplaza a Dijkstra. Solo sirve para informar si el destino es posible antes de calcular la ruta.

---

### Dijkstra

Dijkstra calcula la ruta mas corta entre origen y destino.

El programa usa:

- Arreglo de distancias.
- Arreglo de predecesores.
- Arreglo de visitados.
- Heap como cola de prioridad.
- Lista de conexiones de cada vértice.

La condicion principal usada por Dijkstra es:

```c
if(nuevaDist < dist[w] && conexion->bloqueada == 0)
```

Esto significa que Dijkstra ignora las conexiones bloqueadas por accidentes.

## Archivos necesarios

La estructura esperada es:

```text
Proyecto/
│
├── ROUTEMAPPER.c
│
├── README.md
│
├── data/
│   └── chile.gpkg
│
├── tdas/
│   ├── list.c
│   ├── list.h
│   ├── hashmap.c
│   ├── hashmap.h
│   ├── stack.c
│   ├── stack.h
│   ├── queue.c
│   ├── queue.h
│   ├── heap.c
│   ├── heap.h
│   ├── extra.c
│   └── extra.h
│
└── libSqlite3/
    ├── sqlite3.c
    └── sqlite3.h
```

El archivo de datos debe estar en:

```text
data/chile.gpkg
```

Esto es importante porque el programa abre la base de datos con:

```c
sqlite3_open("data/chile.gpkg", &db);
```
---

## Como ejecutar o Compilar

### Windows

#### Ejecutar el programa
```bash
./RouteMapper.exe
```
#### Compilar el programa

```bash
gcc ROUTEMAPPER.c tdas/extra.c tdas/heap.c tdas/list.c tdas/hashmap.c libSqlite3/sqlite3.c -o ROUTEMAPPER -lm
```
---

## Menu del programa

Al iniciar, el programa muestra un menú por consola:

```text
========================================
        Escoge alguna opcion
========================================
1) Cargar Datos
  2) Calcular ruta
    3) Mostrar informacion
      4) Reportar accidente
                                5) Salir
========================================
Ingrese su opcion ->
```

---

## Explicacion de cada Opcion

### Opcion 1: Cargar Datos

Carga las calles desde el archivo `data/chile.gpkg`.

El programa:

1. Abre la base SQLite.
2. Consulta la tabla `gis_osm_roads_free`.
3. Extrae geometria, nombre, clase y sentido de calle.
4. Convierte las coordenadas en vertices.
5. Calcula la distancia con Haversine.
6. Crea conexiones entre los vertices.
7. Guarda los vertices en una lista.
8. Guarda las coordenadas en un HashMap.
9. Guarda los nombres en otro HashMap.

Al terminar muestra la cantidad de vertices y aristas cargadas.

---

### Opcion 2: Calcular ruta

Permite calcular una ruta entre un origen y un destino.

El flujo es:

```text
1. El usuario ingresa el nombre de la calle de origen.
2. El programa muestra coincidencias encontradas.
3. El usuario selecciona un punto por ID o coordenadas exactas.
4. El programa calcula qué puntos son alcanzables desde el origen.
5. El usuario ingresa el nombre de la calle de destino.
6. El programa muestra coincidencias con columnas de Ruta y Accidentes.
7. El usuario selecciona el destino.
8. Dijkstra calcula la ruta más corta.
9. La ruta queda guardada como última ruta calculada.
```

La tabla de destino muestra:

```text
ID | Nombre | Latitud | Longitud | Ruta | Accidentes
```

Estados posibles:

```text
Hay Ruta    | No Hay Accidente
No Hay Ruta | Si Hay Accidente
No Hay Ruta | No Hay Accidente
```

---

### Opcion 3: Mostrar información

Muestra la ultima ruta calculada.

- IDs de los puntos.
- Nombre asociado a cada punto.
- Latitud.
- Longitud.
- Distancia total.
- Tiempo estimado en auto.
- Tiempo estimado en bicicleta.
- Tiempo estimado caminando.
- Combustible aproximado.

Ejemplo:

```text
1) ID: 100 | Avenida Argentina | Lat: -33.045000 | Lon: -71.620000
                         |
2) ID: 101 | Avenida Argentina | Lat: -33.046000 | Lon: -71.621000
                         |
3) ID: 102 | Calle Ejemplo | Lat: -33.047000 | Lon: -71.622000
```

Si no existe ruta calculada muestra:

```text
Primero Calcula una ruta en la Opcion 2.
```

---

### Opcion 4: Reportar accidente

Esta Opcion permite marcar una conexion como bloqueada.

Funcionamiento actual:

```text
1. El usuario ingresa el ID del origen.
2. El programa muestra las conexiones salientes desde ese origen.
3. El usuario selecciona un ID de destino.
4. El programa busca la conexion origen -> destino.
5. Si existe, la marca como bloqueada.
6. Si existe conexion de vuelta, también la bloquea.
7. Se invalida la última ruta calculada.
```

Cuando una conexion queda bloqueada, Dijkstra ya no la considera para futuras rutas.

Ejemplo de tabla:

```text
Destino    | Nombre                         | Latitud      | Longitud     | Estado
120        | Avenida Argentina              | -33.045000   | -71.620000   | LIBRE
121        | Calle Ejemplo                  | -33.046000   | -71.621000   | ACCIDENTE
```

Esta opción funciona de forma basica, pero no es la parte más pulida del programa. Se recomienda usar un destino que aparezca en la tabla de conexiones mostrada.

---

### Opción 5: Salir

Finaliza el programa.

---

## Estado actual de las funcionalidades

| Opción | Funcionalidad | Estado |
|---|---|---|
| 1 | Cargar datos desde GeoPackage | Funciona |
| 2 | Calcular ruta con Dijkstra | Funciona |
| 3 | Mostrar información de la última ruta | Funciona |
| 4 | Reportar accidente | Funciona de forma básica, pero no está completamente pulida |
| 5 | Salir | Funciona |

---

## Opciones con limitaciones

### Opción 4: Reportar accidente

La opción de reportar accidente funciona de forma basica: permite bloquear una conexión directa y si existe, también bloquea la vuelta.

Sin embargo tiene limitaciones:

- El usuario debe ingresar IDs validos.
- El destino debe ser una conexión directa del origen.
- Si se elige un destino que no aparece en la tabla, no se debe continuar con el cálculo.
- La interfaz todavia es menos intuitiva que las demás opciones.
- No permite eliminar accidentes reportados.
- Los accidentes no se guardan al cerrar el programa.
- Si existe una ruta alternativa, Dijkstra igualmente podra calcular otra ruta, por lo que puede parecer que el accidente no afecto aunque si se haya bloqueado una arista.

Recomendación de uso:

```text
1. Entrar a la opción 4.
2. Ingresar un ID de origen.
3. Elegir como destino uno de los IDs mostrados en la tabla.
4. Volver a calcular la ruta en la opción 2.
```

---

### busqueda por coordenadas exactas

La busqueda por coordenadas exactas puede fallar si las coordenadas ingresadas no coinciden exactamente con las guardadas por el programa.

La clave usa 6 decimales:

```c
sprintf(limite, "%.6lf,%.6lf", lon, lat);
```

Por eso, pequeñas diferencias en los decimales pueden hacer que no se encuentre el vertice.

---

### Calles sin nombre

Algunos segmentos de OpenStreetMap no tienen nombre. En ese caso el programa usa:

```text
Sin nombre
```

Estas calles no siempre son comodas de buscar desde la consola.

---

### Nombres con tildes

La normalización de nombres no elimina tildes. Por ejemplo:

```text
Jose
José
```

pueden tratarse como nombres diferentes.

---

### Carga completa de Chile

La carga completa del archivo puede ser lenta, dependiendo del computador y del tamaño de la base de datos.

Para pruebas rapidas se puede usar un `LIMIT` en la consulta SQL, por ejemplo:

```sql
SELECT fid, geom, name, fclass, oneway
FROM gis_osm_roads_free
LIMIT 10000;
```

---

## Complejidades temporales

| operacion | Complejidad aproximada |
|---|---|
| Buscar coordenada en HashMap | O(1) promedio |
| Crear vértice | O(1) promedio |
| Insertar conexión | O(1) |
| Buscar nombre en HashMap | O(1) promedio + O(k) |
| Verificar alcanzabilidad | O(V + E) |
| Dijkstra con Heap | O((V + E) log V) |
| Mostrar última ruta | O(k) |
| Reportar accidente | O(grado del vértice) |

Donde:

- `V` es la cantidad de vertices.
- `E` es la cantidad de conexiones.
- `k` es la cantidad de coincidencias o vertices en la ruta.
- `grado del vértice` es la cantidad de conexiones salientes desde el origen.

---

## Limitaciones generales

- No tiene interfaz grafica.
- No muestra la ruta sobre un mapa visual.
- La interacción es por consola.
- La seleccion de puntos depende de IDs, nombres o coordenadas exactas.
- Algunos datos de OpenStreetMap vienen incompletos.
- Algunos segmentos pueden estar desconectados.
- El calculo de ruta usa distancia como criterio principal.
- Los tiempos son aproximados.
- El combustible es aproximado.
- Los accidentes no se guardan de forma permanente.
- No existe opcion para desbloquear una calle.
- El manejo de accidentes funciona, pero no es tan robusto como el calculo de ruta.

---

## Mejoras futuras

Algunas mejoras posibles son:

- Pulir la opcion de reportar accidentes.
- Agregar una opcion para eliminar accidentes.
- Guardar accidentes en un archivo.
- Permitir busqueda aproximada por coordenadas.
- Mejorar busqueda de nombres con tildes.
- Permitir calcular rutas por menor tiempo y no solo menor distancia.
- Cargar solo una ciudad o region específica.
- Agregar filtros por comuna o ciudad.
- Mostrar la ruta graficamente.
- Agregar interfaz visual.
- Mejorar la detenccion de calles desconectadas.
- Agregar un sistema de tráfico simulado.
---

### APORTE DE CADA INTEGRANTE EN EL PROYECTO:

## Roberto Osses:
- Participación en la planificación general del proyecto y definición de idea.
- Trabajo en la presentación.
- Trabajo en la redacción y organización del informe.
- Apoyo en el diseño del grafo como una estructura para representar la red vial.
- Apoyo en la creación de vértices a partir de coordenadas.
- Implementación de la función que trata los grados como radianes.
- Implementación de la función que calcula la distancia entre 2 puntos mediante la fórmula de Haversine.
- Apoyo en la función que se encarga de guardar los datos dentro del grafo.
- Organización como equipo y planificación de futuras tareas.
- Apoyo en la prueba final del programa.

## Jhon Veliz:
- Participación en la planificación general del proyecto y definición de idea.
- Trabajo en la presentación.
- Trabajo en la redacción y organización del informe.
- Investigación del mapa de Chile, exportando como GeoPackage.
- Implementación de la carga de datos mediante SQLite.
- Extracción de las rutas de la tabla obtenida en OpenStreetMap.
- Trabajo en conjunto para la funciones creación de vértices y cálculo de distancias e inserción de conexiones hacia el grafo.
- Apoyo en la integración del HashMap como índice de coordenada para evitar vértices duplicados.
- Apoyo en la prueba final del programa.

Martin Astorga:
- Participación en la planificación general del proyecto y definición de idea.
- Revisión de coherencia entre código, documentación y exposición.
- Trabajo por implementar principalmente “cálculo de ruta”, selección de origen y destino, validación de entrada de usuario y visualización en consola en la función “mostrar información”,“reportar accidentes”.
- Trabajo en la redacción y organización del informe.
- Trabajo en la presentación.
- Documentación de las nuevas funcionalidades implementadas.
- Apoyo en la prueba final del programa.

Diego Rojas:
- Participación en la planificación general del proyecto y definición de idea.
- Trabajo en la presentación.
- Trabajo en la redacción y organización del informe.
- Trabajo por implementar principalmente “cálculo de ruta”, selección de origen y destino, validación de entrada de usuario y visualización en consola en la función “mostrar información, “reportar accidentes”.
- Apoyo pendiente en la integración del heap como cola de prioridad para Dijkstra.
- Documentación de las nuevas funcionalidades implementadas.
- Apoyo en la prueba final del programa.