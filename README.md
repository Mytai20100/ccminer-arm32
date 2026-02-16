# CCMiner with ARM NEON Support

Multi-architecture CPU miner with automatic SIMD optimization.

## Supported Architectures

- **x86_64** - AVX2, AES-NI, PCLMULQDQ
- **x86** - SSE4.1, AES-NI, PCLMULQDQ  
- **ARM** - NEON, ARM Crypto Extensions (if available)

## Quick Build

```bash
./build.sh
```

Auto-detects your CPU and builds with optimal flags.

## Manual Build

```bash
./autogen.sh
./configure --with-curl
make -j$(nproc)
```

## Usage

```bash
./ccminer -a verus -o stratum+tcp://pool:port -u address -p x
```

## Architecture Detection

The build system automatically:
- Detects CPU architecture
- Selects optimal SIMD instructions
- Enables hardware crypto acceleration
- Uses ARM NEON on ARM processors
- Uses AVX2/SSE on x86 processors

## ARM Notes

On ARM, the build uses NEON intrinsics for 2-3x performance vs portable C.
Hardware AES is used if ARM Crypto Extensions are available.

## License

MIT - See LICENSE.txt
