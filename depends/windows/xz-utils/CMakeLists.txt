cmake_minimum_required(VERSION 3.5)
project(xz-utils)

add_definitions(-DWIN32 -DHAVE_CONFIG_H -D_LIB -D_CRT_SECURE_NO_WARNINGS)
if(CMAKE_SYSTEM_NAME STREQUAL WindowsStore)
  add_definitions(-D_WIN32_WINNT=0x0A00)
endif()
include_directories(${PROJECT_SOURCE_DIR}/windows/vs2013
                    ${PROJECT_SOURCE_DIR}/src/liblzma/common
                    ${PROJECT_SOURCE_DIR}/src/common
                    ${PROJECT_SOURCE_DIR}/src/liblzma/api
                    ${PROJECT_SOURCE_DIR}/src/liblzma/check
                    ${PROJECT_SOURCE_DIR}/src/liblzma/delta
                    ${PROJECT_SOURCE_DIR}/src/liblzma/lz
                    ${PROJECT_SOURCE_DIR}/src/liblzma/lzma
                    ${PROJECT_SOURCE_DIR}/src/liblzma/rangecoder
                    ${PROJECT_SOURCE_DIR}/src/liblzma/simple)

set(SOURCES src/common/tuklib_cpucores.c
            src/common/tuklib_physmem.c
            src/liblzma/check/check.c
            src/liblzma/check/crc32_fast.c
            src/liblzma/check/crc32_table.c
            src/liblzma/check/crc64_fast.c
            src/liblzma/check/crc64_table.c
            src/liblzma/check/sha256.c
            src/liblzma/common/alone_decoder.c
            src/liblzma/common/alone_encoder.c
            src/liblzma/common/auto_decoder.c
            src/liblzma/common/block_buffer_decoder.c
            src/liblzma/common/block_buffer_encoder.c
            src/liblzma/common/block_decoder.c
            src/liblzma/common/block_encoder.c
            src/liblzma/common/block_header_decoder.c
            src/liblzma/common/block_header_encoder.c
            src/liblzma/common/block_util.c
            src/liblzma/common/common.c
            src/liblzma/common/easy_buffer_encoder.c
            src/liblzma/common/easy_decoder_memusage.c
            src/liblzma/common/easy_encoder.c
            src/liblzma/common/easy_encoder_memusage.c
            src/liblzma/common/easy_preset.c
            src/liblzma/common/filter_buffer_decoder.c
            src/liblzma/common/filter_buffer_encoder.c
            src/liblzma/common/filter_common.c
            src/liblzma/common/filter_decoder.c
            src/liblzma/common/filter_encoder.c
            src/liblzma/common/filter_flags_decoder.c
            src/liblzma/common/filter_flags_encoder.c
            src/liblzma/common/hardware_cputhreads.c
            src/liblzma/common/hardware_physmem.c
            src/liblzma/common/index.c
            src/liblzma/common/index_decoder.c
            src/liblzma/common/index_encoder.c
            src/liblzma/common/index_hash.c
            src/liblzma/common/outqueue.c
            src/liblzma/common/stream_buffer_decoder.c
            src/liblzma/common/stream_buffer_encoder.c
            src/liblzma/common/stream_decoder.c
            src/liblzma/common/stream_encoder.c
            src/liblzma/common/stream_encoder_mt.c
            src/liblzma/common/stream_flags_common.c
            src/liblzma/common/stream_flags_decoder.c
            src/liblzma/common/stream_flags_encoder.c
            src/liblzma/common/vli_decoder.c
            src/liblzma/common/vli_encoder.c
            src/liblzma/common/vli_size.c
            src/liblzma/delta/delta_common.c
            src/liblzma/delta/delta_decoder.c
            src/liblzma/delta/delta_encoder.c
            src/liblzma/lzma/fastpos_table.c
            src/liblzma/lzma/lzma2_decoder.c
            src/liblzma/lzma/lzma2_encoder.c
            src/liblzma/lzma/lzma_decoder.c
            src/liblzma/lzma/lzma_encoder.c
            src/liblzma/lzma/lzma_encoder_optimum_fast.c
            src/liblzma/lzma/lzma_encoder_optimum_normal.c
            src/liblzma/lzma/lzma_encoder_presets.c
            src/liblzma/lz/lz_decoder.c
            src/liblzma/lz/lz_encoder.c
            src/liblzma/lz/lz_encoder_mf.c
            src/liblzma/rangecoder/price_table.c
            src/liblzma/simple/arm.c
            src/liblzma/simple/armthumb.c
            src/liblzma/simple/ia64.c
            src/liblzma/simple/powerpc.c
            src/liblzma/simple/simple_coder.c
            src/liblzma/simple/simple_decoder.c
            src/liblzma/simple/simple_encoder.c
            src/liblzma/simple/sparc.c
            src/liblzma/simple/x86.c)

add_library(lzma STATIC ${SOURCES})

install(TARGETS lzma DESTINATION lib)

install(FILES src/liblzma/api/lzma.h DESTINATION include)
install(FILES src/liblzma/api/lzma/base.h
              src/liblzma/api/lzma/bcj.h
              src/liblzma/api/lzma/block.h
              src/liblzma/api/lzma/check.h
              src/liblzma/api/lzma/container.h
              src/liblzma/api/lzma/delta.h
              src/liblzma/api/lzma/filter.h
              src/liblzma/api/lzma/hardware.h
              src/liblzma/api/lzma/index.h
              src/liblzma/api/lzma/index_hash.h
              src/liblzma/api/lzma/lzma12.h
              src/liblzma/api/lzma/stream_flags.h
              src/liblzma/api/lzma/version.h
              src/liblzma/api/lzma/vli.h
        DESTINATION include/lzma)
