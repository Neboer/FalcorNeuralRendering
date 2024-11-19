from falcor import *

def render_graph_GBuffer(g):
    # 添加GBufferRT pass（生成GBuffer数据）
    GBufferRT = createPass("GBufferRT", {'sampleCount': 1})  # 可以根据需要调整参数
    g.addPass(GBufferRT, "GBufferRT")

    # 如果只想输出Position，而不经过ToneMapper，可以直接标记GBufferRT的posW输出
    # g.markOutput("GBufferRT.posW")

    return g

def render_graph_PathTracer(g):
    PathTracer = createPass("PathTracer", {'samplesPerPixel': 1})
    g.addPass(PathTracer, "PathTracer")
    VBufferRT = createPass("VBufferRT", {'samplePattern': 'Stratified', 'sampleCount': 16, 'useAlphaTest': True})
    g.addPass(VBufferRT, "VBufferRT")
    AccumulatePass = createPass("AccumulatePass", {'enabled': True, 'precisionMode': 'Single'})
    g.addPass(AccumulatePass, "AccumulatePass")
    g.addEdge("VBufferRT.vbuffer", "PathTracer.vbuffer")
    g.addEdge("VBufferRT.viewW", "PathTracer.viewW")
    g.addEdge("VBufferRT.mvec", "PathTracer.mvec")
    g.addEdge("PathTracer.color", "AccumulatePass.input")
    g.markOutput("AccumulatePass.output")
    return g

g = RenderGraph("RadiosityCollecter")
render_graph_GBuffer(g)
render_graph_PathTracer(g)

RadiosityCollecter = createPass("RadiosityCollecter")
g.addPass(RadiosityCollecter, "RadiosityCollecter")
g.addEdge("GBufferRT.posW", "RadiosityCollecter.posW")
g.addEdge("AccumulatePass.output", "RadiosityCollecter.accumulatedColor")
g.markOutput("AccumulatePass.output")
