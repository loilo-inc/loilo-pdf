cmake_minimum_required(VERSION 3.4.1)

set(loilopdf_SRCS src/PdfRendererCompat.cpp )

add_library( pdfium-loilo
             SHARED
             IMPORTED )

set_target_properties( pdfium-loilo
                       PROPERTIES IMPORTED_LOCATION
                       ${CMAKE_CURRENT_SOURCE_DIR}/libs/${ANDROID_ABI}/libpdfium-loilo.so )

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
                       pdfium-loilo )

include_directories(./external/pdfium)
include_directories(./external/pdfium/core/include)
include_directories(./external/pdfium/fpdfsdk/include)
include_directories(./external/pdfium/public)
include_directories(./include)

add_custom_command(TARGET loilopdf POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/${ANDROID_ABI}/libz-loilo.so
        ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libz-loilo.so)

add_custom_command(TARGET loilopdf POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/${ANDROID_ABI}/libpng-loilo.so
        ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libpng-loilo.so)

add_custom_command(TARGET loilopdf POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/${ANDROID_ABI}/libft2-loilo.so
        ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libft2-loilo.so)

add_custom_command(TARGET loilopdf POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/${ANDROID_ABI}/libpdfium-loilo.so
        ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libpdfium-loilo.so)