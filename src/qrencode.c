#include "qrencode.h"

// TODO: Add Kanji and ECI support
static EncodingMode calculateEncodingMode(char* data) {
    size_t dataLength = strnlen(data, MAX_QR_CHARS);

    // Step through data and determine what kind of encoding to use
    // based on the characters in the string
    EncodingMode encodingMode = MODE_NUMERIC;
    for (int i = 0; i < dataLength; i++) {
        char c = data[i];
        if (encodingMode == MODE_NUMERIC) {
            if (isdigit(c) == 0)
                encodingMode = MODE_ALPHANUMERIC;
        }

        if (encodingMode == MODE_ALPHANUMERIC) {
            if (!(isdigit(c) || isupper(c)) && (strchr(" $%*+-./:", c) == NULL))
                encodingMode = MODE_BYTE;
        }

        if (encodingMode == MODE_BYTE)
            return encodingMode;
    }

    return encodingMode;
}

unsigned int calculateQRVersion(char* data, ErrorCorrectionLevel ecLevel) {
    size_t dataLength = strnlen(data, MAX_QR_CHARS);

	unsigned int qrVersion = 0;

    EncodingMode encodingMode = calculateEncodingMode(data);
	
    for (int i = 1; i < 41; i++) {
        unsigned int maxCharacters = 0;
        switch (encodingMode) {
            case MODE_NUMERIC:
                maxCharacters = numericCharCapacityLUT[i][ecLevel];
                break;
            case MODE_ALPHANUMERIC:
                maxCharacters = alphanumericCharCapacityLUT[i][ecLevel];
                break;
            case MODE_BYTE:
                maxCharacters = byteCharCapacityLUT[i][ecLevel];
                break;
            case MODE_KANJI:
                maxCharacters = kanjiCharCapacityLUT[i][ecLevel];
                break;
            case MODE_ECI:
                break;
        }
        if (maxCharacters >= dataLength) {
            qrVersion = i;
            break;
        }
    }

    // The data is too big to fit into the largest QR code.
    // We will truncate the data to fit the largest QR later
    if (qrVersion == 0) {
        qrVersion = 40;
    }

    return qrVersion;
};

static void convertToBitStream(unsigned int data, int numBits, linkedlist* dataStream) {
    for (int i = 0; i < numBits; i++) {
        unsigned int shiftDistance = numBits - i - 1;
        unsigned char bit = (data >> shiftDistance) & 1;
        addLinkedListNode(dataStream, bit);
    }
}

static void numericEncoding(char* data, linkedlist* dataStream, int qrVersion) {
    unsigned char modeIndicator[] = {0, 0, 0, 1};

    for (int i = 0; i < 4; i++)
        addLinkedListNode(dataStream, modeIndicator[i]);

    size_t dataLength = strnlen(data, MAX_QR_CHARS);

    assert(dataLength > 0);
    assert(qrVersion >= 1 && qrVersion <= 40);

    int numBits;
    if (qrVersion <= 9) {
        // 10 bit character count
        numBits = 10;
    } else if (qrVersion >= 10 && qrVersion <= 26) {
        // 12 bit character count
        numBits = 12;
    } else {
        // 14 bit character count
        numBits = 14;
    }

    // Write data length to the data stream
    convertToBitStream(dataLength, numBits, dataStream);

    // Write groups of 3 digits to the data stream
    for (int i = 0; i < dataLength / 3; i++) {
        char threeDigitNum[4] = {0};
        strncpy(threeDigitNum, &data[3*i], 3);

        unsigned int num = atoi(threeDigitNum);

        numBits = 10;
        if (num < 100)
            numBits = 7;
        if (num < 10)
            numBits = 4;

        convertToBitStream(num, numBits, dataStream);
    }

    if (dataLength % 3 == 0)
        return;

    // Encode the remaining 1 or 2 digits
    char lastDigits[4] = {0};
    strncpy(lastDigits, &data[(dataLength / 3) * 3], 3);
    unsigned int num = atoi(lastDigits);
    if (num < 10)
        numBits = 4;
    else
        numBits = 7;

    convertToBitStream(num, numBits, dataStream);
}

static unsigned int getAlphanumericCode(char c) {
    // Convert from ASCII to QR Alphanumeric

    if (c >= 48 && c <= 57) // Digits 0-9
        return c - 48;
    if (c >= 65 && c <= 90) // Letters A-Z
        return c - 55;

    // Handle punctuation
    switch (c) {
        case 32:
            return 36;
        case '$':
            return 37;
        case '%':
            return 38;
        case '*':
            return 39;
        case '+':
            return 40;
        case '-':
            return 41;
        case '.':
            return 42;
        case '/':
            return 43;
        case ':':
            return 44;
        default:
            assert(0);
    }

    return 0;
}

