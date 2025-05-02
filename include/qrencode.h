#ifndef QRENCODE_H
#define QRENCODE_H

#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>

#include "linkedlist.h"
#include "polynomial.h"
#include "qrluts.h"
#include "reedsolomon.h"

// value to mark a module as unset
#define UNSET_MODULE 0xFF
// 7089 is the maximum number of characters storable in a QR code - Version 40-L, Numeric
#define MAX_QR_CHARS 7089

typedef enum {
    MODE_NUMERIC,
    MODE_ALPHANUMERIC,
    MODE_BYTE,
    MODE_KANJI,
    MODE_ECI,
} EncodingMode;

typedef enum {
    EC_L,
    EC_M,
    EC_Q,
    EC_H,
} ErrorCorrectionLevel;

typedef struct QR {
    unsigned int version;
    unsigned int width;
    unsigned int** data;
} QR;

typedef struct DataBlocks {
    Polynomial** group1;
    unsigned int numGroup1Blocks;
    unsigned int codewordsPerGroup1Block;
    Polynomial** group2;
    unsigned int numGroup2Blocks;
    unsigned int codewordsPerGroup2Block;
} DataBlocks;

// Data encoding functions
unsigned int calculateQRVersion(char* data, ErrorCorrectionLevel ecLevel);
unsigned int getMaxQRCharacters(char* data, ErrorCorrectionLevel ecLevel);
Polynomial* encodeData(char* data, unsigned int qrVersion, ErrorCorrectionLevel ecLevel);
DataBlocks* fragmentEncodedData(Polynomial* encodedData, unsigned int qrVersion,
        ErrorCorrectionLevel ecLevel);
DataBlocks* rsEncodeDataBlocks(DataBlocks* dataBlocks, unsigned int qrVersion,
        ErrorCorrectionLevel ecLevel);
linkedlist* structureFinalMessage(DataBlocks* encodedDataBlocks, DataBlocks* rsCodewordBlocks,
        unsigned int qrVersion, ErrorCorrectionLevel ecLevel);
void freeDataBlocks(DataBlocks* dataBlocks);

// Module placement functions
QR* initQR(unsigned int version);
QR* copyQR(QR* qr);
void freeQR(QR* qr);

void addFinderPatterns(QR* qr);
void addSeparators(QR* qr);
void addAlignmentPatterns(QR* qr);
void addTimingPatterns(QR* qr);
void addDarkModule(QR* qr);
void reserveFormatInfo(QR* qr);
void reserveVersionInfo(QR* qr);
QR* createMask(QR* qr, unsigned int maskType);
void placeDataBits(QR* qr, linkedlist* data);
QR* applyMask(QR* qr, QR* mask);
void addFormatInformation(QR* qr, ErrorCorrectionLevel ecLevel, unsigned int maskType);
void addVersionInformation(QR* qr);

unsigned int scoreQR(QR* qr);
unsigned int calculateBestMask(QR* qr, QR* blankQR);

QR* createQRCode(char* data, ErrorCorrectionLevel ecLevel);
void printQR(QR* qr);

#endif
