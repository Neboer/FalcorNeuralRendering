from falcor import *

def render_graph_GBuffer():
    # 创建渲染图并命名
    g = RenderGraph("GBuffer")

    # 添加GBufferRT pass（生成GBuffer数据）
    GBufferRT = createPass("GBufferRT", {'sampleCount': 1})  # 可以根据需要调整参数
    g.addPass(GBufferRT, "GBufferRT")

    # 添加ToneMapper用于显示输出（可以选择性添加）
    ToneMapper = createPass("ToneMapper", {'autoExposure': False, 'exposureCompensation': 0.0})
    g.addPass(ToneMapper, "ToneMapper")

    # 链接Pass的数据流
    g.addEdge("GBufferRT.posW", "ToneMapper.src")  # 提取Position输出并连接到ToneMapper

    # 将输出的Position通道标记为最终输出
    g.markOutput("ToneMapper.dst")

    # 如果只想输出Position，而不经过ToneMapper，可以直接标记GBufferRT的posW输出
    # g.markOutput("GBufferRT.posW")

    return g

# 创建GBuffer渲染图实例
GBufferGraph = render_graph_GBuffer()

# 将渲染图添加到场景中
try: m.addGraph(GBufferGraph)
except NameError: None
