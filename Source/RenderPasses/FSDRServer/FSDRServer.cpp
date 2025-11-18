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
    {kOutputRTMask       , "rtMask"        , "Ray Tracing Mask"    , true, ResourceFormat::R8Uint}
    // clang-format on
};

extern "C" FALCOR_API_EXPORT void registerPlugin(Falcor::PluginRegistry& registry)
{
    registry.registerClass<RenderPass, FSDRServer>();
}

FSDRServer::FSDRServer(ref<Device> pDevice, const Properties& props)
    : RenderPass(pDevice)
    /*, httpBackend(std::string("127.0.0.1"), 11452, FalcorContext{&kInputChannels, nullptr, std::nullopt, mpScene.get(), &abSync})*/
{
    // Initialize the socket server
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

void FSDRServer::execute(RenderContext* pRenderContext, const RenderData& renderData)
{
    //abSync.enterFromA();
    /*httpBackend.SetRenderingContext(pRenderContext, renderData);*/


    //rtMaskTexture->getTextureSizeInBytes();
}

void FSDRServer::renderUI(Gui::Widgets& widget)
{
    //widget.textbox("output directory", mOutputDirectory);
    //needSendNextFrame = widget.button("capture");
    widget.slider("rtMask x", rtMaskParams.x, 0, 100);
    widget.slider("rtMask y", rtMaskParams.y, 0, 100);
    widget.slider("rtMask width", rtMaskParams.width, 0, 100);
    widget.slider("rtMask height", rtMaskParams.height, 0, 100);
}
