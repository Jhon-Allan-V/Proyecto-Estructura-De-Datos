#ifndef EXTRA_H
#define EXTRA_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>


int parse_gpkg_linestring(const unsigned char *blob, int blob_size,
                                 double *lon1, double *lat1,
                                 double *lon2, double *lat2);



// Función para limpiar la pantalla
void limpiarPantalla();

void presioneTeclaParaContinuar();



#endif