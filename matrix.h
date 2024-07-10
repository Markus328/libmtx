#ifndef MTX_MATRIX_H
#define MTX_MATRIX_H

#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct mtx_matrix_data {
  double **m;
  size_t size1;
  size_t size2;
} mtx_matrix_data_t;

typedef struct mtx_matrix {
  mtx_matrix_data_t *data;
  int dx, dy;
  int offX, offY;
} mtx_matrix_t;

typedef struct mtx_matrix_view {
  mtx_matrix_t matrix;
} mtx_matrix_view_t;

typedef mtx_matrix_t mtx_matrix_perm_t;

#define _mod(x) ((x) < 0 ? -(x) : (x))

#define mtx_matrix_at(M, i, j) (M)->data->m[(M)->offY + i][(M)->offX + j]

#define mtx_matrix_row(M, i) &(M)->data->m[(M)->offY + i][(M)->offX]

#define MTX_MATRIX_IS_SQUARE(M) ((M)->dx == (M)->dy)

#define MTX_MATRIX_MAX_ROWS 1024
#define MTX_MATRIX_MAX_COLUMNS 1024

#define MTX_MATRIX_SAME_DIMENSIONS(M1, M2)                                     \
  ((M1)->dx == (M2)->dx && (M1)->dy == (M2)->dy)

#define MTX_MATRIX_ARE_SHARED(M1, M2) ((M1)->data->m == (M2)->data->m)

#define MTX_MATRIX_ARE_SAME(M1, M2)                                            \
  (MTX_MATRIX_ARE_SHARED(M1, M2) && MTX_MATRIX_SAME_DIMENSIONS(M1, M2) &&      \
   (M1)->offY == (M2)->offY && (M1)->offX == (M2)->offX)

#define MTX_MATRIX_IS_VIEW(M)                                                  \
  ((M)->offX > 0 || (M)->offY > 0 || (M)->dy != (M)->data->size1 ||            \
   (M)->dx != (M)->data->size2)

#define MTX_MATRIX_OVERLAP_OF(M, M_OF)                                         \
  ((M)->offY + (M)->dy - 1 >= (M_OF)->offY &&                                  \
   (M)->offX + (M)->dx - 1 >= (M_OF)->offX)
#define MTX_MATRIX_OVERLAP(M1, M2)                                             \
  (MTX_MATRIX_ARE_SHARED(M1, M2) &&                                            \
   (MTX_MATRIX_OVERLAP_OF(M1, M2) && MTX_MATRIX_OVERLAP_OF(M2, M1)))

#define MTX_MATRIX_OVERLAP_AFTER(M1, M2)                                       \
  (MTX_MATRIX_OVERLAP(M1, M2) &&                                               \
   ((M2)->offY > (M1)->offY ||                                                 \
    ((M2)->offX > (M1)->offX && (M2)->offY == (M1)->offY)))

// void matrix_init0(matrix_t *M, unsigned int dy, unsigned int dx) {
//   _m_init_dy(M, dy);
//   M->dx = dx;

//   for (int i = 0; i < dy; ++i) {
//     M->m[i] = (double *)calloc(dx, sizeof(double));
//   }
// }

// Inicializa a matriz _M com dy linhas e dx colunas.
void mtx_matrix_init(mtx_matrix_t *_M, int dy, int dx);

// Inicializa a matriz _M_PERM como uma matriz de permutação de dimensões dxd.
void mtx_matrix_init_perm(mtx_matrix_perm_t *_M_PERM, int d);

// Libera a memória alocada da matriz M.
void mtx_matrix_free(mtx_matrix_t *M);

// Transformaa a matriz _M em matriz identidade. Caso _M->dy > _M->dx, apenas a
// submatriz dyxdy se transforma em identidade e o restante será zerado.
void mtx_matrix_set_identity(mtx_matrix_t *_M);

// Preenche a matriz M com elementos do array.
void mtx_matrix_fill_a(mtx_matrix_t *M, double *array);

// Preenche a matriz M com elementos de matrix, lidos da mesma forma que seriam
// lidos de M.
void mtx_matrix_fill_m(mtx_matrix_t *M, double **matrix);

