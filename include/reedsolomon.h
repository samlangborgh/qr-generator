#ifndef REEDSOLOMON_H
#define REEDSOLOMON_H

#include <assert.h>
#include <stdlib.h>
#include <sys/param.h>

#include "polynomial.h"

typedef struct PolyDivisionResult {
    Polynomial* quotient;
    Polynomial* remainder;
} PolyDivisionResult;

unsigned int gf256Multiply(unsigned int a, unsigned int b);

void freePolyDivisionResult(PolyDivisionResult* result);

Polynomial* gf256PolyScalarMultiply(Polynomial* p, unsigned int s);
Polynomial* gf256PolyAdd(Polynomial* a, Polynomial* b);
Polynomial* gf256PolyMultiply(Polynomial* a, Polynomial* b);
PolyDivisionResult* gf256PolyDivide(Polynomial* dividend, Polynomial* divisor);

Polynomial* createGeneratorPolynomial(unsigned int numECCodewords);
Polynomial* rsEncodePolynomial(Polynomial* msg, Polynomial* generator);

#endif
