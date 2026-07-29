#ifndef CODEC_H
#define CODEC_H

#include <stddef.h>
#include <stdint.h>

#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "codec_types.h"
#include "protocol_registry.h"

#ifdef __cplusplus
extern "C"{
#endif

CodecStatus codec_encode(
    CodecProtocol protocol,
    const void* message,
    uint8_t* buffer,
    size_t buffer_size,
    size_t* encoded_size
);

CodecStatus codec_decode(
    CodecProtocol protocol,
    const uint8_t* buffer,
    size_t buffer_size,
    void **message
);

CodecStatus codec_free(
    CodecProtocol protocol,
    void *message
);

//to handle hex dumps
CodecStatus codec_decode_hex(
    CodecProtocol protocol,
    const char *hex_stroing,
    void **message
);

CodecStatus codec_encode_hex(
    CodecProtocol protocol,
    const void *message,
    char *hex_buffer,
    size_t hex_buffer_size
);

void codec_print_hex(
    const uint8_t *buffer,
    size_t length
);

static int hex_value(char c);

#ifdef __cplusplus
}
#endif
#endif