//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// NOTE: this is experimental!
//       1. it breaks the select renderer and used opengl at the moment
//       2. it require SDL2 at the moment
//-----------------------------------------------------------------------------
#ifndef _IMGUI_CTRL_H_
#define _IMGUI_CTRL_H_

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif

#include "gui/containers/guiContainer.h"
#include "imgui.h"
#include "backends/imgui_impl_sdl2.h" // FIXME SDL3 .. when ready
#include "backends/imgui_impl_opengl3.h" //FIXME DirectX  ?!!
#include "windowManager/sdl/sdlWindow.h"

#include <functional>
#include <vector>

// ImGui Drawcalls
using ImGuiDrawCall = std::function<void(Point2I offset, const RectI &updateRect)>;
using ImGuiRemoveCall = std::function<void()>;


class ImGuiCtrl : public GuiContainer
{
    typedef GuiContainer Parent;

    struct DrawCallEntry {
        U32 id;
        ImGuiDrawCall onDraw = nullptr;
        ImGuiRemoveCall onRemove = nullptr;
    };
    inline static std::vector<DrawCallEntry> smDrawCallers;
    inline static U32 smNextId = 0;

public:
    static U32 addDrawCaller(ImGuiDrawCall drawCaller,  ImGuiRemoveCall removeCaller = nullptr) {
        U32 id = ++smNextId;
        smDrawCallers.push_back({id, drawCaller, removeCaller});
        return id;
    }

    static void removeDrawCaller(U32 id) {
        std::erase_if(smDrawCallers, [id](const auto& entry) {
            return entry.id == id;
        });
    }


protected:
    bool mAwake;
    static bool smGlobalImGuiInitialized;
    // bool mInitialized;
    U32 mListenerId;

    ImGuiStyle mBaseStyle;
    bool mEnableDockSpace = true;
    ImGuiID mDockSpaceId;
    const char* mIniFileName = nullptr;

    ImGuiIO* mGuiIO = nullptr;
    PlatformWindowManagerSDL* mWinManager = nullptr;

public:
    ImGuiCtrl();
    virtual ~ImGuiCtrl();

    // ImGui Stuff ...
    ImGuiIO* getGuiIO() { return mGuiIO; }
    ImGuiStyle getBaseStyle()  const { return  mBaseStyle; }
    void setEnableDockSpace( bool value ) { mEnableDockSpace = value; }
    bool getEnableDockSpace() { return mEnableDockSpace; }
    ImGuiID getDockSpaceId() { return mDockSpaceId; }
    bool Initialize();
    void Deinitialize();


    // Torque3D Object Requirements
    DECLARE_CONOBJECT(ImGuiCtrl);
    static void initPersistFields();

    // Core GUI Methods
    virtual bool onWake();
    virtual void onSleep();
    virtual void onRender(Point2I offset, const RectI &updateRect);

    // Input Redirection
    virtual bool onInputEvent(const InputEventInfo &event);

    // ImGui Render Call
    virtual void onImGuiRender(Point2I offset, const RectI &updateRect);
};

#endif
