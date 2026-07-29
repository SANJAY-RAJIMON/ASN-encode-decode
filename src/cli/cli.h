#ifndef CLI_H
#define CLI_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "protocol_map.h"

#ifdef __cplusplus
extern "C" {
#endif

// Parse hex string into a newly allocated binary buffer.
// Returns 0 on success, -1 on error.
int cli_parse_hex(const char *hex_str, uint8_t **out_buffer, size_t *out_size);

// Read binary file into a newly allocated buffer.
// Returns 0 on success, -1 on error.
int cli_parse_file(const char *filepath, uint8_t **out_buffer, size_t *out_size);

// Attempt to infer protocol from filename.
// Returns true if inferred, false otherwise.
bool cli_infer_protocol(const char *filename, codec_protocol_t *out_protocol);

// Map codec status to a string error message
const char *cli_status_to_string(CodecStatus status);

#ifdef __cplusplus
}
#endif

#endif // CLI_H
