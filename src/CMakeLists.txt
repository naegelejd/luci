set(LUCI_EXECUTABLE luci)

find_package(BISON)
find_package(FLEX)

#BISON_TARGET(myparser parser.y ${CMAKE_CURRENT_SOURCE_DIR}/parser.tab.c VERBOSE parser.output COMPILE_FLAGS -d)
BISON_TARGET(myparser parser.y ${CMAKE_CURRENT_SOURCE_DIR}/parser.tab.c COMPILE_FLAGS -d)
FLEX_TARGET(myscanner lexer.l ${CMAKE_CURRENT_SOURCE_DIR}/lexer.yy.c)
ADD_FLEX_BISON_DEPENDENCY(myscanner myparser)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(LUCI_SOURCE_FILES
    main.c
    gc.c
    ast.c
    lucitypes.c
    inttype.c
    floattype.c
    stringtype.c
    listtype.c
    maptype.c
    functiontype.c
    iteratortype.c
    filetype.c
    nativefunctype.c
    symbol.c
    constant.c
    compile.c
    interpret.c
    builtin.c
)

set(LUCI_HEADER_FILES
    luci.h
    gc.h
    ast.h
    lucitypes.h
    inttype.h
    floattype.h
    stringtype.h
    listtype.h
    maptype.h
    functiontype.h
    iteratortype.h
    filetype.h
    nativefunctype.h
    symbol.h
    constant.h
    compile.h
    interpret.h
    builtin.h
    dispatch.h 
)

add_executable(
    ${LUCI_EXECUTABLE}
    ${BISON_myparser_OUTPUTS}
    ${FLEX_myscanner_OUTPUTS}
    ${LUCI_SOURCE_FILES}
)

# link only against math library
target_link_libraries(${LUCI_EXECUTABLE} m)

install(TARGETS ${LUCI_EXECUTABLE} DESTINATION bin)

set_source_files_properties(tags PROPERTIES GENERATED true)
add_custom_command(
    OUTPUT tags
    COMMAND ctags ${LUCI_SOURCE_FILES} ${LUCI_HEADER_FILES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
add_custom_target(ctags DEPENDS tags)
