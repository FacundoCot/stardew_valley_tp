file(GLOB CPP_SOURCES "*.cpp")
file(GLOB H_SOURCES "*.h")

target_sources(taller_tests
    PRIVATE ${CPP_SOURCES} ${H_SOURCES}
)