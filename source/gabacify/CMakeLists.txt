project("gabacify")

set(source_files
        analyze.cc
        code.cc
        main.cc
        program-options.cc
)

add_executable(gabacify ${source_files})

target_link_libraries(gabacify PRIVATE gabac)
