file(GLOB CPP_SOURCES "*.cpp")
file(GLOB H_SOURCES "*.h")
file(GLOB UI_SOURCES "*.ui")

target_sources(game_client
    PRIVATE ${CPP_SOURCES} ${H_SOURCES} ${UI_SOURCES}
)