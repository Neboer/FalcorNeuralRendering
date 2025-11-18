add_plugin(FSDRServer)

target_sources(FSDRServer PRIVATE
    FSDRServer.cpp
    FSDRServer.h
)

target_source_group(FSDRServer "RenderPasses")
