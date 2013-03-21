set(LUCI_EXECUTABLE luci)

find_package(BISON)
find_package(FLEX)

#BISON_TARGET(myparser parser.y ${CMAKE_CURRENT_SOURCE_DIR}/parser.tab.c VERBOSE parser.output COMPILE_FLAGS -d)
BISON_TARGET(myparser parser.y ${CMAKE_CURRENT_SOURCE_DIR}/parser.tab.c COMPILE_FLAGS -d)
FLEX_TARGET(myscanner lexer.l ${CMAKE_CURRENT_SOURCE_DIR}/lexer.yy.c)
ADD_FLEX_BISON_DEPENDENCY(myscanner myparser)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(
    ${LUCI_EXECUTABLE}
    ${BISON_myparser_OUTPUTS}
    ${FLEX_myscanner_OUTPUTS}
    ${CMAKE_CURRENT_SOURCE_DIR}/main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/gc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/stack.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ast.c
    ${CMAKE_CURRENT_SOURCE_DIR}/object.c
    ${CMAKE_CURRENT_SOURCE_DIR}/map.c
    ${CMAKE_CURRENT_SOURCE_DIR}/symbol.c
    ${CMAKE_CURRENT_SOURCE_DIR}/constant.c
    ${CMAKE_CURRENT_SOURCE_DIR}/compile.c
    ${CMAKE_CURRENT_SOURCE_DIR}/interpret.c
    ${CMAKE_CURRENT_SOURCE_DIR}/builtin.c
)

# link only against math library
target_link_libraries(${LUCI_EXECUTABLE} m)

install(TARGETS ${LUCI_EXECUTABLE} DESTINATION bin)
