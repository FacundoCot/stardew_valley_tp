file(GLOB CPP_SOURCES "*.cpp")
file(GLOB H_SOURCES "*.h")
file(GLOB UI_SOURCES "*.ui")

target_sources(map_editor
    PRIVATE ${CPP_SOURCES} ${H_SOURCES} ${UI_SOURCES}
)
