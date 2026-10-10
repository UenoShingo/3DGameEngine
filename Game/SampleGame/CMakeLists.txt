add_executable(SampleGame
    main.cpp
    SampleGameApplication.h
    SampleGameApplication.cpp
)

target_link_libraries(SampleGame
    PRIVATE Engine
)

target_compile_options(SampleGame
    PRIVATE /W4 /permissive- /utf-8
)

add_custom_command(
    TARGET SampleGame
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${DXC_RUNTIME_DIR}/dxcompiler.dll"
        "$<TARGET_FILE_DIR:SampleGame>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${DXC_RUNTIME_DIR}/dxil.dll"
        "$<TARGET_FILE_DIR:SampleGame>"
    VERBATIM
)
