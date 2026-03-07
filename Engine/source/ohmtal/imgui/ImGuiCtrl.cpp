//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// NOTE: this is experimental!
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

//-----------------------------------------------------------------------------
#include "ImGuiCtrl.h"
#include "console/consoleTypes.h"
#include "gfx/gfxDevice.h"
#include "gfx/gl/gfxGLDevice.h"
#include "windowManager/sdl/sdlWindow.h"
#include "windowManager/sdl/sdlWindowMgr.h"
#include "windowManager/platformWindowMgr.h"


IMPLEMENT_CONOBJECT(ImGuiCtrl);

bool ImGuiCtrl::smGlobalImGuiInitialized = false;
//-----------------------------------------------------------------------------
ImGuiCtrl::ImGuiCtrl() : mInitialized(false)
{
    mActive = true;
}
//-----------------------------------------------------------------------------
ImGuiCtrl::~ImGuiCtrl()
{
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


    // Global Context Initialization
    if (!smGlobalImGuiInitialized)
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();

        // Use the native handles i  worked so hard for :D
        if (!ImGui_ImplSDL2_InitForOpenGL(window->getSDLWindow(), device->getContext()))
            return false;

        if (!ImGui_ImplOpenGL3_Init("#version 130"))
            return false;



        // ImGui EventListener added to PlatformWindowManagerSDL :D :: hackfest
        //FIXME c++20: mListenerId = PlatformWindowManagerSDL::addEventListener([this](const void* ev){
        PlatformWindowManagerSDL::addEventListener([](const void* event) -> bool {
            // Pass to ImGui backend
            ImGui_ImplSDL2_ProcessEvent((const SDL_Event*)event);

            // Tell the manager if ImGui wants to "keep" the input
            ImGuiIO& io = ImGui::GetIO();
            return (io.WantCaptureMouse || io.WantCaptureKeyboard);
        });


        smGlobalImGuiInitialized = true;
    }

    mInitialized = true;
    return true;
}
//-----------------------------------------------------------------------------
void ImGuiCtrl::onSleep()
{
    //FIXME c++20  PlatformWindowManagerSDL::removeEventListener(mListenerId);
    mInitialized = false;
    Parent::onSleep();
}
//-----------------------------------------------------------------------------
void ImGuiCtrl::onRender(Point2I offset, const RectI &updateRect)
{
    if (!mInitialized)
        return;


    // --- Update ImGui Display Size to match this Control ---
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)getWidth(), (float)getHeight());



    // Start ImGui Frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    // IMGUI test code >>>>>>>>>>>>>>>>>>>>>>>>

    // 2. Your ImGui Code (e.g., Script Editor)
    ImGui::SetNextWindowPos(ImVec2(offset.x, offset.y), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(updateRect.extent.x, updateRect.extent.y), ImGuiCond_Always);

    // ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    // ImGui::SetNextWindowSize(io.DisplaySize, ImGuiCond_Always);


    ImGui::Begin("ImGuiCtrl Editor", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
    ImGui::Text("Hello from C++20 and ImGui!");
    if (ImGui::Button("Save Script"))
    {

    }
    ImGui::End();

    // <<<<<<<<<<<<<<<<<< IMGUI test code

    // Rendering
    ImGui::Render();

    // imgui render draw
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

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
