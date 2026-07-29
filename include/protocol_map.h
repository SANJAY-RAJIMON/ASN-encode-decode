#ifndef PROTOCOL_MAP_H
#define PROTOCOL_MAP_H

#include <stdbool.h>
#include "codec_types.h"

#ifdef __cplusplus
extern "C" {
#endif

codec_protocol_t codec_protocol_from_name(const char *name);

const char *codec_protocol_name(codec_protocol_t protocol);

bool codec_protocol_is_supported(const char *name);

#ifdef __cplusplus
}
#endif

#endif // PROTOCOL_MAP_H
