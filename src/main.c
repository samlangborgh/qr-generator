#include <stdio.h>

#include "qrencode.h"

int main(int argc, char** argv) {
    ErrorCorrectionLevel ecLevel = EC_H;

    char helloWorld[] = "Hello, world!";
    QR* qr = NULL;

    if (argc > 1) {
        printf("Message: %s\n", argv[1]);
        qr = createQRCode(argv[1], ecLevel);
    } else {
        printf("Message: %s\n", helloWorld);
        qr = createQRCode(helloWorld, ecLevel);
    }

    printf("Version %d - Size: %dx%d\n", qr->version, qr->width, qr->width);
    printQR(qr);

    freeQR(qr);
    qr = NULL;

    return 0;
}
