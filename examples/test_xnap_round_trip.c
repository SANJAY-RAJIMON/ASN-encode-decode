#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "codec.h"
#include "ProtocolIE-Container.h"
#include "XnAP-PDU.h"

int main(void)
{
    printf("========================================\n");
    printf(" XNAP CODEC LIBRARY TEST\n");
    printf("========================================\n\n");

    printf("[1] Allocating message hierarchy...\n");

    XnAP_PDU_t *message = calloc(1, sizeof(*message));
    InitiatingMessage_t *initiating_message = calloc(1, sizeof(*initiating_message));
    ProtocolIE_Container_108P56_t *protocol_ies = calloc(1, sizeof(*protocol_ies));

    assert(message != NULL);
    assert(initiating_message != NULL);
    assert(protocol_ies != NULL);

    message->present = XnAP_PDU_PR_initiatingMessage;
    message->choice.initiatingMessage = initiating_message;

    printf("    ✓ Hierarchy allocated\n");

    printf("[2] Filling IEs...\n");

    initiating_message->procedureCode = ProcedureCode_id_errorIndication;
    initiating_message->criticality = Criticality_ignore;
    initiating_message->value.present = InitiatingMessage__value_PR_ErrorIndication;
    initiating_message->value.choice.ErrorIndication.protocolIEs =
        (struct ProtocolIE_Container *)protocol_ies;

    printf("    ✓ Error Indication configured\n");

    printf("[3] Encoding ASN -> Bytes...\n");

    uint8_t buffer[1024];
    size_t encoded_size = 0;

    CodecStatus status = codec_encode(
        CODEC_XNAP_PDU,
        message,
        buffer,
        sizeof(buffer),
        &encoded_size
    );

    if(status != CODEC_SUCCESS)
    {
        printf("Encode FAILED (%d)\n", status);
        return -1;
    }

    printf("    ✓ Encode Success\n");
    printf("Encoded Size : %zu bytes\n", encoded_size);

    codec_print_hex(buffer, encoded_size);

    printf("\n[4] Encoding ASN -> HEX...\n");

    char hex[4096];

    status = codec_encode_hex(
        CODEC_XNAP_PDU,
        message,
        hex,
        sizeof(hex)
    );

    if(status != CODEC_SUCCESS)
    {
        printf("HEX Encode FAILED\n");
        return -1;
    }

    printf("%s\n", hex);

    printf("\n[5] Decoding Bytes...\n");

    XnAP_PDU_t *decoded = NULL;

    status = codec_decode(
        CODEC_XNAP_PDU,
        buffer,
        encoded_size,
        (void **)&decoded
    );

    if(status != CODEC_SUCCESS)
    {
        printf("Decode FAILED\n");
        return -1;
    }

    printf("    ✓ Decode Success\n");

    printf("\n[6] Decoding HEX...\n");

    XnAP_PDU_t *decoded_hex = NULL;

    status = codec_decode_hex(
        CODEC_XNAP_PDU,
        hex,
        (void **)&decoded_hex
    );

    if(status != CODEC_SUCCESS)
    {
        printf("HEX Decode FAILED\n");
        return -1;
    }

    printf("    ✓ HEX Decode Success\n");

    printf("\n[7] Verifying...\n");

    assert(decoded != NULL);
    assert(decoded_hex != NULL);

    assert(decoded->present == XnAP_PDU_PR_initiatingMessage);
    assert(decoded_hex->present == XnAP_PDU_PR_initiatingMessage);

    printf("✓ Top level choice\n");

    assert(decoded->choice.initiatingMessage->procedureCode == ProcedureCode_id_errorIndication);
    assert(decoded_hex->choice.initiatingMessage->procedureCode == ProcedureCode_id_errorIndication);

    assert(decoded->choice.initiatingMessage->criticality == Criticality_ignore);
    assert(decoded_hex->choice.initiatingMessage->criticality == Criticality_ignore);

    assert(decoded->choice.initiatingMessage->value.present == InitiatingMessage__value_PR_ErrorIndication);
    assert(decoded_hex->choice.initiatingMessage->value.present == InitiatingMessage__value_PR_ErrorIndication);

    printf("✓ Procedure Code and Criticality\n");

    ProtocolIE_Container_108P56_t *d_protocol_ies = (ProtocolIE_Container_108P56_t *)
        decoded->choice.initiatingMessage->value.choice.ErrorIndication.protocolIEs;
    ProtocolIE_Container_108P56_t *h_protocol_ies = (ProtocolIE_Container_108P56_t *)
        decoded_hex->choice.initiatingMessage->value.choice.ErrorIndication.protocolIEs;

    assert(d_protocol_ies != NULL);
    assert(h_protocol_ies != NULL);
    assert(d_protocol_ies->list.count == 0);
    assert(h_protocol_ies->list.count == 0);

    printf("✓ Protocol IEs\n");

    printf("\n✓ ALL FIELD VALIDATIONS PASSED\n");

    printf("\n[8] Freeing...\n");

    codec_free(CODEC_XNAP_PDU, message);
    codec_free(CODEC_XNAP_PDU, decoded);
    codec_free(CODEC_XNAP_PDU, decoded_hex);

    printf("========================================\n");
    printf(" ALL TESTS PASSED\n");
    printf("========================================\n");

    return 0;
}
