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
#include "RadiosityCollecter.h"
#include "RenderGraph/RenderPassHelpers.h"
#include "RenderGraph/RenderPassStandardFlags.h"
// #include "Rendering/Lights/EmissiveUniformSampler.h"
#include "Utils/UI/Gui.h"
#include <fmt/format.h>

namespace
{

const std::string kInputPosition = "posW";
const std::string kInputAccumulatedColor = "accumulatedColor";

} // namespace

const Falcor::ChannelList kInputChannels = {
    // clang-format off
    {kInputPosition        , "gPosW"            , "Position in world space", true, ResourceFormat::RGBA32Float},
    {kInputAccumulatedColor, "gAccumulatedColor", "Accumulated color"      , true, ResourceFormat::RGBA32Float}
    // clang-format on
};

extern "C" FALCOR_API_EXPORT void registerPlugin(Falcor::PluginRegistry& registry)
{
    registry.registerClass<RenderPass, RadiosityCollecter>();
}

RadiosityCollecter::RadiosityCollecter(ref<Device> pDevice, const Properties& props) : RenderPass(pDevice), cameraInfoCSVFile()
{
    const char* envOutputDir = std::getenv("RadiosityCollecterOutputDir");
    mOutputDirectory = envOutputDir ? envOutputDir : "RadiosityCollecterOutput";

    cameraInfoCSVFileLocation = std::filesystem::path(mOutputDirectory) / "camera.csv";
    cameraInfoCSVFile.open(cameraInfoCSVFileLocation, std::ios::out | std::ios::trunc);
    if (!cameraInfoCSVFile.is_open())
    {
        logWarning("Failed to open CSV file for writing");
    }
}

Properties RadiosityCollecter::getProperties() const
{
    return {};
}

RenderPassReflection RadiosityCollecter::reflect(const CompileData& compileData)
{
    // Define the required resources here
    RenderPassReflection reflector;
    // reflector.addOutput("dst");
    addRenderPassInputs(reflector, kInputChannels);
    return reflector;
}

void RadiosityCollecter::setOutputDirectory(std::string newOutputDir)
{
    mOutputDirectory = newOutputDir;
}

void RadiosityCollecter::execute(RenderContext* pRenderContext, const RenderData& renderData)
{
    // renderData holds the requested resources
    // auto& pTexture = renderData.getTexture("src");
    if (needCatpreNextFrame)
    {
        ref<Texture> posWTexture = renderData.getTexture(kInputPosition);
        ref<Texture> accumulatedColorTexture = renderData.getTexture(kInputAccumulatedColor);
        captureAndSaveCollectedData(posWTexture, accumulatedColorTexture);
        needCatpreNextFrame = false;
    }
}

void RadiosityCollecter::captureAndSaveCollectedData(ref<Texture> posWTexture, ref<Texture> accumulatedColorTexture)
{
    // Capture and save the collected data
    if (mpScene)
    {
        auto cameraPos = mpScene->getCamera()->getPosition();
        auto cameraDir = mpScene->getCamera()->getTarget();

        // save two textures into new folder
        std::filesystem::path outputDir = mOutputDirectory;
        auto imageDirName = fmt::format("{:04}", imageCount);
        outputDir /= imageDirName;

        if (!std::filesystem::exists(outputDir))
        {
            std::filesystem::create_directories(outputDir);
        }

        // append to camera.csv
        cameraInfoCSVFile << fmt::format(
            "{},{},{},{},{},{},{}\n", imageDirName, cameraPos.x, cameraPos.y, cameraPos.z, cameraDir.x, cameraDir.y, cameraDir.z
        );

        // Save textures to the directory
        std::filesystem::path posWPath = outputDir / "posw.exr";
        std::filesystem::path accumulatedColorPath = outputDir / "color.exr";

        posWTexture->captureToFile(0, 0, posWPath, Bitmap::FileFormat::ExrFile);
        accumulatedColorTexture->captureToFile(0, 0, accumulatedColorPath, Bitmap::FileFormat::ExrFile);

        imageCount++;
    }
    else
    {
        logWarning("No scene available");
    }
}

void RadiosityCollecter::renderUI(Gui::Widgets& widget)
{
    widget.textbox("output directory", mOutputDirectory);
    needCatpreNextFrame = widget.button("capture");
}
