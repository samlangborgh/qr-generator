#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "qrencode.h"

void printHelpMessage(const char* progName);
char* readFile(char* filePath);

int main(int argc, char** argv) {
    ErrorCorrectionLevel ecLevel = EC_M;
    bool verbose = false;

    int opt;
    char* filePath = NULL;
    bool fileMode = false;

    const struct option long_options[] = {
        {"file", required_argument, NULL, 'f'},
        {"help", no_argument, NULL, 0},
        {"verbose", no_argument, NULL, 'v'},
        {0, 0, 0, 0},
    };

    while ((opt = getopt_long(argc, argv, "LMQHvf:", long_options, NULL)) != -1) {
        switch (opt) {
            case 0:
                printHelpMessage(argv[0]);
                exit(EXIT_SUCCESS);
                break;
            case 'L':
                ecLevel = EC_L;
                break;
            case 'M':
                ecLevel = EC_M;
                break;
            case 'Q':
                ecLevel = EC_Q;
                break;
            case 'H':
                ecLevel = EC_H;
                break;
            case 'v':
                verbose = true;
                break;
            case 'f':
                filePath = optarg;
                fileMode = true;
                break;
            default:
                fprintf(stderr, "Try '%s --help' for more information.\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    char* message = argv[optind];
    bool stdinMode = optind >= argc; // No argument provided - read from stdin

    if (fileMode) {
        if (verbose)
            printf("Input file path: %s\n", filePath);
        message = readFile(filePath);
    } else if (stdinMode) {
        message = (char*)malloc(sizeof(char) * (MAX_QR_CHARS + 1));
        if (message == NULL) {
            perror("main() - failed to malloc");
            exit(EXIT_FAILURE);
        }

        int offset = 0;
        while(fgets(message + offset, MAX_QR_CHARS + 1 - offset, stdin)) {
            offset = strnlen(message, MAX_QR_CHARS);
            if (offset == MAX_QR_CHARS)
                break;
        }
    }

    QR* qr = createQRCode(message, ecLevel);

    if (verbose) {
        printf("Message: %s\n", message);
        printf("Version %d - Size: %dx%d\n", qr->version, qr->width, qr->width);
    }
    if (fileMode || stdinMode) {
        free(message);
        message = NULL;
    }

    // TODO: Check if terminal has enough rows, cols to properly display QR code
    // TODO: Provide functionality to save QR as an image file
    // TODO: Allow for flipping the printed QR "polarity"
    // TODO: Encode data more efficiently by using different encoding blocks
    // e.g. encode some data with numeric encoding, other with byte encoding, etc.
    printQR(qr);

    freeQR(qr);
    qr = NULL;

    return 0;
}

void printHelpMessage(const char* progName) {
    printf("Usage: %s [options] [message]\n", progName);
    printf("\nArguments:\n");
    printf("  message           message used to create QR code (optional)\n");
    printf("\nOptions:\n");
    printf("  -L                set error correction level to low\n");
    printf("  -M                set error correction level to medium\n");
    printf("  -Q                set error correction level to quartile\n");
    printf("  -H                set error correction level to high\n");
    printf("  -f FILE, --file=FILE\n");
    printf("                    create QR from file\n");
    printf("  -v, --verbose     print verbose output\n");
    printf("  --help            display this help message\n");
    printf("\nNotes:\n");
    printf("  If no message argument or file is provided, %s reads from standard input.\n", progName);
    printf("  The default error correction level is medium.\n");
    printf("\nExamples:\n");
    printf("  %s -L \"Hello, world!\"\n", progName);
    printf("  %s --file ~/.ssh/id_rsa.pub\n", progName);
    printf("  %s -H < myfile.txt\n", progName);
    printf("  ls | %s\n", progName);
}

char* readFile(char* filePath) {
    int rc = 0;

    FILE* fd = fopen(filePath, "r");
    if (fd == NULL) {
        perror(filePath);
        exit(EXIT_FAILURE);
    }

    // Seek to the end of the file so we can get the file length
    rc = fseek(fd, 0, SEEK_END);
    if (rc == -1) {
        perror(filePath);
        exit(EXIT_FAILURE);
    }

    long fileLength = ftell(fd);
    if (fileLength == -1) {
        perror("readFile() - error getting file length");
        exit(EXIT_FAILURE);
    }

    rewind(fd);

    if (fileLength > MAX_QR_CHARS)
        fileLength = MAX_QR_CHARS;

    char* dataBuffer = (char*)malloc(sizeof(char) * (fileLength + 1));
    if (dataBuffer == NULL) {
        perror("readFile() - failed to malloc");
        exit(EXIT_FAILURE);
    }

    // Read the entire file into dataBuffer -- up to fileLength bytes
    int offset = 0;
    while(fgets(dataBuffer + offset, fileLength + 1 - offset, fd)) {
        offset = strnlen(dataBuffer, fileLength);
        if (offset == fileLength)
            break;
    }

    rc = fclose(fd);
    if (rc != 0) {
        perror("readFile() - error closing file");
        exit(EXIT_FAILURE);
    }

    return dataBuffer;
}
