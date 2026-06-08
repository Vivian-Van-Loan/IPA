include_guard(GLOBAL)

find_program(CTR_MKBCFNT_EXE
        NAMES mkbcfnt
        HINTS "${DEVKITPRO}/tools/bin"
)

function(ctr_add_font_target target)
    cmake_parse_arguments(
            CTR_FONT
            ""
            "OUTPUT"
            "INPUTS;OPTIONS"
            ${ARGN}
    )

    if(NOT CTR_MKBCFNT_EXE)
        message(FATAL_ERROR
                "Could not find mkbcfnt: try installing 3ds-tools")
    endif()

    if(DEFINED CTR_FONT_OUTPUT)
        get_filename_component(
                CTR_FONT_OUTPUT
                "${CTR_FONT_OUTPUT}"
                ABSOLUTE
                BASE_DIR "${CMAKE_BINARY_DIR}"
        )
    else()
        set(CTR_FONT_OUTPUT
                "${CMAKE_BINARY_DIR}/${target}.bcfnt")
    endif()

    list(LENGTH CTR_FONT_INPUTS numinputs)

    if(NOT numinputs EQUAL 1)
        message(FATAL_ERROR
                "ctr_add_font_target: exactly one input font required")
    endif()

    list(GET CTR_FONT_INPUTS 0 input)
    dkp_resolve_file(input "${input}")

    add_custom_command(
            OUTPUT "${CTR_FONT_OUTPUT}"
            COMMAND "${CTR_MKBCFNT_EXE}"
            ${CTR_FONT_OPTIONS}
            -o "${CTR_FONT_OUTPUT}"
            "${input}"
            DEPENDS "${input}"
            COMMENT "Building font target ${target}"
            VERBATIM
    )

    add_custom_target(${target} ALL
            DEPENDS "${CTR_FONT_OUTPUT}"
    )

    #
    # Makes dkp_install_assets(TARGETS ...)
    # work exactly like graphics/shaders.
    #
    dkp_set_target_file(${target}
            "${CTR_FONT_OUTPUT}")
endfunction()
