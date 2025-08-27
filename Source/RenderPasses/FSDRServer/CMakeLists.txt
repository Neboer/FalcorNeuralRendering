add_plugin(FSDRServer)

target_sources(FSDRServer PRIVATE
    FSDRServer.cpp
    FSDRServer.h
    HTTPBackend.cpp
    HTTPBackend.h
    ABSynchronizer.cpp
    ABSynchronizer.h
)

target_source_group(FSDRServer "RenderPasses")
target_link_libraries(FSDRServer PRIVATE httplib::httplib)