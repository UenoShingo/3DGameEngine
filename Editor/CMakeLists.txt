add_executable(Editor
    EditorApp/main.cpp
    EditorApp/EditorApplication.h
    EditorApp/EditorApplication.cpp
)

target_link_libraries(Editor
    PRIVATE Engine
)

target_compile_options(Editor
    PRIVATE /W4 /permissive- /utf-8
)

add_custom_command(
    TARGET Editor
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${DXC_RUNTIME_DIR}/dxcompiler.dll"
        "$<TARGET_FILE_DIR:Editor>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${DXC_RUNTIME_DIR}/dxil.dll"
        "$<TARGET_FILE_DIR:Editor>"
    VERBATIM
)
