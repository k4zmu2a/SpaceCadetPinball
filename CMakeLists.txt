cmake_minimum_required(VERSION 3.16)
project(SpaceCadetPinball)

set(CMAKE_CXX_STANDARD 11)

option(MUSIC_TSF "Use TinySoundFont for MIDI playback" ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/CMakeModules")

# On Windows, set paths to SDL-devel packages here
if(WIN32)
   set(SDL2_PATH "${CMAKE_CURRENT_LIST_DIR}/Libs/SDL2")
   set(SDL2_MIXER_PATH "${CMAKE_CURRENT_LIST_DIR}/Libs/SDL2_mixer")
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
        set(USE_FLAGS "-s USE_SDL=2 -s USE_SDL_MIXER=2")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${USE_FLAGS}")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${USE_FLAGS}")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${USE_FLAGS}")
        set(CMAKE_EXECUTABLE_SUFFIX .html)

        set(SDL2_FOUND TRUE)
        set(SDL2_INCLUDE_DIR "${EMSCRIPTEN_ROOT_PATH}/system/include/SDL2/")
        set(SDL2_MIXER_INCLUDE_DIR "${EMSCRIPTEN_ROOT_PATH}/system/include/SDL2/")
        set(SDL2_LIBRARIES "nul")
else()
        find_package(SDL2 REQUIRED)
        find_package(SDL2_mixer REQUIRED)
endif()

include_directories(${SDL2_INCLUDE_DIR} ${SDL2_MIXER_INCLUDE_DIR})
get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
  message(STATUS "Include dir='${dir}'")
endforeach()

