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
