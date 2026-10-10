add_executable(Editor
    EditorApp/main.cpp
)

target_link_libraries(Editor
    PRIVATE Engine
)

target_compile_options(Editor
    PRIVATE /W4 /permissive- /utf-8
)
