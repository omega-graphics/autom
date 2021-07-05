include(CMakeParseArguments)

set(PYEXEC)
if(WIN32)
    set(PYEXEC "py")
    find_program(PYTHON "py")
else()
    set(PYEXEC "python3")
    find_program(PYTHON "python3")
endif()

if(PYTHON)
    message("Found Python3")
else()
    message(FATAL_ERROR "Python 3 is NOT found...")
endif()

macro(add_script_target _NAME SCRIPT)
    cmake_parse_arguments("_ARG" "" "" "OUTPUTS;DEPS;ARGS" ${ARGN})
    add_custom_target(${_NAME} DEPENDS ${_ARG_OUTPUTS})
    add_custom_command(
        OUTPUT ${_ARG_OUTPUTS} 
        COMMAND ${PYEXEC} ${SCRIPT} ${_ARG_ARGS}
        DEPENDS ${_ARG_DEPS})
endmacro(add_script_target)

macro(get_target_output_names)
    cmake_parse_arguments("_ARG" "STATIC;SHARED;" "VAR" "TARGETS" ${ARGN})
    set(${_ARG_VAR})
    foreach(t ${_ARG_TARGETS})
        if(_ARG_STATIC)
            list(APPEND ${_ARG_VAR} $<TARGET_FILE:${t}>)
        elseif(_ARG_SHARED)
            list(APPEND ${_ARG_VAR} $<TARGET_FILE:${t}>)
        endif() 
    endforeach()

  
endmacro()



if(XCODE)
    set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_IDENTITY ${CODE_SIGNATURE})
else()

    macro(code_sign_bundle _NAME IS_APP VERSION OUTPUT_DIR)
        if(TARGET ${_NAME})
            if(IS_APP)
                set(_OUT "$<TARGET_BUNDLE_CONTENT_DIR:${_NAME}>/_CodeSignature")
            else()
                set(_OUT "$<TARGET_BUNDLE_DIR:${_NAME}>/Versions/${VERSION}/_CodeSignature")
            endif()
            set(_CODE "$<TARGET_BUNDLE_DIR:${_NAME}>")
            if(IS_APP)
                add_custom_command(OUTPUT "${OUTPUT_DIR}/_CodeSignature"
                COMMAND ${PYEXEC} ${CMAKE_SOURCE_DIR}/gn-utils/codesign.py 
                --sig ${CODE_SIGNATURE} --code ${_CODE}
                DEPENDS "${_NAME}__prep")
            else()
                add_custom_command(OUTPUT "${OUTPUT_DIR}/_CodeSignature"
                COMMAND ${PYEXEC} ${CMAKE_SOURCE_DIR}/gn-utils/codesign.py --sig ${CODE_SIGNATURE} 
                --code ${_CODE} 
                -framework
                -F ${_NAME}.framework
                --name ${_NAME}
                --version ${VERSION}
                DEPENDS "${_NAME}__prep"
                )
            endif()
            add_custom_target("${_NAME}__codesign" DEPENDS "${OUTPUT_DIR}/_CodeSignature")
        endif()
    endmacro(code_sign_bundle)  
    
endif()

