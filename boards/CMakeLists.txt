option (BOARD_RP2040_GEEK "Enable board support for the Waveshare RP2040-GEEK board" OFF)
if (BOARD_RP2040_GEEK)
    # Waveshare LCD driver example
    add_subdirectory(external/rp2040_geek/01-LCD/lib/Config)
    add_subdirectory(external/rp2040_geek/01-LCD/lib/LCD)
    add_subdirectory(external/rp2040_geek/01-LCD/lib/Fonts)
    add_subdirectory(external/rp2040_geek/01-LCD/lib/GUI)

    add_library(board_ui
        # UI frontend
        src/board_rp2040_geek.c
    )

    target_include_directories(board_ui
        PRIVATE
            external/rp2040_geek/01-LCD/lib/Config
            external/rp2040_geek/01-LCD/lib/LCD
            external/rp2040_geek/01-LCD/lib/Fonts
            external/rp2040_geek/01-LCD/lib/GUI
    )

    target_link_libraries(board_ui
        PRIVATE
            LCD GUI Fonts Config
    )

else ()
    # NULL board
    add_library(board_ui
        board_null.c
    )
endif ()

target_include_directories(
    board_ui
        PUBLIC  inc
        PRIVATE ../src
)
