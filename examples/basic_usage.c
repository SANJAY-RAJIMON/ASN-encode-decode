/**
 * @file basic_usage.c
 * @brief An ultra-simple tutorial on how to use the Airspan ASN.1 Codec API.
 * 
 * This example demonstrates:
 * 1. How to manually populate an ASN.1 C structure.
 * 2. How to validate its contents against 3GPP constraints.
 * 3. How to cleanly handle CodecErrors.
 * 4. How to encode it into a byte buffer.
 * 5. How to decode it back into a C structure.
 */

#include <stdio.h>
#include <stdlib.h>

#include "codec.h"
#include "NGAP-PDU.h"
#include "ProtocolIE-Container.h"

int main(void) {
    printf("=== Airspan ASN.1 Codec Tutorial ===\n\n");

    // ---------------------------------------------------------
    // 1. POPULATE AN ASN.1 STRUCTURE
    // ---------------------------------------------------------
    printf("[1] Allocating a simple NGAP Error Indication message...\n");
    
    // Allocate all components using calloc (asn1c requires this!)
    NGAP_PDU_t *message = calloc(1, sizeof(NGAP_PDU_t));
    InitiatingMessage_t *initiating_msg = calloc(1, sizeof(InitiatingMessage_t));
    ProtocolIE_Container_112P73_t *protocol_ies = calloc(1, sizeof(ProtocolIE_Container_112P73_t));

    // Connect them together
    message->present = NGAP_PDU_PR_initiatingMessage;
    message->choice.initiatingMessage = initiating_msg;
    
    initiating_msg->procedureCode = ProcedureCode_id_ErrorIndication;
    initiating_msg->criticality = Criticality_ignore;
    initiating_msg->value.present = InitiatingMessage__value_PR_ErrorIndication;
    initiating_msg->value.choice.ErrorIndication.protocolIEs = (struct ProtocolIE_Container *)protocol_ies;

    // ---------------------------------------------------------
    // 2. VALIDATE AGAINST 3GPP CONSTRAINTS
    // ---------------------------------------------------------
    printf("[2] Validating message against 3GPP constraints...\n");
    
    CodecError error;
    if (codec_validate(CODEC_NGAP_PDU, message, &error) != CODEC_SUCCESS) {
        printf("Validation Failed on field '%s': %s\n", error.failed_type_name, error.message);
        codec_free(CODEC_NGAP_PDU, message);
        return 1;
    }
    printf("  -> Validation Passed!\n\n");

    // ---------------------------------------------------------
    // 3. ENCODE TO BYTES
    // ---------------------------------------------------------
    printf("[3] Encoding message into raw bytes...\n");
    
    uint8_t buffer[1024];
    size_t encoded_size = 0;

    CodecStatus status = codec_encode(
        CODEC_NGAP_PDU,     // The protocol we are encoding
        message,            // The C structure pointer
        buffer,             // Output buffer
        sizeof(buffer),     // Max capacity
        &encoded_size,      // Resulting size will be stored here
        &error              // Pass the error struct to catch detailed failures
    );

    if (status != CODEC_SUCCESS) {
        printf("Encode Failed on field '%s': %s\n", error.failed_type_name, error.message);
        codec_free(CODEC_NGAP_PDU, message);
        return 1;
    }
    
    printf("  -> Encoded %zu bytes: ", encoded_size);
    for(size_t i = 0; i < encoded_size; i++) {
        printf("%02X ", buffer[i]);
    }
    printf("\n\n");

    // ---------------------------------------------------------
    // 4. DECODE FROM BYTES
    // ---------------------------------------------------------
    printf("[4] Decoding bytes back into a C structure...\n");

    NGAP_PDU_t *decoded_message = NULL; // Must be NULL initially

    status = codec_decode(
        CODEC_NGAP_PDU,
        buffer,
        encoded_size,
        (void **)&decoded_message,
        &error
    );

    if (status != CODEC_SUCCESS) {
        // We know exactly how many bytes failed thanks to the error struct
        printf("Decode Failed! %s\n", error.message);
        codec_free(CODEC_NGAP_PDU, message);
        return 1;
    }
    printf("  -> Decode Passed! Message is an NGAP PDU. Contents:\n\n");
    
    char xml_output[4096];
    size_t xml_size = 0;
    status = codec_encode_xml(
        CODEC_NGAP_PDU,
        decoded_message,
        xml_output,
        sizeof(xml_output),
        &xml_size,
        &error
    );

    if (status == CODEC_SUCCESS) {
        printf("\n--- XER (XML) OUTPUT ---\n");
        printf("%s\n", xml_output);
        printf("------------------------\n");
    } else {
        printf("Failed to encode XML: %s\n", error.message);
    }

    // ---------------------------------------------------------
    // 5. CLEANUP
    // ---------------------------------------------------------
    // ALWAYS use codec_free to recursively free memory safely.
    codec_free(CODEC_NGAP_PDU, message);
    codec_free(CODEC_NGAP_PDU, decoded_message);

    printf("\n=== Tutorial Complete! ===\n");
    return 0;
}