static void alphanumericEncoding(char* data, linkedlist* dataStream, int qrVersion) {
    unsigned char modeIndicator[] = {0, 0, 1, 0};

    for (int i = 0; i < 4; i++)
        addLinkedListNode(dataStream, modeIndicator[i]);

    size_t dataLength = strnlen(data, MAX_QR_CHARS);

    assert(dataLength > 0);
    assert(qrVersion >= 1 && qrVersion <= 40);

    int numBits;
    if (qrVersion <= 9) {
        // 9 bit character count
        numBits = 9;
    } else if (qrVersion >= 10 && qrVersion <= 26) {
        // 11 bit character count
        numBits = 11;
    } else {
        // 13 bit character count
        numBits = 13;
    }

    // Write data length to the data stream
    convertToBitStream(dataLength, numBits, dataStream);

    // Write groups of 2 characters to the data stream
    for (int i = 0; i < dataLength / 2; i++) {
        unsigned int firstNumber = getAlphanumericCode(data[2*i]);
        unsigned int secondNumber = getAlphanumericCode(data[2*i+1]);

        unsigned int num = 45 * firstNumber + secondNumber;

        convertToBitStream(num, 11, dataStream);
    }

    if (dataLength % 2 == 1) {
        unsigned int num = getAlphanumericCode(data[dataLength - 1]);
        convertToBitStream(num, 6, dataStream);
    }
}

static void byteEncoding(char* data, linkedlist* dataStream, int qrVersion) {
    unsigned char modeIndicator[] = {0, 1, 0, 0};

    for (int i = 0; i < 4; i++)
        addLinkedListNode(dataStream, modeIndicator[i]);

    size_t dataLength = strnlen(data, MAX_QR_CHARS);

    assert(dataLength > 0);
    assert(qrVersion >= 1 && qrVersion <= 40);

    int numBits;
    if (qrVersion <= 9) {
        // 8 bit character count
        numBits = 8;
    } else {
        // 16 bit character count
        numBits = 16;
    }
    // Write data length to the data stream
    convertToBitStream(dataLength, numBits, dataStream);

    for (int i = 0; i < dataLength; i++)
        convertToBitStream(data[i], 8, dataStream);
}

static void addTerminator(linkedlist* dataStream, int qrVersion, ErrorCorrectionLevel ecLevel) {
    unsigned int numRequiredBits = totalDataCodewordsLUT[qrVersion][ecLevel] * 8;
    assert(dataStream->size <= numRequiredBits);
    if (dataStream->size == numRequiredBits)
        return;

    unsigned int numTerminatingZeros = MIN(numRequiredBits - dataStream->size, 4);

    for (int i = 0; i < numTerminatingZeros; i++)
        addLinkedListNode(dataStream, 0);
}

static void addMoreZeros(linkedlist* dataStream) {
    if (dataStream->size % 8 == 0)
        return;

    unsigned int numMoreZeros = 8 - dataStream->size % 8;
    for (int i = 0; i < numMoreZeros; i++)
        addLinkedListNode(dataStream, 0);
}

static void addPadding(linkedlist* dataStream, int qrVersion, ErrorCorrectionLevel ecLevel) {
    unsigned int numRequiredBits = totalDataCodewordsLUT[qrVersion][ecLevel] * 8;
    assert(dataStream->size <= numRequiredBits);
    if (dataStream->size == numRequiredBits)
        return;

    uint8_t fillPattern1 = 0b11101100;
    uint8_t fillPattern2 = 0b00010001;

    unsigned int bytesToAdd = (numRequiredBits - dataStream->size) / 8;
    for (int i = 0; i < bytesToAdd; i++) {
        if (i % 2 == 0)
            convertToBitStream(fillPattern1, 8, dataStream);
        else
            convertToBitStream(fillPattern2, 8, dataStream);
    }
}

unsigned int getMaxQRCharacters(char* data, ErrorCorrectionLevel ecLevel) {
    unsigned int qrVersion = calculateQRVersion(data, ecLevel);
    EncodingMode encodingMode = calculateEncodingMode(data);

    unsigned int maxCharacters = 0;
    switch (encodingMode) {
        case MODE_NUMERIC:
            maxCharacters = numericCharCapacityLUT[qrVersion][ecLevel];
            break;
        case MODE_ALPHANUMERIC:
            maxCharacters = alphanumericCharCapacityLUT[qrVersion][ecLevel];
            break;
        case MODE_BYTE:
            maxCharacters = byteCharCapacityLUT[qrVersion][ecLevel];
            break;
        case MODE_KANJI:
            assert(0);
            break;
        case MODE_ECI:
            assert(0);
            break;
    }

    return maxCharacters;
}

