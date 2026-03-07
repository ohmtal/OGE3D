//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// NOTE: this is experimental! using SDL2 and OpenGL
//-----------------------------------------------------------------------------
// WARNING: imgui_impl_opengl3.cpp modification:
// in ImGui_ImplOpenGL3_SetupRenderState
// //XXTH OGE3D Hack for fix upside down mirrored rendering
// float L = draw_data->DisplayPos.x;
// float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
// float T = draw_data->DisplayPos.y + draw_data->DisplaySize.y; // Swap T and B
// float B = draw_data->DisplayPos.y;
//
// /* orig:
//  *    float L = draw_data->DisplayPos.x;
//  *    float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
//  *    float T = draw_data->DisplayPos.y;
//  *    float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;
//  */

// AND in ImGui_ImplOpenGL3_RenderDrawData:
// // Apply scissor/clipping rectangle (Y is inverted in OpenGL)
// //XXTH OGE3D Hack
// GL_CALL(glScissor((int)clip_min.x, (int)clip_min.y, (int)(clip_max.x - clip_min.x), (int)(clip_max.y - clip_min.y)));
// //ORIG:
// //GL_CALL(glScissor((int)clip_min.x, (int)((float)fb_height - clip_max.y), (int)(clip_max.x - clip_min.x), (int)(clip_max.y - clip_min.y)));


//-----------------------------------------------------------------------------
#include "ImGuiCtrl.h"
#include "console/consoleTypes.h"
#include "gfx/gfxDevice.h"
#include "gfx/gl/gfxGLDevice.h"
#include "windowManager/sdl/sdlWindow.h"
#include "windowManager/sdl/sdlWindowMgr.h"
#include "windowManager/platformWindowMgr.h"

#include <format>

IMPLEMENT_CONOBJECT(ImGuiCtrl);
//-----------------------------------------------------------------------------
bool ImGuiCtrl::smGlobalImGuiInitialized = false;
//-----------------------------------------------------------------------------
bool ImGuiCtrl::Initialize(){
    if (mInitialized || smGlobalImGuiInitialized) {
        return false;
    };

    // Get Window Manager and First Window
    auto* winMgr = PlatformWindowManager::get();
    if (!winMgr || !winMgr->getFirstWindow())
        return false;

    // SDL Window
    auto* window = dynamic_cast<PlatformWindowSDL*>(winMgr->getFirstWindow());
    if (!window)
    {
        Con::errorf("ImGuiCtrl: Only SDL2 Platform is supported.");
        return false;
    }

    // OpenGL Device
    auto* device = dynamic_cast<GFXGLDevice*>(window->getGFXDevice());
    if (!device)
    {
        Con::errorf("ImGuiCtrl: Only OpenGL GFX Device is supported.");
        return false;
    }


    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    mGuiIO = &ImGui::GetIO();

    mGuiIO->IniFilename = mIniFileName;
    // mGuiIO->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    // mGuiIO->DisplaySize  = ImVec2(getScreen()->getHeight(), getScreen()->getWidth());
    if ( mEnableDockSpace ) {
        mGuiIO->ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    }


    // Use the native handles i  worked so hard for :D
    if (!ImGui_ImplSDL2_InitForOpenGL(window->getSDLWindow(), device->getContext()))
        return false;

    #if defined(__EMSCRIPTEN__) || defined(__ANDROID__)
    const char* glsl_version = "#version 300 es";
    #elif defined(__APPLE__)
    const char* glsl_version = "#version 150"; // Required for GL 3.2+ Core on macOS
    #else
    const char* glsl_version = "#version 130"; // Standard for GL 3.0+ on Desktop
    #endif

    if (!ImGui_ImplOpenGL3_Init(glsl_version))
        return false;



    // ImGui EventListener added to PlatformWindowManagerSDL :D :: hackfest
    mListenerId = PlatformWindowManagerSDL::addEventListener([this](const void* ev){
        // Pass to ImGui backend
        ImGui_ImplSDL2_ProcessEvent((const SDL_Event*)ev);
        ImGuiIO& io = ImGui::GetIO();
        return (io.WantCaptureMouse || io.WantCaptureKeyboard);
    });


    //FIXME Settings ?!
    // // load settings...if we dont use inifile
    // if (mIniFileName == nullptr && SettingsManager().IsInitialized()) {
    //     std::string savedLayout = SettingsManager().get("imgui_layout", std::string(""));
    //     if (!savedLayout.empty()) {
    //         ImGui::LoadIniSettingsFromMemory(savedLayout.c_str(), savedLayout.size());
    //     }
    // }


    smGlobalImGuiInitialized = true;

    return true;
}
//-----------------------------------------------------------------------------
void ImGuiCtrl::Deinitialize() {

    if (mInitialized || smGlobalImGuiInitialized) {
        return;
    };
    // FIXME SAVE SETTINGS
    //     size_t out_size;
    //     const char* data = ImGui::SaveIniSettingsToMemory(&out_size);
    //     SettingsManager().set("imgui_layout", std::string(data, out_size));
    //     SettingsManager().save();
    // }

    PlatformWindowManagerSDL::removeEventListener(mListenerId);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    smGlobalImGuiInitialized = false;
}

