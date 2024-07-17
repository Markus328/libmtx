#ifndef MTX_LINALG_H
#define MTX_LINALG_H

#include "matrix.h"

#ifdef __cplusplus
extern "C" {
#endif

// Decomposição LU, use as macros mtx_linalg_LU_decomp para decompsição geral e
// mtx_linalg_LU_decomp_perf para decomposição mais rápida na resolução de
// sistemas lineares.
int mtx_linalg_LU_decomposition(mtx_matrix_perm_t *__M_PERM,
                                mtx_matrix_t *_M_LU, const mtx_matrix_t *M,
                                int perfect);

// Transforma uma matriz mxn na forma reduzida. O algoritmo vai tentar reduzir a
// matriz 100%, permitindo linhas zeradas e movendo-as para o fim da matriz.
// Retorna o signum >= 0 se sucesso e negativo caso haja falha.
#define mtx_linalg_LU_decomp(__M_PERM, _M, M)                                  \
  mtx_linalg_LU_decomposition((__M_PERM), (_M), (M), 0)

// Exatamente como mtx_linalg_LU_decomp() porém mais rápida caso seja resolução
// de sistemas lineares (se falhar, a submatriz mxm de uma matriz mxn tem
// determinante zero). Na prática, falha ao encontrar linhas zeradas e sempre
// falhará se n < m. Retorna o signum >= se sucesso e negativo caso haja
// falha.
#define mtx_linalg_LU_decomp_perf(__M_PERM, _M, M)                             \
  mtx_linalg_LU_decomposition((__M_PERM), (_M), (M), 1)

// Permuta a matriz M de acordo com a matriz de permutação M_PERM. O
// resultado final é o mesmo de M_PERM x M, porém é mais otimizado para tal.
int mtx_linalg_permutate(mtx_matrix_t *_M, const mtx_matrix_t *M,
                         const mtx_matrix_perm_t *M_PERM);

// Calcula o determinante de uma dada matriz na forma decomposta. A
// matriz não necessariamente precisa ser quadrada: se a matriz for mxn, a
// submatriz mxm será considerada. Caso a n < m ou a matriz não estiver
// decomposta: undefined behavior. Recomendado usar somente combinado com
// mtx_linalg_LU_decomp_perf().
double mtx_linalg_det_LU(const mtx_matrix_t *M_LU, int signum);

// Calcula o determinante de uma dada matriz quadrada M. Retorna zero se a
// matriz não for quadrada (para mxn e n > m, use mtx_linalg_det_LU()).
double mtx_linalg_det(const mtx_matrix_t *M);

// Realiza a back substitution (debaixo pra cima) do sistema Ux = B. A matriz
// U é uma matriz quadrada upper triangular.
//
// Caso a matriz tenha a diagnonal principal como sendo de apenas 1's, passe
// jordan = 1 para ignorar a diagnonal, obtendo o mesmo resultado.
//
// O resultado (x) é retornada na matriz X.
int mtx_linalg_back_subs(mtx_matrix_t *_X, const mtx_matrix_t *U,
                         const mtx_matrix_t *B, int jordan);

// Realiza a forward substitution (de cima para baixo) do sistema Lx = B. A
// matriz L é uma lower triangular.
//
// Caso a matriz tenha a diagnonal principal como sendo de apenas 1's, passe
// jordan = 1 para ignorar a diagnonal, obtendo o mesmo resultado.
//
// O resultado (x) é retornada na matriz X.
int mtx_linalg_forward_subs(mtx_matrix_t *_X, const mtx_matrix_t *L,
                            const mtx_matrix_t *B, int jordan);

// Resolve o sistema linear Ax = B, representado pela matriz aumentada A_LU. A
// matriz A_LU tem que estar previamente decomposta. Como as icógnitas podem
// representar vetores (nesse caso, o resultado X é uma matriz e não um
// vetor), B será separado de A nas ultimas colunas de A_LU de acordo com
// X->dx.
//
// Falha caso o sistema seja impossível, indeterminado ou caso não seja
// possível separar A de B (X-dx > A_LU->dx - 1).
int mtx_linalg_LU_AB_solve(mtx_matrix_t *X, const mtx_matrix_t *AB_LU);

// Resolve o sistema linear Ax = B, representado pela matriz decomposta de A
// em A_LU e pela matriz B. Tanto A_LU quanto B precisam ter o mesmo número de
// linhas.
//
// Falha caso o sistema seja indeterminado.
int mtx_linalg_LU_solve(mtx_matrix_t *_X, const mtx_matrix_perm_t *M_PERM,
                        const mtx_matrix_t *A_LU, const mtx_matrix_t *B);

// Refina a solução do sistema linear Ax = B. Recebe _M_WORK como matriz para
// cálculos intermediários, X a solução atual, M_PERM e A_LU como sendo a matriz
// de permutação e A decomposto e por fim a matriz A e B originais. No fim da
// execução, X será substituído por uma versão mais próxima da solução exata.
//
// Dado que a resolução de um sistema linear que gerou resíduos: Ax' = B + B', a
// função retorna a distancia entre B e B', indicando o grau de distancia que a
// solução exata x tem de x'.
double mtx_linalg_LU_refine(mtx_matrix_t *_M_WORK, mtx_matrix_t *X,
                            const mtx_matrix_perm_t *M_PERM,
                            const mtx_matrix_t *A_LU, const mtx_matrix_t *A,
                            const mtx_matrix_t *B);

#ifdef __cplusplus
}
#endif
#endif
