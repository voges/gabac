project("gabacify")

set(source_files
        analyze.cpp
        code.cpp
        main.cpp
        program_options.cpp
)

add_executable(gabacify ${source_files})

target_link_libraries(gabacify PRIVATE gabac)