set(SOURCE_FILES
        SpaceCadetPinball/control.cpp
        SpaceCadetPinball/control.h
        SpaceCadetPinball/fullscrn.cpp
        SpaceCadetPinball/fullscrn.h
        SpaceCadetPinball/gdrv.cpp
        SpaceCadetPinball/gdrv.h
        SpaceCadetPinball/GroupData.cpp
        SpaceCadetPinball/GroupData.h
        SpaceCadetPinball/high_score.cpp
        SpaceCadetPinball/high_score.h
        SpaceCadetPinball/loader.cpp
        SpaceCadetPinball/loader.h
        SpaceCadetPinball/maths.cpp
        SpaceCadetPinball/maths.h
        SpaceCadetPinball/memory.cpp
        SpaceCadetPinball/memory.h
        SpaceCadetPinball/midi.cpp
        SpaceCadetPinball/midi.h
        SpaceCadetPinball/nudge.cpp
        SpaceCadetPinball/nudge.h
        SpaceCadetPinball/objlist_class.h
        SpaceCadetPinball/options.cpp
        SpaceCadetPinball/options.h
        SpaceCadetPinball/partman.cpp
        SpaceCadetPinball/partman.h
        SpaceCadetPinball/pb.cpp
        SpaceCadetPinball/pb.h
        SpaceCadetPinball/pch.h
        SpaceCadetPinball/pinball.cpp
        SpaceCadetPinball/pinball.h
        SpaceCadetPinball/proj.cpp
        SpaceCadetPinball/proj.h
        SpaceCadetPinball/render.cpp
        SpaceCadetPinball/render.h
        SpaceCadetPinball/score.cpp
        SpaceCadetPinball/score.h
        SpaceCadetPinball/Sound.cpp
        SpaceCadetPinball/Sound.h
        SpaceCadetPinball/SpaceCadetPinball.cpp
        SpaceCadetPinball/TBall.cpp
        SpaceCadetPinball/TBall.h
        SpaceCadetPinball/TBlocker.cpp
        SpaceCadetPinball/TBlocker.h
        SpaceCadetPinball/TBumper.cpp
        SpaceCadetPinball/TBumper.h
        SpaceCadetPinball/TCircle.cpp
        SpaceCadetPinball/TCircle.h
        SpaceCadetPinball/TCollisionComponent.cpp
        SpaceCadetPinball/TCollisionComponent.h
        SpaceCadetPinball/TComponentGroup.cpp
        SpaceCadetPinball/TComponentGroup.h
        SpaceCadetPinball/TDemo.cpp
        SpaceCadetPinball/TDemo.h
        SpaceCadetPinball/TDrain.cpp
        SpaceCadetPinball/TDrain.h
        SpaceCadetPinball/TEdgeBox.cpp
        SpaceCadetPinball/TEdgeBox.h
        SpaceCadetPinball/TEdgeManager.cpp
        SpaceCadetPinball/TEdgeManager.h
        SpaceCadetPinball/TEdgeSegment.cpp
        SpaceCadetPinball/TEdgeSegment.h
        SpaceCadetPinball/TFlagSpinner.cpp
        SpaceCadetPinball/TFlagSpinner.h
        SpaceCadetPinball/TFlipper.cpp
        SpaceCadetPinball/TFlipper.h
        SpaceCadetPinball/TFlipperEdge.cpp
        SpaceCadetPinball/TFlipperEdge.h
        SpaceCadetPinball/TGate.cpp
        SpaceCadetPinball/TGate.h
        SpaceCadetPinball/THole.cpp
        SpaceCadetPinball/THole.h
        SpaceCadetPinball/timer.cpp
        SpaceCadetPinball/timer.h
        SpaceCadetPinball/TKickback.cpp
        SpaceCadetPinball/TKickback.h
        SpaceCadetPinball/TKickout.cpp
        SpaceCadetPinball/TKickout.h
        SpaceCadetPinball/TLight.cpp
        SpaceCadetPinball/TLight.h
        SpaceCadetPinball/TLightBargraph.cpp
        SpaceCadetPinball/TLightBargraph.h
        SpaceCadetPinball/TLightGroup.cpp
        SpaceCadetPinball/TLightGroup.h
        SpaceCadetPinball/TLightRollover.cpp
        SpaceCadetPinball/TLightRollover.h
        SpaceCadetPinball/TLine.cpp
        SpaceCadetPinball/TLine.h
        SpaceCadetPinball/TOneway.cpp
        SpaceCadetPinball/TOneway.h
        SpaceCadetPinball/TPinballComponent.cpp
        SpaceCadetPinball/TPinballComponent.h
        SpaceCadetPinball/TPinballTable.cpp
        SpaceCadetPinball/TPinballTable.h
        SpaceCadetPinball/TPlunger.cpp
        SpaceCadetPinball/TPlunger.h
        SpaceCadetPinball/TPopupTarget.cpp
        SpaceCadetPinball/TPopupTarget.h
        SpaceCadetPinball/TRamp.cpp
        SpaceCadetPinball/TRamp.h
        SpaceCadetPinball/TRollover.cpp
        SpaceCadetPinball/TRollover.h
        SpaceCadetPinball/TSink.cpp
        SpaceCadetPinball/TSink.h
        SpaceCadetPinball/TSoloTarget.cpp
        SpaceCadetPinball/TSoloTarget.h
        SpaceCadetPinball/TSound.cpp
        SpaceCadetPinball/TSound.h
        SpaceCadetPinball/TTableLayer.cpp
        SpaceCadetPinball/TTableLayer.h
        SpaceCadetPinball/TTextBox.cpp
        SpaceCadetPinball/TTextBox.h
        SpaceCadetPinball/TTextBoxMessage.cpp
        SpaceCadetPinball/TTextBoxMessage.h
        SpaceCadetPinball/TTimer.cpp
        SpaceCadetPinball/TTimer.h
        SpaceCadetPinball/TTripwire.cpp
        SpaceCadetPinball/TTripwire.h
        SpaceCadetPinball/TWall.cpp
        SpaceCadetPinball/TWall.h
        SpaceCadetPinball/winmain.cpp
        SpaceCadetPinball/winmain.h
        SpaceCadetPinball/zdrv.cpp
        SpaceCadetPinball/zdrv.h
        SpaceCadetPinball/imconfig.h
        SpaceCadetPinball/imgui_internal.h
        SpaceCadetPinball/imgui.cpp
        SpaceCadetPinball/imgui.h
        SpaceCadetPinball/imgui_sdl.cpp
        SpaceCadetPinball/imgui_sdl.h
        SpaceCadetPinball/imgui_draw.cpp
        SpaceCadetPinball/imgui_widgets.cpp
        SpaceCadetPinball/imgui_tables.cpp
        SpaceCadetPinball/imgui_demo.cpp
        SpaceCadetPinball/imgui_impl_sdl.cpp
        SpaceCadetPinball/imgui_impl_sdl.h
        SpaceCadetPinball/imstb_textedit.h
        SpaceCadetPinball/imstb_rectpack.h
        SpaceCadetPinball/imstb_truetype.h
        )