Polynomial* encodeData(char* data, unsigned int qrVersion, ErrorCorrectionLevel ecLevel) {
    assert(qrVersion >= 1 && qrVersion <= 40);

    EncodingMode encodingMode = calculateEncodingMode(data);
    linkedlist* dataStream = createLinkedList();

    switch (encodingMode) {
        case MODE_NUMERIC:
            numericEncoding(data, dataStream, qrVersion);
            break;
        case MODE_ALPHANUMERIC:
            alphanumericEncoding(data, dataStream, qrVersion);
            break;
        case MODE_BYTE:
            byteEncoding(data, dataStream, qrVersion);
            break;
        case MODE_KANJI:
            assert(0);
            break;
        case MODE_ECI:
            assert(0);
            break;
    }

    addTerminator(dataStream, qrVersion, ecLevel);
    addMoreZeros(dataStream);
    addPadding(dataStream, qrVersion, ecLevel);

    size_t dataStreamBits = dataStream->size;

    assert(dataStreamBits % 8 == 0 && dataStreamBits > 0);

    size_t codewordsSize = dataStreamBits / 8;
    Polynomial* codewordsPolynomial = createPolynomial(codewordsSize);

    for (int i = 0; i < codewordsSize; i++) {
        for (int j = 0; j < 8; j++) {
            unsigned char val = getNodeData(dataStream, i*8 + j) << (7 - j);
            codewordsPolynomial->data[i] |= val;
        }
    }

    freeLinkedList(dataStream);
    dataStream = NULL;

    return codewordsPolynomial;
};

DataBlocks* fragmentEncodedData(Polynomial* encodedData, unsigned int qrVersion,
        ErrorCorrectionLevel ecLevel) {
    const unsigned int dataBlocksInGroup1 = dataBlocksInGroup1LUT[qrVersion][ecLevel];
    const unsigned int dataBlocksInGroup2 = dataBlocksInGroup2LUT[qrVersion][ecLevel];

    const unsigned int dataCodewordsPerGroup1Block =
        dataCodewordsPerGroup1BlockLUT[qrVersion][ecLevel];
    const unsigned int dataCodewordsPerGroup2Block =
        dataCodewordsPerGroup2BlockLUT[qrVersion][ecLevel];

    DataBlocks* dataBlocks = (DataBlocks*)malloc(sizeof(DataBlocks));

    dataBlocks->group1 = (Polynomial**)malloc(sizeof(Polynomial*) * dataBlocksInGroup1);
    dataBlocks->numGroup1Blocks = dataBlocksInGroup1;
    dataBlocks->codewordsPerGroup1Block = dataCodewordsPerGroup1Block;
    dataBlocks->group2 = NULL;
    if (dataBlocksInGroup2 > 0)
        dataBlocks->group2 = (Polynomial**)malloc(sizeof(Polynomial*) * dataBlocksInGroup2);
    dataBlocks->numGroup2Blocks = dataBlocksInGroup2;
    dataBlocks->codewordsPerGroup2Block = dataCodewordsPerGroup2Block;

    unsigned int idx = 0;
    for (int i = 0; i < dataBlocksInGroup1; i++) {
        dataBlocks->group1[i] = slicePolynomial(encodedData, idx, idx + dataCodewordsPerGroup1Block);
        idx += dataCodewordsPerGroup1Block;
    }

    for (int i = 0; i < dataBlocksInGroup2; i++) {
        dataBlocks->group2[i] = slicePolynomial(encodedData, idx, idx + dataCodewordsPerGroup2Block);
        idx += dataCodewordsPerGroup2Block;
    }

    return dataBlocks;
}

DataBlocks* rsEncodeDataBlocks(DataBlocks* dataBlocks, unsigned int qrVersion,
        ErrorCorrectionLevel ecLevel) {
    unsigned int numECCodewords = ecCodewordsPerBlockLUT[qrVersion][ecLevel];
    DataBlocks* rsDataBlocks = (DataBlocks*)malloc(sizeof(DataBlocks));
    rsDataBlocks->numGroup1Blocks = dataBlocks->numGroup1Blocks;
    rsDataBlocks->codewordsPerGroup1Block = numECCodewords;
    rsDataBlocks->numGroup2Blocks = dataBlocks->numGroup2Blocks;
    rsDataBlocks->codewordsPerGroup2Block = numECCodewords;

    rsDataBlocks->group1 = (Polynomial**)malloc(sizeof(Polynomial*) * rsDataBlocks->numGroup1Blocks);
    rsDataBlocks->group2 = (Polynomial**)malloc(sizeof(Polynomial*) * rsDataBlocks->numGroup2Blocks);

    Polynomial* generator = createGeneratorPolynomial(numECCodewords);
    for (int i = 0; i < rsDataBlocks->numGroup1Blocks; i++)
        rsDataBlocks->group1[i] = rsEncodePolynomial(dataBlocks->group1[i], generator);

    for (int i = 0; i < rsDataBlocks->numGroup2Blocks; i++)
        rsDataBlocks->group2[i] = rsEncodePolynomial(dataBlocks->group2[i], generator);

    freePolynomial(generator);
    generator = NULL;

    return rsDataBlocks;
}

