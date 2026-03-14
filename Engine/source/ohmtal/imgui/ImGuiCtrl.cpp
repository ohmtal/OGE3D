//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// NOTE: this is experimental! using SDL2 and OpenGL
//-----------------------------------------------------------------------------
// WARNING: imgui_impl_opengl3.cpp modification:
// ~~~ 1. upside down ~~~~~
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

// ~~~ 2. position of objects ~~~
// AND in ImGui_ImplOpenGL3_RenderDrawData:
// // Apply scissor/clipping rectangle (Y is inverted in OpenGL)
// //XXTH OGE3D Hack
// GL_CALL(glScissor((int)clip_min.x, (int)clip_min.y, (int)(clip_max.x - clip_min.x), (int)(clip_max.y - clip_min.y)));
// //ORIG:
// //GL_CALL(glScissor((int)clip_min.x, (int)((float)fb_height - clip_max.y), (int)(clip_max.x - clip_min.x), (int)(clip_max.y - clip_min.y)));

// ~~~ 3. color correction  ~~~
// const GLchar* fragment_shader_glsl_130 =
// "uniform sampler2D Texture;\n"
// "in vec2 Frag_UV;\n"
// "in vec4 Frag_Color;\n"
// "out vec4 Out_Color;\n"
// "void main()\n"
// "{\n"
// "    vec4 col = Frag_Color * texture(Texture, Frag_UV.st);\n"
// "    Out_Color = vec4(pow(col.rgb, vec3(2.2)), col.a);\n"
// "}\n";


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
    #ifndef TORQUE_DEDIVATED // :/

    if (smGlobalImGuiInitialized && mGuiIO) {
        return true;
    };

    // Get Window Manager and First Window
    auto* winMgr = PlatformWindowManager::get();
    if (!winMgr || !winMgr->getFirstWindow())
        return false;

    mWinManager =  dynamic_cast<PlatformWindowManagerSDL*>(winMgr);
    if (!mWinManager)
    {
        Con::errorf("ImGuiCtrl: Only SDL Platform is supported.");
        return false;
    }

    if ( smGlobalImGuiInitialized ) {
        mGuiIO = &ImGui::GetIO();
        return true;
    }


    // SDL Window
    auto* window = dynamic_cast<PlatformWindowSDL*>(winMgr->getFirstWindow());
    if (!window)
    {
        Con::errorf("ImGuiCtrl: Only SDL Platform is supported.");
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
    // ImGui::StyleColorsDark();
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

        return (mGuiIO->WantCaptureMouse || mGuiIO->WantCaptureKeyboard);
    });


    if (mIniFileName == nullptr) {
        String savedLayout =  Con::getVariable("$pref::imgui_layout");
        if (!savedLayout.isEmpty()) {
            ImGui::LoadIniSettingsFromMemory(savedLayout.c_str(), savedLayout.size());
        }
    }



    smGlobalImGuiInitialized = true;

    return true;
    #else
    return false;
    #endif

}

//-----------------------------------------------------------------------------
void ImGuiCtrl::Deinitialize() {

    if (!smGlobalImGuiInitialized) {
        return;
    };


    for (auto& drawCaller : smDrawCallers) {
        if (drawCaller.onRemove) {
            drawCaller.onRemove();
        }
    }



    PlatformWindowManagerSDL::removeEventListener(mListenerId);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    smGlobalImGuiInitialized = false;
}

//-----------------------------------------------------------------------------
ImGuiCtrl::ImGuiCtrl() : mAwake(false)
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
    if (!smGlobalImGuiInitialized || !mGuiIO) {
       if (! Initialize() )
           return false;
    }

    mAwake = true;
    return true;
}
//-----------------------------------------------------------------------------
void ImGuiCtrl::onSleep()
{
    if (mIniFileName == nullptr) {
        Con::setVariable("$pref::imgui_layout", ImGui::SaveIniSettingsToMemory());
    }

    mAwake = false;
    Parent::onSleep();
}
//-----------------------------------------------------------------------------
void ImGuiCtrl::onRender(Point2I offset, const RectI &updateRect)
{
    if (!mAwake || !mGuiIO)
        return;


    if (mGuiIO->WantTextInput) {
        if (!SDL_IsTextInputActive()) {
            SDL_StartTextInput();
        }
    } else {
        if (SDL_IsTextInputActive() && mWinManager->getSDLTextInputState() == PlatformWindowManagerSDL::KeyboardInputState::TEXT_INPUT ) {
            SDL_StopTextInput();
        }
    }

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
    if (!mGuiIO) return Parent::onInputEvent(event);

    // Redirect input based on ImGui's needs
    if (mGuiIO->WantCaptureMouse || mGuiIO->WantCaptureKeyboard)
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


    for (auto& drawCaller : smDrawCallers) {
        if (drawCaller.onDraw) {
            drawCaller.onDraw(offset, updateRect);
        }
    }


    // ImGui::End();

}
//-----------------------------------------------------------------------------
