#ifndef CODEC_TYPES_H
#define CODEC_TYPES_H

#include <stddef.h>
#include <stdint.h>

typedef enum
{
    CODEC_SUCCESS = 0,
    CODEC_ERROR_INVALID_PROTOCOL,
    CODEC_ERROR_INVALID_ARGUMENT,
    CODEC_ERROR_ENCODE_FAILED,
    CODEC_ERROR_DECODE_FAILED,
    CODEC_ERROR_BUFFER_TOO_SMALL,
    CODEC_ERROR_UNSUPPORTED_ENCODING,
    CODEC_ERROR_VALIDATION_FAILED
} CodecStatus;

typedef struct {
    char message[256];
    const char *failed_type;
} CodecError;

/* Transfer Syntax (Encoding Rule) */
typedef enum
{
    CODEC_TS_UPER = 0,
    CODEC_TS_APER
} CodecTransferSyntax;

/* Supported Top-Level PDUs */
typedef enum
{
    /* NR RRC */

    CODEC_UL_CCCH_MESSAGE = 0,
    CODEC_DL_CCCH_MESSAGE,

    CODEC_UL_DCCH_MESSAGE,
    CODEC_DL_DCCH_MESSAGE,

    CODEC_BCCH_BCH_MESSAGE,
    CODEC_BCCH_DL_SCH_MESSAGE,

    CODEC_PCCH_MESSAGE,

     /* NGAP */
    CODEC_NGAP_PDU,

    /* S1AP */
    // CODEC_S1AP_PDU,

    /* X2AP */
    CODEC_X2AP_PDU,

    /* XNAP */
    CODEC_XNAP_PDU

} CodecProtocol;

#endif