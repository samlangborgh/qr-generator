#include "polynomial.h"

Polynomial* createPolynomial(size_t size) {
    assert(size > 0);
    Polynomial* p = (Polynomial*)malloc(sizeof(Polynomial));
    p->data = (unsigned int*)malloc(sizeof(unsigned int) * size);;
    p->size = size;
    memset(p->data, 0, sizeof(unsigned int) * size);
    return p;
}

void freePolynomial(Polynomial* p) {
    if (p->data != NULL) {
        free(p->data);
        p->data = NULL;
        p->size = 0;
    }
    free(p);
}

Polynomial* copyPolynomial(Polynomial* p) {
    Polynomial* newPolynomial = createPolynomial(p->size);

    for (int i = 0; i < p->size; i++)
        newPolynomial->data[i] = p->data[i];

    return newPolynomial;
}

Polynomial* slicePolynomial(Polynomial* p, unsigned int start, unsigned int end) {
    assert(start < end);

    assert(start >= 0);
    assert(start <= p->size);
    assert(end >= 0);
    assert(end <= p->size);

    size_t size = end - start;

    Polynomial* newPolynomial = createPolynomial(size);

    for (int i = 0; i < size; i++)
        newPolynomial->data[i] = p->data[start+i];

    return newPolynomial;
}

void printPolynomial(Polynomial* p) {
    size_t size = p->size;
    for (int i = 0; i < size; i++) {
        printf("%.2X ", p->data[i]);
    }
    printf("\n");
}

