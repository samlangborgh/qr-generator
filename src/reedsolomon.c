#include "reedsolomon.h"

static const unsigned int gf256logLUT[256] = {
    0, 0, 1, 25, 2, 50, 26, 198,
    3, 223, 51, 238, 27, 104, 199, 75,
    4, 100, 224, 14, 52, 141, 239, 129,
    28, 193, 105, 248, 200, 8, 76, 113,
    5, 138, 101, 47, 225, 36, 15, 33,
    53, 147, 142, 218, 240, 18, 130, 69,
    29, 181, 194, 125, 106, 39, 249, 185,
    201, 154, 9, 120, 77, 228, 114, 166,
    6, 191, 139, 98, 102, 221, 48, 253,
    226, 152, 37, 179, 16, 145, 34, 136,
    54, 208, 148, 206, 143, 150, 219, 189,
    241, 210, 19, 92, 131, 56, 70, 64,
    30, 66, 182, 163, 195, 72, 126, 110,
    107, 58, 40, 84, 250, 133, 186, 61,
    202, 94, 155, 159, 10, 21, 121, 43,
    78, 212, 229, 172, 115, 243, 167, 87,
    7, 112, 192, 247, 140, 128, 99, 13,
    103, 74, 222, 237, 49, 197, 254, 24,
    227, 165, 153, 119, 38, 184, 180, 124,
    17, 68, 146, 217, 35, 32, 137, 46,
    55, 63, 209, 91, 149, 188, 207, 205,
    144, 135, 151, 178, 220, 252, 190, 97,
    242, 86, 211, 171, 20, 42, 93, 158,
    132, 60, 57, 83, 71, 109, 65, 162,
    31, 45, 67, 216, 183, 123, 164, 118,
    196, 23, 73, 236, 127, 12, 111, 246,
    108, 161, 59, 82, 41, 157, 85, 170,
    251, 96, 134, 177, 187, 204, 62, 90,
    203, 89, 95, 176, 156, 169, 160, 81,
    11, 245, 22, 235, 122, 117, 44, 215,
    79, 174, 213, 233, 230, 231, 173, 232,
    116, 214, 244, 234, 168, 80, 88, 175
};

static const unsigned int gf256antilogLUT[256] = {
    1, 2, 4, 8, 16, 32, 64, 128,
    29, 58, 116, 232, 205, 135, 19, 38,
    76, 152, 45, 90, 180, 117, 234, 201,
    143, 3, 6, 12, 24, 48, 96, 192,
    157, 39, 78, 156, 37, 74, 148, 53,
    106, 212, 181, 119, 238, 193, 159, 35,
    70, 140, 5, 10, 20, 40, 80, 160,
    93, 186, 105, 210, 185, 111, 222, 161,
    95, 190, 97, 194, 153, 47, 94, 188,
    101, 202, 137, 15, 30, 60, 120, 240,
    253, 231, 211, 187, 107, 214, 177, 127,
    254, 225, 223, 163, 91, 182, 113, 226,
    217, 175, 67, 134, 17, 34, 68, 136,
    13, 26, 52, 104, 208, 189, 103, 206,
    129, 31, 62, 124, 248, 237, 199, 147,
    59, 118, 236, 197, 151, 51, 102, 204,
    133, 23, 46, 92, 184, 109, 218, 169,
    79, 158, 33, 66, 132, 21, 42, 84,
    168, 77, 154, 41, 82, 164, 85, 170,
    73, 146, 57, 114, 228, 213, 183, 115,
    230, 209, 191, 99, 198, 145, 63, 126,
    252, 229, 215, 179, 123, 246, 241, 255,
    227, 219, 171, 75, 150, 49, 98, 196,
    149, 55, 110, 220, 165, 87, 174, 65,
    130, 25, 50, 100, 200, 141, 7, 14,
    28, 56, 112, 224, 221, 167, 83, 166,
    81, 162, 89, 178, 121, 242, 249, 239,
    195, 155, 43, 86, 172, 69, 138, 9,
    18, 36, 72, 144, 61, 122, 244, 245,
    247, 243, 251, 235, 203, 139, 11, 22,
    44, 88, 176, 125, 250, 233, 207, 131,
    27, 54, 108, 216, 173, 71, 142, 1
};

unsigned int gf256Multiply(unsigned int a, unsigned int b) {
    unsigned int logA = gf256logLUT[a];
    unsigned int logB = gf256logLUT[b];

    unsigned int sum = logA + logB;

    if (sum >= 256)
        sum %= 255;

    return gf256antilogLUT[sum];
}

