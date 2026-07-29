cmake_minimum_required(VERSION 3.16)

project(asn_codec C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Each ASN.1 protocol generates global symbols with common names (such as
# asn_DEF_ProtocolIE_Field).  Keep each protocol in a separate library so a
# consumer links exactly one codec protocol library at a time.
file(GLOB_RECURSE RRC_SOURCES
    CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/generated/rrc/*.c
)
file(GLOB_RECURSE NGAP_SOURCES
    CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/generated/ngap/*.c
)
file(GLOB_RECURSE X2AP_SOURCES
    CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/generated/x2ap/*.c
)
# file(GLOB_RECURSE S1AP_SOURCES
#     CONFIGURE_DEPENDS
#     ${CMAKE_CURRENT_SOURCE_DIR}/generated/s1ap/*.c
# )
file(GLOB_RECURSE XNAP_SOURCES
    CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/generated/xnap/*.c
)

# Remove asn1c example programs
foreach(SOURCE_LIST RRC_SOURCES NGAP_SOURCES X2AP_SOURCES XNAP_SOURCES)
    list(FILTER ${SOURCE_LIST} EXCLUDE REGEX ".*/converter-example\\.c$")
    list(FILTER ${SOURCE_LIST} EXCLUDE REGEX ".*/converter-sample\\.c$")
    list(FILTER ${SOURCE_LIST} EXCLUDE REGEX ".*/converter-example-pdu\\.c$")
    list(FILTER ${SOURCE_LIST} EXCLUDE REGEX ".*/genhash\\.c$")
endforeach()

# ============================================================
# Wrapper Sources
# ============================================================
set(WRAPPER_SOURCES
    src/codec.c
    src/protocol_registry.c
)

function(add_codec_protocol_library protocol source_list)
    string(TOLOWER "${protocol}" protocol_name)
    add_library(codec_${protocol_name} STATIC
        ${WRAPPER_SOURCES}
        ${${source_list}}
    )
    target_compile_definitions(codec_${protocol_name} PRIVATE CODEC_PROTOCOL_${protocol})
    target_include_directories(codec_${protocol_name} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/generated/${protocol_name}
    )
endfunction()

add_codec_protocol_library(RRC RRC_SOURCES)
add_codec_protocol_library(NGAP NGAP_SOURCES)
add_codec_protocol_library(X2AP X2AP_SOURCES)
# add_codec_protocol_library(S1AP S1AP_SOURCES)
add_codec_protocol_library(XNAP XNAP_SOURCES)
# asn1c's config.h lives in the asn1c source root — expose it only to the xnap target
target_include_directories(codec_xnap PRIVATE
    /home/srajimon/airspan/asn1c
)

# ============================================================
# Examples
# ============================================================
add_executable(test_codec
    examples/test_codec.c
)

target_link_libraries(test_codec
    PRIVATE codec_rrc
)

add_executable(test_x2ap_codec
    examples/test_x2ap_round_trip.c
)

target_link_libraries(test_x2ap_codec
    PRIVATE codec_x2ap
)
