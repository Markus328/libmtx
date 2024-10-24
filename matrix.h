#ifndef MTX_MATRIX_H
#define MTX_MATRIX_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

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

typedef void *(*mtx_mem_allocator_t)(size_t size);

extern mtx_mem_allocator_t __mtx_cfg_mem_allocator;

// Função padrão para alocação de memória.
void *mtx_default_mem_alloc(size_t size);

// Seta a função alocadora de memória.
void mtx_cfg_set_mem_alloc(mtx_mem_allocator_t allocator);

#define mtx_mem_alloc(size) __mtx_cfg_mem_allocator(size)

// Inicializa a matriz _M com dy linhas e dx colunas.
void mtx_matrix_init(mtx_matrix_t *_M, int dy, int dx);

// Inicializa a matriz _M através de stream, auto-detectando as dimensões de
// acoro com o conteúdo de stream.
void mtx_matrix_finit(FILE *stream, mtx_matrix_t *_M);

// Swapa os recursos de M1 e M2, se e somente se M1 e M2 tenham as mesmas
// dimensões e não sejam views.
void mtx_matrix_swap(mtx_matrix_t *M1, mtx_matrix_t *M2);

// Libera a memória alocada da matriz M, tanto os metadados quanto os elementos.
// Apenas é totalmente seguro usar essa função em uma matriz M gerada por
// mtx_matrix_init(), mtx_matrix_init_perm() ou mtx_matrix_finit().
void mtx_matrix_free(mtx_matrix_t *M);

// Preenche a matriz M com elementos do array.
void mtx_matrix_fill_a(mtx_matrix_t *M, double *array);

// Preenche a matriz M com elementos de matrix, lidos da mesma forma que seriam
// lidos de M.
void mtx_matrix_fill_m(mtx_matrix_t *M, double **matrix);

// Cria uma nova matriz _M (se necessário) e copia os elementos de M.
void mtx_matrix_clone(mtx_matrix_t *_M, const mtx_matrix_t *M);

// Copia os elementos da matriz M_FROM para M_TO.
void mtx_matrix_copy(mtx_matrix_t *M_TO, const mtx_matrix_t *M_FROM);

// Retorna uma view (essencialmente uma submatriz) que refere-se a uma parte dos
// elementos de M_OF, de acordo com a posição relativa e as dimensões
// especificadas.
mtx_matrix_view_t mtx_matrix_view_of(const mtx_matrix_t *M_OF, int init_i,
                                     int init_j, int dy, int dx);

// Cria uma matriz que se referencia a todos os elementos de um dado array, de
// acordo com dy e dx. Caso 'arr' aponte para memória não alocada por
// malloc(),calloc() ou realloc(), JAMAIS use mtx_matrix_free() na matriz _M,
// use mtx_matrix_unref().
void mtx_matrix_ref_a(mtx_matrix_t *_M, double *arr, int dy, int dx);

// Retorna o ponteiro C do array dos elementos de uma matriz.
double *mtx_matrix_raw_a(mtx_matrix_t *M);

// Destroi a estrutura da matriz, liberando a memória dos metadados, mas sem
// liberar a memória do array de elementos.
void mtx_matrix_unref(mtx_matrix_t *__M);

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
void mtx_matrix_print(const mtx_matrix_t *M);

// Escreve a matriz M em stream.
void mtx_matrix_fprint(FILE *stream, const mtx_matrix_t *M);

// Lê a matriz M de stream.
void mtx_matrix_fread(FILE *stream, mtx_matrix_t *M);

// Lê uma matriz de stream, auto-detectando as dimensões e retornando o array
// que representa os elementos lidos na ordem row-major. Provavelmente isso será
// usado em conjunto com mtx_matrix_fill_a().
double *mtx_matrix_fread_raw(FILE *stream, int *dy, int *dx);

// Compara as matrizes A e B. Retorna 1 caso todos os elementos de A sejam
// exatamente iguais aos de B, caso contrário, retorna 0.
int mtx_matrix_equals(const mtx_matrix_t *A, const mtx_matrix_t *B);

#ifdef __cplusplus
}
#endif
#endif