if (MUSIC_TSF)
        set(SOURCE_FILES ${SOURCE_FILES}
                SpaceCadetPinball/gm_sf2.cpp
                SpaceCadetPinball/tsf.cpp
                SpaceCadetPinball/tml.h
                SpaceCadetPinball/tsf.h)
endif()

add_executable(SpaceCadetPinball ${SOURCE_FILES})

# Skip pch on foreign code
set_source_files_properties(
        SpaceCadetPinball/imgui.cpp
        SpaceCadetPinball/imgui_sdl.cpp
        SpaceCadetPinball/imgui_draw.cpp
        SpaceCadetPinball/imgui_widgets.cpp
        SpaceCadetPinball/imgui_tables.cpp
        SpaceCadetPinball/imgui_demo.cpp
        SpaceCadetPinball/imgui_impl_sdl.cpp
        SpaceCadetPinball/gm_sf2.cpp
        SpaceCadetPinball/tsf.cpp        

        PROPERTIES SKIP_PRECOMPILE_HEADERS 1
)
target_precompile_headers(SpaceCadetPinball
  PUBLIC
    SpaceCadetPinball/pch.h
)

if (MUSIC_TSF)
        target_compile_definitions(SpaceCadetPinball PRIVATE -DMUSIC_TSF)
else()
        target_compile_definitions(SpaceCadetPinball PRIVATE -DMUSIC_SDL)
endif()

target_link_libraries(SpaceCadetPinball ${SDL2_LIBRARY} ${SDL2_MIXER_LIBRARY})

if(${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
        target_link_libraries(SpaceCadetPinball idbfs.js)
        set_target_properties(SpaceCadetPinball PROPERTIES LINK_FLAGS
        "-s ALLOW_MEMORY_GROWTH=1 -s FORCE_FILESYSTEM=1 -s DEMANGLE_SUPPORT=1 \
        --shell-file ${CMAKE_CURRENT_SOURCE_DIR}/SpaceCadetPinball/emscripten_shell.html \
        --preload-file ${CMAKE_CURRENT_SOURCE_DIR}/game_resources@game_resources --bind")
endif()

# On Windows, copy DLL to output
if(WIN32)
    list(GET SDL2_LIBRARY 1 SDL2_LIB_PATH)
    get_filename_component(SDL2_DLL_PATH ${SDL2_LIB_PATH} DIRECTORY)
    get_filename_component(SDL2_MIXER_DLL_PATH ${SDL2_MIXER_LIBRARY} DIRECTORY)
    message(STATUS "copy paths='${SDL2_DLL_PATH}' '${SDL2_MIXER_DLL_PATH}'") 
    add_custom_command(TARGET SpaceCadetPinball POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${SDL2_DLL_PATH}/SDL2.dll" $<TARGET_FILE_DIR:SpaceCadetPinball>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${SDL2_MIXER_DLL_PATH}/SDL2_mixer.dll" $<TARGET_FILE_DIR:SpaceCadetPinball>
        )
endif()