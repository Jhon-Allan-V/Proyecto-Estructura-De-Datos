#include "extra.h"


#define MAX_LINE_LENGTH 4096
#define MAX_FIELDS      128

char **leer_linea_csv(FILE *archivo, char separador) {
    static char linea[MAX_LINE_LENGTH];
    static char *campos[MAX_FIELDS];
    int idx = 0;

    if (fgets(linea, MAX_LINE_LENGTH, archivo) == NULL)
        return NULL;  // fin de fichero

    // quitar salto de línea
    linea[strcspn(linea, "\r\n")] = '\0';

    char *ptr = linea;
    while (*ptr && idx < MAX_FIELDS - 1) {
        char *start;

        if (*ptr == '\"') {
            // campo entrecomillado
            ptr++;              // saltar la comilla inicial
            start = ptr;

            // compactar contenido: convertir "" → " y copiar el resto
            char *dest = ptr;
            while (*ptr) {
                if (*ptr == '\"' && *(ptr + 1) == '\"') {
                    *dest++ = '\"';  // una comilla literal
                    ptr += 2;        // saltar ambas
                }
                else if (*ptr == '\"') {
                    ptr++;           // fin del campo
                    break;
                }
                else {
                    *dest++ = *ptr++;
                }
            }
            *dest = '\0';        // terminar cadena

            // ahora ptr apunta justo después de la comilla de cierre
            if (*ptr == separador) ptr++;
        }
        else {
            // campo sin comillas
            start = ptr;
            while (*ptr && *ptr != separador)
                ptr++;
            if (*ptr == separador) {
                *ptr = '\0';
                ptr++;
            }
        }

        campos[idx++] = start;
    }

    campos[idx] = NULL;
    return campos;
}


List *split_string(const char *str, const char *delim) {
  List *result = list_create();
  char *token = strtok((char *)str, delim);

  while (token != NULL) {
    // Eliminar espacios en blanco al inicio del token
    while (*token == ' ') {
      token++;
    }

    // Eliminar espacios en blanco al final del token
    char *end = token + strlen(token) - 1;
    while (*end == ' ' && end > token) {
      *end = '\0';
      end--;
    }

    // Copiar el token en un nuevo string
    char *new_token = strdup(token);

    // Agregar el nuevo string a la lista
    list_pushBack(result, new_token);

    // Obtener el siguiente token
    token = strtok(NULL, delim);
  }

  return result;
}

// Función para limpiar la pantalla
void limpiarPantalla() { system("clear"); }

void presioneTeclaParaContinuar() {
  puts("Presione una tecla para continuar...");
  getchar(); // Consume el '\n' del buffer de entrada
  getchar(); // Espera a que el usuario presione una tecla
}




// Función recursiva (DFS) para encontrar un camino aleatorio garantizado
int build_safe_path(int x, int y, int safe[N][N], int visited[N][N]) {
    // Verificar límites y si ya visitamos la celda
    if (x < 0 || y < 0 || x >= N || y >= N || visited[x][y]) {
        return 0;
    }

    visited[x][y] = 1;
    safe[x][y] = 1; // Lo marcamos temporalmente como parte del camino

    // Condición de éxito: Llegamos a la meta
    if (x == N - 1 && y == N - 1) {
        return 1;
    }

    // Direcciones: Derecha, Abajo, Izquierda, Arriba
    int dirs[4][2] = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}};

    // Mezclar las direcciones aleatoriamente para que el camino no sea predecible
    for (int i = 0; i < 4; i++) {
        int r = rand() % 4;
        int tempX = dirs[i][0];
        int tempY = dirs[i][1];
        dirs[i][0] = dirs[r][0];
        dirs[i][1] = dirs[r][1];
        dirs[r][0] = tempX;
        dirs[r][1] = tempY;
    }

    // Explorar los vecinos en el orden aleatorio
    for (int i = 0; i < 4; i++) {
        int nx = x + dirs[i][0];
        int ny = y + dirs[i][1];
        if (build_safe_path(nx, ny, safe, visited)) {
            return 1; // Si este camino llega a la meta, detenemos la búsqueda
        }
    }

    // Backtracking: Si llegamos a un callejón sin salida, desmarcamos el camino
    safe[x][y] = 0;
    return 0;
}

// Función principal para generar el laberinto
void generate_maze(int maze[N][N], int difficulty) {
    int safe[N][N] = {0};
    int visited[N][N] = {0};

    // 1. Trazar el camino seguro desde el inicio (0,0) a la meta (N-1, N-1)
    build_safe_path(0, 0, safe, visited);

    // 2. Rellenar el resto de la matriz basándonos en la dificultad
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (safe[i][j] == 1) {
                // Si es parte del camino seguro, obligatoriamente es espacio libre
                maze[i][j] = 0; 
            } else {
                // Si no es el camino seguro, tiramos un dado de 0 a 99
                // 'difficulty' actúa como el porcentaje de aparición de obstáculos
                int r = rand() % 100;
                if (r < difficulty) {
                    maze[i][j] = 1; // Pared / Obstáculo
                } else {
                    maze[i][j] = 0; // Espacio libre
                }
            }
        }
    }
}

