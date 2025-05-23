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
#include "SimpleSocketServer/SimpleSocketServer.h"
#include <fstream>

using namespace Falcor;

#pragma pack(push, 1) // disable memory alignment
struct CameraControl
{
    float x;
    float y;
    float z;
};
#pragma pack(pop)

class FSDRServer : public RenderPass
{
public:
    FALCOR_PLUGIN_CLASS(FSDRServer, "FSDRServer", "Insert pass description here.");

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
    int imageCount = 0;
    void waitRecvCamPosSendFilm(ref<Texture> posWTexture, ref<Texture> accumulatedColorTexture);
    ref<IScene> mpScene;
    std::string mOutputDirectory;
    void setOutputDirectory(std::string newOutputDir);

    std::filesystem::path cameraInfoCSVFileLocation;

    bool needSendNextFrame = false;

    SimpleSocketServer* socketServer = nullptr;
    SimpleSocket* clientSocket = nullptr;

protected:
};
