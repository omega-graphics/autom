# OmegaGSuite CMake Functions


include(CMakeParseArguments)

set(CMAKE_C_STANDARD_REQUIRED YES)
set(CMAKE_C_STANDARD 11)

set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_STANDARD 17)

	
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
	set(PYTHON TRUE CACHE INTERNAL "Python 3 has been found!")
else()
    message(FATAL_ERROR "Python 3 is NOT found...")
endif()



set(CODESIGN_SCRIPT ${CMAKE_CURRENT_LIST_DIR}/codesign.py)



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


if(APPLE)
	if(NOT CODE_SIGNATURE)
		message(FATAL_ERROR "CODE_SIGNATURE Variable must be defined in order to sign Apple App and Framework Bundles.")
	endif()
	
	set(APP_BUNDLE_OUTPUT_DIR "${CMAKE_BINARY_DIR}/Apps")
	set(FRAMEWORK_OUTPUT_DIR "${CMAKE_BINARY_DIR}/Frameworks")
	
	set(UNSIGNED_TARGET_SUFFIX __unsigned)
endif()



if(XCODE)
    set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_IDENTITY ${CODE_SIGNATURE})
else()

    macro(code_sign_bundle _NAME IS_APP VERSION OUTPUT_DIR)
        if(TARGET ${_NAME})
            if(IS_APP)
                set(_OUT "${APP_BUNDLE_OUTPUT_DIR}/${_NAME}.app/Contents/_CodeSignature")
				set(_CODE "${APP_BUNDLE_OUTPUT_DIR}/${_NAME}.app/Contents")
            else()
                set(_OUT "${FRAMEWORK_OUTPUT_DIR}/${_NAME}.framework/Versions/${VERSION}/_CodeSignature")
				set(_CODE "${FRAMEWORK_OUTPUT_DIR}/${_NAME}.framework/Versions/${VERSION}")
            endif()
            
            if(IS_APP)
                add_custom_command(OUTPUT "${OUTPUT_DIR}/_CodeSignature"
                COMMAND ${PYEXEC} ${CODESIGN_SCRIPT} 
                --sig ${CODE_SIGNATURE} --code ${_CODE}
                DEPENDS "${_NAME}${UNSIGNED_TARGET_SUFFIX};${_NAME}"
				COMMENT "Code Signing App Bundle ${_NAME}.app")
            else()
                add_custom_command(OUTPUT "${OUTPUT_DIR}/_CodeSignature"
                COMMAND ${PYEXEC} ${CODESIGN_SCRIPT} --sig ${CODE_SIGNATURE} 
                --code ${_CODE} 
                --framework
                -F "${FRAMEWORK_OUTPUT_DIR}/${_NAME}.framework"
                --name ${_NAME}
                --current_version ${VERSION}
                DEPENDS "${_NAME}${UNSIGNED_TARGET_SUFFIX};${_NAME}"
				COMMENT "Code Signing Framework Bundle ${_NAME}.framework"
                )
            endif()
            add_custom_target("${_NAME}__codesign" DEPENDS "${OUTPUT_DIR}/_CodeSignature")
        endif()
    endmacro(code_sign_bundle)  
    
endif()

