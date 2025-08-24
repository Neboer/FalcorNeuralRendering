add_plugin(FSDRServer)

target_sources(FSDRServer PRIVATE
    FSDRServer.cpp
    FSDRServer.h
    StreamServer.cpp
    StreamServer.h
    Router/CameraHandler.cpp
    Router/CameraHandler.h
    Router/Handler.h
    Router/Router.h
    Router/SceneHandler.cpp
    Router/SceneHandler.h
    
)

target_source_group(FSDRServer "RenderPasses")
