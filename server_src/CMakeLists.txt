file(GLOB CPP_SOURCES "*.cpp")
file(GLOB H_SOURCES "*.h")

target_sources(game_server
    PRIVATE ${CPP_SOURCES} ${H_SOURCES}
)