function(add_framework_bundle _NAME)
    cmake_parse_arguments(
    "_ARG" 
    "" 

    "INFO_PLIST;VERSION" 
    "SOURCES;
    RESOURCES;
    DEPS;
    LIBS;
    FRAMEWORKS;EMBEDDED_FRAMEWORKS;" 
    ${ARGN})

    message("EMBEDDED_FRAMEWORKS:${_ARG_EMBEDDED_FRAMEWORKS}")
    add_library(${_NAME} SHARED ${_ARG_SOURCES} ${_ARG_RESOURCES})
    set_target_properties(${_NAME} 
    PROPERTIES
    FRAMEWORK TRUE
    FRAMEWORK_VERSION ${_ARG_VERSION}
    MACOSX_FRAMEWORK_INFO_PLIST ${_ARG_INFO_PLIST}
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Frameworks"
    )

    # if(TARGET ${_NAME})
    #     message("FRAMEWORK:${_NAME}")
    # endif()

    add_custom_target("${_NAME}__prep")

    add_custom_target("${_NAME}__res" DEPENDS ${_ARG_RESOURCES})
    add_dependencies(${_NAME} "${_NAME}__res")

    foreach(r ${_ARG_RESOURCES})
        set_target_properties(${_NAME} PROPERTIES RESOURCE ${_ARG_RESOURCES})
    endforeach()

    # if(${_ARG_EMBEDDED_HEADERS})
    #     set_target_properties()
    # set_source_files_properties(${_ARG_RESOURCES} MACOSX_PACKAGE_LOCATION "Resources")

    if(XCODE)
        set_target_properties(${_NAME} PROPERTIES XCODE_EMBED_FRAMEWORKS ${_ARG_EMBEDDED_FRAMEWORKS})
    else()
        set(__outputted_frameworks)
        foreach(f ${_ARG_EMBEDDED_FRAMEWORKS})
            set(__outputted_frameworks ${__outputted_frameworks} "${CMAKE_BINARY_DIR}/Frameworks/${_NAME}.framework/Versions/${_ARG_VERSION}/Frameworks/${f}.framework")
            add_dependencies(${_NAME} ${f})
            add_custom_command(
                OUTPUT "${CMAKE_BINARY_DIR}/Frameworks/${_NAME}.framework/Versions/${_ARG_VERSION}/Frameworks/${f}.framework"
                COMMAND ${PYEXEC} "${CMAKE_SOURCE_DIR}/gn-utils/__copy.py" --src $<TARGET_BUNDLE_DIR:${f}> --dest "${CMAKE_BINARY_DIR}/Frameworks/${_NAME}.framework/Versions/${_ARG_VERSION}/Frameworks/${f}.framework"  
                DEPENDS ${f})
        endforeach()
        add_custom_target("${_NAME}__framework_embed" DEPENDS ${__outputted_frameworks})
        add_dependencies("${_NAME}__prep" "${_NAME}__framework_embed")
    
        code_sign_bundle(${_NAME} FALSE ${_ARG_VERSION} "${CMAKE_BINARY_DIR}/Frameworks/${_NAME}.framework/Versions/${_ARG_VERSION}")
    endif()
   
    
    foreach(_dep ${_ARG_DEPS})
        add_dependencies(${_NAME} ${_dep})
    endforeach()

    target_link_libraries(${_NAME} PRIVATE ${_ARG_LIBS} ${_ARG_FRAMEWORKS} ${_ARG_EMBEDDED_FRAMEWORKS})
    
endfunction()

