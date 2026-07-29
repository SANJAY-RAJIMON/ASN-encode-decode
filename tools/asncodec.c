#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "codec.h"
#include "protocol_map.h"
#include "protocol_registry.h"
#include "cli.h"

#define VERSION "1.0.0"

static void print_help() {
    printf("asncodec - ASN.1 Encoder/Decoder CLI\n\n");
    printf("Usage:\n");
    printf("  asncodec list\n");
    printf("  asncodec version\n");
    printf("  asncodec help\n");
    printf("  asncodec decode [--protocol PROTOCOL] [--hex HEX | --file FILE | FILE] [--xml]\n");
    printf("  asncodec validate [--protocol PROTOCOL] --file FILE\n");
    printf("  asncodec encode [--protocol PROTOCOL] --xml FILE [--out-hex FILE | --out-bin FILE]\n\n");
}

static void print_version() {
    printf("asncodec version %s\n", VERSION);
}

static void print_list() {
    printf("Supported protocols\n");
    printf("-------------------\n");
    printf("NGAP\n");
    printf("X2AP\n");
    printf("XNAP\n");
    printf("UL_CCCH\n");
    printf("DL_CCCH\n");
    printf("UL_DCCH\n");
    printf("DL_DCCH\n");
    printf("BCCH_BCH\n");
    printf("BCCH_DL_SCH\n");
    printf("PCCH\n");
}

static int cmd_decode_validate(int argc, char **argv, bool validate_only) {
    const char *protocol_str = NULL;
    const char *hex_str = NULL;
    const char *file_str = NULL;
    bool out_xml = false;

    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "--protocol") == 0 && i + 1 < argc) {
            protocol_str = argv[++i];
        } else if (strcmp(argv[i], "--hex") == 0 && i + 1 < argc) {
            hex_str = argv[++i];
        } else if (strcmp(argv[i], "--file") == 0 && i + 1 < argc) {
            file_str = argv[++i];
        } else if (strcmp(argv[i], "--xml") == 0) {
            out_xml = true;
        } else if (argv[i][0] != '-') {
            file_str = argv[i];
        } else {
            printf("Unknown argument: %s\n", argv[i]);
            return 1;
        }
    }

    if (!hex_str && !file_str) {
        printf("Error: Must provide --hex or --file (or just pass the file).\n");
        return 1;
    }
    if (hex_str && file_str) {
        printf("Error: Cannot provide both --hex and --file.\n");
        return 1;
    }
    if (validate_only && !file_str) {
        printf("Error: validate command requires a file.\n");
        return 1;
    }

    codec_protocol_t protocol = CODEC_PROTOCOL_UNKNOWN;

    if (protocol_str) {
        protocol = codec_protocol_from_name(protocol_str);
    } else if (file_str) {
        if (!cli_infer_protocol(file_str, &protocol)) {
            printf("Error: Could not infer protocol from filename. Please use --protocol.\n");
            return 1;
        }
    }

    if (protocol == CODEC_PROTOCOL_UNKNOWN) {
        printf("Unknown protocol\n");
        return 1;
    }

    uint8_t *buffer = NULL;
    size_t buffer_size = 0;

    if (hex_str) {
        if (cli_parse_hex(hex_str, &buffer, &buffer_size) != 0) {
            printf("Error: Invalid hex string.\n");
            return 1;
        }
    } else {
        if (cli_parse_file(file_str, &buffer, &buffer_size) != 0) {
            printf("Error: Could not read file %s\n", file_str);
            return 1;
        }
    }

    void *message = NULL;
    CodecError error;

    CodecStatus status = codec_decode(protocol, buffer, buffer_size, &message, &error);
    
    if (status != CODEC_SUCCESS) {
        printf("Decode failed: %s (%s)\n", cli_status_to_string(status), error.message);
        free(buffer);
        return 1;
    }

    if (!validate_only) {
        printf("Decode successful\n");
        const ProtocolEntry *entry = protocol_registry_lookup(protocol);
        if (entry && entry->descriptor) {
            if (out_xml) {
                xer_fprint(stdout, entry->descriptor, message);
            } else {
                asn_fprint(stdout, entry->descriptor, message);
            }
        }
    }

    CodecStatus vstatus = codec_validate(protocol, message, &error);
    if (vstatus != CODEC_SUCCESS) {
        printf("Validation failed: %s on field '%s' (%s)\n", 
               cli_status_to_string(vstatus), error.failed_type_name, error.message);
        codec_free(protocol, message);
        free(buffer);
        return 1;
    }

    if (validate_only) {
        printf("Validation successful\n");
    }

    codec_free(protocol, message);
    free(buffer);
    return 0;
}

