# Minimal CPM.cmake bootstrapper bundled with the project.
# Fetches the requested CPM version into the build tree if not already present.
#
# SECURITY WARNING:
# This script downloads and directly includes remote CMake code from GitHub without
# cryptographic integrity verification (beyond TLS). If the upstream repository or
# release asset is compromised, attackers can inject arbitrary build logic into your
# build environment with full access to secrets, environment variables, and CI systems.
#
# MITIGATION OPTIONS (in order of preference):
#
# 1. VENDOR CPM.CMAKE INTO THE REPOSITORY (RECOMMENDED)
#    - Download a known-good copy of CPM.cmake to cmake/cpm/CPM.cmake
#    - Update this script to use the vendored copy instead of downloading
#    - Monitor upstream for security updates and re-vendor periodically
#    - This is the most secure approach for production builds
#
# 2. PIN TO A SPECIFIC COMMIT + VERIFY HASH
#    - Use a specific commit SHA instead of a release tag
#    - After download, compute and verify a known-good cryptographic hash (SHA256)
#    - Example:
#        file(DOWNLOAD ... <file>)
#        file(SHA256 <file> computed_hash)
#        if(NOT computed_hash STREQUAL "expected_hash")
#          message(FATAL_ERROR "CPM.cmake integrity check failed")
#        endif()
#    - Requires updating the hash when CPM.cmake is upgraded
#
# 3. USE GITHUB RELEASE ASSET WITH SIGNATURE VERIFICATION
#    - GitHub releases may include GPG signatures; download and verify them
#    - More complex but provides cryptographic proof of authenticity
#
# 4. MIRROR TO INTERNAL REPOSITORY
#    - Host a known-good copy on an internal/private artifact server
#    - Use local URLs instead of GitHub
#    - Reduces external dependency risk
#
# For now, this script performs basic network validation (TLS) and error checking
# to catch obvious failures. For production / hardened builds, implement one of
# the mitigation strategies above.

set(CPM_DOWNLOAD_VERSION "0.38.7")
set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")

if(NOT EXISTS "${CPM_DOWNLOAD_LOCATION}")
    message(STATUS "[CPM] Downloading CPM.cmake v${CPM_DOWNLOAD_VERSION}...")
    file(DOWNLOAD
        "https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake"
        "${CPM_DOWNLOAD_LOCATION}"
        TLS_VERIFY ON
        STATUS CPM_DOWNLOAD_STATUS
    )

    list(GET CPM_DOWNLOAD_STATUS 0 CPM_DOWNLOAD_STATUS_CODE)
    list(GET CPM_DOWNLOAD_STATUS 1 CPM_DOWNLOAD_ERROR_MESSAGE)

    if(NOT CPM_DOWNLOAD_STATUS_CODE EQUAL 0)
        file(REMOVE "${CPM_DOWNLOAD_LOCATION}")
        message(FATAL_ERROR
            "[CPM] Failed to download CPM.cmake v${CPM_DOWNLOAD_VERSION}\n"
            "Error: ${CPM_DOWNLOAD_ERROR_MESSAGE}\n"
            "URL: https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake\n"
            "Please check your network connection or download the file manually."
        )
    endif()
endif()

include("${CPM_DOWNLOAD_LOCATION}")
