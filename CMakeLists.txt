cmake_minimum_required(VERSION 2.8)

project(luci)

#set(CMAKE_BUILD_TYPE Release)
set(CMAKE_BUILD_TYPE Debug)

add_definitions(-Wall)# -Weverything -pedantic)

# Uncomment the next line to enable lots of debug output during runtime
# add_definitions(-DDEBUG)

if(CMAKE_COMPILER_IS_GNUCC)
    # for address-of-label and indirect-goto GNU extensions
    # i.e. computed gotos in interpret.c
    add_definitions(-fno-gcse)  
endif()

# variables used by Doxygen configuration
set(LUCI_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(LUCI_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# cmake Luci itself
add_subdirectory(${LUCI_SOURCE_DIR} bin)

# cmake some tests
enable_testing()
add_subdirectory(tests)

# cmake some documentation
find_package(Doxygen)
if(DOXYGEN_FOUND)
    set(DOXYGEN_TARGET "doc")
    set(DOXYGEN_OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${DOXYGEN_TARGET}/html/index.html)
    set(DOXYFILE_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
    set(DOXYFILE ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    # configure Doxyfile.in, generating Doxyfile
    configure_file(${DOXYFILE_IN} ${DOXYFILE} @ONLY)

    add_custom_command(
        OUTPUT ${DOXYGEN_OUTPUT}
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API Documentation with Doxygen..." VERBATIM
        DEPENDS ${DOXYFILE}
    )

    add_custom_target(${DOXYGEN_TARGET} DEPENDS ${DOXYGEN_OUTPUT})

else(DOXYGEN_FOUND)
    message("Doxygen is needed to build Luci's documentation")
endif(DOXYGEN_FOUND)