static int cmd_encode(int argc, char **argv) {
    const char *protocol_str = NULL;
    const char *xml_file = NULL;
    const char *out_hex = NULL;
    const char *out_bin = NULL;

    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "--protocol") == 0 && i + 1 < argc) {
            protocol_str = argv[++i];
        } else if (strcmp(argv[i], "--xml") == 0 && i + 1 < argc) {
            xml_file = argv[++i];
        } else if (strcmp(argv[i], "--out-hex") == 0 && i + 1 < argc) {
            out_hex = argv[++i];
        } else if (strcmp(argv[i], "--out-bin") == 0 && i + 1 < argc) {
            out_bin = argv[++i];
        } else if (argv[i][0] != '-') {
            if (!xml_file) xml_file = argv[i];
            else {
                printf("Unknown argument: %s\n", argv[i]);
                return 1;
            }
        } else {
            printf("Unknown argument: %s\n", argv[i]);
            return 1;
        }
    }

    if (!xml_file) {
        printf("Error: Must provide an XML file to encode.\n");
        return 1;
    }

    if (out_hex && out_bin) {
        printf("Error: Cannot specify both --out-hex and --out-bin.\n");
        return 1;
    }

    codec_protocol_t protocol = CODEC_PROTOCOL_UNKNOWN;

    if (protocol_str) {
        protocol = codec_protocol_from_name(protocol_str);
    } else {
        if (!cli_infer_protocol(xml_file, &protocol)) {
            printf("Error: Could not infer protocol from filename. Please use --protocol.\n");
            return 1;
        }
    }

    if (protocol == CODEC_PROTOCOL_UNKNOWN) {
        printf("Unknown protocol\n");
        return 1;
    }

    uint8_t *xml_buffer = NULL;
    size_t xml_size = 0;

    if (cli_parse_file(xml_file, &xml_buffer, &xml_size) != 0) {
        printf("Error: Could not read XML file %s\n", xml_file);
        return 1;
    }

    char *safe_xml = (char*)malloc(xml_size + 1);
    if (!safe_xml) {
        free(xml_buffer);
        return 1;
    }
    memcpy(safe_xml, xml_buffer, xml_size);
    safe_xml[xml_size] = '\0';

    void *message = NULL;
    CodecError error;

    CodecStatus status = codec_decode_xml(protocol, safe_xml, xml_size, &message, &error);
    free(xml_buffer);
    free(safe_xml);

    if (status != CODEC_SUCCESS) {
        printf("XML Decode failed: %s (%s)\n", cli_status_to_string(status), error.message);
        return 1;
    }

    printf("XML Decode successful\n");
    const ProtocolEntry *entry = protocol_registry_lookup(protocol);
    if (entry && entry->descriptor) {
        asn_fprint(stdout, entry->descriptor, message);
    }

    CodecStatus vstatus = codec_validate(protocol, message, &error);
    if (vstatus != CODEC_SUCCESS) {
        printf("Validation failed on field '%s': %s\n", error.failed_type_name, error.message);
        codec_free(protocol, message);
        return 1;
    }

    uint8_t out_buf[8192];
    size_t out_size = 0;

    status = codec_encode(protocol, message, out_buf, sizeof(out_buf), &out_size, &error);
    if (status != CODEC_SUCCESS) {
        printf("Encode failed: %s (%s)\n", cli_status_to_string(status), error.message);
        codec_free(protocol, message);
        return 1;
    }

    if (out_bin) {
        FILE *f = fopen(out_bin, "wb");
        if (!f) {
            printf("Error: Could not open output file %s\n", out_bin);
            codec_free(protocol, message);
            return 1;
        }
        fwrite(out_buf, 1, out_size, f);
        fclose(f);
        printf("Successfully wrote %zu bytes to %s\n", out_size, out_bin);
    } else if (out_hex) {
        FILE *f = fopen(out_hex, "w");
        if (!f) {
            printf("Error: Could not open output file %s\n", out_hex);
            codec_free(protocol, message);
            return 1;
        }
        for(size_t i = 0; i < out_size; ++i) {
            fprintf(f, "%02X", out_buf[i]);
        }
        fprintf(f, "\n");
        fclose(f);
        printf("Successfully wrote hex to %s\n", out_hex);
    } else {
        for(size_t i = 0; i < out_size; ++i) {
            printf("%02X", out_buf[i]);
        }
        printf("\n");
    }

    codec_free(protocol, message);
    return 0;
}


int main(int argc, char **argv) {
    if (argc < 2) {
        print_help();
        return 1;
    }

    const char *cmd = argv[1];

    if (strcmp(cmd, "list") == 0) {
        print_list();
    } else if (strcmp(cmd, "version") == 0) {
        print_version();
    } else if (strcmp(cmd, "help") == 0) {
        print_help();
    } else if (strcmp(cmd, "decode") == 0) {
        return cmd_decode_validate(argc, argv, false);
    } else if (strcmp(cmd, "validate") == 0) {
        return cmd_decode_validate(argc, argv, true);
    } else if (strcmp(cmd, "encode") == 0) {
        return cmd_encode(argc, argv);
    } else {
        printf("Unknown command: %s\n", cmd);
        print_help();
        return 1;
    }

    return 0;
}
