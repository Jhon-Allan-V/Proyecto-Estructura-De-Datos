#include "extra.h"
#include <stdint.h>
#include <string.h>



static uint32_t read_u32(const unsigned char *p, int little) {
    if (little)
        return (uint32_t)p[0] | ((uint32_t)p[1]<<8) | ((uint32_t)p[2]<<16) | ((uint32_t)p[3]<<24);
    return ((uint32_t)p[0]<<24) | ((uint32_t)p[1]<<16) | ((uint32_t)p[2]<<8) | (uint32_t)p[3];
}

static double read_double(const unsigned char *p, int little) {
    uint64_t v;
    if (little) {
        v = (uint64_t)p[0] | ((uint64_t)p[1]<<8) | ((uint64_t)p[2]<<16) | ((uint64_t)p[3]<<24)
          | ((uint64_t)p[4]<<32) | ((uint64_t)p[5]<<40) | ((uint64_t)p[6]<<48) | ((uint64_t)p[7]<<56);
    } else {
        v = ((uint64_t)p[0]<<56) | ((uint64_t)p[1]<<48) | ((uint64_t)p[2]<<40) | ((uint64_t)p[3]<<32)
          | ((uint64_t)p[4]<<24) | ((uint64_t)p[5]<<16) | ((uint64_t)p[6]<<8) | (uint64_t)p[7];
    }
    double d;
    memcpy(&d, &v, sizeof(d));
    return d;
}

static int gpkg_envelope_size(int env_flag) {
    switch (env_flag) {
        case 0: return 0;
        case 1: return 32;
        case 2: return 48;
        case 3: return 48;
        case 4: return 64;
        default: return 0;
    }
}

int parse_gpkg_linestring(const unsigned char *blob, int blob_size,
                                 double *lon1, double *lat1,
                                 double *lon2, double *lat2)
{
    if (blob_size < 40 || blob[0] != 'G' || blob[1] != 'P') return 0;

    int flags = blob[3];
    int env_flag = (flags >> 1) & 0x07;
    int env_size = gpkg_envelope_size(env_flag);
    int header_size = 8 + env_size;
    if (blob_size < header_size + 9) return 0;

    const unsigned char *wkb = blob + header_size;
    int wkb_little = wkb[0] == 1;
    uint32_t geom_type = read_u32(wkb + 1, wkb_little);
    if (geom_type != 2) return 0; // LINESTRING

    uint32_t num_points = read_u32(wkb + 5, wkb_little);
    if (num_points < 2) return 0;

    const unsigned char *p0 = wkb + 9;
    const unsigned char *pN = wkb + 9 + (num_points - 1) * 16;

    *lon1 = read_double(p0, wkb_little);
    *lat1 = read_double(p0 + 8, wkb_little);
    *lon2 = read_double(pN, wkb_little);
    *lat2 = read_double(pN + 8, wkb_little);
    return 1;
}


// Función para limpiar la pantalla en Windows
void limpiarPantalla() { system("cls"); }

void presioneTeclaParaContinuar() {
  puts("Presione una tecla para continuar...");
  getchar(); // Consume el '\n' del buffer de entrada
  getchar(); // Espera a que el usuario presione una tecla
}


