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
// #include "Rendering/Lights/EmissiveUniformSampler.h"
#include "Utils/UI/Gui.h"
#include <fmt/format.h>

namespace
{

const std::string kInputPosition = "posW";
const std::string kInputNorm = "normW";
const std::string kInputAlbedo = "albedo";
const std::string kInputEmissive = "emissive";
//const std::string kInputSpecularAlbedo = "specularAlbedo";
//const std::string kInputIndirectAlbedo = "indirectAlbedo";
const std::string kInputTangent = "tangentW";
const std::string kInputDepth = "depth";


const std::string kInputAccumulatedColor = "color";

const std::string kOutputResult = "colorx";

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
    {kOutputResult        , "ptResult"        , "Accumulated Result"    , true, ResourceFormat::RGBA32Float}
    // clang-format on
};

extern "C" FALCOR_API_EXPORT void registerPlugin(Falcor::PluginRegistry& registry)
{
    registry.registerClass<RenderPass, FSDRServer>();
}

FSDRServer::FSDRServer(ref<Device> pDevice, const Properties& props) : RenderPass(pDevice)
{
    const char* envOutputDir = std::getenv("FSDRServerOutputDir");
    mOutputDirectory = envOutputDir ? envOutputDir : "FSDRServerOutput";
    if (!std::filesystem::exists(mOutputDirectory))
    {
        std::filesystem::create_directories(mOutputDirectory);
    }

    // Initialize the socket server
    socketServer = new SimpleSocketServer(11451);
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

void FSDRServer::setOutputDirectory(std::string newOutputDir)
{
    mOutputDirectory = newOutputDir;
}

void FSDRServer::execute(RenderContext* pRenderContext, const RenderData& renderData)
{
    // make sure the socket is open
    if (clientSocket == nullptr)
    {
        logInfo("Waiting for client connection...");
        clientSocket = socketServer->accept();
        logInfo("Client connected");
    }

    // renderData holds the requested resources
    // auto& pTexture = renderData.getTexture("src");
    ref<Texture> accumulatedColorTexture = renderData.getTexture(kInputAccumulatedColor);
    waitRecvCamPosSendFilm(renderData);

    // Render the frame same as GBufferRT.posW in posW output
    if (mpScene)
    {
        auto copyTexture = [pRenderContext](Texture* pDst, const Texture* pSrc)
        {
            if (pDst && pSrc)
            {
                FALCOR_ASSERT(pDst && pSrc);
                FALCOR_ASSERT(pDst->getFormat() == pSrc->getFormat());
                FALCOR_ASSERT(pDst->getWidth() == pSrc->getWidth() && pDst->getHeight() == pSrc->getHeight());
                pRenderContext->copyResource(pDst, pSrc);
            }
            else if (pDst)
            {
                pRenderContext->clearUAV(pDst->getUAV().get(), uint4(0, 0, 0, 0));
            }
        };

        copyTexture(renderData.getTexture(kOutputResult).get(), accumulatedColorTexture.get());
    }
    else
    {
        logWarning("No scene available");
    }
}

// send file to the client, send a unsinged long long size_t first.
void sendFilePacket(const std::string& filePath, SimpleSocket* clientSocket)
{
    // Open the file
    std::ifstream file(filePath, std::ios::binary);
    if (!file)
    {
        logError("Failed to open file: " + filePath);
        return;
    }
    // Get the file size
    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    // Send the file size
    clientSocket->write(reinterpret_cast<const char*>(&fileSize), sizeof(fileSize));
    // Send the file data
    char buffer[4096];
    while (file.read(buffer, sizeof(buffer)))
    {
        clientSocket->write(buffer, sizeof(buffer));
    }
    // Send any remaining bytes
    if (file.gcount() > 0)
    {
        clientSocket->write(buffer, file.gcount());
    }
    // Close the file
    file.close();
}

// frame driver
void FSDRServer::waitRecvCamPosSendFilm(const RenderData& renderData)
{
    // Capture and save the collected data
    if (mpScene)
    {
        if (needSendNextFrame)
        {
            logInfo("Sending frame " + std::to_string(imageCount++));
            // Save textures to the directory and send it via socket
            for (auto channel : kInputChannels)
            {
                std::filesystem::path channel_path = fmt::format("{}/{}-{}.exr", mOutputDirectory, imageCount, channel.name);

                // Capture the texture to a file
                if (channel.format == ResourceFormat::RGBA32Float || channel.format == ResourceFormat::R32Float)
                {
                    renderData.getTexture(channel.name)
                        ->captureToFile(0, 0, channel_path, Bitmap::FileFormat::ExrFile, Falcor::Bitmap::ExportFlags::None, false);
                }
                //else if (channel.format == ResourceFormat::RGBA8Unorm)
                //{
                //    renderData.getTexture(channel.name)
                //        ->captureToFile(0, 0, channel_path, Bitmap::FileFormat::BmpFile, Falcor::Bitmap::ExportFlags::None, false);
                //}
                else
                {
                    logError(fmt::format("Unsupported format for channel {}: {}", channel.name, channel.format));
                    continue;
                }

            }

            logInfo("Sending files: ");

            for (auto channel : kInputChannels)
            {
                std::filesystem::path channel_path = fmt::format("{}/{}-{}.exr", mOutputDirectory, imageCount, channel.name);
                sendFilePacket(channel_path.string(), std::move(clientSocket));
            }

            logInfo("Sent files successfully");
            // delete saved files
            /* std::filesystem::remove(posWPath);
               std::filesystem::remove(accumulatedColorPath);*/
            needSendNextFrame = false;
        }
        else
        {
            // wait to read a camera positon info
            CameraControl recvCameraControl;
            if (clientSocket->read_exact(reinterpret_cast<char*>(&recvCameraControl), sizeof(CameraControl)))
            {
                logInfo(fmt::format(
                    "Received camera position: x = {}, y = {}, z = {}", recvCameraControl.x, recvCameraControl.y, recvCameraControl.z
                ));
                mpScene->getCamera()->setPosition(float3(recvCameraControl.x, recvCameraControl.y, recvCameraControl.z));
                needSendNextFrame = true;
            }
            else
            {
                logError("socket error, start socket server again...");
                delete socketServer;
                socketServer = new SimpleSocketServer(11451);
            }
        }
    }
    else
    {
        logWarning("No scene available");
    }
}

void FSDRServer::renderUI(Gui::Widgets& widget)
{
    widget.textbox("output directory", mOutputDirectory);
    needSendNextFrame = widget.button("capture");
}
