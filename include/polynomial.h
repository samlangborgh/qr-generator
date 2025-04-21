#ifndef POLYNOMIAL_H
#define POLYNOMIAL_H

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Polynomial {
    unsigned int* data;
    size_t size;
} Polynomial;

Polynomial* createPolynomial(size_t size);
void freePolynomial(Polynomial* p);

Polynomial* copyPolynomial(Polynomial* p);
Polynomial* slicePolynomial(Polynomial* p, unsigned int start, unsigned int end);
void printPolynomial(Polynomial* p);

#endif
