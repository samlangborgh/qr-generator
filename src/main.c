#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "qrencode.h"

int main(int argc, char** argv) {
    ErrorCorrectionLevel ecLevel = EC_H;

    int opt;
    while ((opt = getopt(argc, argv, "LMQHh")) != -1) {
        switch (opt) {
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
            case 'h':
                printf("help message oh yeah\n");
                exit(EXIT_SUCCESS);
                break;
            default:
                fprintf(stderr, "Usage: %s message [-L|-M|-Q|-H] [-h]", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    char* message = NULL;
    char helloWorld[] = "Hello, world!";

    if (optind >= argc) {
        // No argument provided
        message = helloWorld;
    } else {
        message = argv[optind];
    }

    printf("Message: %s\n", message);

    QR* qr = createQRCode(message, ecLevel);;

    printf("Version %d - Size: %dx%d\n", qr->version, qr->width, qr->width);
    printQR(qr);

    freeQR(qr);
    qr = NULL;

    return 0;
}
