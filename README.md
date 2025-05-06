# qr-generator

## About

This tool generates QR codes in the terminal. It supports numeric, alphanumeric,
and byte encoding modes for all QR versions (1 through 40). QR codes can be
generated from a positional argument, a file, or standard input.

![Demonstration](extras/demo.png)

## Building

```bash
git clone https://github.com/Slangborgh16/qr-generator.git
cd qr-generator
make
```

## Usage

### Positional Arguments

|**Argument**|**Description**|
|---|---|
|message|Message used to create QR code (optional)|

### Options

|**Option**|**Description**|
|---|---|
|-L|Set error correction level to low (7% of data bytes can be restored)|
|-M|Set error correction level to medium (15% of data bytes can be restored)|
|-Q|Set error correction level to quartile (25% of data bytes can be restored)|
|-H|Set error correction level to high (30% of data bytes can be restored)|
|-i, --invert|Invert the colors of the QR code|
|-f, --file=FILE|Create QR from file (optional)|
|-v, --verbose|Print verbose output|
|--help|Display the help message|

- If no message argument or file is provided, the program reads from standard
input.
- The default error correction level is medium.

### Examples

```bash
./qr -L "The quick brown fox jumps over the lazy dog."
./qr https://github.com/Slangborgh16/qr-generator
./qr --file ~/.ssh/id_rsa.pub
./qr -H < myfile.txt
ls | ./qr
```

## Resources

I relied on the following resources while working on this project. I highly
recommend checking them out if you are interested in understanding how QR codes
work.

- [Thonky's QR Code Tutorial](https://www.thonky.com/qr-code-tutorial/)
- [Wikiversity: Reed-Solomon codes for coders](
https://en.wikiversity.org/wiki/Reed%E2%80%93Solomon_codes_for_coders)
- [Wikipedia: QR code](https://en.wikipedia.org/wiki/QR_code)
- [Veritasium: I built a QR code with my bare hands to see how it works](
https://youtu.be/w5ebcowAJD8)
