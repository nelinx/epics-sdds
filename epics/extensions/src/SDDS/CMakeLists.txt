# vim: set noexpandtab:

ADD_LIBRARY(
	lzma
	STATIC
	lzma/alone_decoder.c
	lzma/alone_encoder.c
	lzma/arm.c
	lzma/armthumb.c
	lzma/auto_decoder.c
	lzma/block_buffer_decoder.c
	lzma/block_buffer_encoder.c
	lzma/block_decoder.c
	lzma/block_encoder.c
	lzma/block_header_decoder.c
	lzma/block_header_encoder.c
	lzma/block_util.c
	lzma/check.c
	lzma/common.c
	lzma/crc32_fast.c
	lzma/crc32_table.c
	lzma/crc64_fast.c
	lzma/crc64_table.c
	lzma/delta_common.c
	lzma/delta_decoder.c
	lzma/delta_encoder.c
	lzma/easy_buffer_encoder.c
	lzma/easy_decoder_memusage.c
	lzma/easy_encoder.c
	lzma/easy_encoder_memusage.c
	lzma/easy_preset.c
	lzma/fastpos_table.c
	lzma/filter_buffer_decoder.c
	lzma/filter_buffer_encoder.c
	lzma/filter_common.c
	lzma/filter_decoder.c
	lzma/filter_encoder.c
	lzma/filter_flags_decoder.c
	lzma/filter_flags_encoder.c
	lzma/hardware_physmem.c
	lzma/ia64.c
	lzma/index.c
	lzma/index_decoder.c
	lzma/index_encoder.c
	lzma/index_hash.c
	lzma/lz_decoder.c
	lzma/lz_encoder.c
	lzma/lz_encoder_mf.c
	lzma/lzma2_decoder.c
	lzma/lzma2_encoder.c
	lzma/lzma_decoder.c
	lzma/lzma_encoder.c
	lzma/lzma_encoder_optimum_fast.c
	lzma/lzma_encoder_optimum_normal.c
	lzma/lzma_encoder_presets.c
	lzma/powerpc.c
	lzma/price_table.c
	lzma/sha256.c
	lzma/simple_coder.c
	lzma/simple_decoder.c
	lzma/simple_encoder.c
	lzma/sparc.c
	lzma/stream_buffer_decoder.c
	lzma/stream_buffer_encoder.c
	lzma/stream_decoder.c
	lzma/stream_encoder.c
	lzma/stream_flags_common.c
	lzma/stream_flags_decoder.c
	lzma/stream_flags_encoder.c
	lzma/tuklib_physmem.c
	lzma/vli_decoder.c
	lzma/vli_encoder.c
	lzma/vli_size.c
	lzma/x86.c
	)

SET_TARGET_PROPERTIES(lzma PROPERTIES COMPILE_FLAGS "-std=gnu99")
ADD_DEFINITIONS("-DHAVE_CONFIG_H -DTUKLIB_SYMBOL_PREFIX=lzma_ -DHAVE_BSWAP_16
      -DHAVE_BSWAP_32 -DHAVE_BSWAP_64 -DHAVE_BYTESWAP_H
      -DHAVE_DECL_PROGRAM_INVOCATION_NAME -DHAVE_VISIBILITY -DSIZEOF_SIZE_T=8
      -DTUKLIB_FAST_UNALIGNED_ACCESS -DHAVE_DCGETTEXT -DHAVE_FUTIMENS
      -DHAVE_GETTEXT -DHAVE_STRUCT_STAT_ST_ATIM_TV_NSEC
      -DTUKLIB_CPUCORES_SYSCONF -DTUKLIB_PHYSMEM_SYSCONF -DHAVE_INTTYPES_H
      -DHAVE_STDBOOL_H -DHAVE_STRINGS_H -DHAVE_PTHREAD -DHAVE__BOOL")

FIND_PACKAGE(ZLIB)

ADD_DEFINITIONS("-DzLib")

INCLUDE_DIRECTORIES(
	${CMAKE_CURRENT_SOURCE_DIR}/lzma
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${ZLIB_INCLUDE_DIR}
	)

ADD_LIBRARY(
	SDDSlib
	STATIC
	SDDSlib/SDDS_ascii.c
	SDDSlib/SDDS_binary.c
	SDDSlib/SDDS_copy.c
	SDDSlib/SDDS_data.c
	SDDSlib/SDDS_dataprep.c
	SDDSlib/SDDS_extract.c
	SDDSlib/SDDS_info.c
	SDDSlib/SDDS_input.c
	SDDSlib/SDDS_lzma.c
	SDDSlib/SDDS_mplsupport.c
	SDDSlib/SDDS_output.c
	SDDSlib/SDDS_process.c
	SDDSlib/SDDS_rpn.c
	SDDSlib/SDDS_transfer.c
	SDDSlib/SDDS_utils.c
	SDDSlib/SDDS_write.c
	mdblib/array.c
	mdblib/binsert.c
	mdblib/bomb.c
	mdblib/cp_str.c
	mdblib/data_scan.c
	mdblib/fexists.c
	mdblib/fopen_e.c
	mdblib/match_string.c
	mdblib/searchPath.c
	mdblib/str_copy.c
	mdblib/tmpname.c
	mdblib/unpack.c
	mdblib/wild_match.c
	)

TARGET_LINK_LIBRARIES(
	SDDSlib
	lzma
	)

ADD_LIBRARY(
	SDDS3lib
	STATIC
	SDDS3lib/SDDSArray.cc
	SDDS3lib/SDDSColumn.cc
	SDDS3lib/SDDSFile.cc
	SDDS3lib/SDDSParameter.cc
	SDDS3lib/SDDSUtil.cc
	)

TARGET_LINK_LIBRARIES(
	SDDS3lib
	SDDSlib
	${ZLIB_LIBRARIES}
	)

SET_TARGET_PROPERTIES(SDDS3lib PROPERTIES COMPILE_FLAGS "-fPIC")

SET(SDDS_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include PARENT_SCOPE)
SET(SDDS3_LIBRARIES SDDS3lib PARENT_SCOPE)