linkedlist* structureFinalMessage(DataBlocks* encodedDataBlocks, DataBlocks* rsCodewordBlocks,
        unsigned int qrVersion, ErrorCorrectionLevel ecLevel) {

    const unsigned int totalDataCodewords = totalDataCodewordsLUT[qrVersion][ecLevel];
    const unsigned int totalECCodewords = ecCodewordsPerBlockLUT[qrVersion][ecLevel] *
        (rsCodewordBlocks->numGroup1Blocks + rsCodewordBlocks->numGroup2Blocks);

    Polynomial* interleavedDataCodewords = createPolynomial(totalDataCodewords);
    Polynomial* interleavedECCodewords = createPolynomial(totalECCodewords);

    // Interleave the data codewords
    unsigned int idx = 0;
    unsigned int col = 0;
    while (idx < totalDataCodewords) {
        // Group 1 blocks
        for (int j = 0; j < encodedDataBlocks->numGroup1Blocks; j++) {
            if (col < encodedDataBlocks->codewordsPerGroup1Block) {
                Polynomial* currentBlock = encodedDataBlocks->group1[j];
                interleavedDataCodewords->data[idx] = currentBlock->data[col];
                idx++;
            }
        }

        for (int j = 0; j < encodedDataBlocks->numGroup2Blocks; j++) {
            if (col < encodedDataBlocks->codewordsPerGroup2Block) {
                Polynomial* currentBlock = encodedDataBlocks->group2[j];
                interleavedDataCodewords->data[idx] = currentBlock->data[col];
                idx++;
            }
        }
        col++;
    }

    // Interleave the error correction codewords
    idx = 0;
    col = 0;
    for (int i = 0; i < totalECCodewords; i++) {
        // Group 1 blocks
        for (int j = 0; j < rsCodewordBlocks->numGroup1Blocks; j++) {
            if (col < rsCodewordBlocks->codewordsPerGroup1Block) {
                Polynomial* currentBlock = rsCodewordBlocks->group1[j];
                interleavedECCodewords->data[idx] = currentBlock->data[col];
                idx++;
            }
        }

        for (int j = 0; j < rsCodewordBlocks->numGroup2Blocks; j++) {
            if (col < rsCodewordBlocks->codewordsPerGroup2Block) {
                Polynomial* currentBlock = rsCodewordBlocks->group2[j];
                interleavedECCodewords->data[idx] = currentBlock->data[col];
                idx++;
            }
        }
        col++;
    }

    Polynomial* finalMessagePolynomial = createPolynomial(totalDataCodewords + totalECCodewords);

    // Append the error correction codewords after the data codewords
    for (int i = 0; i < totalDataCodewords; i++)
        finalMessagePolynomial->data[i] = interleavedDataCodewords->data[i];

    for (int i = 0; i < totalECCodewords; i++)
        finalMessagePolynomial->data[totalDataCodewords + i] = interleavedECCodewords->data[i];

    freePolynomial(interleavedDataCodewords);
    interleavedDataCodewords = NULL;
    freePolynomial(interleavedECCodewords);
    interleavedECCodewords = NULL;

    linkedlist* finalMessage = createLinkedList();
    for (int i = 0; i < finalMessagePolynomial->size; i++)
        convertToBitStream(finalMessagePolynomial->data[i], 8, finalMessage);

    freePolynomial(finalMessagePolynomial);
    finalMessagePolynomial = NULL;

    unsigned int remainderBits = finalMessageRemainderBitsLUT[qrVersion];

    for (int i = 0; i < remainderBits; i++) {
        addLinkedListNode(finalMessage, 0);
    }

    return finalMessage;
}

void freeDataBlocks(DataBlocks* dataBlocks) {
    // Free the polynomial slices in group 1
    for (int i = 0; i < dataBlocks->numGroup1Blocks; i++) {
        freePolynomial(dataBlocks->group1[i]);
        dataBlocks->group1[i] = NULL;
    }
    free(dataBlocks->group1);
    dataBlocks->group1 = NULL;

    // Free the polynomial slices in group 2
    for (int i = 0; i < dataBlocks->numGroup2Blocks; i++) {
        freePolynomial(dataBlocks->group2[i]);
        dataBlocks->group2[i] = NULL;
    }
    free(dataBlocks->group2);
    dataBlocks->group2 = NULL;

    free(dataBlocks);
}

QR* initQR(unsigned int qrVersion) {
    assert(qrVersion > 0);
    assert(qrVersion < 41);

    QR* qr = (QR*)malloc(sizeof(QR));
    qr->version = qrVersion;
    qr->width = 4 * qrVersion + 17;
    qr->data = (unsigned int**)malloc(sizeof(unsigned int*) * qr->width);
    if (qr->data == NULL) {
        perror("intiQR() - failed to malloc");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < qr->width; i++) {
        qr->data[i] = (unsigned int*)malloc(sizeof(unsigned int) * qr->width);
        if (qr->data[i] == NULL) {
            perror("intiQR() - failed to malloc");
            exit(EXIT_FAILURE);
        }

        for (int j = 0; j < qr->width; j++) {
            // init to an arbitrary value (0xFF) to represent an unset module
            qr->data[i][j] = UNSET_MODULE;  
        }
    }

    return qr;
}

