#ifndef PROTOCOL_REGISTRY_H
#define PROTOCOL_REGISTRY_H

#include "codec_types.h"
#include <asn_application.h>

#include <aper_decoder.h>
#include <aper_encoder.h>
#include <per_decoder.h>
#include <per_encoder.h>

typedef asn_enc_rval_t(*CodecEncodeFn)(
    const asn_TYPE_descriptor_t *td,
    const asn_per_constraints_t *constraints,
    const void *sptr,
    void *buffer,
    size_t buffer_size
);

typedef asn_dec_rval_t(*CodecDecodeFn)(
    const asn_codec_ctx_t *opt_codec_ctx,
    const asn_TYPE_descriptor_t *td,
    void **struct_ptr, 
    const void *buffer,
    size_t size,
    int skip_bits,
    int unused_bits
);

typedef struct
{
    CodecEncodeFn encode;
    CodecDecodeFn decode;
} CodecTransferSyntaxOps;


typedef struct
{
    CodecProtocol protocol;

    const asn_TYPE_descriptor_t *descriptor;

    const CodecTransferSyntaxOps *ops;

} ProtocolEntry;

const ProtocolEntry *
protocol_registry_lookup(
    CodecProtocol protocol
);

#endif
