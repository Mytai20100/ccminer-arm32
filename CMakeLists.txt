cmake_minimum_required(VERSION 3.10)
project(ccminer-arm C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Detect or set target architecture
if(NOT DEFINED TARGET_ARCH)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
        set(TARGET_ARCH "x64")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "i686|i386")
        set(TARGET_ARCH "x86")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "armv7l?")
        set(TARGET_ARCH "armv7l")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
        if(ARM_32BIT)
            set(TARGET_ARCH "armv9")
        else()
            set(TARGET_ARCH "aarch64")
        endif()
    else()
        message(FATAL_ERROR "Unsupported architecture: ${CMAKE_SYSTEM_PROCESSOR}")
    endif()
endif()

message(STATUS "Building for architecture: ${TARGET_ARCH}")

# Common flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -O3 -fomit-frame-pointer -fno-strict-aliasing")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O3 -fomit-frame-pointer -fno-strict-aliasing")

# Architecture-specific settings
if(TARGET_ARCH STREQUAL "x64")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m64 -march=native -maes -mpclmul -msse4.1 -mavx2")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64 -march=native -maes -mpclmul -msse4.1 -mavx2")
    set(VERUS_SOURCES
        verus/haraka.c
        verus/verus_clhash.cpp
    )
elseif(TARGET_ARCH STREQUAL "x86")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32 -march=native -maes -mpclmul -msse4.1")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32 -march=native -maes -mpclmul -msse4.1")
    set(VERUS_SOURCES
        verus/haraka.c
        verus/verus_clhash.cpp
    )
elseif(TARGET_ARCH STREQUAL "armv7l")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=armv7-a -mfpu=neon -mfloat-abi=hard")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=armv7-a -mfpu=neon -mfloat-abi=hard")
    
    # Check for Crypto Extensions
    include(CheckCCompilerFlag)
    check_c_compiler_flag("-march=armv7-a+crypto" HAS_ARM_CRYPTO)
    if(HAS_ARM_CRYPTO)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=armv7-a+crypto")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=armv7-a+crypto")
    endif()
    
    set(VERUS_SOURCES
        verus/armv7l/haraka_neon_armv7l.c
        verus/armv7l/verus_clhash_neon_armv7l.cpp
    )
    include_directories(verus/armv7l)
elseif(TARGET_ARCH STREQUAL "armv9")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=armv8-a+crypto -mfpu=neon-fp-armv8 -mfloat-abi=hard")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=armv8-a+crypto -mfpu=neon-fp-armv8 -mfloat-abi=hard")
    set(VERUS_SOURCES
        verus/armv9/haraka_neon_armv9.c
        verus/armv9/verus_clhash_neon_armv9.cpp
    )
    include_directories(verus/armv9)
elseif(TARGET_ARCH STREQUAL "aarch64")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=armv8-a+crypto")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=armv8-a+crypto")
    set(VERUS_SOURCES
        verus/armv9/haraka_neon_armv9.c
        verus/armv9/verus_clhash_neon_armv9.cpp
    )
    include_directories(verus/armv9)
endif()

# Find required packages
find_package(CURL REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/compat
    ${CMAKE_CURRENT_SOURCE_DIR}/compat/jansson
    ${CURL_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIR}
)

# Common sources
set(COMMON_SOURCES
    ccminer.cpp
    pools.cpp
    util.cpp
    bench.cpp
    api.cpp
    hashlog.cpp
    stats.cpp
    sysinfos.cpp
    crc32.c
    bignum.cpp
    equi/equi-stratum.cpp
    verus/verusscan.cpp
)

# Jansson sources
set(JANSSON_SOURCES
    compat/jansson/dump.c
    compat/jansson/error.c
    compat/jansson/hashtable.c
    compat/jansson/hashtable_seed.c
    compat/jansson/load.c
    compat/jansson/memory.c
    compat/jansson/pack_unpack.c
    compat/jansson/strbuffer.c
    compat/jansson/strconv.c
    compat/jansson/utf.c
    compat/jansson/value.c
)

# Executable
add_executable(ccminer-${TARGET_ARCH}
    ${COMMON_SOURCES}
    ${VERUS_SOURCES}
    ${JANSSON_SOURCES}
)

# Link libraries
target_link_libraries(ccminer-${TARGET_ARCH}
    ${CURL_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
)

# Installation
install(TARGETS ccminer-${TARGET_ARCH}
    RUNTIME DESTINATION bin
)

# Print configuration
message(STATUS "")
message(STATUS "========================================")
message(STATUS "Configuration Summary")
message(STATUS "========================================")
message(STATUS "Target Architecture: ${TARGET_ARCH}")
message(STATUS "C Compiler:          ${CMAKE_C_COMPILER}")
message(STATUS "CXX Compiler:        ${CMAKE_CXX_COMPILER}")
message(STATUS "C Flags:             ${CMAKE_C_FLAGS}")
message(STATUS "CXX Flags:           ${CMAKE_CXX_FLAGS}")
message(STATUS "========================================")
message(STATUS "")
