cmake_minimum_required(VERSION 3.16)

project(asn_codec C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

option(ENABLE_ASAN "Enable AddressSanitizer (Memory Leak and Bounds Checking)" OFF)
if(ENABLE_ASAN)
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer -g -O1)
    add_link_options(-fsanitize=address)
endif()

option(ENABLE_FUZZING "Enable libFuzzer instrumentation" OFF)
if(ENABLE_FUZZING)
    add_compile_options(-fsanitize=fuzzer,address -fno-omit-frame-pointer -g -O1)
    add_link_options(-fsanitize=fuzzer,address)
endif()

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
    src/protocol_map.c
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
set(ASN1C_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../asn1c" CACHE PATH "Path to asn1c source directory containing config.h")
target_include_directories(codec_xnap PRIVATE
    ${ASN1C_DIR}
)

# ============================================================
# Tools and Examples
# ============================================================
add_executable(asncodec
    tools/asncodec.c
    src/cli/cli.c
    src/codec.c
    src/protocol_registry.c
    src/protocol_map.c
)
target_compile_definitions(asncodec PRIVATE
    CODEC_PROTOCOL_RRC
    CODEC_PROTOCOL_NGAP
    CODEC_PROTOCOL_X2AP
    CODEC_PROTOCOL_XNAP
add_executable(asncodec tools/asncodec.c tools/parse_log.c)
target_link_libraries(asncodec codec_ngap codec_rrc codec_x2ap codec_xnap codec_s1ap -lm)
target_compile_definitions(asncodec PRIVATE EMIT_ASN_DEBUG=1)

add_executable(asncodec_ngap tools/asncodec.c tools/parse_log.c)
target_link_libraries(asncodec_ngap codec_ngap -lm)
target_compile_definitions(asncodec_ngap PRIVATE EMIT_ASN_DEBUG=1)

install(TARGETS asncodec asncodec_ngap DESTINATION bin OPTIONAL)


add_executable(basic_usage
    examples/basic_usage.c
)
target_link_libraries(basic_usage
    PRIVATE codec_ngap
)

add_executable(rrc_setup_request_example
    examples/rrc_setup_request_example.c
)
target_link_libraries(rrc_setup_request_example
    PRIVATE codec_rrc
)


add_executable(test_codec
    tests/test_codec.c
)
target_link_libraries(test_codec
    PRIVATE codec_rrc
)

add_executable(test_x2ap_codec
    tests/test_x2ap_round_trip.c
)
target_link_libraries(test_x2ap_codec
    PRIVATE codec_x2ap
)

add_executable(test_ngap_codec
    tests/test_ngap_round_trip.c
)
target_link_libraries(test_ngap_codec
    PRIVATE codec_ngap
)

add_executable(test_xnap_codec
    tests/test_xnap_round_trip.c
)
target_link_libraries(test_xnap_codec
    PRIVATE codec_xnap
)

# ============================================================
# CTest Integration
# ============================================================
enable_testing()

add_test(NAME test_rrc_codec COMMAND test_codec)
add_test(NAME test_x2ap_codec COMMAND test_x2ap_codec)
add_test(NAME test_ngap_codec COMMAND test_ngap_codec)
add_test(NAME test_xnap_codec COMMAND test_xnap_codec)

if(ENABLE_FUZZING)
    add_executable(asn1_fuzzer
        fuzz/fuzz_target.c
    )
    target_compile_definitions(asn1_fuzzer PRIVATE
        CODEC_PROTOCOL_NGAP
    )
    target_link_libraries(asn1_fuzzer
        PRIVATE codec_ngap
    )
endif()