function(add_framework_bundle)
    cmake_parse_arguments(
    "_ARG" 
	
    "" 

    "NAME;
	INFO_PLIST;
	VERSION" 
	
    "SOURCES;
    RESOURCES;
    DEPS;
    LIBS;
    FRAMEWORKS;EMBEDDED_FRAMEWORKS" 
    ${ARGN})
	

    message("EMBEDDED_FRAMEWORKS:${_ARG_EMBEDDED_FRAMEWORKS}")
	set(_NAME ${_ARG_NAME})
	
	
    add_library(${_NAME} SHARED ${_ARG_SOURCES})
	
	# get_target_property(_PREFIX ${_NAME} SUFFIX)
		#
	# message("${_PREFIX}")
	message("Framework Output Dir:${FRAMEWORK_OUTPUT_DIR}")
	file(MAKE_DIRECTORY ${FRAMEWORK_OUTPUT_DIR}/${_ARG_NAME}.framework)
	
    set_target_properties(${_ARG_NAME} 
    PROPERTIES
	SUFFIX ""
	PREFIX ""
	MACOSX_RPATH TRUE
    LIBRARY_OUTPUT_DIRECTORY "${FRAMEWORK_OUTPUT_DIR}/${_ARG_NAME}.framework/Versions/${_ARG_VERSION}"
    )

    # if(TARGET ${_NAME})
    #     message("FRAMEWORK:${_NAME}")
    # endif()
	set(UNSIGNED_TARGET "${_NAME}${UNSIGNED_TARGET_SUFFIX}")
    add_custom_target(${UNSIGNED_TARGET})
	add_dependencies(${UNSIGNED_TARGET} ${_ARG_NAME})
	
	set( _ARG_RESOURCES ${_ARG_RESOURCES} ${_ARG_INFO_PLIST})
	# message("RES: ${_ARG_RESOURCES}")
	
	set(ALL_RES_FINAL )
	
	file(MAKE_DIRECTORY "${FRAMEWORK_OUTPUT_DIR}/${_ARG_NAME}.framework/Versions/${_ARG_VERSION}/Resources")
	
    foreach(r ${_ARG_RESOURCES})
		get_filename_component(RES_NAME ${r} NAME)
		set(RES_OUTPUT "${FRAMEWORK_OUTPUT_DIR}/${_NAME}.framework/Versions/${_ARG_VERSION}/Resources/${RES_NAME}")
		list(APPEND ALL_RES_FINAL ${RES_OUTPUT})
        add_custom_command(
		OUTPUT ${RES_OUTPUT}
		COMMAND ${CMAKE_COMMAND} -E copy ${r}  "${FRAMEWORK_OUTPUT_DIR}/${_NAME}.framework/Versions/${_ARG_VERSION}/Resources/${RES_NAME}"
		DEPENDS ${r}
		)
    endforeach()
	
    add_custom_target("${_NAME}__res" DEPENDS ${ALL_RES_FINAL})
    add_dependencies(${UNSIGNED_TARGET} "${_NAME}__res")

    # if(${_ARG_EMBEDDED_HEADERS})
    #     set_target_properties()
    # set_source_files_properties(${_ARG_RESOURCES} MACOSX_PACKAGE_LOCATION "Resources")
	
	add_custom_target(${_NAME}.framework)

    if(XCODE)
		if(_ARG_EMBEDDED_FRAMEWORKS)
        	set_target_properties(${_NAME} PROPERTIES XCODE_EMBED_FRAMEWORKS ${_ARG_EMBEDDED_FRAMEWORKS})
		endif()
    else()
        set(__outputted_frameworks)
        foreach(f ${_ARG_EMBEDDED_FRAMEWORKS})
            set(__outputted_frameworks ${__outputted_frameworks} "${CMAKE_BINARY_DIR}/Frameworks/${_NAME}.framework/Versions/${_ARG_VERSION}/Frameworks/${f}.framework")
            add_dependencies(${_NAME} ${f})
            add_custom_command(
                OUTPUT "${FRAMEWORK_OUTPUT_DIR}/${_NAME}.framework/Versions/${_ARG_VERSION}/Frameworks/${f}.framework"
                COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_BUNDLE_DIR:${f}>"  "${FRAMEWORK_OUTPUT_DIR}/${_NAME}.framework/Versions/${_ARG_VERSION}/Frameworks/${f}.framework"  
                DEPENDS ${f})
        endforeach()
        add_custom_target("${_NAME}__framework_embed" DEPENDS ${__outputted_frameworks})
        add_dependencies(${UNSIGNED_TARGET} "${_NAME}__framework_embed")
    
        code_sign_bundle(${_NAME} FALSE ${_ARG_VERSION} "${FRAMEWORK_OUTPUT_DIR}/${_NAME}.framework/Versions/${_ARG_VERSION}")
		add_dependencies(${_NAME}.framework "${_NAME}__codesign")
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
        DEPENDS;
        LIBS;
        EMBEDDED_FRAMEWORKS" ${ARGN})

    message("EMBEDDED_FRAMEWORKS:${_ARG_EMBEDDED_FRAMEWORKS}")

    file(MAKE_DIRECTORY "${APP_BUNDLE_OUTPUT_DIR}/${_ARG_NAME}.app/Contents/MacOS")
    file(COPY ${_ARG_INFO_PLIST} DESTINATION ${APP_BUNDLE_OUTPUT_DIR}/${_ARG_NAME}.app/Contents)

    add_executable("${_ARG_NAME}__exec" ${_ARG_SOURCES})
    set_target_properties("${_ARG_NAME}__exec"
    PROPERTIES
    RUNTIME_OUTPUT_NAME ${_ARG_NAME}
    RUNTIME_OUTPUT_DIRECTORY "${APP_BUNDLE_OUTPUT_DIR}/${_ARG_NAME}.app/Contents/MacOS")

    add_custom_target(${_NAME})

    set(_RES_FINAL)

    foreach(r ${_ARG_RESOURCES})
        get_filename_component(RES_NAME ${r} NAME)
        list(APPEND _RES_FINAL ${RES_NAME})
        add_custom_command(
            OUTPUT "${APP_BUNDLE_OUTPUT_DIR}/${_ARG_NAME}.app/Contents/${RES_NAME}"
            COMMAND ${CMAKE_COMMAND} -E copy ${r} "${APP_BUNDLE_OUTPUT_DIR}/${_ARG_NAME}.app/Contents/${RES_NAME}"  
            DEPENDS ${f})
    endforeach()

    add_custom_target("${_NAME}__res" DEPENDS ${_RES_FINAL})
    add_dependencies(${_NAME} "${_NAME}__res")

    if(XCODE)
        set_target_properties(${_NAME} PROPERTIES XCODE_EMBED_FRAMEWORKS ${_ARG_EMBEDDED_FRAMEWORKS})
    else()

        set(__outputted_frameworks)
        file(MAKE_DIRECTORY "${APP_BUNDLE_OUTPUT_DIR}/${_ARG_NAME}.app/Contents/Frameworks")
        foreach(f ${_ARG_EMBEDDED_FRAMEWORKS})
            set(__outputted_frameworks ${__outputted_frameworks} "${CMAKE_BINARY_DIR}/Apps/${_ARG_NAME}.app/Contents/Frameworks/${f}.framework")
            add_dependencies(${_ARG_NAME} ${f})
            add_custom_command(
                OUTPUT "${APP_BUNDLE_OUTPUT_DIR}/${_ARG_NAME}.app/Contents/Frameworks/${f}.framework"
                COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_BUNDLE_DIR:${f}> "${APP_BUNDLE_OUTPUT_DIR}/${_ARG_NAME}.app/Contents/Frameworks/${f}.framework"  
                DEPENDS ${f})
        endforeach()
        add_custom_target("${_NAME}__framework_embed" DEPENDS ${__outputted_frameworks})
        add_dependencies("${_NAME}" "${_NAME}__framework_embed")

        code_sign_bundle(${_ARG_NAME} TRUE "" "${APP_BUNDLE_OUTPUT_DIR}/${_NAME}.app/Contents")
    endif()
    
    foreach(_dep ${_ARG_DEPS})
        add_dependencies(${_ARG_NAME} ${_dep})
    endforeach()

    target_link_libraries(${_ARG_NAME} PRIVATE ${_ARG_LIBS} ${_ARG_FRAMEWORKS} ${_ARG_EMBEDDED_FRAMEWORKS})
