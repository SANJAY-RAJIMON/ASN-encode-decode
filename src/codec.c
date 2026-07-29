#include "codec.h"
#include "protocol_registry.h"

#include <aper_encoder.h>
#include <aper_decoder.h>

#include <per_encoder.h>
#include <per_decoder.h>

#include <xer_decoder.h>


CodecStatus codec_encode(
    CodecProtocol protocol,
    const void *message,
    uint8_t *buffer,
    size_t buffer_size,
    size_t *encoded_size,
    CodecError *error
)
{
    if(!message || !buffer || !encoded_size)
        return CODEC_ERROR_INVALID_ARGUMENT;

    const ProtocolEntry *entry =
        protocol_registry_lookup(protocol);

    if(!entry)
        return CODEC_ERROR_INVALID_PROTOCOL;

    asn_enc_rval_t result;

    result = entry->ops->encode(
            entry->descriptor,
            NULL,
            message,
            buffer,
            buffer_size
        );

    if(result.encoded == -1) {
        if(error) {
            snprintf(error->failed_type_name, sizeof(error->failed_type_name), "%s", result.failed_type ? result.failed_type->name : "Unknown");
            snprintf(error->message, sizeof(error->message), "Encode failed at type %s", result.failed_type ? result.failed_type->name : "Unknown");
        }
        return CODEC_ERROR_ENCODE_FAILED;
    }

    *encoded_size = (result.encoded + 7) / 8;

    return CODEC_SUCCESS;
}

CodecStatus codec_decode(
    CodecProtocol protocol,
    const uint8_t *buffer,
    size_t buffer_size,
    void **message,
    CodecError *error
)
{
    if(!buffer || !message)
        return CODEC_ERROR_INVALID_ARGUMENT;

    const ProtocolEntry *entry =
        protocol_registry_lookup(protocol);

    if(!entry)
        return CODEC_ERROR_INVALID_PROTOCOL;

    asn_dec_rval_t result;

    result = entry->ops->decode(
        NULL,
        entry->descriptor,
        message,
        buffer,
        buffer_size,
        0,
        0
    );

    if(result.code != RC_OK)
    {
        if(error) {
            // result.consumed is in bits for PER decoding
            snprintf(error->message, sizeof(error->message), "Decode failed after %zu bits", result.consumed);
        }
        if (*message) {
            ASN_STRUCT_FREE(*entry->descriptor, *message);
            *message = NULL;
        }
        return CODEC_ERROR_DECODE_FAILED;
    }

    return CODEC_SUCCESS;
}

CodecStatus codec_decode_xml(
    CodecProtocol protocol,
    const char *xml_buffer,
    size_t xml_size,
    void **message,
    CodecError *error
)
{
    if(!xml_buffer || !message)
        return CODEC_ERROR_INVALID_ARGUMENT;

    const ProtocolEntry *entry =
        protocol_registry_lookup(protocol);

    if(!entry)
        return CODEC_ERROR_INVALID_PROTOCOL;

    asn_dec_rval_t result;

    result = xer_decode(
        NULL,
        entry->descriptor,
        message,
        xml_buffer,
        xml_size
    );

    if(result.code != RC_OK)
    {
        if(error) {
            snprintf(error->message, sizeof(error->message), "XER decode failed after %zu bytes", result.consumed);
            error->failed_type_name[0] = '\0';
        }
        if (*message) {
            ASN_STRUCT_FREE(*entry->descriptor, *message);
            *message = NULL;
        }
        return CODEC_ERROR_DECODE_FAILED;
    }

    return CODEC_SUCCESS;
}

CodecStatus codec_validate(
    CodecProtocol protocol,
    const void *message,
    CodecError *error
)
{
    if(!message)
        return CODEC_ERROR_INVALID_ARGUMENT;

    const ProtocolEntry *entry =
        protocol_registry_lookup(protocol);

    if(!entry)
        return CODEC_ERROR_INVALID_PROTOCOL;

    char errbuf[256];
    size_t errlen = sizeof(errbuf);

    int ret = asn_check_constraints(
        entry->descriptor,
        message,
        errbuf,
        &errlen
    );

    if(ret != 0) {
        if(error) {
            snprintf(error->message, sizeof(error->message), "%s", errbuf);
            // In asn_check_constraints, we don't always get the failed type cleanly like in encode,
            // but the message usually contains the field name.
            snprintf(error->failed_type_name, sizeof(error->failed_type_name), "Constraint_Violation");
        }
        return CODEC_ERROR_VALIDATION_FAILED;
    }

    return CODEC_SUCCESS;
}

CodecStatus codec_free(
    CodecProtocol protocol,
    void *message
)
{
    if(!message)
        return CODEC_ERROR_INVALID_ARGUMENT;

    const ProtocolEntry *entry =
        protocol_registry_lookup(protocol);

    if(!entry)
        return CODEC_ERROR_INVALID_PROTOCOL;

    ASN_STRUCT_FREE(*entry->descriptor, message);

    return CODEC_SUCCESS;
}

//hex helper
static int hex_value(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    else if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    else if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    return -1;
}

CodecStatus codec_decode_hex(
    CodecProtocol protocol,
    const char *hex_string,
    void **message,
    CodecError *error
)
{
    if(!hex_string || !message)
        return CODEC_ERROR_INVALID_ARGUMENT;

    size_t len = strlen(hex_string);

    uint8_t *buffer = (uint8_t *)malloc(len / 2 + 1);

    if (!buffer)
        return CODEC_ERROR_INVALID_ARGUMENT;
    
    size_t bytes = 0;
    int high = - 1;

    for (size_t i = 0; i < len; i++){
        int value = hex_value(hex_string[i]);
        if (value < 0) continue;

        if (high == -1) high = value;

        else {
            buffer[bytes++] = (high << 4) | value;
            high = -1;
        }
    }
    CodecStatus status = codec_decode(
            protocol,
            buffer,
            bytes,
            message,
            error
        );

    free(buffer);

    return status;
}

CodecStatus codec_encode_hex(
    CodecProtocol protocol,
    const void *message,
    char *hex_buffer,
    size_t hex_buffer_size,
    CodecError *error
)
{
    if(!message || !hex_buffer)
        return CODEC_ERROR_INVALID_ARGUMENT;

    uint8_t buffer[4096];
    size_t encoded_size = 0;

    CodecStatus status = codec_encode(
        protocol,
        message,
        buffer,
        sizeof(buffer),
        &encoded_size,
        error
    );

    if(status != CODEC_SUCCESS)
        return status;

    /* Two hex chars per byte + null terminator */
    if(hex_buffer_size < (encoded_size * 2 + 1))
        return CODEC_ERROR_BUFFER_TOO_SMALL;

    static const char hex[] = "0123456789ABCDEF";

    for(size_t i = 0; i < encoded_size; ++i)
    {
        hex_buffer[i * 2]     = hex[(buffer[i] >> 4) & 0x0F];
        hex_buffer[i * 2 + 1] = hex[buffer[i] & 0x0F];
    }

    hex_buffer[encoded_size * 2] = '\0';

    return CODEC_SUCCESS;
}

void codec_print_hex(
    const uint8_t *buffer,
    size_t length
)
{
    for(size_t i = 0; i < length; ++i)
        printf("%02X ", buffer[i]);

    printf("\n");
}

