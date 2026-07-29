# Airspan ASN.1 Codec

A robust, production-grade wrapper library for encoding, decoding, and validating 3GPP and O-RAN ASN.1 protocols (RRC, NGAP, X2AP, XNAP). 

Built on top of `asn1c`, this library abstracts away the extreme complexity of thousands of generated C structs into a simple, unified API that is hardened for production use.

## Features

- **Unified API**: A single entry point (`codec_encode`, `codec_decode`) for all protocols.
- **Deep Constraint Validation**: Built-in bounds checking via `codec_validate()` to catch out-of-range values defined by 3GPP specs.
- **Context-Aware Error Debugging**: Gone are the days of silent failures. The `CodecError` struct provides granular context, pinpointing exactly which field failed constraint validation or exactly how many bytes were consumed before a decode crashed.
- **Production Hardened**: 
  - **Memory Safety**: Safely cleans up partial allocations on decode failures.
  - **AddressSanitizer (ASAN)** integrated directly into the CMake build system.
  - **Fuzz Testing**: First-class support for `libFuzzer` instrumentation to hunt down hidden vulnerabilities by bombarding the codec with mutated packets.
- **Universal CLI Tool**: Contains a command-line `asn1_decoder` capable of ingesting Hex dumps and automatically dumping the XML/JSON representation of the packet for human debugging.

## Directory Structure

- `include/` - Public API headers (`codec.h`, `codec_types.h`).
- `src/` - Core library implementation (`codec.c`, `protocol_registry.c`).
- `tests/` - Automated round-trip unit tests.
- `tools/` - Standalone binaries for engineers (e.g., `asn1_decoder`).
- `fuzz/` - Security and Fuzz testing targets.
- `examples/` - Basic tutorial scripts for API usage.

## Build Instructions

### Standard Build (GCC)
```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### Running the Test Suite
We use CTest to run all internal round-trip automated tests:
```bash
cd build
ctest --output-on-failure
```

### Building with AddressSanitizer (ASAN)
To run the tests while tracking memory leaks:
```bash
cmake -B build-asan -DENABLE_ASAN=ON
cmake --build build-asan
cd build-asan
ctest
```

### Fuzz Testing (Requires Clang)
To build the Fuzzing target, you must compile with Clang and ensure `libclang-rt-dev` is installed on your system.
```bash
# Export Clang as the default compiler
export CC=clang CXX=clang++

# Build the Fuzzer
cmake -B build-fuzz -DENABLE_FUZZING=ON
cmake --build build-fuzz --target asn1_fuzzer

# Run the Fuzzer
./build-fuzz/asn1_fuzzer -runs=1000000
```

## Basic Usage

```c
#include "codec.h"

int main() {
    uint8_t buffer[] = { /* ... hex bytes ... */ };
    void *decoded_message = NULL;
    CodecError error;

    // Decode an NGAP packet
    CodecStatus status = codec_decode(
        CODEC_NGAP_PDU,
        buffer,
        sizeof(buffer),
        &decoded_message,
        &error
    );

    if (status != CODEC_SUCCESS) {
        printf("Decode failed: %s\n", error.message);
        return 1;
    }

    // Validate 3GPP Constraints
    if (codec_validate(CODEC_NGAP_PDU, decoded_message, &error) != CODEC_SUCCESS) {
        printf("Constraint violated on field %s: %s\n", error.failed_type_name, error.message);
    }

    // Always free memory
    codec_free(CODEC_NGAP_PDU, decoded_message);
    return 0;
}
```
