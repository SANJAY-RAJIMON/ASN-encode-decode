#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codec.h"

void print_usage(const char *prog_name) {
    printf("Usage: %s <protocol> <hex_string>\n", prog_name);
    printf("Supported protocols: NGAP, X2AP, XNAP, UL_CCCH, DL_CCCH, UL_DCCH, DL_DCCH, BCCH_BCH, BCCH_DL_SCH, PCCH\n");
}

int main(int argc, char **argv) {
    if (argc != 3) {
        print_usage(argv[0]);
        return 1;
    }

    const char *protocol_str = argv[1];
    const char *hex_str = argv[2];

    CodecProtocol protocol;
    if (strcmp(protocol_str, "NGAP") == 0) protocol = CODEC_NGAP_PDU;
    else if (strcmp(protocol_str, "X2AP") == 0) protocol = CODEC_X2AP_PDU;
    else if (strcmp(protocol_str, "XNAP") == 0) protocol = CODEC_XNAP_PDU;
    else if (strcmp(protocol_str, "UL_CCCH") == 0) protocol = CODEC_UL_CCCH_MESSAGE;
    else if (strcmp(protocol_str, "DL_CCCH") == 0) protocol = CODEC_DL_CCCH_MESSAGE;
    else if (strcmp(protocol_str, "UL_DCCH") == 0) protocol = CODEC_UL_DCCH_MESSAGE;
    else if (strcmp(protocol_str, "DL_DCCH") == 0) protocol = CODEC_DL_DCCH_MESSAGE;
    else if (strcmp(protocol_str, "BCCH_BCH") == 0) protocol = CODEC_BCCH_BCH_MESSAGE;
    else if (strcmp(protocol_str, "BCCH_DL_SCH") == 0) protocol = CODEC_BCCH_DL_SCH_MESSAGE;
    else if (strcmp(protocol_str, "PCCH") == 0) protocol = CODEC_PCCH_MESSAGE;
    else {
        printf("Unknown protocol: %s\n", protocol_str);
        return 1;
    }

    void *decoded_message = NULL;
    CodecError error;

    CodecStatus status = codec_decode_hex(protocol, hex_str, &decoded_message, &error);

    if (status != CODEC_SUCCESS) {
        printf("Decode failed: %s\n", error.message);
        return 1;
    }

    printf("Decode Successful!\n");

    if (codec_validate(protocol, decoded_message, &error) != CODEC_SUCCESS) {
        printf("Validation failed on field '%s': %s\n", error.failed_type_name, error.message);
    } else {
        printf("Validation Passed!\n");
    }

    codec_free(protocol, decoded_message);
    return 0;
}