QR* copyQR(QR* qr) {
    QR* newQR = initQR(qr->version);
    for (int i = 0; i < qr->width; i++) {
        for (int j = 0; j < qr->width; j++) {
            newQR->data[i][j] = qr->data[i][j];
        }
    }

    return newQR;
}

void freeQR(QR* qr) {
    for (int i = 0; i < qr->width; i++) {
        free(qr->data[i]);
        qr->data[i] = NULL;
    }

    free(qr->data);
    qr->data = NULL;

    free(qr);
}

void addFinderPatterns(QR* qr) {
    unsigned int width = qr->width;
    for (int i = 0; i < 7; i++) {
        for (int j = 0; j < 7; j++) {
            if (i % 6 == 0 || j % 6 == 0 || (j > 1 && j < 5 && i > 1 && i < 5)) {
                qr->data[i][j] = 1;              //top left
                qr->data[i][width - 7 + j] = 1;   //top right
                qr->data[width - 7 + i][j] = 1;   //bottom left
            } else {
                qr->data[i][j] = 0;              //top left
                qr->data[i][width - 7 + j] = 0;   //top right
                qr->data[width - 7 + i][j] = 0;   //bottom left
            }
        }
    }
}

void addSeparators(QR* qr) {
    unsigned int width = qr->width;
    for (int i = 0; i < 8; i++) {
        qr->data[7][i] = 0;                 //top left
        qr->data[i][7] = 0;
        qr->data[7][width - 8 + i] = 0;     //top right
        qr->data[i][width - 8] = 0;         //top right
        qr->data[width - 8][i] = 0;         //bottom left
        qr->data[width - 8 + i][7] = 0;
    }
}

static void placeAlignmentPattern(QR* qr, unsigned int row, unsigned int col) {
    unsigned int width = qr->width;
    assert(row < width - 2);
    assert(row > 1);
    assert(col < width - 2);
    assert(col > 1);

    // Check for populated cells
    for (int i = 0; i < 5; i++)
        for (int j = 0; j < 5; j++)
            if (qr->data[row - 2 + i][col - 2 + j] != UNSET_MODULE)
                return;

    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            if (i % 4 == 0 || j % 4 ==0 || (i == 2 && j == 2))
                qr->data[row - 2 + i][col - 2 + j] = 1;
            else
                qr->data[row - 2 + i][col - 2 + j] = 0;
        }
    }
}

void addAlignmentPatterns(QR* qr) {
    // QR Version 1 has no alignment patterns
    if (qr->version == 1)
        return;

    for (int i = 0; i < 7; i++) {
        if (alignmentLUT[qr->version][i] == 0)
            break;
        unsigned int row = alignmentLUT[qr->version][i];
        for (int j = 0; j < 7; j++) {
            if (alignmentLUT[qr->version][j] == 0)
                break;
            unsigned int col = alignmentLUT[qr->version][j];
            placeAlignmentPattern(qr, row, col);
        }
    }
}

void addTimingPatterns(QR* qr) {
    int numTimingModules = qr->width - 16;
    for (int i = 0; i < numTimingModules; i++) {
        if (qr->data[6][8 + i] != UNSET_MODULE)
            continue;
        else if (i % 2 == 0)
            qr->data[6][8 + i] = 1;
        else
            qr->data[6][8 + i] = 0;

        if (qr->data[8 + i][6] != UNSET_MODULE)
            continue;
        else if (i % 2 == 0)
            qr->data[8 + i][6] = 1;
        else
            qr->data[8 + i][6] = 0;
    }
}

void addDarkModule(QR* qr) {
    qr->data[4 * qr->version + 9][8] = 1;
}

void reserveFormatInfo(QR* qr) {
    unsigned int width = qr->width;
    // Top left format info
    for (int i = 0; i < 9; i++) {
        if (i == 6)
            continue;
        qr->data[8][i] = 2;
        qr->data[i][8] = 2;
    }

    // Top right format info
    for (int i = 0; i < 8; i++) {
        qr->data[8][width - 8 + i] = 2;
    }

    // Bottom left format info
    for (int i = 0; i < 7; i++) {
        qr->data[width - 7 + i][8] = 2;
    }
}

void reserveVersionInfo(QR* qr) {
    if (qr->version < 7)
        return;

    unsigned int width = qr->width;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 6; j++) {
            qr->data[width - 11 + i][j] = 3;
            qr->data[j][width - 11 + i] = 3;
        }
    }
}

