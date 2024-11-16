#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <cmath>
using namespace std;

struct Matrix {
    size_t rows;
    size_t cols;
    double* data;
};

typedef struct Matrix Matrix;


enum MatrixExceptionLevel { ERROR, WARNING, INFO, DEBUG };


void matrix_exception(const enum MatrixExceptionLevel level, const char* msg)
{
    if (level == 0) {
        printf("ERROR: %s", msg);
    }

    if (level == 1) {
        printf("WARNING: %s", msg);
    }
}


Matrix matrix_allocate(const size_t rows, const size_t cols)
{
    Matrix A = { 0, 0, NULL };

    if (rows == 0 || cols == 0) {
        return { rows, cols, NULL };
    }

    A.data = (double*)malloc(rows * cols * sizeof(double));  // TODO: check for overflow
    if (A.data == NULL) {
        matrix_exception(ERROR, "Allocation memory fail.");
        return A;
    }

    A.rows = rows;
    A.cols = cols;
    return A;
}


void matrix_free(Matrix* A)
{
    free(A->data);  // free( (*A).data  )
    *A = { 0, 0, NULL };
}



// no checks at all
void matrix_set(const Matrix A, const double* values)
{
    memcpy(A.data, values, A.rows * A.cols * sizeof(double));
}


void matrix_print(const Matrix A)
{
    for (size_t row = 0; row < A.rows; ++row) {
        for (size_t col = 0; col < A.cols; ++col) {
            cout << A.data[row * A.cols + col] << " ";
        }
        printf("\n");
    }
    printf("\n");
}



// A += B
int matrix_add(const Matrix A, const Matrix B, const Matrix C)
{
    if (!((A.cols == B.cols) && (A.rows == B.rows))) {
        matrix_exception(ERROR, "matrix_add: incompatible sizes.\n \n");
        return (0);
    }

    for (size_t idx = 0; idx < A.rows * A.cols; ++idx) {
        C.data[idx] = A.data[idx] + B.data[idx];
    }
    printf("A + B =\n");
    matrix_print(C);
    return 1;
}



int matrix_subtraction(const Matrix A, const Matrix B, const Matrix C)
{
    if (!((A.cols == B.cols) && (A.rows == B.rows))) {
        matrix_exception(ERROR, "matrix_subtraction: incompatible sizes.\n \n");
        return (0);
    }


    for (size_t idx = 0; idx < A.rows * A.cols; ++idx) {
        C.data[idx] = A.data[idx] - B.data[idx];
    }
    printf("A - B =\n");
    matrix_print(C);
    return 1;
}


int matrix_multiplication(const Matrix A, const Matrix B, Matrix D) //TODO: CHECK
{
    if (!(A.cols == B.rows)) {
        matrix_exception(ERROR, "matrix_multiplication: A.cols must be equal B.rows\n");
        exit(0);
    }

    else {
        for (size_t row = 0; row < D.rows; ++row) {
            for (size_t col = 0; col < D.cols; ++col) {
                D.data[row * D.cols + col] = 0;
                for (size_t k = 0; k < A.cols; ++k) {
                    D.data[row * D.cols + col] += A.data[row * A.cols + k] * B.data[k * B.cols + col];
                }
            }
        }
    }
    return 1;
}

int matrix_transposition(const Matrix A, const Matrix T)
{
    for (int row = 0; row < A.cols; ++row) {
        for (int col = 0; col < A.rows; ++col) {
            T.data[col * T.cols + row] = A.data[row * T.rows + col];
        }
    }
    printf("Transposed matrix A = \n");
    matrix_print(T);
    return (1);
}

// Function to calculate the determinant of a matrix
double matrix_determinant(const Matrix A) {
    if (A.rows != A.cols) {
        matrix_exception(ERROR, "matrix_determinant: Matrix must be square\n");
        return 0;
    }

    if (A.rows == 1) {
        return A.data[0];
    }

    if (A.rows == 2) {
        return A.data[0] * A.data[3] - A.data[1] * A.data[2];
    }

    double det = 0;
    for (size_t col = 0; col < A.cols; ++col) {
        Matrix submatrix = matrix_allocate(A.rows - 1, A.cols - 1);
        size_t sub_row = 0;
        for (size_t row = 1; row < A.rows; ++row) {
            size_t sub_col = 0;
            for (size_t k = 0; k < A.cols; ++k) {
                if (k != col) {
                    submatrix.data[sub_row * submatrix.cols + sub_col] = A.data[row * A.cols + k];
                    sub_col++;
                }
            }
            sub_row++;
        }
        det += (col % 2 == 0 ? 1 : -1) * A.data[0 * A.cols + col] * matrix_determinant(submatrix);
        matrix_free(&submatrix);
    }
    return det;
}


