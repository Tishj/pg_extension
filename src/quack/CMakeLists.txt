# Copyright 2020 Mats Kindahl
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may not
# use this file except in compliance with the License. You may obtain a copy of
# the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
# License for the specific language governing permissions and limitations under
# the License.

include(ExternalProject)
ExternalProject_Add(
    DUCKDB_LIB
    #PREFIX ${CMAKE_BINARY_DIR}/_deps
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/duckdb
    BINARY_DIR ${CMAKE_BINARY_DIR}/duckdb
    INSTALL_DIR ${CMAKE_BINARY_DIR}/duckdb

    # Build command
    CMAKE_ARGS -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DBUILD_EXTENSIONS='parquet' -DASSERT_EXCEPTION=0 -DENABLE_SANITIZER=FALSE -DENABLE_UBSAN=0
    BUILD_COMMAND ${CMAKE_COMMAND} --build <BINARY_DIR> --config ${CMAKE_BUILD_TYPE}

    # Specify the build byproduct
    BUILD_BYPRODUCTS
		<BINARY_DIR>/src/libduckdb_static.a
		<BINARY_DIR>/extension/parquet/libparquet_extension.a
		<BINARY_DIR>/third_party/utf8proc/libduckdb_utf8proc.a
		<BINARY_DIR>/third_party/skiplist/libduckdb_skiplistlib.a
		<BINARY_DIR>/third_party/re2/libduckdb_re2.a
		<BINARY_DIR>/third_party/miniz/libduckdb_miniz.a
		<BINARY_DIR>/third_party/mbedtls/libduckdb_mbedtls.a
		<BINARY_DIR>/third_party/libpg_query/libduckdb_pg_query.a
		<BINARY_DIR>/third_party/imdb/libimdb.a
		<BINARY_DIR>/third_party/hyperloglog/libduckdb_hyperloglog.a
		<BINARY_DIR>/third_party/fsst/libduckdb_fsst.a
		<BINARY_DIR>/third_party/fmt/libduckdb_fmt.a
		<BINARY_DIR>/third_party/fastpforlib/libduckdb_fastpforlib.a

    # Optional: Pass additional CMake arguments if needed
    # CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> ...

    LOG_DOWNLOAD ON
    LOG_CONFIGURE ON
    LOG_BUILD ON
    LOG_INSTALL ON  # Optional: Enable logging for the install step
)

ExternalProject_Get_Property(DUCKDB_LIB install_dir)

# Add the library directory to the linker search path
link_directories(${install_dir}/src)

add_library(duckdb STATIC IMPORTED GLOBAL)
set_target_properties(duckdb PROPERTIES IMPORTED_LOCATION ${install_dir}/src/libduckdb_static.a)

add_library(duckdb_parquet STATIC IMPORTED GLOBAL)
set_target_properties(duckdb_parquet PROPERTIES IMPORTED_LOCATION ${install_dir}/extension/parquet/libparquet_extension.a)

include_directories(third_party/duckdb/src/include)
include_directories(include)
add_subdirectory(src)

set(ALL_OBJECT_FILES ${ALL_OBJECT_FILES})


add_postgresql_extension(
  quack
  VERSION 0.0.1
  SOURCES ${ALL_OBJECT_FILES}
  SCRIPTS quack--0.0.1.sql
)

add_dependencies(quack ${quack_LIBRARY} DUCKDB_LIB)
target_link_libraries(
	quack
	${install_dir}/src/libduckdb_static.a
	${install_dir}/third_party/utf8proc/libduckdb_utf8proc.a
	${install_dir}/third_party/skiplist/libduckdb_skiplistlib.a
	${install_dir}/third_party/re2/libduckdb_re2.a
	${install_dir}/third_party/miniz/libduckdb_miniz.a
	${install_dir}/third_party/mbedtls/libduckdb_mbedtls.a
	${install_dir}/third_party/libpg_query/libduckdb_pg_query.a
	${install_dir}/third_party/imdb/libimdb.a
	${install_dir}/third_party/hyperloglog/libduckdb_hyperloglog.a
	${install_dir}/third_party/fsst/libduckdb_fsst.a
	${install_dir}/third_party/fmt/libduckdb_fmt.a
	${install_dir}/third_party/fastpforlib/libduckdb_fastpforlib.a
	${install_dir}/extension/parquet/libparquet_extension.a
)
