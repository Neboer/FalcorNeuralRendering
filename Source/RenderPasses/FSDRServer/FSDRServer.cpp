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
#include "FSDRServer.h"
#include "RenderGraph/RenderPassHelpers.h"
#include "RenderGraph/RenderPassStandardFlags.h"
#include "Rendering/Lights/EmissiveUniformSampler.h"
#include "Utils/UI/Gui.h"
#include <fmt/format.h>
#include "EnvHelper/EnvHelper.hpp"

namespace
{

const std::string kInputPosition = "posW";
const std::string kInputNorm = "normW";
const std::string kInputAlbedo = "albedo";
const std::string kInputEmissive = "emissive";
// const std::string kInputSpecularAlbedo = "specularAlbedo";
// const std::string kInputIndirectAlbedo = "indirectAlbedo";
const std::string kInputTangent = "tangentW";
const std::string kInputDepth = "depth";

const std::string kInputAccumulatedColor = "color";

const std::string kOutputResult = "colorx";
const std::string kOutputRTMask = "rtMask";
const std::string kOutputFake = "fake";
} // namespace

const Falcor::ChannelList kInputChannels = {
    // clang-format off
    {kInputPosition , "gPosW"            , "Position in world space", true, ResourceFormat::RGBA32Float},
    {kInputNorm , "gNormW"           , "Normal Map"             , true, ResourceFormat::RGBA32Float},
    {kInputAlbedo , "ptAlbedo"         , "Albedo"                 , true, ResourceFormat::RGBA32Float}, // GbufferRT's diffuse opacity is albedo.
    // { "diffuseOpacity",             "gDiffOpacity",                 "Diffuse reflection albedo and opacity",                   true /* optional */, ResourceFormat::RGBA32Float  
    {kInputEmissive , "gEmissive"        , "Emissive"               , true, ResourceFormat::RGBA32Float},
    {kInputTangent,       "gTangentW",        "Shading tangent in world space (xyz) and sign (w)", true /* optional */, ResourceFormat::RGBA32Float },
    {kInputDepth, "gDepth", "Depth buffer (NDC)", true , ResourceFormat::R32Float},

    {kInputAccumulatedColor, "ptResult"         , "Accumulate "            , true, ResourceFormat::RGBA32Float}
    // clang-format on
};

const Falcor::ChannelList kOutputChannels = {
    // clang-format off
    //{kOutputResult        , "ptResult"        , "Accumulated Result"    , true, ResourceFormat::RGBA32Float}
    //{kOutputRTMask       , "rtMask"        , "Ray Tracing Mask"    , true, ResourceFormat::R8Uint}
    {kOutputFake       , "fake"        , "FakeOutput"    , true, ResourceFormat::R8Uint}
    // clang-format on
};

extern "C" FALCOR_API_EXPORT void registerPlugin(Falcor::PluginRegistry& registry)
{
    registry.registerClass<RenderPass, FSDRServer>();
}

FSDRServer::FSDRServer(ref<Device> pDevice, const Properties& props) : RenderPass(pDevice)
{
    // initialize rendering server pointer from environment variable
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

Properties FSDRServer::getProperties() const
{
    return {};
}

RenderPassReflection FSDRServer::reflect(const CompileData& compileData)
{
    // Define the required resources here
    RenderPassReflection reflector;
    // reflector.addOutput("dst");
    addRenderPassInputs(reflector, kInputChannels);
    addRenderPassOutputs(reflector, kOutputChannels);
    return reflector;
}

nlohmann::json FSDRServer::SerializeTextureInfoToJson(ref<Texture> pTex)
{
    const int arraySlice = 0;
    const int mipLevel = 0;

    // {"width": int, "height": int, "channels": int, "bytesPerPixel": int}
    return {
        {"width", pTex->getWidth(mipLevel)},
        {"height", pTex->getHeight(mipLevel)},
        {"channels", getFormatChannelCount(pTex->getFormat())},
        {"bytesPerPixel", getFormatBytesPerBlock(pTex->getFormat())}
    };
}

std::vector<uint8_t> FSDRServer::ExtractSimpleTextureDataToBinary(ref<Texture> pTex)
{
    const int arraySlice = 0;
    const int mipLevel = 0;
    RenderContext* pContext = pTex->getDevice()->getRenderContext();
    uint32_t subresource = pTex->getSubresourceIndex(arraySlice, mipLevel);
    return pContext->readTextureSubresource(pTex.get(), subresource);
}

float3 FSDRServer::JsonToFloat3(const nlohmann::json& arr)
{
    if (!arr.is_array() || arr.size() != 3)
    {
        throw std::runtime_error("JsonToFloat3: Input JSON is not a valid array of size 3.");
    }
    float x = arr[0].get<float>();
    float y = arr[1].get<float>();
    float z = arr[2].get<float>();
    return float3(x, y, z);
}

void FSDRServer::UpdateSceneFromJson(const nlohmann::json& sceneParam)
{
    if (mpScene == nullptr)
    {
        logWarning("FSDRServer::UpdateSceneFromJson: mpScene is null, cannot update scene.");
        return;
    }

    if (sceneParam.contains("camera"))
    {
        if (sceneParam["camera"].contains("position"))
            mpScene->getCamera()->setPosition(JsonToFloat3(sceneParam["camera"]["position"]));
        if (sceneParam["camera"].contains("upvector"))
            mpScene->getCamera()->setUpVector(JsonToFloat3(sceneParam["camera"]["upvector"]));
        if (sceneParam["camera"].contains("target"))
            mpScene->getCamera()->setTarget(JsonToFloat3(sceneParam["camera"]["target"]));
    }
}

void FSDRServer::execute(RenderContext* pRenderContext, const RenderData& renderData)
{
    if (mpScene)
    {
        if (needSendNextFrame)
        {
            // 将渲染结果传递给RenderingServer
            for (auto& channel : kInputChannels)
            {
                ref<Texture> texture = renderData.getTexture(channel.name);
                // 可能还需要通过此结构获取aabb。
                server->renderingResultInfoCache["info"][channel.name] = SerializeTextureInfoToJson(texture);
                server->renderingResultDataCache[channel.name] = std::move(ExtractSimpleTextureDataToBinary(texture));
            }
            server->NotifyRenderingComplete();
        }
        server->WaitForRenderingRequest();
        // 需要更新场景参数。
        UpdateSceneFromJson(server->sceneParamCache);
        needSendNextFrame = true;
    }
}

void FSDRServer::renderUI(Gui::Widgets& widget) {}
