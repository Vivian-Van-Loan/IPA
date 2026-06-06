cmake_minimum_required(VERSION 3.13)

project(matrix-client LANGUAGES C VERSION 1.0.0)

set (CMAKE_C_STANDARD 23)

find_package(PkgConfig)
pkg_check_modules(curl REQUIRED libcurl)
pkg_check_modules(jansson REQUIRED jansson)
#pkg_check_modules(citro2d REQUIRED IMPORTED_TARGET citro2d)
#pkg_check_modules(citro3d REQUIRED IMPORTED_TARGET citro3d)

set(MATRIXC_SRCS
        main.c
        utils.c
        utils.h
        matrix/matrix-client.h
        matrix/login.c
        matrix/login.h
        matrix/matrix-utils.c
        matrix/matrix-utils.h
        matrix/matrix-client.c
)

add_executable(${PROJECT_NAME} ${MATRIXC_SRCS})

target_compile_options(${PROJECT_NAME} PRIVATE
        -g -O0 -Wall
        ${curl_CFLAGS_OTHER}
        ${jansson_CFLAGS_OTHER}
)

target_include_directories(${PROJECT_NAME} PRIVATE
        include
        ${curl_INCLUDE_DIRS}
        ${jansson_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
        ${curl_LIBRARIES}
        ${jansson_LIBRARIES}
		citro2d
		citro3d
)

ctr_generate_smdh(${PROJECT_NAME}.smdh
        NAME "${PROJECT_NAME}"
        DESCRIPTION "v${PROJECT_VERSION}"
        AUTHOR "A Sickly Silver Moon"
        ICON "3DS IPA.png"
)

# Declare an asset target for the executable's RomFS (optional)
dkp_add_asset_target(${PROJECT_NAME}_romfs romfs)

# Texture atlas (example)
ctr_add_graphics_target(IPA_TEXTURES_MAIN ATLAS
	INPUTS gfx/main/D.png gfx/main/S.png gfx/main/R.png gfx/main/Gear.png
)

ctr_add_graphics_target(IPA_TEXTURES_GRADIENT ATLAS
	INPUTS gfx/gradient/border.png gfx/gradient/button.png
)

# Install assets to RomFS (uncomment as needed)
dkp_install_assets(${PROJECT_NAME}_romfs
	DESTINATION gfx
	TARGETS IPA_TEXTURES_MAIN IPA_TEXTURES_GRADIENT
)

ctr_create_3dsx(${PROJECT_NAME}
        SMDH ${PROJECT_NAME}.smdh
        ROMFS ${PROJECT_NAME}_romfs  # Uncomment this if using RomFS
)