endfunction()

function(target_link_system_frameworks _NAME)
	set(FRAMEWORKS_TO_LINK ${ARGN})
	set(FRAMEWORKS_FLAGS)
	foreach(f ${FRAMEWORKS_TO_LINK})
		set(FRAMEWORKS_FLAGS "${FRAMEWORKS_FLAGS} -framework ${f}")
	endforeach()
	get_target_property(LINK_FLAGS_PRIOR ${_NAME} LINK_FLAGS)
	if(${LINK_FLAGS_PRIOR} STREQUAL "LINK_FLAGS_PRIOR-NOTFOUND")
		set_target_properties(${_NAME} PROPERTIES LINK_FLAGS ${FRAMEWORKS_FLAGS})
	else()
		set_target_properties(${_NAME} PROPERTIES LINK_FLAGS "${LINK_FLAGS_PRIOR} ${FRAMEWORKS_FLAGS}")
	endif()
	
endfunction()

function(target_link_frameworks _NAME)
	set(FRAMEWORKS_TO_LINK ${ARGN})
	set(FRAMEWORKS_FLAGS "-F${FRAMEWORK_OUTPUT_DIR}")
	set(FRAMEWORK_INCLUDE_DIRS)
	foreach(f ${FRAMEWORKS_TO_LINK})
		get_target_property(INC_DIR ${f} INCLUDE_DIRECTORIES)
		# message("${f} Include Dir: ${INC_DIR}")
		set(FRAMEWORK_INCLUDE_DIRS "${FRAMEWORK_INCLUDE_DIRS};${INC_DIR}")
		set(FRAMEWORKS_FLAGS "${FRAMEWORKS_FLAGS} -framework ${f}")
	endforeach()
	get_target_property(LINK_FLAGS_PRIOR ${_NAME} LINK_FLAGS)
	
	if(${LINK_FLAGS_PRIOR} STREQUAL "LINK_FLAGS_PRIOR-NOTFOUND")
		set_target_properties(${_NAME} PROPERTIES LINK_FLAGS ${FRAMEWORKS_FLAGS})
	else()
		set_target_properties(${_NAME} PROPERTIES LINK_FLAGS "${LINK_FLAGS_PRIOR} ${FRAMEWORKS_FLAGS}")
	endif()
	
	target_include_directories(${_NAME} PRIVATE ${FRAMEWORK_INCLUDE_DIRS})
	
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
            CONFIGURE_COMMAND ${CMAKE_COMMAND} -S ${_ARG_SOURCE_DIR} -G${CMAKE_GENERATOR} -B ${_ARG_BINARY_DIR} -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=${_ARG_INSTALL_DIR} ${_ARG_CMAKE_BUILD_ARGS}
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




