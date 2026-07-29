#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "codec.h"

void print_usage(const char *prog_name)
{
    printf("Usage: %s <PROTOCOL> <HEX_STRING>\n", prog_name);
    printf("Supported Protocols:\n");
    printf("  RRC\n");
    printf("  NGAP\n");
    printf("  XNAP\n");
    printf("  X2AP\n");
    printf("\nExample:\n");
    printf("  %s NGAP 00094003000000\n", prog_name);
}

int main(int argc, char **argv)
{
    if (argc != 3) {
        print_usage(argv[0]);
        return 1;
    }

    const char *protocol_str = argv[1];
    const char *hex_string = argv[2];

    CodecProtocol protocol;

    if (strcmp(protocol_str, "RRC") == 0) {
        protocol = CODEC_DL_DCCH_MESSAGE; /* Defaulting to DL_DCCH for RRC demo */
    } else if (strcmp(protocol_str, "NGAP") == 0) {
        protocol = CODEC_NGAP_PDU;
    } else if (strcmp(protocol_str, "XNAP") == 0) {
        protocol = CODEC_XNAP_PDU;
    } else if (strcmp(protocol_str, "X2AP") == 0) {
        protocol = CODEC_X2AP_PDU;
    } else {
        printf("Error: Unsupported protocol '%s'\n", protocol_str);
        print_usage(argv[0]);
        return 1;
    }

    void *decoded_message = NULL;
    CodecError error;

    CodecStatus status = codec_decode_hex(
        protocol,
        hex_string,
        &decoded_message,
        &error
    );

    if (status != CODEC_SUCCESS) {
        printf("Error: Failed to decode hex string (CodecStatus: %d)\n", status);
        if (error.message[0] != '\0') {
            printf("Details: %s\n", error.message);
        }
        return 1;
    }

    // Now validate it
    status = codec_validate(protocol, decoded_message, &error);
    if (status != CODEC_SUCCESS) {
        printf("Error: Message decoded but failed ASN.1 constraint validation (CodecStatus: %d)\n", status);
        if (error.message[0] != '\0') {
            printf("Validation details: %s\n", error.message);
        }
        codec_free(protocol, decoded_message);
        return 1;
    }

    printf("Successfully decoded message:\n");
    printf("========================================\n");
    codec_print(protocol, stdout, decoded_message);
    printf("========================================\n");

    codec_free(protocol, decoded_message);

    return 0;
}
