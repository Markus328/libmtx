#ifndef MTX_MATRIX_OP_H
#define MTX_MATRIX_OP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "matrix.h"

// Transforma a matriz _M em matriz identidade. Caso _M não seja uma matriz
// quadrada, essa função atua na primeira e maior submatriz quadrada de _M.
void mtx_matrix_set_identity(mtx_matrix_t *_M);

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

// Salva uma matriz triangular superior em _M com os elementos não nulos sendo
// os respectivos elementos em M.
int mtx_matrix_get_upper(mtx_matrix_t *_M, const mtx_matrix_t *M);

// Salva uma matriz triangular inferior em _M com os elementos não nulos sendo
// os respectivos elementos em M.
int mtx_matrix_get_lower(mtx_matrix_t *_M, const mtx_matrix_t *M);

#ifdef __cplusplus
}
#endif

#endif