QR* createMask(QR* qr, unsigned int maskType) {
    assert(maskType >= 0);
    assert(maskType <= 7);

    QR* maskPattern = initQR(qr->version);

    unsigned int width = qr->width;
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < width; j++) {
            if (qr->data[i][j] != UNSET_MODULE) {
                maskPattern->data[i][j] = 0;
                continue;
            }
            switch (maskType) {
                case 0:
                    if ((i + j) % 2 == 0) {
                        maskPattern->data[i][j] = 1;
                    } else {
                        maskPattern->data[i][j] = 0;
                    }
                    break;
                case 1:
                    if (i % 2 == 0) {
                        maskPattern->data[i][j] = 1;
                    } else {
                        maskPattern->data[i][j] = 0;
                    }
                    break;
                case 2:
                    if (j % 3 == 0) {
                        maskPattern->data[i][j] = 1;
                    } else {
                        maskPattern->data[i][j] = 0;
                    }
                    break;
                case 3:
                    if ((i + j) % 3 == 0) {
                        maskPattern->data[i][j] = 1;
                    } else {
                        maskPattern->data[i][j] = 0;
                    }
                    break;
                case 4:
                    if ((i/2 + j/3) % 2 == 0) {
                        maskPattern->data[i][j] = 1;
                    } else {
                        maskPattern->data[i][j] = 0;
                    }
                    break;
                case 5:
                    if ((i * j) % 2 + (i * j) % 3 == 0) {
                        maskPattern->data[i][j] = 1;
                    } else {
                        maskPattern->data[i][j] = 0;
                    }
                    break;
                case 6:
                    if (((i * j) % 2 + (i * j) % 3) % 2 == 0) {
                        maskPattern->data[i][j] = 1;
                    } else {
                        maskPattern->data[i][j] = 0;
                    }
                    break;
                case 7:
                    if (((i + j) % 2 + (i * j) % 3) % 2 == 0) {
                        maskPattern->data[i][j] = 1;
                    } else {
                        maskPattern->data[i][j] = 0;
                    }
                    break;
            }
        }
    }

    return maskPattern;
}

void placeDataBits(QR* qr, linkedlist* data) {
    typedef enum {
        UP,
        DOWN
    } Direction;

    // Start in the bottom right-hand corner of the QR code
    // Place bits starting in the UP direction
    int xPos = (int)qr->width - 1;
    int yPos = (int)qr->width - 1;
    Direction dir = UP;

    unsigned int counter = 0;   // Used to help fill in the modules in a zigzag pattern

    unsigned int placedBits = 0;

    // Place each data bit into the QR code
    while (placedBits < data->size) {
        // Check if we are at an edge and need to switch directions
        if (yPos >= (int)qr->width) {
            xPos -= 2;
            if (xPos == 6)
                xPos--;
            yPos--;
            dir = UP;
            counter = 0;
        } else if (yPos < 0) {
            xPos -= 2;
            if (xPos == 6)
                xPos--;
            yPos++;
            dir = DOWN;
            counter = 0;
        }

        // Only place a bit if the module is unset (0xFF)
        // So we don't overwrite finder patterns, etc.
        if (qr->data[yPos][xPos] == UNSET_MODULE) {
            qr->data[yPos][xPos] = getNodeData(data, placedBits);
            placedBits++;
        }

        switch (dir) {
            case UP:
                if (counter == 0) {
                    xPos--;
                } else if (counter == 1) {
                    xPos++;
                    yPos--;
                }
                break;
            case DOWN:
                if (counter == 0) {
                    xPos--;
                } else if (counter == 1) {
                    xPos++;
                    yPos++;
                }
                break;
        }

        counter = (counter + 1) % 2;
    }
}

QR* applyMask(QR* qr, QR* mask) {
    assert(qr->version == mask->version);

    QR* maskedQR = initQR(qr->version);

    for (int i = 0; i < qr->width; i++) {
        for (int j = 0; j < qr->width; j++) {
            maskedQR->data[i][j] = qr->data[i][j] ^ mask->data[i][j];
        }
    }

    return maskedQR;
}

void addFormatInformation(QR* qr, ErrorCorrectionLevel ecLevel, unsigned int maskType) {
    unsigned int formatInfo = formatInfoLUT[ecLevel][maskType];
    unsigned int formatInfoString[15] = {0};

    for (int i = 0; i < 15; i++) {
        unsigned int shiftDistance = 15 - i - 1;
        formatInfoString[i] = (formatInfo >> shiftDistance) & 1;
    }

    // Place format info copy #1 around top left finder pattern
    for (int i = 0; i < 15; i++) {
        if (i < 6)
            qr->data[8][i] = formatInfoString[i];
        else if (i < 8)
            qr->data[8][i + 1] = formatInfoString[i];
        else if (i == 8)
            qr->data[7][8] = formatInfoString[i];
        else
            qr->data[14 - i][8] = formatInfoString[i];
    }

    // Place format info copy #2 next to bottom left and top right finder patterns
    for (int i = 0; i < 15; i++) {
        if (i < 7)
            qr->data[qr->width - i - 1][8] = formatInfoString[i];
        else
            // qr->data[8][qr->width - 15 + i] = 3;
            qr->data[8][qr->width - 15 + i] = formatInfoString[i];
    }
}

