#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include "codec.h"

#ifdef __cplusplus
extern "C" {
#endif

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size == 0) return 0;

    void *decoded_message = NULL;
    CodecError error;

    CodecStatus status = codec_decode(
        CODEC_NGAP_PDU,
        data,
        size,
        &decoded_message,
        &error
    );

    if (status == CODEC_SUCCESS) {
        codec_validate(CODEC_NGAP_PDU, decoded_message, &error);

        uint8_t encode_buffer[4096];
        size_t encoded_size = 0;
        codec_encode(
            CODEC_NGAP_PDU,
            decoded_message,
            encode_buffer,
            sizeof(encode_buffer),
            &encoded_size,
            &error
        );
    }

    codec_free(CODEC_NGAP_PDU, decoded_message);
    return 0;
}

#ifdef __cplusplus
}
#endif
