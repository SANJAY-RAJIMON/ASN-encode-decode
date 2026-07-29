#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "codec.h"
#include "RRCSetupRequest.h"


int main(void){
    RRCSetupRequest_t request;
    memset(&request, 0, sizeof(request));

    //UE identity with random value choice
    request.rrcSetupRequest.ue_Identity.present = InitialUE_Identity_PR_randomValue;
    BIT_STRING_t *randomValue = &request.rrcSetupRequest.ue_Identity.choice.randomValue;
    randomValue->buf = calloc(5,1);
    randomValue->size = 5;
    randomValue->bits_unused = 1; //39 bit value so 1 out of 40 unused

    randomValue->buf[0] = 0x12;
    randomValue->buf[1] = 0x34;
    randomValue->buf[2] = 0x56;
    randomValue->buf[3] = 0x78;
    randomValue->buf[4] = 0x9A;

    //establishment cause 
    request.rrcSetupRequest.establishmentCause = EstablishmentCause_mo_Data;

    //spare 1 bit
    request.rrcSetupRequest.spare.buf = calloc(1,1);
    request.rrcSetupRequest.spare.size = 1;
    request.rrcSetupRequest.spare.bits_unused = 7;
    request.rrcSetupRequest.spare.buf[0] = 0;

    printf("========== ORIGINAL MESSAGE ==========\n");

    printf("Establishment Cause : %ld\n",
        request.rrcSetupRequest.establishmentCause);

    printf("UE Identity Type : %d\n",
        request.rrcSetupRequest.ue_Identity.present);

    printf("Random Value : ");

    for (int i = 0; i < randomValue->size; i++)
        printf("%02X ", randomValue->buf[i]);

    printf("\n\n");
    //start encoding
    uint8_t buffer[1024];
    size_t encoded_size;

    CodecStatus status = codec_encode(
        CODEC_RRC_SETUP_REQUEST,
        &request,
        buffer,
        sizeof(buffer),
        &encoded_size
    );
    if (status != CODEC_SUCCESS){
        printf("Encoding failed with status: %d\n", status);
        free(randomValue->buf);
        return 1;
    }

    printf("Encoding Successful\n\n");

    printf("Encoded Size : %zu bytes\n\n", encoded_size);

    printf("Encoded Bytes : ");

    for(size_t i = 0; i < encoded_size; i++)
        printf("%02X ", buffer[i]);

    printf("\n\n");

    //Decode section
    RRCSetupRequest_t *decoded = NULL;
    status = codec_decode(
        CODEC_RRC_SETUP_REQUEST,
        buffer,
        encoded_size, 
        (void**)&decoded
    );

    if (status != CODEC_SUCCESS){
        printf("Decoding failed with status: %d\n", status);
        free(randomValue->buf);
        return 1;
    }

    printf("========== DECODED MESSAGE ==========\n");

    printf("Establishment Cause : %ld\n",
        decoded->rrcSetupRequest.establishmentCause);

    printf("UE Identity Type : %d\n",
        decoded->rrcSetupRequest.ue_Identity.present);

    printf("Random Value : ");

    BIT_STRING_t *rv =
    &decoded->rrcSetupRequest.ue_Identity.choice.randomValue;

    for (int i = 0; i < rv->size; i++)
    printf("%02X ", rv->buf[i]);

    printf("\n\n");
    printf("========== VERIFICATION ==========\n");

int success = 1;

/* Establishment Cause */
if (request.rrcSetupRequest.establishmentCause ==
    decoded->rrcSetupRequest.establishmentCause)
{
    printf("[PASS] Establishment Cause matches\n");
}
else
{
    printf("[FAIL] Establishment Cause mismatch\n");
    success = 0;
}

/* UE Identity Type */
if (request.rrcSetupRequest.ue_Identity.present ==
    decoded->rrcSetupRequest.ue_Identity.present)
{
    printf("[PASS] UE Identity Type matches\n");
}
else
{
    printf("[FAIL] UE Identity Type mismatch\n");
    success = 0;
}

    /* Random Value */
    if (request.rrcSetupRequest.ue_Identity.choice.randomValue.size ==
        rv->size &&
        memcmp(
            request.rrcSetupRequest.ue_Identity.choice.randomValue.buf,
            rv->buf,
            rv->size) == 0)
    {
        printf("[PASS] Random Value matches\n");
    }
    else
    {
        printf("[FAIL] Random Value mismatch\n");
        success = 0;
    }

    if (success)
        printf("\nEncode/Decode Verification Successful\n");
    else
        printf("\nEncode/Decode Verification Failed\n");
    // Cleanup
    free(request.rrcSetupRequest.ue_Identity.choice.randomValue.buf);
    free(request.rrcSetupRequest.spare.buf);

    codec_free(
        CODEC_RRC_SETUP_REQUEST,
        decoded);

    return 0;


}