void addVersionInformation(QR* qr) {
    // Only add version information modules if we have a v7 or greater QR code
    if (qr->version < 7)
        return;

    unsigned int versionInfo = versionInfoLUT[qr->version];
    unsigned int versionInfoString[18] = {0};

    for (int i = 0; i < 18; i++) {
        unsigned int shiftDistance = 18 - i - 1;
        versionInfoString[17 - i] = (versionInfo >> shiftDistance) & 1;
    }

    // Place version info copy #1 next to the bottom left finder pattern
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 6; j++) {
            qr->data[qr->width - 11 + i][j] = versionInfoString[j * 3 + i];
        }
    }

    // Place version info copy #2 next to the top right finder pattern
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 3; j++) {
            // qr->data[i][qr->width - 11 + j] = 2;
            qr->data[i][qr->width - 11 + j] = versionInfoString[i * 3 + j];
        }
    }
}

static unsigned int scoreCondition1(QR* qr) {
    /* Condition 1:
     * Check rows and columns for consecutive and same-colored modules
     * If there are five consecutive modules of the same color, add 3 to the penalty
     * +1 penalty for every same-colored module after the first 5
     */
    unsigned int score = 0;

    // Calculate horizontal penalty
    unsigned int prevColor = 2;
    unsigned int consecutiveCount = 1;
    for (int i = 0; i < qr->width; i++) {
        for (int j = 0; j < qr->width; j++) {
            unsigned int curColor = qr->data[i][j];
            if (curColor != prevColor) {
                prevColor = curColor;
                consecutiveCount = 1;
                continue;
            }

            consecutiveCount++;

            if (consecutiveCount == 5)
                score += 3;

            if (consecutiveCount > 5)
                score++;
        }
        prevColor = 2;
        consecutiveCount = 1;
    }
    // printf("Horizontal penalty: %d\n", horizontalPenalty);

    // Calculate vertical penalty
    prevColor = 2;
    consecutiveCount = 1;
    for (int j = 0; j < qr->width; j++) {
        for (int i = 0; i < qr->width; i++) {
            unsigned int curColor = qr->data[i][j];
            if (curColor != prevColor) {
                prevColor = curColor;
                consecutiveCount = 1;
                continue;
            }

            consecutiveCount++;

            if (consecutiveCount == 5)
                score += 3;

            if (consecutiveCount > 5)
                score++;
        }
        prevColor = 2;
        consecutiveCount = 1;
    }
    // printf("Vertical penalty: %d\n", verticalPenalty);
    return score;
}

static unsigned int scoreCondition2(QR* qr) {
    /* Condition 2:
     * Looks for areas of the same color that are at least 2x2 modules or larger
     * Add 3 to the penalty score for every 2x2 block of the same color
     * Make sure to count overlapping blocks
     */
    unsigned int score = 0;

    for (int i = 0; i < qr->width - 1; i++) {
        for (int j = 0; j < qr->width - 1; j++) {
            // Check for 2x2 block
            unsigned int module1 = qr->data[i][j];
            unsigned int module2 = qr->data[i+1][j];
            unsigned int module3 = qr->data[i][j+1];
            unsigned int module4 = qr->data[i+1][j+1];
            if ((module1 == module2) && (module1 == module3) && (module1 == module4))
                score += 3;
        }
    }

    return score;
}

static unsigned int scoreCondition3(QR* qr) {
    /* Condition 3:
     * Check for patterns that look similar to the finder patterns
     * Any time the pattern 10111010000 or 00001011101 is found, add 40 to the penalty score
     * Make sure to check both rows and columns
     */
    unsigned int score = 0;

    unsigned int pattern1[] = {1, 0, 1, 1, 1, 0, 1, 0, 0, 0, 0};
    unsigned int pattern2[] = {0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 1};

    // Check the rows
    for (int i = 0; i < qr->width; i++) {
        for (int j = 0; j < qr->width - 10; j++) {
            bool pattern1Match = true;
            bool pattern2Match = true;
            for (int k = 0; k < 11; k++) {
                if (qr->data[i][j+k] != pattern1[k]) {
                    pattern1Match = false;
                }
                if (qr->data[i][j+k] != pattern2[k]) {
                    pattern2Match = false;
                }
                if ((pattern1Match == false) && (pattern2Match == false))
                    break;
            }
            if (pattern1Match || pattern2Match)
                score += 40;
        }
    }

    // Check the columns
    for (int j = 0; j < qr->width; j++) {
        for (int i = 0; i < qr->width - 10; i++) {
            bool pattern1Match = true;
            bool pattern2Match = true;
            for (int k = 0; k < 11; k++) {
                if (qr->data[i+k][j] != pattern1[k]) {
                    pattern1Match = false;
                }
                if (qr->data[i+k][j] != pattern2[k]) {
                    pattern2Match = false;
                }
                if ((pattern1Match == false) && (pattern2Match == false))
                    break;
            }
            if (pattern1Match || pattern2Match)
                score += 40;
        }
    }

    return score;
}