// Cria uma nova matriz _M (se necessário) e copia os elementos de M.
void mtx_matrix_clone(mtx_matrix_t *_M, const mtx_matrix_t *M);

// Copia os elementos da matriz M_FROM para M_TO.
int mtx_matrix_copy(mtx_matrix_t *M_TO, const mtx_matrix_t *M_FROM);

// Retorna uma view (essencialmente uma submatriz) que refere-se a uma parte dos
// elementos de M_OF, de acordo com a posição relativa e as dimensões
// especificadas.
mtx_matrix_view_t mtx_matrix_view_of(const mtx_matrix_t *M_OF, int init_i,
                                     int init_j, int dy, int dx);

// Retorna uma view da coluna j de M_OF.
#define mtx_matrix_column_of(M_OF, j)                                          \
  mtx_matrix_view_of(M_OF, 0, j, (M_OF)->dy, 1)

// Retorna uma view da linha i de M_OF.
#define mtx_matrix_row_of(M_OF, i) mtx_matrix_view_of(M_OF, i, 0, 1, (M_OF)->dx)

// Copia uma parte dos elementos de M_FROM para M_TO de acordo com a posição
// relativa a M_FROM.
int mtx_matrix_copy_from(mtx_matrix_t *M_TO, const mtx_matrix_t *M_FROM,
                         int init_i, int init_j);

// Escreve a matriz M para o stdout com o prefixo indicando as dimensões dela.
void print_matrix(const mtx_matrix_t *M);

// Escreve a matriz M em stream.
void mtx_matrix_fprintf(FILE *stream, const mtx_matrix_t *M);

// Lê a matriz M de stream.
void mtx_matrix_fread(FILE *stream, mtx_matrix_t *M);

// Realiza a multiplicação AxB e salva o resultado em _C.
int mtx_matrix_mul(mtx_matrix_t *_C, const mtx_matrix_t *A,
                   const mtx_matrix_t *B);

// Multiplica todos os elementos de M com scalar e salva na matriz _M.
int mtx_matrix_s_mul(mtx_matrix_t *_M, const mtx_matrix_t *M, double scalar);

// Realiza a soma A+B e salva o resultado em _C.
int mtx_matrix_add(mtx_matrix_t *_C, const mtx_matrix_t *A,
                   const mtx_matrix_t *B);

// Realiza a subtração A-B e salva o resultado em _C.
int mtx_matrix_sub(mtx_matrix_t *_C, const mtx_matrix_t *A,
                   const mtx_matrix_t *B);

// Multiplica cada elemento de A pelo respectivo elemento de B e salva em _C.
int mtx_matrix_mul_elements(mtx_matrix_t *_C, const mtx_matrix_t *A,
                            const mtx_matrix_t *B);

// Divide cada elemento de A pelo respectivo elemento de B e salva em _C.
int mtx_matrix_div_elements(mtx_matrix_t *_C, const mtx_matrix_t *A,
                            const mtx_matrix_t *B);

// Salva a matriz transposta de M em _M.
int mtx_matrix_transpose(mtx_matrix_t *_M, const mtx_matrix_t *M);

// Compara as matrizes A e B. Retorna 1 caso todos os elementos de A sejam
// exatamente iguais aos de B, caso contrário, retorna 0.
int mtx_matrix_equals(const mtx_matrix_t *A, const mtx_matrix_t *B);

// Calcula e retorna o grau de diferença entre as matrizes A e B. Retorna
// sempre um número positivo, exceto na falha na qual o número retornado é
// negativo.
double mtx_matrix_distance(const mtx_matrix_t *A, const mtx_matrix_t *B);

// Subtrai a matriz A da matriz B com o resultado em _M_D e retorna o
// somátorio dos módulos de cada elemento de _M_D, que é sempre um número
// positivo. O valor retornado é exatamente a distancia entre as duas
// matrizes, sendo equivalente ao retornado por mtx_matrix_distance().
//
// Retorna zero se as matrizes forem idênticas e negativo caso ocorra erro. Em
// ambos os casos, _M_DU pode não conter a subtração de A e B, portanto, deve
// ser ignorado.
double mtx_matrix_distance_each(mtx_matrix_t *_M_D, const mtx_matrix_t *A,
                                const mtx_matrix_t *B);

#endif
