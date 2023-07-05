add_library(
    libTrie
    OBJECT 
    trie.cpp
    trie_store.cpp)

set(ALL_OBJECT_FILES
    ${ALL_OBJECT_FILES} $<TARGET_OBJECTS:libTrie>
    PARENT_SCOPE)