function(add_omega_graphics_tool _NAME)
	cmake_parse_arguments("_ARG" "" "" "LIBS;SOURCES" ${ARGN})
	set(_SOURCES ${_ARG_SOURCES})
	add_executable(${_NAME} ${_SOURCES})
	set_target_properties(${_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
	install(TARGETS ${_NAME} RUNTIME DESTINATION bin)
    foreach(dep ${_ARG_LIBS})
        add_dependencies(${_NAME} ${dep})
    endforeach()
    target_link_libraries(${_NAME} PRIVATE ${_ARG_LIBS})
endfunction()



function(add_omega_graphics_module _NAME)
	
	cmake_parse_arguments("_ARG" "STATIC;SHARED;FRAMEWORK" "HEADER_DIR;INFO_PLIST;VERSION" "DEPENDS;SOURCES" ${ARGN})
	
	message("-- Adding Module:${_NAME}")

	if(${_ARG_STATIC})
		add_library(${_NAME} STATIC ${_ARG_SOURCES})
		set_target_properties(${_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
		install(TARGETS ${_NAME} ARCHIVE DESTINATION lib)

		foreach(dep ${_ARG_DEPENDS})
			add_dependencies(${_NAME} ${dep})
		endforeach()
	else()

		if(APPLE AND ${_ARG_FRAMEWORK})
			if(_ARG_DEPENDS)
				add_framework_bundle(NAME ${_NAME} INFO_PLIST ${_ARG_INFO_PLIST} VERSION ${_ARG_VERSION} DEPENDS ${_ARG_DEPENDS} SOURCES ${_ARG_SOURCES})
			else()
				add_framework_bundle(NAME ${_NAME} INFO_PLIST ${_ARG_INFO_PLIST} VERSION ${_ARG_VERSION} SOURCES ${_ARG_SOURCES})
			endif()
			install(DIRECTORY ${FRAMEWORK_OUTPUT_DIR}/${_NAME}.framework DESTINATION lib)
		else()

			add_library(${_NAME} SHARED ${_ARG_SOURCES})
			if(WIN32)
				set_target_properties(${_NAME} PROPERTIES 
				ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib 
				RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
				
				install(TARGETS ${_NAME} RUNTIME DESTINATION bin ARCHIVE DESTINATION lib)
			else()
				set_target_properties(${_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
				install(TARGETS ${_NAME} LIBRARY DESTINATION lib)
			endif()

			foreach(dep ${_ARG_DEPENDS})
				add_dependencies(${_NAME} ${dep})
			endforeach()

		endif()
	
	endif()

	install(DIRECTORY ${_ARG_HEADER_DIR} DESTINATION "include")
    target_include_directories(${_NAME} PUBLIC ${_ARG_HEADER_DIR})



	

endfunction()

function(omega_graphics_project _NAME)
	if(NOT ${${_NAME}_INCLUDE})
		project(${_NAME} ${ARGN})
	endif()
endfunction()



function(omega_graphics_add_subdir _PROJECT_NAME _NAME)
	
	set(${_PROJECT_NAME}_INCLUDE TRUE)
	add_subdirectory(${_NAME})
	unset(${_PROJECT_NAME}_INCLUDE)
	
endfunction()





