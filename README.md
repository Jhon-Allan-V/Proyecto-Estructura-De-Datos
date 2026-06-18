\# ROUTEMAPPER



ROUTEMAPPER utiliza datos reales obtenidos desde OpenStreetMap para modelar la red vial de Chile, sobre la cual se ejecutan algoritmos de búsqueda y optimización de rutas.



\## Fuente de datos



Los datos geográficos utilizados por el proyecto provienen de Geofabrik:



🔗 https://download.geofabrik.de/south-america/chile.html



\---



\## Dependencias



El proyecto utiliza \*\*SQLite3\*\* para consultar la información almacenada en la base de datos geográfica ubicada en el directorio `data/`.



\### Descargar SQLite3



Para Windows, descargar SQLite3 desde su página oficial:



🔗 https://sqlite.org/download.html



Versión utilizada:



```

sqlite-amalgamation-3530200.zip

```



Dentro del proyecto solo se utilizan los siguientes archivos:



```

libSqlite3/

├── sqlite3.c

└── sqlite3.h

```



\---



\## Archivos requeridos



El programa necesita los siguientes archivos dentro del directorio `data/`:



```

data/

├── chile-latest-free.gpkg

└── limitesChile.txt

```



\### Descripción



| Archivo | Descripción |

|----------|-------------|

| `chile-latest-free.gpkg` | Base de datos geográfica de Chile obtenida desde OpenStreetMap. |

| `limitesChile.txt` | Archivo de texto que contiene los límites geográficos de Chile. |



\---



\## Configuración inicial



Debido al tamaño del archivo `.gpkg`, este no pudo ser incluido en el repositorio mediante GitHub.



Por lo tanto, el usuario deberá descargarlo manualmente y copiarlo dentro del directorio:



```text

data/

```



\---



\## Estructura de la base de datos



La base de datos contiene múltiples tablas de interés para el proyecto.



\### `gis\_osm\_roads\_free`



Contiene aproximadamente \*\*1.001.470 segmentos de vía\*\*.



Campos relevantes:



| Campo | Descripción |

|---------|-------------|

| `fid` | Identificador interno |

| `geom` | Geometría tipo LINESTRING |

| `osm\_id` | Identificador original de OpenStreetMap |

| `fclass` | Tipo de vía (`primary`, `secondary`, `residential`, etc.) |

| `name` | Nombre de la calle |

| `ref` | Referencia de carretera |

| `oneway` | Indica si la vía es de sentido único |

| `maxspeed` | Velocidad máxima permitida |



\### `gis\_osm\_places\_free`



Contiene aproximadamente \*\*18.176 lugares y puntos geográficos\*\*.



\### `gis\_osm\_pois\_free`



Contiene puntos de interés (POI) que pueden utilizarse para realizar búsquedas de origen y destino por nombre.



> \*\*Nota:\*\* La información proviene de OpenStreetMap, por lo que no se garantiza una cobertura o precisión absoluta en todas las zonas del país.



\---



\## Estructuras de datos utilizadas



Las principales estructuras de datos implementadas son:



| TDA | Uso |

|------|------|

| Grafo | Representación principal de la red vial |

| Mapa (Hash Table) | Búsqueda rápida de nodos por ID |

| Lista | Almacenamiento dinámico de colecciones |

| Heap | Implementación del algoritmo de Dijkstra |



\---



\## Compilación



Compilar el proyecto con:



```bash

gcc ROUTEMAPPER.c \\

&#x20;   tdas/extra.c \\

&#x20;   tdas/heap.c \\

&#x20;   tdas/list.c \\

&#x20;   tdas/map.c \\

&#x20;   libSqlite3/sqlite3.c \\

&#x20;   -o ROUTEMAPPER \\

&#x20;   -lm

```



\---



\## Flujo de trabajo Git



Comandos recomendados para guardar y subir cambios:



```bash

git add .

git commit -m "Descripción del cambio"

git push

```





