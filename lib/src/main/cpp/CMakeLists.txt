cmake_minimum_required(VERSION 3.4.1)

set(loilopdf_SRCS src/PdfRendererCompat.cpp )

add_library( pdfium
             SHARED
             IMPORTED )

set_target_properties( pdfium
                       PROPERTIES IMPORTED_LOCATION
                       ${CMAKE_CURRENT_SOURCE_DIR}/external/pdfium/fpdfsdk/libs/${ANDROID_ABI}/libpdfium.so )

add_library( loilopdf
             SHARED
             ${loilopdf_SRCS} )

find_library( log-lib
              log )

target_link_libraries( loilopdf
                       ${log-lib}
                       atomic
                       android
                       jnigraphics
                       pdfium )

include_directories(./external/pdfium/core/include)
include_directories(./external/pdfium/fpdfsdk/include)
include_directories(./external/utils)
include_directories(./include)

add_custom_command(TARGET loilopdf POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/external/pdfium/fpdfsdk/libs/${ANDROID_ABI}/libpdfium.so
        ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libpdfium.so)