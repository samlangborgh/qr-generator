#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "qrencode.h"

void printUsageMessage(const char* progName);
void printHelpMessage(const char* progName);

int main(int argc, char** argv) {
    ErrorCorrectionLevel ecLevel = EC_H;
    bool verbose = false;

    // TODO: Allow for reading data from files
    // Check if the file will fit in QR code
    // If not, eventually add structured append functionality
    int opt;
    const struct option long_options[] = {
        {"help", no_argument, NULL, 0},
        {"verbose", no_argument, NULL, 'v'},
        {0, 0, 0, 0},
    };

    while ((opt = getopt_long(argc, argv, "LMQHv", long_options, NULL)) != -1) {
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
            default:
                printUsageMessage(argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    char* message = NULL;
    char helloWorld[] = "Hello, world!";

    if (optind >= argc) {
        // No argument provided
        // TODO: Read from stdin instead
        message = helloWorld;
    } else {
        message = argv[optind];
    }

    QR* qr = createQRCode(message, ecLevel);;

    if (verbose) {
        printf("Message: %s\n", message);
        printf("Version %d - Size: %dx%d\n", qr->version, qr->width, qr->width);
    }

    printQR(qr);

    freeQR(qr);
    qr = NULL;

    return 0;
}

void printUsageMessage(const char* progName) {
    fprintf(stderr, "Usage: %s message [-L|-M|-Q|-H] [--help]\n", progName);
}

void printHelpMessage(const char* progName) {
    printUsageMessage(progName);
    printf("\nArguments:\n");
    printf("  message           message used to create QR code\n");
    printf("\nOptions:\n");
    printf("  -L                set error correction level to low\n");
    printf("  -M                set error correction level to medium\n");
    printf("  -Q                set error correction level to quartile\n");
    printf("  -H                set error correction level to high\n");
    printf("  -v, --verbose     print verbose output\n");
    printf("  --help            display this help message\n");
    printf("\nNote: The default error correction level is high\n");
    printf("\nExample:\n");
    printf("  %s \"Hello, world!\" -L\n", progName);
}
