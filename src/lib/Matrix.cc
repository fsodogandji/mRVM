#include "Matrix.h"
#include <iostream>
#include <gsl/gsl_blas.h>

using namespace std;

namespace jason {

Matrix::Matrix(gsl_matrix *data) {
	this->m = data;
}

Matrix::Matrix(double *data, size_t height, size_t width) {
	this->m = gsl_matrix_alloc(height, width);
	for (size_t row = 0; row < height; ++row) {
		for (size_t col = 0; col < width; ++col) {
			gsl_matrix_set(m, row, col, *(data + row*width + col));
		}
	}
}

Matrix::Matrix(const char* filename) {
	cout << "read " << filename << "\n";
	int rows, cols;
	FILE *f;
	f = fopen(filename, "r");
	rows = NumberOfRows(f);
	cols = NumberOfColumns(f);
	this->m = gsl_matrix_alloc(rows, cols);
	gsl_matrix_fscanf(f, this->m);
	fclose(f);
}

Matrix::~Matrix() {
	gsl_matrix_free(this->m);
}

Matrix *Matrix::Clone() {
	return new Matrix(this->Data(), this->Width(), this->Height());
}

Matrix *Matrix::Invert() {
	int n = this->Width();
	gsl_matrix *copy = this->CloneGSLMatrix();
	gsl_matrix *inverse = gsl_matrix_alloc(n, n);
	gsl_permutation *perm = gsl_permutation_alloc(n);
	int s = 0;
	gsl_linalg_LU_decomp(copy, perm, &s);
	gsl_linalg_LU_invert(copy, perm, inverse);
	gsl_permutation_free(perm);
	gsl_matrix_free(copy);
	return new Matrix(inverse);
}

double Matrix::Get(int row, int col) {
	return gsl_matrix_get(this->m, row, col);
}

void Matrix::Set(int row, int col, double val) {
	gsl_matrix_set(this->m, row, col, val);
}

void Matrix::Print() {
	for (size_t row = 0; row < this->Height(); ++row) {
		for (size_t col = 0; col < this->Width(); ++col) {
			printf("%.2f ", gsl_matrix_get(m, row, col));
		}
		printf("\n");
	}
}

size_t Matrix::Height() {
	return this->m->size1;
}

size_t Matrix::Width() {
	return this->m->size2;
}

double *Matrix::Data() {
	return this->m->data;
}

Matrix *Matrix::Multiply(Matrix *other) {
	gsl_matrix *result = gsl_matrix_alloc(this->Height(), other->Height());
	cblas_dgemm(CblasRowMajor,// const enum CBLAS_ORDER Order
			CblasNoTrans,     // const enum CBLAS_TRANSPOSE TransA
			CblasTrans,       // const enum CBLAS_TRANSPOSE TransB
			this->Height(),   // const int M
			other->Height(),  // const int N
			this->Width(),    // const int K
			1.0f,             // const double alpha
			this->Data(),     // const double * A
			this->Width(),    // const int lda
			other->Data(),    // const double * B
			other->Width(),   // const int ldb
			0.0f,             // const double beta
			result->data,     // double * C
			result->size2);   // const int ldc
	return new Matrix(result);
}

int Matrix::NumberOfRows(FILE *f) {
	char lastChar = '\n';
	char currentChar = NULL;
	int count = 0;
	while ((currentChar = fgetc(f)) != EOF) {
		if (lastChar == '\n' && currentChar != '\n') {
			++count;
		}
		lastChar = currentChar;
	}
	fseek(f, 0, SEEK_SET);
	return count;
}

int Matrix::NumberOfColumns(FILE *f) {
	char lastChar = ' ';
	char currentChar = NULL;
	int count = 0;
	while ((currentChar = fgetc(f)) != '\n') {
		if (isspace(lastChar) && !isspace(currentChar)) {
			++count;
		}
		lastChar = currentChar;
	}
	fseek(f, 0, SEEK_SET);
	return count;
}

gsl_matrix *Matrix::CloneGSLMatrix() {
	size_t width = this->Width();
	size_t height = this->Height();
	gsl_matrix *m = gsl_matrix_alloc(height, width);
	for (size_t row = 0; row < this->Height(); ++row) {
		for (size_t col = 0; col < this->Width(); ++col) {
			gsl_matrix_set(m, row, col, *(this->m->data + row*width + col));
		}
	}
	return m;
}

}