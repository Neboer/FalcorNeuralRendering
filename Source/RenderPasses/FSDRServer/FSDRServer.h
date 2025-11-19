/***************************************************************************
 # Copyright (c) 2015-23, NVIDIA CORPORATION. All rights reserved.
 #
 # Redistribution and use in source and binary forms, with or without
 # modification, are permitted provided that the following conditions
 # are met:
 #  * Redistributions of source code must retain the above copyright
 #    notice, this list of conditions and the following disclaimer.
 #  * Redistributions in binary form must reproduce the above copyright
 #    notice, this list of conditions and the following disclaimer in the
 #    documentation and/or other materials provided with the distribution.
 #  * Neither the name of NVIDIA CORPORATION nor the names of its
 #    contributors may be used to endorse or promote products derived
 #    from this software without specific prior written permission.
 #
 # THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS "AS IS" AND ANY
 # EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 # IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 # PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 # CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 # EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 # PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 # PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 # OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 # (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 # OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 **************************************************************************/
#pragma once
#include "Falcor.h"
#include "RenderGraph/RenderPass.h"
#include "RenderingServer/RenderingServer.h"
#include <fstream>

using namespace Falcor;

class FSDRServer : public RenderPass
{
public:
    FALCOR_PLUGIN_CLASS(FSDRServer, "FSDRServer", "Rendering agent send rendering result to Client via renderingserver.");

    static ref<FSDRServer> create(ref<Device> pDevice, const Properties& props) { return make_ref<FSDRServer>(pDevice, props); }

    FSDRServer(ref<Device> pDevice, const Properties& props);

    virtual Properties getProperties() const override;
    virtual RenderPassReflection reflect(const CompileData& compileData) override;
    virtual void compile(RenderContext* pRenderContext, const CompileData& compileData) override {}
    virtual void execute(RenderContext* pRenderContext, const RenderData& renderData) override;
    virtual void renderUI(Gui::Widgets& widget) override;
    virtual void setScene(RenderContext* pRenderContext, const ref<Scene>& pScene) override { mpScene = pScene; }
    virtual bool onMouseEvent(const MouseEvent& mouseEvent) override { return false; }
    virtual bool onKeyEvent(const KeyboardEvent& keyEvent) override { return false; }

private:
    RenderingServer* server;
    // 不需要发送下一帧，因为还没有收到请求。一旦收到请求，之后就都需要发送了。
    bool needSendNextFrame = false;
    ref<IScene> mpScene;
    // 一些帮助函数和方法

    static float3 JsonToFloat3(const nlohmann::json& arr); // 将json数组转换为float3

    // 序列化的格式为：{"width": int, "height": int, "channels": int, "bytesPerPixel": int}
    static nlohmann::json SerializeTextureInfoToJson(ref<Texture> pTex); // 将texture的信息序列化为json格式。
    static std::vector<uint8_t> ExtractSimpleTextureDataToBinary(ref<Texture> pTex); // 将texture的数据序列化为二进制格式。

    // 反序列化的格式为：{"camera": {"position"?: [int, int, int], "upvector"?: [int, int, int], "target"?: [int, int, int] }}
    void UpdateSceneFromJson(const nlohmann::json& sceneParam); // 根据json格式的场景参数更新场景。

protected:
};
