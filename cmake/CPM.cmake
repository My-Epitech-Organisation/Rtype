# Minimal CPM.cmake bootstrapper bundled with the project.
# Fetches the requested CPM version into the build tree if not already present.

set(CPM_DOWNLOAD_VERSION "0.38.7")
set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")

if(NOT EXISTS "${CPM_DOWNLOAD_LOCATION}")
    message(STATUS "[CPM] Downloading CPM.cmake v${CPM_DOWNLOAD_VERSION}...")
    file(DOWNLOAD
        "https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake"
        "${CPM_DOWNLOAD_LOCATION}"
        TLS_VERIFY ON
    )
endif()

include("${CPM_DOWNLOAD_LOCATION}")