function(add_app_bundle)
    cmake_parse_arguments(
        "_ARG" 

        "" 

        "NAME;INFO_PLIST" 

        "SOURCES;
        RESOURCES;
        DEPS;
        LIBS;
        EMBEDDED_FRAMEWORKS" ${ARGN})

    message("EMBEDDED_FRAMEWORKS:${_ARG_EMBEDDED_FRAMEWORKS}")

    add_executable(${_ARG_NAME} MACOSX_BUNDLE ${_ARG_SOURCES} ${_ARG_RESOURCES})
    set_target_properties(${_ARG_NAME}
    PROPERTIES
    MACOSX_BUNDLE_INFO_PLIST ${_ARG_INFO_PLIST}
    RESOURCE "${_ARG_RESOURCES}"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Apps")

    add_custom_target("${_NAME}__prep")

    add_custom_target("${_NAME}__res" DEPENDS ${_ARG_RESOURCES})
    add_dependencies(${_NAME} "${_NAME}__res")

    foreach(r ${_ARG_RESOURCES})
        set_target_properties(${_NAME} PROPERTIES RESOURCE ${_ARG_RESOURCES})
    endforeach()

    if(XCODE)
        set_target_properties(${_NAME} PROPERTIES XCODE_EMBED_FRAMEWORKS ${_ARG_EMBEDDED_FRAMEWORKS})
    else()

        set(__outputted_frameworks)
        foreach(f ${_ARG_EMBEDDED_FRAMEWORKS})
            set(__outputted_frameworks ${__outputted_frameworks} "${CMAKE_BINARY_DIR}/Apps/${_ARG_NAME}.app/Contents/Frameworks/${f}.framework")
            add_dependencies(${_ARG_NAME} ${f})
            add_custom_command(
                OUTPUT "${CMAKE_BINARY_DIR}/Apps/${_ARG_NAME}.app/Contents/Frameworks/${f}.framework"
                COMMAND ${PYEXEC} "${CMAKE_SOURCE_DIR}/gn-utils/__copy.py" --src $<TARGET_BUNDLE_DIR:${f}> --dest "${CMAKE_BINARY_DIR}/Apps/${_ARG_NAME}.app/Contents/Frameworks/${f}.framework"  
                DEPENDS ${f})
        endforeach()
        add_custom_target("${_NAME}__framework_embed" DEPENDS ${__outputted_frameworks})
        add_dependencies("${_NAME}__prep" "${_NAME}__framework_embed")

        code_sign_bundle(${_ARG_NAME} TRUE "" "${CMAKE_BINARY_DIR}/Apps/${_NAME}.app/Contents")
    endif()
    
    foreach(_dep ${_ARG_DEPS})
        add_dependencies(${_ARG_NAME} ${_dep})
    endforeach()

    target_link_libraries(${_ARG_NAME} PRIVATE ${_ARG_LIBS} ${_ARG_FRAMEWORKS} ${_ARG_EMBEDDED_FRAMEWORKS})
endfunction()



include(ExternalProject)

function(add_third_party)

    cmake_parse_arguments(
        "_ARG" "CUSTOM_PROJECT" "NAME;SOURCE_DIR;BINARY_DIR;INSTALL_DIR" "DEPS;CMAKE_BUILD_ARGS;CONF;BUILD;INSTALL" 
        ${ARGN})


    if(NOT ${_ARG_CUSTOM_PROJECT})
        message("CMAKE PROJECT:${_ARG_NAME}")
        ExternalProject_Add(
            ${_ARG_NAME}
            SOURCE_DIR "${_ARG_SOURCE_DIR}"
            BINARY_DIR "${_ARG_BINARY_DIR}"
            INSTALL_DIR "${_ARG_INSTALL_DIR}"
            CONFIGURE_COMMAND ${CMAKE_COMMAND} -S ${_ARG_SOURCE_DIR} -G${CMAKE_GENERATOR} -B ${_ARG_BINARY_DIR} -DCMAKE_INSTALL_PREFIX=${_ARG_INSTALL_DIR} ${_ARG_CMAKE_BUILD_ARGS}
            BUILD_COMMAND ${CMAKE_COMMAND} --build ${_ARG_BINARY_DIR}
            INSTALL_COMMAND ${CMAKE_COMMAND} --install ${_ARG_BINARY_DIR}
            DEPENDS ${_ARG_DEPS}
        )
    else()
        message("CUSTOM PROJECT:${_ARG_NAME}")
        ExternalProject_Add(
            ${_ARG_NAME}
            SOURCE_DIR "${_ARG_SOURCE_DIR}"
            BINARY_DIR "${_ARG_BINARY_DIR}"
            INSTALL_DIR "${_ARG_INSTALL_DIR}"
            CONFIGURE_COMMAND "${_ARG_CONF}"
            BUILD_COMMAND "${_ARG_BUILD}"
            INSTALL_COMMAND "${_ARG_INSTALL}"
            DEPENDS ${_ARG_DEPS}
        )
    endif()
    
endfunction()