// Function to calculate the inverse of a matrix
int matrix_inverse(const Matrix A, Matrix inverse) {
    if (A.rows != A.cols) {
        matrix_exception(ERROR, "matrix_inverse: Matrix must be square\n");
        return 0;
    }

    double det = matrix_determinant(A);
    if (det == 0) {
        matrix_exception(ERROR, "matrix_inverse: Matrix is singular (determinant is 0)\n");
        return 0;
    }

    Matrix adjugate = matrix_allocate(A.rows, A.cols);

    for (size_t row = 0; row < A.rows; ++row) {
        for (size_t col = 0; col < A.cols; ++col) {
            Matrix submatrix = matrix_allocate(A.rows - 1, A.cols - 1);
            size_t sub_row = 0;
            for (size_t i = 0; i < A.rows; ++i) {
                if (i == row) continue;
                size_t sub_col = 0;
                for (size_t j = 0; j < A.cols; ++j) {
                    if (j == col) continue;
                    submatrix.data[sub_row * submatrix.cols + sub_col] = A.data[i * A.cols + j];
                    sub_col++;
                }
                sub_row++;
            }
            adjugate.data[col * adjugate.cols + row] = ((row + col) % 2 == 0 ? 1 : -1) * matrix_determinant(submatrix);
            matrix_free(&submatrix);
        }
    }

    for (size_t i = 0; i < inverse.rows * inverse.cols; ++i) {
        inverse.data[i] = adjugate.data[i] / det;
    }

    matrix_free(&adjugate);
    return 1;
}


// Function to calculate the matrix exponential using Taylor series approximation
int matrix_exponential(const Matrix A, Matrix expA, int numTerms = 10) {
    if (A.rows != A.cols) {
        matrix_exception(ERROR, "matrix_exponential: Matrix must be square\n");
        return 0;
    }

    // Initialize expA to the identity matrix
    for (size_t i = 0; i < expA.rows; ++i) {
        for (size_t j = 0; j < expA.cols; ++j) {
            expA.data[i * expA.cols + j] = (i == j) ? 1.0 : 0.0;
        }
    }

    Matrix term = matrix_allocate(A.rows, A.cols);
    Matrix temp = matrix_allocate(A.rows, A.cols);

    for (int k = 1; k <= numTerms; ++k) {
        // Calculate the k-th term of the Taylor series
        if (k == 1) {
            matrix_set(term, A.data);
        }
        else {
            matrix_multiplication(term, A, temp);
            memcpy(term.data, temp.data, term.rows * term.cols * sizeof(double));
        }

        double factorial = 1.0;
        for (int i = 1; i <= k; ++i) {
            factorial *= i;
        }

        for (size_t i = 0; i < term.rows * term.cols; ++i) {
            term.data[i] /= factorial;
            expA.data[i] += term.data[i];
        }
    }

    matrix_free(&term);
    matrix_free(&temp);
    return 1;
}


int main()
{
    Matrix A, B, C, D, T, inverse, expA;
    A = matrix_allocate(3, 3);  // TODO: check for allocated
    B = matrix_allocate(3, 2);  // TODO: check for allocated
    double valuesA[] = {
      5., 2., 3.,
      2., -2., 5.,
       3., 4., 2.
    };
    double valuesB[] = {
        2., 3., 3.,
        -3., 2., 1.,
        -2., 5., 1.
    };
    matrix_set(A, valuesA);
    matrix_set(B, valuesB);
    matrix_print(A);
    matrix_print(B);

    C = matrix_allocate(A.rows, A.cols);
    matrix_add(A, B, C);
    matrix_free(&C);

    C = matrix_allocate(A.rows, A.cols);
    matrix_subtraction(A, B, C);
    matrix_free(&C);

    D = matrix_allocate(A.rows, B.cols);
    matrix_multiplication(A, B, D);
    printf("A * B =\n");
    matrix_print(D);

    T = matrix_allocate(A.cols, A.rows);
    matrix_transposition(A, T);


    printf("Determinant of A: %f\n\n", matrix_determinant(A));

    inverse = matrix_allocate(A.rows, A.cols);
    if (matrix_inverse(A, inverse)) {
        printf("Inverse of A:\n");
        matrix_print(inverse);
    }

    expA = matrix_allocate(A.rows, A.cols);
    if (matrix_exponential(A, expA)) {
        printf("Matrix exponential of A:\n");
        matrix_print(expA);
    }


    matrix_free(&A);
    matrix_free(&B);
    matrix_free(&C);
    matrix_free(&D);
    matrix_free(&T);
    matrix_free(&inverse);
    matrix_free(&expA);

    return 0;
}