//-----------------------------------------------------------------------------
ImGuiCtrl::ImGuiCtrl() : mInitialized(false)
{
    mActive = true;
}
//-----------------------------------------------------------------------------
ImGuiCtrl::~ImGuiCtrl()
{
    Deinitialize();

}
//-----------------------------------------------------------------------------
void ImGuiCtrl::initPersistFields()
{
    Parent::initPersistFields();
}
//-----------------------------------------------------------------------------
bool ImGuiCtrl::onWake()
{
     if (!Parent::onWake()) return false;



    // Global Context Initialization
    if (!smGlobalImGuiInitialized)
    {
        Initialize();
    }

    mInitialized = true;
    return true;
}
//-----------------------------------------------------------------------------
void ImGuiCtrl::onSleep()
{
    //bad idea :P PlatformWindowManagerSDL::removeEventListener(mListenerId);
    mInitialized = false;
    Parent::onSleep();
}
//-----------------------------------------------------------------------------
void ImGuiCtrl::onRender(Point2I offset, const RectI &updateRect)
{
    if (!mInitialized)
        return;



    // Start ImGui Frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    if (mEnableDockSpace)
    {
        ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode; //<< this makes it transparent
        mDockSpaceId = ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), dockspace_flags);
    }


    // Call ImGuiRender
    onImGuiRender(offset, updateRect);

    // Rendering
    ImGui::Render();

    // imgui render draw
    ImDrawData* drawData = ImGui::GetDrawData();
    if (drawData)
    {
        ImGui_ImplOpenGL3_RenderDrawData(drawData);
    }

    // Render children if any
    renderChildControls(offset, updateRect);
}
//-----------------------------------------------------------------------------
bool ImGuiCtrl::onInputEvent(const InputEventInfo &event)
{
    ImGuiIO& io = ImGui::GetIO();

    // Redirect input based on ImGui's needs
    if (io.WantCaptureMouse || io.WantCaptureKeyboard)
    {
//FIXME
        // You need a bridge function to translate Torque GuiEvent to ImGui/SDL Event
        // or call ImGui_ImplSDL2_ProcessEvent(sdlEvent) in the main loop.
        return true; // Consume event so Torque doesn't use it
    }

    return Parent::onInputEvent(event);
}
//-----------------------------------------------------------------------------
void ImGuiCtrl::onImGuiRender(Point2I offset, const RectI& updateRect){
    // // IMGUI test code >>>>>>>>>>>>>>>>>>>>>>>>
    // ImGui::SetNextWindowPos(ImVec2(updateRect.point.x, updateRect.point.y), ImGuiCond_Always);
    // ImGui::SetNextWindowSize(ImVec2(updateRect.extent.x, updateRect.extent.y), ImGuiCond_Always);
    //
    // ImGui::Begin("ImGuiCtrl Editor", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
    // ImGui::Text("Hello from C++20 and ImGui!");
    // if (ImGui::Button("Save Script"))
    // {
    // }
    // if (ImGui::BeginPopupContextItem("##SliderContextMenu")) {
    //     ImGui::SeparatorText("Test PopUp");
    //     for (S32 i = 0 ; i < 20; i++) {
    //         if (ImGui::Button(std::format("Hello World #{}", i).c_str() )) {
    //             Con::printf("Hello World #%d Button PRESSED! ", i);
    //         }
    //     }
    //     ImGui::EndPopup();
    // }
    //

    // <<<<<<<<<<<<<<<<<< IMGUI test code


    for (auto& drawCallers : smDrawCallers) {
        drawCallers.onDraw(offset, updateRect);
    }


    // ImGui::End();

}
//-----------------------------------------------------------------------------
