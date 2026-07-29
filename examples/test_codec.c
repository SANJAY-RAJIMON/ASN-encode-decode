#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "codec.h"

#include "UL-CCCH-Message.h"
#include "UL-CCCH-MessageType.h"
#include "RRCSetupRequest.h"
#include "RRCSetupRequest-IEs.h"
#include "InitialUE-Identity.h"

int main(void)
{
    printf("========================================\n");
    printf(" ASN CODEC LIBRARY TEST\n");
    printf("========================================\n\n");

    printf("[1] Allocating message hierarchy...\n");

    UL_CCCH_Message_t *msg = calloc(1, sizeof(*msg));
    msg->message = calloc(1, sizeof(*msg->message));

    msg->message->present = UL_CCCH_MessageType_PR_c1;
    msg->message->choice.c1.present =
        UL_CCCH_MessageType__c1_PR_rrcSetupRequest;

    RRCSetupRequest_t *setup =
        &msg->message->choice.c1.choice.rrcSetupRequest;

    setup->rrcSetupRequest =
        calloc(1, sizeof(*setup->rrcSetupRequest));

    setup->rrcSetupRequest->ue_Identity =
        calloc(1, sizeof(InitialUE_Identity_t));

    printf("    ✓ Hierarchy allocated\n");

    printf("[2] Filling InitialUE_Identity...\n");

    InitialUE_Identity_t *identity =
        setup->rrcSetupRequest->ue_Identity;

    identity->present =
        InitialUE_Identity_PR_randomValue;

    identity->choice.randomValue.size = 5;
    identity->choice.randomValue.buf = calloc(5, 1);

    identity->choice.randomValue.buf[0] = 0x12;
    identity->choice.randomValue.buf[1] = 0x34;
    identity->choice.randomValue.buf[2] = 0x56;
    identity->choice.randomValue.buf[3] = 0x78;
    identity->choice.randomValue.buf[4] = 0x9A;

    identity->choice.randomValue.bits_unused = 1;

    printf("    ✓ Random Value configured\n");

    printf("[3] Filling remaining IEs...\n");

    setup->rrcSetupRequest->establishmentCause =
        EstablishmentCause_mo_Data;

    setup->rrcSetupRequest->spare.size = 1;
    setup->rrcSetupRequest->spare.buf = calloc(1,1);
    setup->rrcSetupRequest->spare.buf[0] = 0;
    setup->rrcSetupRequest->spare.bits_unused = 7;

    printf("    ✓ Establishment Cause = MO-DATA\n");

    printf("[4] Encoding ASN -> Bytes...\n");

    uint8_t encoded[1024];
    size_t encoded_size = 0;

    CodecError error;
    CodecStatus status =
        codec_encode(
            CODEC_UL_CCCH_MESSAGE,
            msg,
            encoded,
            sizeof(encoded),
            &encoded_size,
            &error
        );

    if(status != CODEC_SUCCESS)
    {
        printf("Encode FAILED (%d)\n", status);
        return -1;
    }

    printf("    ✓ Encode Success\n");
    printf("Encoded Size : %zu bytes\n", encoded_size);

    codec_print_hex(encoded, encoded_size);

    printf("\n[5] Encoding ASN -> HEX...\n");

    char hex[4096];

    status =
        codec_encode_hex(
            CODEC_UL_CCCH_MESSAGE,
            msg,
            hex,
            sizeof(hex),
            &error
        );

    if(status != CODEC_SUCCESS)
    {
        printf("HEX Encode FAILED\n");
        return -1;
    }

    printf("%s\n", hex);

    printf("\n[6] Decoding Bytes...\n");

    UL_CCCH_Message_t *decoded = NULL;

    status =
        codec_decode(
            CODEC_UL_CCCH_MESSAGE,
            encoded,
            encoded_size,
            (void **)&decoded,
            &error
        );

    if(status != CODEC_SUCCESS)
    {
        printf("Decode FAILED\n");
        return -1;
    }

    printf("    ✓ Decode Success\n");

    printf("\n[7] Decoding HEX...\n");

    UL_CCCH_Message_t *decoded_hex = NULL;

    status =
        codec_decode_hex(
            CODEC_UL_CCCH_MESSAGE,
            hex,
            (void **)&decoded_hex,
            &error
        );

    if(status != CODEC_SUCCESS)
    {
        printf("HEX Decode FAILED\n");
        return -1;
    }

    printf("    ✓ HEX Decode Success\n");

    printf("\n[8] Verifying...\n");
    #include <assert.h>
#include <string.h>

printf("\n[8] Verifying...\n");

assert(decoded != NULL);
assert(decoded_hex != NULL);

/* ---------- Verify top level ---------- */

assert(decoded->message != NULL);
assert(decoded_hex->message != NULL);

assert(decoded->message->present ==
       UL_CCCH_MessageType_PR_c1);

assert(decoded_hex->message->present ==
       UL_CCCH_MessageType_PR_c1);

printf("✓ Top level choice\n");

/* ---------- Verify c1 ---------- */

assert(decoded->message->choice.c1.present ==
       UL_CCCH_MessageType__c1_PR_rrcSetupRequest);

assert(decoded_hex->message->choice.c1.present ==
       UL_CCCH_MessageType__c1_PR_rrcSetupRequest);

printf("✓ Message type\n");

/* ---------- Verify RRCSetupRequest ---------- */

RRCSetupRequest_t *d_setup =
    &decoded->message->choice.c1.choice.rrcSetupRequest;

RRCSetupRequest_t *h_setup =
    &decoded_hex->message->choice.c1.choice.rrcSetupRequest;

assert(d_setup->rrcSetupRequest != NULL);
assert(h_setup->rrcSetupRequest != NULL);

printf("✓ RRCSetupRequest\n");

/* ---------- Verify InitialUEIdentity ---------- */

InitialUE_Identity_t *d_id =
    d_setup->rrcSetupRequest->ue_Identity;

InitialUE_Identity_t *h_id =
    h_setup->rrcSetupRequest->ue_Identity;

assert(d_id != NULL);
assert(h_id != NULL);

assert(d_id->present ==
       InitialUE_Identity_PR_randomValue);

assert(h_id->present ==
       InitialUE_Identity_PR_randomValue);

printf("✓ UE Identity\n");

/* ---------- Verify RandomValue ---------- */

assert(d_id->choice.randomValue.size == 5);
assert(h_id->choice.randomValue.size == 5);

assert(d_id->choice.randomValue.bits_unused == 1);
assert(h_id->choice.randomValue.bits_unused == 1);

assert(memcmp(
    d_id->choice.randomValue.buf,
    identity->choice.randomValue.buf,
    5) == 0);

assert(memcmp(
    h_id->choice.randomValue.buf,
    identity->choice.randomValue.buf,
    5) == 0);

printf("✓ RandomValue\n");

/* ---------- Verify EstablishmentCause ---------- */

assert(
    d_setup->rrcSetupRequest->establishmentCause ==
    EstablishmentCause_mo_Data
);

assert(
    h_setup->rrcSetupRequest->establishmentCause ==
    EstablishmentCause_mo_Data
);

printf("✓ EstablishmentCause\n");

/* ---------- Verify Spare ---------- */

assert(d_setup->rrcSetupRequest->spare.size == 1);
assert(h_setup->rrcSetupRequest->spare.size == 1);

assert(d_setup->rrcSetupRequest->spare.bits_unused == 7);
assert(h_setup->rrcSetupRequest->spare.bits_unused == 7);

assert(d_setup->rrcSetupRequest->spare.buf[0] == 0);
assert(h_setup->rrcSetupRequest->spare.buf[0] == 0);

printf("✓ Spare\n");

/* ---------- Dump decoded values ---------- */

printf("\nDecoded RandomValue : ");

for(size_t i = 0; i < d_id->choice.randomValue.size; i++)
    printf("%02X ", d_id->choice.randomValue.buf[i]);

printf("\n");

printf("Decoded Cause       : %ld\n",
    d_setup->rrcSetupRequest->establishmentCause);

printf("Decoded Spare       : %02X\n",
    d_setup->rrcSetupRequest->spare.buf[0]);

printf("\n✓ ALL FIELD VALIDATIONS PASSED\n");
    printf("Cause = %ld\n",
        decoded->message
            ->choice.c1.choice.rrcSetupRequest
            .rrcSetupRequest
            ->establishmentCause);

    printf("\n[9] Freeing...\n");

    codec_free(
        CODEC_UL_CCCH_MESSAGE,
        msg
    );

    codec_free(
        CODEC_UL_CCCH_MESSAGE,
        decoded
    );

    codec_free(
        CODEC_UL_CCCH_MESSAGE,
        decoded_hex
    );

    printf("========================================\n");
    printf(" ALL TESTS PASSED\n");
    printf("========================================\n");

    return 0;
}