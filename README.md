# v7sh-amd64

A modern x86_64 port of the original UNIX 7th Edition (V7) Bourne shell, developed with the assistance of AI (Gemini and Claude).

## Overview

This project is a functional port of the classic Version 7 Bourne shell to modern 64-bit Linux environments. It preserves the original syntax and behavior while updating the underlying memory management and pointer handling to meet 64-bit standards.

### Key Enhancements

- **64-bit Support**: Fully compatible with x86_64 architecture.
- **Modern Memory Management**: Replaced legacy `sbrk` allocations with `malloc`/`realloc` for stability and standard compliance.
- **Portability**: Fixed pointer-to-integer truncation issues by using `intptr_t`.
- **Clean Build**: Compiled with `-Wall` and `-std=gnu89` with all major warnings resolved.

## Getting Started

### Prerequisites

- A modern Linux environment.
- `gcc` and `make`.

### Building

```bash
make clean
make
```

### Running

```bash
./v7sh
```

## Licensing

- Original code by **S. R. Bourne** (Bell Telephone Laboratories).
- Original source released under the **Caldera Ancient Unix License** (BSD-style).
- Port modifications and enhancements licensed under **GPL-v2**.

See the [NOTICE](NOTICE) and [LICENSE](LICENSE) files for full details.
