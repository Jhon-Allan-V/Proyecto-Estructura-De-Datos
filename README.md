"ROUTEMAPPER utiliza una representación basada en datos reales obtenidos de OpenStreetMap para modelar la red vial de todo Chile, sobre la cual se ejecutan algoritmos de búsqueda y optimización de rutas."
LINK: https://download.geofabrik.de/south-america/chile.html?utm_source=chatgpt.com

Se hara uso de SQLite3 para poder consultar la informacion que se encuentra en el directorio (data). Para llevar esto a cabo, primero se debera de descargar la libreria (para Windows) en su pagina oficial:
LINK: https://sqlite.org/download.html
 
ARCHIVOS: chile-latest-free.gpkg.zip y chile.poly



(recordatorio)
Tu flujo normal desde ahora será:

git add .
git commit -m "Descripcion del cambio"
git push

comando para compilar:
gcc ROUTEMAPPER.c tdas/extra.c tdas/heap.c tdas/list.c tdas/map.c libSqlite3/sqlite3.c -o ROUTEMAPPER -lm
