
function(stackwell_set_warnings target)

    if(MSVC)

        target_compile_options(${target} PRIVATE
            /W4
            /permissive-
            /EHsc
        )

    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")

        target_compile_options(${target} PRIVATE
            -Wall
            -Wextra
            -Wpedantic
            -Wconversion
            -Wshadow
        )

    endif()

endfunction()
