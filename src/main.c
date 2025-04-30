#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "qrencode.h"

void printHelpMessage(const char* progName);

int main(int argc, char** argv) {
    ErrorCorrectionLevel ecLevel = EC_H;
    bool verbose = false;

    int rc = 0;
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

    char* message = NULL;
    char dataBuffer[MAX_QR_CHARS + 1] = {0};

    if (fileMode) {
        if (verbose)
            printf("Input file path: %s\n", filePath);

        FILE* fd = fopen(filePath, "r");
        if (fd == NULL) {
            perror(filePath);
            exit(EXIT_FAILURE);
        }

        // Read the entire file into dataBuffer -- up to MAX_QR_CHARS
        int offset = 0;
        while(fgets(dataBuffer + offset, MAX_QR_CHARS + 1 - offset, fd)) {
            offset = strnlen(dataBuffer, MAX_QR_CHARS);
            if (offset == MAX_QR_CHARS)
                break;
        }
        message = dataBuffer;

        rc = fclose(fd);
        if (rc != 0) {
            perror("main() - error closing file");
            exit(EXIT_FAILURE);
        }
    } else {
        if (optind >= argc) {
            // No argument provided
            int offset = 0;
            while(fgets(dataBuffer + offset, MAX_QR_CHARS + 1 - offset, stdin)) {
                offset = strnlen(dataBuffer, MAX_QR_CHARS);
                if (offset == MAX_QR_CHARS)
                    break;
            }
            message = dataBuffer;
        } else {
            message = argv[optind];
        }
    }

    QR* qr = createQRCode(message, ecLevel);

    if (verbose) {
        printf("Message: %s\n", message);
        printf("Version %d - Size: %dx%d\n", qr->version, qr->width, qr->width);
    }

    // TODO: Check if terminal has enough rows, cols to properly display QR code
    // TODO: Provide functionality to save QR as an image file
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
    printf("  The default error correction level is high.\n");
    printf("\nExamples:\n");
    printf("  %s -L \"Hello, world!\"\n", progName);
    printf("  %s --file ~/.ssh/id_rsa.pub\n", progName);
    printf("  %s -M < myfile.txt\n", progName);
    printf("  ls | %s\n", progName);
}
