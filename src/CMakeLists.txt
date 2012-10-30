find_package(BISON)
find_package(FLEX)

#BISON_TARGET(myparser parser.y ${CMAKE_CURRENT_SOURCE_DIR}/parser.tab.c VERBOSE parser.output COMPILE_FLAGS -d)
BISON_TARGET(myparser parser.y ${CMAKE_CURRENT_SOURCE_DIR}/parser.tab.c COMPILE_FLAGS -d)
FLEX_TARGET(myscanner lexer.l ${CMAKE_CURRENT_SOURCE_DIR}/lexer.yy.c)
ADD_FLEX_BISON_DEPENDENCY(myscanner myparser)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(
    ${PROJECT_NAME}
    ${BISON_myparser_OUTPUTS}
    ${FLEX_myscanner_OUTPUTS}
    ${CMAKE_CURRENT_SOURCE_DIR}/main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/common.c
    ${CMAKE_CURRENT_SOURCE_DIR}/stack.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ast.c
    ${CMAKE_CURRENT_SOURCE_DIR}/object.c
    ${CMAKE_CURRENT_SOURCE_DIR}/symbol.c
    ${CMAKE_CURRENT_SOURCE_DIR}/constant.c
    ${CMAKE_CURRENT_SOURCE_DIR}/compile.c
    ${CMAKE_CURRENT_SOURCE_DIR}/interpret.c
    # ${CMAKE_CURRENT_SOURCE_DIR}/env.c
    ${CMAKE_CURRENT_SOURCE_DIR}/functions.c
)

find_package(DOXYGEN)
if(DOXYGEN_FOUND)
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
        ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        @ONLY
    )
    add_custom_target(
        doc
        ${DOXYGEN_EXECUTABLE}
        ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API Documentation with Doxygen" VERBATIM
    )
endif(DOXYGEN_FOUND)
