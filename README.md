"ROUTEMAPPER utiliza una representación basada en datos reales obtenidos de OpenStreetMap para modelar la red vial de todo Chile, sobre la cual se ejecutan algoritmos de búsqueda y optimización de rutas."
LINK: https://download.geofabrik.de/south-america/chile.html?utm_source=chatgpt.com

Se hara uso de SQLite3 para poder consultar la informacion que se encuentra en el directorio (data/). Para llevar esto a cabo, primero se debera de descargar la libreria (para Windows) en su pagina oficial:
LINK SQLite3: https://sqlite.org/download.html (sqlite-amalgamation-3530200.zip(2.81 MiB)).
(En este proyecto solamente se hara uso de sqlite3.c y sqlite3.h, los cuales se encuentran dentro del directorio (libSqlite3/))

 
ARCHIVOS (data/) que el programa necesitara para procesar: 
        chile-latest-free.gpkg      -> base de datos con respecto a la geografia de Chile
        limitesChile.txt            -> archivo txt el cual marca los limites de Chile

El usuario tendra que descargar y agregar la base de datos manualmente en el directorio (data/chile-260615-free.gpkg/). Esto porque este archivo no se logro subir al github via comandos git debido que sobre pasaba el limite de peso.

(recordatorio)
Tu flujo normal desde ahora será:

git add .
git commit -m "Descripcion del cambio"
git push

comando para compilar:
gcc ROUTEMAPPER.c tdas/extra.c tdas/heap.c tdas/list.c tdas/map.c libSqlite3/sqlite3.c -o ROUTEMAPPER -lm

tdas a utilizar: grafo(principal), mapa(tabla hash, busqueda por id), lista, heap (algoritmo dijkstra)