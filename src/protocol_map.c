#include "protocol_map.h"
#include <string.h>
#include <ctype.h>
#include <stddef.h>

typedef struct {
    const char *name;
    codec_protocol_t protocol;
} ProtocolMapping;

static const ProtocolMapping g_protocol_mappings[] = {
    {"NGAP", CODEC_NGAP_PDU},
    {"X2AP", CODEC_X2AP_PDU},
    {"XNAP", CODEC_XNAP_PDU},
    {"UL_CCCH", CODEC_UL_CCCH_MESSAGE},
    {"DL_CCCH", CODEC_DL_CCCH_MESSAGE},
    {"UL_DCCH", CODEC_UL_DCCH_MESSAGE},
    {"DL_DCCH", CODEC_DL_DCCH_MESSAGE},
    {"BCCH_BCH", CODEC_BCCH_BCH_MESSAGE},
    {"BCCH_DL_SCH", CODEC_BCCH_DL_SCH_MESSAGE},
    {"PCCH", CODEC_PCCH_MESSAGE}
};

static const size_t g_num_mappings = sizeof(g_protocol_mappings) / sizeof(g_protocol_mappings[0]);

static int case_insensitive_match(const char *a, const char *b) {
    if (!a || !b) return 0;
    while (*a && *b) {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) {
            return 0;
        }
        a++;
        b++;
    }
    return (*a == '\0' && *b == '\0');
}

codec_protocol_t codec_protocol_from_name(const char *name) {
    if (!name) return CODEC_PROTOCOL_UNKNOWN;

    for (size_t i = 0; i < g_num_mappings; i++) {
        if (case_insensitive_match(name, g_protocol_mappings[i].name)) {
            return g_protocol_mappings[i].protocol;
        }
    }
    return CODEC_PROTOCOL_UNKNOWN;
}

const char *codec_protocol_name(codec_protocol_t protocol) {
    for (size_t i = 0; i < g_num_mappings; i++) {
        if (g_protocol_mappings[i].protocol == protocol) {
            return g_protocol_mappings[i].name;
        }
    }
    return "UNKNOWN";
}

bool codec_protocol_is_supported(const char *name) {
    return codec_protocol_from_name(name) != CODEC_PROTOCOL_UNKNOWN;
}
