add_subdirectory(ConcurrentTrie)

add_library(dsbus STATIC ${ALL_OBJECT_FILES})

set(DataStructures_LIBS
    libTrie)

target_link_libraries(
    dsbus
    ${DataStructures_LIBS}
)

target_include_directories(
    dsbus PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
)



