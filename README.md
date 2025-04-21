# qr-generator

## About

This project is a tool for generating QR codes in the terminal. It supports the
numeric, alphanumeric, and byte encoding modes. All QR versions (1 through 40)
are supported as well.

![Demonstration](extras/demo.png)

## Building

```bash
git clone https://github.com/Slangborgh16/qr-generator.git
cd qr-generator
make
```

## Usage Examples

```bash
./qr "The quick brown fox jumps over the lazy dog."
./qr https://github.com/Slangborgh16/qr-generator
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
