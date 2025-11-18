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
#include "RTMask.h"
#include "RenderGraph/RenderPassHelpers.h"
#include "RenderGraph/RenderPassStandardFlags.h"
#include "EnvHelper/EnvHelper.hpp"

namespace
{
const std::string kOutputRTMask = "rtMask";

}

const Falcor::ChannelList kOutputChannels = {
    // clang-format off
    {kOutputRTMask       , "rtMask"        , "Ray Tracing Mask"    , true, ResourceFormat::R8Uint}
    // clang-format on
};

extern "C" FALCOR_API_EXPORT void registerPlugin(Falcor::PluginRegistry& registry)
{
    registry.registerClass<RenderPass, RTMask>();
}

RTMask::RTMask(ref<Device> pDevice, const Properties& props) : RenderPass(pDevice)
{
    std::string encodedRenderingServerAddr = getEnvValue("RENDERING_SERVER_ADDR");
    logInfo("rendering server env set to " + encodedRenderingServerAddr);
    server = decodePointer<RenderingServer>(encodedRenderingServerAddr);

    if (server == nullptr)
    {
        throw std::runtime_error("RTMask pass requires a RenderingServer instance.");
    }
    else
    {
        logInfo("GlobalRenderingServer has init and RTMask pass created.");
    }
}

Properties RTMask::getProperties() const
{
    return {};
}

RenderPassReflection RTMask::reflect(const CompileData& compileData)
{
    // Define the required resources here
    // Define the required resources here
    RenderPassReflection reflector;
    // reflector.addOutput("dst");
    // addRenderPassInputs(reflector, kInputChannels);
    addRenderPassOutputs(reflector, kOutputChannels);
    return reflector;
}

// 生成一张图，根据裁剪窗口设置区域为1，其余为0
void RTMask::execute(RenderContext* pRenderContext, const RenderData& renderData)
{
    // create R8Uint texture total black;
    int cx = server->cropWindow["x"].get<int>();
    int cy = server->cropWindow["y"].get<int>();
    int cwidth = server->cropWindow["width"].get<int>();
    int cheight = server->cropWindow["height"].get<int>();

    auto preparedRenderingData = std::vector<uint8_t>(renderData.getDefaultTextureDims().x * renderData.getDefaultTextureDims().y, 0);
    if (cwidth && cheight)
    {
        const int texWidth = renderData.getDefaultTextureDims().x;
        for (int y = 0; y < cheight; y++)
        {
            // 计算该行起始地址
            uint8_t* rowStart = preparedRenderingData.data() + (cy + y) * texWidth + cx;

            // 用 memset 一次性写 width 个字节为 1
            std::memset(rowStart, 1, cwidth);
        }
    }

    Texture* rtMaskTexture = renderData.getTexture(kOutputRTMask).get();
    uint32_t subresourceID = rtMaskTexture->getSubresourceIndex(0, 0);
    rtMaskTexture->setSubresourceBlob(subresourceID, preparedRenderingData.data(), preparedRenderingData.size());
}

void RTMask::renderUI(Gui::Widgets& widget) {}
