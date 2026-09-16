file(GLOB SOURCES
        bitshuffle/*.c
        bitshuffle/*.cpp)

add_library(external STATIC ${SOURCES})
# Vendored code: SYSTEM keeps the headers (rapidjson, date.h) out of the
# -Wall -Wextra output where our own sources include them, while -w does the
# same for the bitshuffle sources compiled into this target.
target_include_directories(external SYSTEM PUBLIC .)
target_compile_options(external PRIVATE -w)
