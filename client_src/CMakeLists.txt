file(GLOB CPP_SOURCES "*.cpp")
file(GLOB H_SOURCES "*.h")

target_sources(game_client
    PRIVATE ${CPP_SOURCES} ${H_SOURCES}
)