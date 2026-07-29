#include "protocol_registry.h"

#if defined(CODEC_PROTOCOL_RRC)
#include "UL-CCCH-Message.h"
#include "DL-CCCH-Message.h"

#include "UL-DCCH-Message.h"
#include "DL-DCCH-Message.h"

#include "BCCH-BCH-Message.h"
#include "BCCH-DL-SCH-Message.h"

#include "PCCH-Message.h"
#endif

#if defined(CODEC_PROTOCOL_NGAP)
#include "NGAP-PDU.h"
#endif

#if defined(CODEC_PROTOCOL_X2AP)
#include "X2AP-PDU.h"
#endif

// #if defined(CODEC_PROTOCOL_S1AP)
// #include "S1AP-PDU.h"
// #endif

#if defined(CODEC_PROTOCOL_XNAP)
#include "XnAP-PDU.h"
#endif

static const CodecTransferSyntaxOps aper_ops =
{
    aper_encode_to_buffer,
    aper_decode
};

static const CodecTransferSyntaxOps uper_ops =
{
    uper_encode_to_buffer,
    uper_decode
};

static const ProtocolEntry registry[] =
{
#if defined(CODEC_PROTOCOL_RRC)
    {
        CODEC_UL_CCCH_MESSAGE,
        &asn_DEF_UL_CCCH_Message,
        &uper_ops
    },

    {
        CODEC_DL_CCCH_MESSAGE,
        &asn_DEF_DL_CCCH_Message,
        &uper_ops
    },

    {
        CODEC_UL_DCCH_MESSAGE,
        &asn_DEF_UL_DCCH_Message,
        &uper_ops
    },

    {
        CODEC_DL_DCCH_MESSAGE,
        &asn_DEF_DL_DCCH_Message,
        &uper_ops
    },

    {
        CODEC_BCCH_BCH_MESSAGE,
        &asn_DEF_BCCH_BCH_Message,
        &uper_ops
    },

    {
        CODEC_BCCH_DL_SCH_MESSAGE,
        &asn_DEF_BCCH_DL_SCH_Message,
        &uper_ops
    },

    {
        CODEC_PCCH_MESSAGE,
        &asn_DEF_PCCH_Message,
        &uper_ops
    },
#endif

#if defined(CODEC_PROTOCOL_NGAP)
    {
        CODEC_NGAP_PDU,
        &asn_DEF_NGAP_PDU,
        &aper_ops
    },
#endif

#if defined(CODEC_PROTOCOL_X2AP)
    {
        CODEC_X2AP_PDU,
        &asn_DEF_X2AP_PDU,
        &aper_ops
    },
#endif

// #if defined(CODEC_PROTOCOL_S1AP)
//     {
//         CODEC_S1AP_PDU,
//         &asn_DEF_S1AP_PDU,
//         &aper_ops
//     },
// #endif

#if defined(CODEC_PROTOCOL_XNAP)
    {
        CODEC_XNAP_PDU,
        &asn_DEF_XnAP_PDU,
        &aper_ops
    },
#endif
};  

const ProtocolEntry *
protocol_registry_lookup(
    CodecProtocol protocol
)
{
    size_t count = sizeof(registry) / sizeof(registry[0]);

    for(size_t i = 0; i < count; ++i)
    {
        if(registry[i].protocol == protocol)
            return &registry[i];
    }

    return NULL;
}
