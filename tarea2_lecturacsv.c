#include "tdas/extra.h"
#include "tdas/list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Carga canciones desde un archivo CSV
 */
void leer_canciones() {
  // Intenta abrir el archivo CSV que contiene datos de películas
  FILE *archivo = fopen("data/song_dataset_.csv", "r");
  if (archivo == NULL) {
    perror(
        "Error al abrir el archivo"); // Informa si el archivo no puede abrirse
    return;
  }

  char **campos;
  // Leer y parsear una línea del archivo CSV. La función devuelve un array de
  // strings, donde cada elemento representa un campo de la línea CSV procesada.
  campos = leer_linea_csv(archivo, ','); // Lee los encabezados del CSV

  int k=0;
  // Lee cada línea del archivo CSV hasta el final
  while ((campos = leer_linea_csv(archivo, ',')) != NULL) {
    k++;
    if (k>10) break; //carga primeras 1000 canciones
    printf("ID: %d\n", atoi(campos[0]));
    printf("Título cancion: %s\n", campos[4]);

    List* artistas = split_string(campos[2], ";");

    printf("Artistas: \n");
    for(char *artista = list_first(artistas); artista != NULL; 
                    artista = list_next(artistas))
        printf("  %s\n", artista);

    printf("Album: %s\n", campos[3]);
    printf("Género: %s\n", campos[20]);
    printf("Tempo: %.2f\n", atof(campos[18]));
    printf(" -------------------------------\n");
    
  }
  fclose(archivo); // Cierra el archivo después de leer todas las líneas

}



int main() {
    leer_canciones();

  return 0;
}
