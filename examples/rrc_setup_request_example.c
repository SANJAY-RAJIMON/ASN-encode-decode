#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "codec.h"
#include "UL-CCCH-Message.h"
#include "RRCSetupRequest.h"

int main(void){
    UL_CCCH_Message_t *msg = calloc(1, sizeof(UL_CCCH_Message_t));

    msg->message = calloc(1, sizeof(*msg->message));
    msg->message->present = UL_CCCH_MessageType_PR_c1;
    msg->message->choice.c1.present = UL_CCCH_MessageType__c1_PR_rrcSetupRequest;

    RRCSetupRequest_t *request = &msg->message->choice.c1.choice.rrcSetupRequest;
    request->rrcSetupRequest = calloc(1, sizeof(*request->rrcSetupRequest));

    //UE identity with random value choice
    request->rrcSetupRequest->ue_Identity = calloc(1, sizeof(*request->rrcSetupRequest->ue_Identity));
    request->rrcSetupRequest->ue_Identity->present = InitialUE_Identity_PR_randomValue;
    BIT_STRING_t *randomValue = &request->rrcSetupRequest->ue_Identity->choice.randomValue;
    randomValue->buf = calloc(5,1);
    randomValue->size = 5;
    randomValue->bits_unused = 1; //39 bit value so 1 out of 40 unused

    randomValue->buf[0] = 0x12;
    randomValue->buf[1] = 0x34;
    randomValue->buf[2] = 0x56;
    randomValue->buf[3] = 0x78;
    randomValue->buf[4] = 0x9A;

    //establishment cause 
    request->rrcSetupRequest->establishmentCause = EstablishmentCause_mo_Data;

    //spare 1 bit
    request->rrcSetupRequest->spare.buf = calloc(1,1);
    request->rrcSetupRequest->spare.size = 1;
    request->rrcSetupRequest->spare.bits_unused = 7;
    request->rrcSetupRequest->spare.buf[0] = 0;

    printf("========== ORIGINAL MESSAGE ==========\n");

    printf("Establishment Cause : %ld\n",
        request->rrcSetupRequest->establishmentCause);

    printf("UE Identity Type : %d\n",
        request->rrcSetupRequest->ue_Identity->present);

    printf("Random Value : ");

    for (int i = 0; i < randomValue->size; i++)
        printf("%02X ", randomValue->buf[i]);

    printf("\n\n");
    //start encoding
    uint8_t buffer[1024];
    size_t encoded_size;
    CodecError error;

    CodecStatus status = codec_encode(
        CODEC_UL_CCCH_MESSAGE,
        msg,
        buffer,
        sizeof(buffer),
        &encoded_size,
        &error
    );
    if (status != CODEC_SUCCESS){
        printf("Encoding failed with status: %d, msg: %s\n", status, error.message);
        free(randomValue->buf);
        free(request->rrcSetupRequest->spare.buf);
        free(request->rrcSetupRequest);
        free(msg->message);
        return 1;
    }

    printf("Encoding Successful\n\n");
    printf("Encoded Size : %zu bytes\n\n", encoded_size);
    printf("Encoded Bytes : ");
    for(size_t i = 0; i < encoded_size; i++)
        printf("%02X ", buffer[i]);
    printf("\n\n");

    //Decode section
    UL_CCCH_Message_t *decoded_msg = NULL;
    status = codec_decode(
        CODEC_UL_CCCH_MESSAGE,
        buffer,
        encoded_size, 
        (void**)&decoded_msg,
        &error
    );

    if (status != CODEC_SUCCESS){
        printf("Decoding failed with status: %d, msg: %s\n", status, error.message);
        free(randomValue->buf);
        free(request->rrcSetupRequest->spare.buf);
        free(request->rrcSetupRequest);
        free(msg->message);
        return 1;
    }

    printf("========== DECODED MESSAGE ==========\n");

    RRCSetupRequest_t *decoded_request = &decoded_msg->message->choice.c1.choice.rrcSetupRequest;

    printf("Establishment Cause : %ld\n",
        decoded_request->rrcSetupRequest->establishmentCause);

    printf("UE Identity Type : %d\n",
        decoded_request->rrcSetupRequest->ue_Identity->present);

    printf("Random Value : ");

    BIT_STRING_t *rv =
    &decoded_request->rrcSetupRequest->ue_Identity->choice.randomValue;

    for (int i = 0; i < rv->size; i++)
        printf("%02X ", rv->buf[i]);

    printf("\n\n");
    printf("========== VERIFICATION ==========\n");

    int success = 1;

    /* Establishment Cause */
    if (request->rrcSetupRequest->establishmentCause ==
        decoded_request->rrcSetupRequest->establishmentCause)
    {
        printf("[PASS] Establishment Cause matches\n");
    }
    else
    {
        printf("[FAIL] Establishment Cause mismatch\n");
        success = 0;
    }

    /* UE Identity Type */
    if (request->rrcSetupRequest->ue_Identity->present ==
        decoded_request->rrcSetupRequest->ue_Identity->present)
    {
        printf("[PASS] UE Identity Type matches\n");
    }
    else
    {
        printf("[FAIL] UE Identity Type mismatch\n");
        success = 0;
    }

    /* Random Value */
    if (request->rrcSetupRequest->ue_Identity->choice.randomValue.size ==
        rv->size &&
        memcmp(
            request->rrcSetupRequest->ue_Identity->choice.randomValue.buf,
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
    codec_free(CODEC_UL_CCCH_MESSAGE, msg);
    codec_free(CODEC_UL_CCCH_MESSAGE, decoded_request);

    codec_free(
        CODEC_UL_CCCH_MESSAGE,
        decoded_msg);

    return 0;
}