static unsigned int scoreCondition4(QR* qr) {
    /* Condition 4:
     * Applies a penalty based on the ratio of light to dark modules
     * More penalty is applied the greater the difference in number between light and dark is
     */
    unsigned int score = 0;
    
    int totalModules = qr->width * qr->width;

    int numDarkModules = 0;
    for (int i = 0; i < qr->width; i++) {
        for (int j = 0; j < qr->width; j++) {
            if (qr->data[i][j] == 1)
                numDarkModules++;
        }
    }

    double ratio = (double) numDarkModules / (double) totalModules * 100;
    int nextMultipleOf5 = ceil(ratio / 5.0) * 5;
    int prevMultipleOf5 = floor(ratio / 5.0) * 5;

    int diff1 = abs(50 - nextMultipleOf5);
    int diff2 = abs(50 - prevMultipleOf5);

    score = MIN(diff1 / 5, diff2 / 5) * 10;

    return score;
}

unsigned int scoreQR(QR* qr) {
    unsigned int score = 0;

    score += scoreCondition1(qr);
    score += scoreCondition2(qr);
    score += scoreCondition3(qr);
    score += scoreCondition4(qr);

    return score;
}

unsigned int calculateBestMask(QR* qr, QR* blankQR) {
    unsigned int bestMaskID = 0;
    unsigned int bestScore = UINT_MAX;

    for (int i = 0; i < 8; i++) {
        QR* mask = createMask(blankQR, i);
        QR* maskedQR = applyMask(qr, mask);

        unsigned int score = scoreQR(maskedQR);

        if (score < bestScore) {
            bestMaskID = i;
            bestScore = score;
        }

        freeQR(mask);
        mask = NULL;
        freeQR(maskedQR);
        maskedQR = NULL;
    }

    return bestMaskID;
}

QR* createQRCode(char* data, ErrorCorrectionLevel ecLevel) {
    unsigned int qrVersion = calculateQRVersion(data, ecLevel);
    Polynomial* encodedData = encodeData(data, qrVersion, ecLevel);

    DataBlocks* dataBlocks = fragmentEncodedData(encodedData, qrVersion, ecLevel);
    freePolynomial(encodedData);
    encodedData = NULL;

    DataBlocks* rsDataBlocks = rsEncodeDataBlocks(dataBlocks, qrVersion, ecLevel);

    linkedlist* finalMessage = structureFinalMessage(dataBlocks, rsDataBlocks, qrVersion, ecLevel);

    freeDataBlocks(dataBlocks);
    dataBlocks = NULL;
    freeDataBlocks(rsDataBlocks);
    rsDataBlocks = NULL;

    QR* qr = initQR(qrVersion);

    addFinderPatterns(qr);
    addSeparators(qr);
    addAlignmentPatterns(qr);
    addTimingPatterns(qr);
    addDarkModule(qr);
    reserveFormatInfo(qr);
    reserveVersionInfo(qr);
    QR* blankQR = copyQR(qr); // Create a copy of the QR code before we add data bits - for masking
    placeDataBits(qr, finalMessage);
    freeLinkedList(finalMessage);
    finalMessage = NULL;

    unsigned int bestMaskID = calculateBestMask(qr, blankQR);
    QR* maskQR = createMask(blankQR, bestMaskID);
    QR* finalQR = applyMask(qr, maskQR);
    freeQR(blankQR);
    blankQR = NULL;
    freeQR(qr);
    qr = NULL;
    freeQR(maskQR);
    maskQR = NULL;

    addFormatInformation(finalQR, ecLevel, bestMaskID);
    addVersionInformation(finalQR);

    return finalQR;
}

void printQR(QR* qr) {
    unsigned int width = qr->width;
    for (int i = 0; i < width + 2; i++)
        printf("██");
    printf("\n");
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < width; j++) {
            if (j == 0)
                printf("██");
            unsigned int num = qr->data[i][j];
            if (num == 0) {
                printf("██");
            } else if (num == 1) {
                printf("  ");
            } else if (num == 2) {
                // print in blue
                printf("\e[0;34m██\e[0m");
            } else if (num == 3) {
                // print in green
                printf("\e[0;32m██\e[0m");
            } else if (num == 4) {
                // print in red
                printf("\e[0;31m██\e[0m");
            } else {
                printf("░░");
            }
            if (j == width - 1)
                printf("██");
        }
        printf("\n");
    }
    for (int i = 0; i < width + 2; i++)
        printf("██");
    printf("\n");
}