void freePolyDivisionResult(PolyDivisionResult* result) {
    if (result->quotient != NULL) {
        freePolynomial(result->quotient);
        result->quotient = NULL;
    }

    if (result->remainder != NULL) {
        freePolynomial(result->remainder);
        result->remainder = NULL;
    }

    free(result);
}

Polynomial* gf256PolyScalarMultiply(Polynomial* p, unsigned int s) {
    assert(p->data != NULL && p->size > 0);
    Polynomial* newPolynomial = createPolynomial(p->size);
    for (int i = 0; i < p->size; i++)
        newPolynomial->data[i] = p->data[i] * s;

    return newPolynomial;
}

Polynomial* gf256PolyAdd(Polynomial* a, Polynomial* b) {
    size_t size = MAX(a->size, b->size);
    assert(size > 0);
    Polynomial* p = createPolynomial(size);

    for (int i = 0; i < a->size; i++)
        p->data[i + p->size - a->size] = a->data[i];

    for (int i = 0; i < b->size; i++)
        p->data[i + p->size - b->size] ^= b->data[i];

    return p;
}

Polynomial* gf256PolyMultiply(Polynomial* a, Polynomial* b) {
    Polynomial* p = createPolynomial(a->size + b->size - 1);

    for (int i = 0; i < a->size; i++) {
        for (int j = 0; j < b->size; j++) {
            p->data[i+j] ^= gf256Multiply(a->data[i], b->data[j]);
        }
    }
    return p;
}

PolyDivisionResult* gf256PolyDivide(Polynomial* dividend, Polynomial* divisor) {
    // GF256-optimized synthetic division
    Polynomial* p = createPolynomial(dividend->size);

    // Copy the dividend into p
    for (int i = 0; i < p->size; i++)
        p->data[i] = dividend->data[i];

    for (int i = 0; i < dividend->size - (divisor->size - 1); i++) {
        unsigned int coefficient = p->data[i];

        if (coefficient == 0)
            continue;

        for (int j = 1; j < divisor->size; j++) {
            if (divisor->data[j] != 0)
                p->data[i+j] ^= gf256Multiply(divisor->data[j], coefficient);
        }
    }

    size_t remainderSize = divisor->size - 1;

    Polynomial* quotient = createPolynomial(p->size - remainderSize);
    Polynomial* remainder = createPolynomial(remainderSize);

    for (int i = 0; i < p->size - remainderSize; i++)
        quotient->data[i] = p->data[i];

    for (int i = 0; i < remainder->size; i++)
        remainder->data[i] = p->data[p->size - remainderSize + i];

    freePolynomial(p);
    p = NULL;
    //
    // Create the result struct
    PolyDivisionResult* result = (PolyDivisionResult*)malloc(sizeof(PolyDivisionResult));
    result->quotient = quotient;
    result->remainder = remainder;

    return result;
}

Polynomial* createGeneratorPolynomial(unsigned int numECCodewords) {
    assert(numECCodewords >= 7 && numECCodewords <= 30);
    Polynomial* p = createPolynomial(1);
    p->data[0] = 1;

    Polynomial* tmpPoly;
    for (int i = 0; i < numECCodewords; i++) {
        Polynomial* q = createPolynomial(2);
        q->data[0] = 1;
        q->data[1] = gf256antilogLUT[i];

        tmpPoly = gf256PolyMultiply(p, q);
        freePolynomial(p);
        p = NULL;
        freePolynomial(q);
        q = NULL;
        p = tmpPoly;
    }

    return p;
}

Polynomial* rsEncodePolynomial(Polynomial* msg, Polynomial* generator) {
    size_t paddedMsgSize = msg->size + generator->size - 1;
    Polynomial* paddedMsg = createPolynomial(paddedMsgSize);
    for (int i = 0; i < paddedMsgSize; i++) {
        if (i < msg->size)
            paddedMsg->data[i] = msg->data[i];
        else
            paddedMsg->data[i] = 0;
    }

    PolyDivisionResult* divisionResult = gf256PolyDivide(paddedMsg, generator);
    freePolynomial(paddedMsg);
    paddedMsg = NULL;

    /*
    size_t remainderSize = divisionResult->remainder->size;
    Polynomial* rsEncodingResult = createPolynomial(msg->size + remainderSize);

    for (int i = 0; i < msg->size; i++)
        rsEncodingResult->data[i] = msg->data[i];

    for (int i = 0; i < remainderSize; i++)
        rsEncodingResult->data[msg->size + i] = divisionResult->remainder->data[i];

    freePolyDivisionResult(divisionResult);
    divisionResult = NULL;

    return rsEncodingResult;
    */

    Polynomial* rsEncodingResult = copyPolynomial(divisionResult->remainder);
    freePolyDivisionResult(divisionResult);
    divisionResult = NULL;

    return rsEncodingResult;
}
