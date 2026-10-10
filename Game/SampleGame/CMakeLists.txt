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
