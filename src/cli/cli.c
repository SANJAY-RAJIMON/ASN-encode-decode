#include "cli.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static int hex_value(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

int cli_parse_hex(const char *hex_str, uint8_t **out_buffer, size_t *out_size) {
    if (!hex_str || !out_buffer || !out_size) return -1;

    size_t len = strlen(hex_str);
    // Ignore prefix if present
    if (len >= 2 && hex_str[0] == '0' && (hex_str[1] == 'x' || hex_str[1] == 'X')) {
        hex_str += 2;
        len -= 2;
    }

    // allocate worst case
    uint8_t *buf = (uint8_t *)malloc((len / 2) + 1);
    if (!buf) return -1;

    size_t byte_count = 0;
    for (size_t i = 0; i < len; ) {
        // Skip spaces or newlines
        if (isspace((unsigned char)hex_str[i])) {
            i++;
            continue;
        }

        if (i + 1 >= len) break; // Incomplete byte

        int high = hex_value(hex_str[i]);
        int low = hex_value(hex_str[i+1]);

        if (high == -1 || low == -1) {
            free(buf);
            return -1; // invalid hex char
        }

        buf[byte_count++] = (uint8_t)((high << 4) | low);
        i += 2;
    }

    *out_buffer = buf;
    *out_size = byte_count;
    return 0;
}

int cli_parse_file(const char *filepath, uint8_t **out_buffer, size_t *out_size) {
    if (!filepath || !out_buffer || !out_size) return -1;

    FILE *f = fopen(filepath, "rb");
    if (!f) return -1;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (size < 0) {
        fclose(f);
        return -1;
    }

    uint8_t *buf = (uint8_t *)malloc((size_t)size);
    if (!buf) {
        fclose(f);
        return -1;
    }

    size_t read_bytes = fread(buf, 1, size, f);
    fclose(f);

    if (read_bytes != (size_t)size) {
        free(buf);
        return -1;
    }

    *out_buffer = buf;
    *out_size = read_bytes;
    return 0;
}

bool cli_infer_protocol(const char *filename, codec_protocol_t *out_protocol) {
    if (!filename || !out_protocol) return false;

    // Check against all supported protocols
    // A simple substring match (case insensitive)
    const char *protos[] = {
        "NGAP", "X2AP", "XNAP", "UL_CCCH", "DL_CCCH", 
        "UL_DCCH", "DL_DCCH", "BCCH_BCH", "BCCH_DL_SCH", "PCCH"
    };

    char upper_filename[256];
    strncpy(upper_filename, filename, sizeof(upper_filename) - 1);
    upper_filename[sizeof(upper_filename) - 1] = '\0';
    
    for (size_t i = 0; i < strlen(upper_filename); i++) {
        upper_filename[i] = (char)toupper((unsigned char)upper_filename[i]);
    }

    for (size_t i = 0; i < sizeof(protos)/sizeof(protos[0]); i++) {
        if (strstr(upper_filename, protos[i]) != NULL) {
            *out_protocol = codec_protocol_from_name(protos[i]);
            return true;
        }
    }

    return false;
}

const char *cli_status_to_string(CodecStatus status) {
    switch (status) {
        case CODEC_SUCCESS: return "Success";
        case CODEC_ERROR_INVALID_PROTOCOL: return "Unknown protocol";
        case CODEC_ERROR_INVALID_ARGUMENT: return "Invalid argument";
        case CODEC_ERROR_ENCODE_FAILED: return "Encode failed";
        case CODEC_ERROR_DECODE_FAILED: return "Invalid APER encoding / Decode failed";
        case CODEC_ERROR_BUFFER_TOO_SMALL: return "Unexpected end of packet / Buffer too small";
        case CODEC_ERROR_VALIDATION_FAILED: return "Constraint violation";
        case CODEC_ERROR_UNSUPPORTED_ENCODING: return "Unsupported encoding";
        default: return "Unknown error";
    }
}
