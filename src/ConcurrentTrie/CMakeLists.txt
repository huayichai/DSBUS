add_library(
    libTrie
    OBJECT 
    trie.cpp)

set(ALL_OBJECT_FILES
    ${ALL_OBJECT_FILES} $<TARGET_OBJECTS:libTrie>
    PARENT_SCOPE)