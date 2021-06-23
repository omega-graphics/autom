
set(PYEXEC)
if(WIN32)
    set(PYEXEC "py -3")
else()
    set(PYEXEC "python3")
endif()

macro(add_script_target _NAME SCRIPT)
    cmake_parse_arguments("_ARG" "" "" "OUTPUTS;DEPS;ARGS" ${ARGN})
    add_custom_target(${_NAME} DEPENDS ${_ARG_OUTPUTS})
    add_custom_command(
        OUTPUT ${_ARG_OUTPUTS} 
        COMMAND ${PYEXEC} ${SCRIPT} ${_ARG_ARGS}
        DEPENDS ${_ARG_DEPS})
endmacro()


if(XCODE)
    set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_IDENTITY ${CODE_SIGNATURE})
else()

    macro(code_sign_bundle _NAME IS_APP)
        get_target_property(OUTPUT_DIR ${_NAME} RUNTIME_OUTPUT_DIRECTORY)
        set(OUTPUT_DIR )
        add_script_target("${_NAME}__code_sign" "${CMAKE_SOURCE_DIR}/gn-utils/codesign.py" OUTPUTS "${OUTPUT_DIR}")
    endmacro()  
    
endif()



function(add_framework_bundle _NAME)
    cmake_parse_arguments("_ARG" "" "INFO_PLIST;VERSION" "SOURCES;RESOURCES;LIBS;FRAMEWORKS;" ${ARGN})

    add_library(${_NAME} SHARED ${_ARG_SOURCES} ${_ARG_RESOURCES})
    set_target_properties(${_NAME} 
    FRAMEWORK TRUE
    FRAMEWORK_VERSION ${_ARG_VERSION}
    MACOSX_FRAMEWORK_INFO_PLIST ${_ARG_INFO_PLIST}
    )
    set_source_files_properties(${_ARG_RESOURCES} MACOSX_PACKAGE_LOCATION "Resources")
    set(DEPS ${_ARG_LIBS} ${_ARG_FRAMEWORKS})
    foreach(_lib ${DEPS})
        add_dependencies(${_NAME} ${_lib})
    endforeach()
    
    

endfunction()

