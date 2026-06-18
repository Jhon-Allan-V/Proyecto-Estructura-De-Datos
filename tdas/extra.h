#ifndef EXTRA_H
#define EXTRA_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>



static uint32_t read_u32(const unsigned char *p, int little);

static double read_double(const unsigned char *p, int little);

static int gpkg_envelope_size(int env_flag);

int parse_gpkg_linestring(const unsigned char *blob, int blob_size,
                                 double *lon1, double *lat1,
                                 double *lon2, double *lat2);



// Función para limpiar la pantalla
void limpiarPantalla();

void presioneTeclaParaContinuar();



#endif