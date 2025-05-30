/*
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */
#include "cachelab.h"

#include <stdio.h>

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/*
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded.
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, k, l, tmp, idx;
    int a0, a1, a2, a3, a4, a5, a6, a7;

    if (M == 32 && N == 32)
    {
        for (i = 0; i < 32; i += 8)
            for (j = 0; j < 32; j += 8)
                for (k = i; k < i + 8; ++k)
                {
                    for (l = j; l < j + 8; ++l)
                    {
                        if (k != l)
                            B[l][k] = A[k][l];
                        else
                        {
                            tmp = A[k][l];
                            idx = k;
                        }
                    }
                    if (i == j)
                        B[idx][idx] = tmp;
                }
    }
    else if (M == 64 && N == 64)
    {
        for (i = 0; i < 64; i += 8)
            for (j = 0; j < 64; j += 8)
            {
                /*  copy upper 4 rows, stash lower half */
                for (k = 0; k < 4; ++k)
                {
                    a0 = A[i + k][j + 0];
                    a1 = A[i + k][j + 1];
                    a2 = A[i + k][j + 2];
                    a3 = A[i + k][j + 3];
                    a4 = A[i + k][j + 4];
                    a5 = A[i + k][j + 5];
                    a6 = A[i + k][j + 6];
                    a7 = A[i + k][j + 7];

                    B[j + 0][i + k] = a0;
                    B[j + 1][i + k] = a1;
                    B[j + 2][i + k] = a2;
                    B[j + 3][i + k] = a3;

                    /* save bottom‐half words in B’s spare space */
                    B[j + 0][i + k + 4] = a4;
                    B[j + 1][i + k + 4] = a5;
                    B[j + 2][i + k + 4] = a6;
                    B[j + 3][i + k + 4] = a7;
                }

                /*  swap the 4×4 blocks across the diagonal */
                for (k = 0; k < 4; ++k)
                {
                    a0 = A[i + 4][j + k];
                    a1 = A[i + 5][j + k];
                    a2 = A[i + 6][j + k];
                    a3 = A[i + 7][j + k];

                    a4 = B[j + k][i + 4];
                    a5 = B[j + k][i + 5];
                    a6 = B[j + k][i + 6];
                    a7 = B[j + k][i + 7];

                    B[j + k][i + 4] = a0;
                    B[j + k][i + 5] = a1;
                    B[j + k][i + 6] = a2;
                    B[j + k][i + 7] = a3;

                    B[j + 4 + k][i + 0] = a4;
                    B[j + 4 + k][i + 1] = a5;
                    B[j + 4 + k][i + 2] = a6;
                    B[j + 4 + k][i + 3] = a7;
                }

                /*  copy bottom-right 4×4 */
                for (k = 4; k < 8; ++k)
                    for (l = 4; l < 8; ++l)
                        B[j + l][i + k] = A[i + k][j + l];
            }
    }
    else
    {
        for (i = 0; i < N; i += 16)
            for (j = 0; j < M; j += 16)
                for (k = i; k < i + 16 && k < N; ++k)
                {
                    for (l = j; l < j + 16 && l < M; ++l)
                    {
                        if (k != l)
                            B[l][k] = A[k][l];
                        else
                        {
                            tmp = A[k][l];
                            idx = k;
                        }
                    }
                    if (i == j)
                        B[idx][idx] = tmp;
                }
    }
}

/*
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started.
 */

/*
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++)
    {
        for (j = 0; j < M; j++)
        {
            tmp     = A[i][j];
            B[j][i] = tmp;
        }
    }
}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc);

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc);
}

/*
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++)
    {
        for (j = 0; j < M; ++j)
        {
            if (A[i][j] != B[j][i])
            {
                return 0;
            }
        }
    }
    return 1;
}
