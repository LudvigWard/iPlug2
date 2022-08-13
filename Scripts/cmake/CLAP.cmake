cmake_minimum_required(VERSION 3.11)

set(CLAP_SDK "${IPLUG2_DIR}/Dependencies/IPlug/CLAP_SDK" CACHE PATH "CLAP SDK directory.")
set(CLAP_HELPERS "${IPLUG2_DIR}/Dependencies/IPlug/CLAP_HELPERS" CACHE PATH "CLAP HELPERS directory.")

if (WIN32)
  
elseif (OS_MAC)
  set(_paths "$ENV{HOME}/Library/Audio/Plug-Ins/CLAP" "/Library/Audio/Plug-Ins/CLAP")
elseif (OS_LINUX)
  set(_paths "$ENV{HOME}/.clap" "/usr/local/lib/clap" "/usr/local/clap")
endif()

iplug_find_path(CLAP_INSTALL_PATH REQUIRED DIR DEFAULT_IDX 0 
  DOC "Path to install CLAP plugins"
  PATHS ${_paths})

set(sdk ${IPLUG2_DIR}/IPlug/CLAP)
add_library(iPlug2_CLAP INTERFACE)
iplug_target_add(iPlug2_CLAP INTERFACE
  INCLUDE ${sdk} ${CLAP_SDK}/include ${CLAP_HELPERS} ${CLAP_HELPERS}/include/clap/helpers
  SOURCE ${sdk}/IPlugCLAP.cpp
  DEFINE "CLAP_API" "IPLUG_DSP=1" "IPLUG_EDITOR=1"
  LINK iPlug2_Core
)
# if (OS_LINUX)
#   iplug_target_add(iPlug2_CLAP INTERFACE
#   )
#   if ("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
#     iplug_target_add(iPlug2_CLAP INTERFACE DEFINE "__cdecl=__attribute__((__cdecl__))")
#   endif()
# endif()

list(APPEND IPLUG2_TARGETS iPlug2_CLAP)

function(iplug_configure_clap target)
  iplug_target_add(${target} PUBLIC LINK iPlug2_CLAP)

  if (WIN32)
    set(out_dir "${CMAKE_BINARY_DIR}/${target}")
    set_target_properties(${target} PROPERTIES
      OUTPUT_NAME "${IPLUG_APP_NAME}"
      LIBRARY_OUTPUT_DIRECTORY "${out_dir}"
      PREFIX ""
      SUFFIX ".clap"
    )
    set(res_dir "${CMAKE_BINARY_DIR}/${target}/resources")

    # After building, we run the post-build script
    add_custom_command(TARGET ${target} POST_BUILD 
      COMMAND "${CMAKE_BINARY_DIR}/postbuild-win.bat" 
      ARGS "\"$<TARGET_FILE:${target}>\"" "\".clap\""
    )
    
  elseif (CMAKE_SYSTEM_NAME MATCHES "Darwin")
    set_target_properties(${target} PROPERTIES
      BUNDLE TRUE
      MACOSX_BUNDLE TRUE
      MACOSX_BUNDLE_INFO_PLIST ${PLUG_RESOURCES_DIR}/${PLUG_NAME}-CLAP-Info.plist
      BUNDLE_EXTENSION "clap"
      PREFIX ""
      SUFFIX "")

    set(install_dir "${CLAP_INSTALL_PATH}/${PLUG_NAME}.clap")

    if (CMAKE_GENERATOR STREQUAL "Xcode")
      set(out_dir "${CMAKE_BINARY_DIR}/$<CONFIG>/${PLUG_NAME}.clap")
      set(res_dir "")
    else()
      set(out_dir "${CMAKE_BINARY_DIR}/${PLUG_NAME}.clap")
      set(res_dir "${CMAKE_BINARY_DIR}/${PLUG_NAME}.clap/Contents/Resources")
    endif()

    add_custom_command(TARGET ${target} POST_BUILD
      COMMAND ${CMAKE_COMMAND} ARGS "-E" "copy_directory" "${out_dir}" "${install_dir}")


  elseif (CMAKE_SYSTEM_NAME MATCHES "Linux")
    set(out_dir "${CMAKE_BINARY_DIR}/${PLUG_NAME}.clap")
    set_target_properties(${target} PROPERTIES
      OUTPUT_NAME "${PLUG_NAME}"
      LIBRARY_OUTPUT_DIRECTORY "${out_dir}"
      PREFIX ""
      SUFFIX ".so"
    )
    set(res_dir "${CMAKE_BINARY_DIR}/${PLUG_NAME}.clap/resources")

    add_custom_command(TARGET ${target} POST_BUILD
      COMMAND ${CMAKE_COMMAND} ARGS "-E" "copy_directory" "${out_dir}" "${CLAP_INSTALL_PATH}/${PLUG_NAME}")
  endif()

  # Handle resources
  if (res_dir)
    iplug_target_bundle_resources(${target} "${res_dir}")
  endif()

endfunction()
