file(GLOB CPP_SOURCES "*.cpp")
file(GLOB H_SOURCES "*.h")

target_sources(map_editor
    PRIVATE ${CPP_SOURCES} ${H_SOURCES}
)