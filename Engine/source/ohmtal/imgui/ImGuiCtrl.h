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

#include "gui/containers/guiContainer.h"
#include "imgui.h"
#include "backends/imgui_impl_sdl2.h" // FIXME SDL3 later
#include "backends/imgui_impl_opengl3.h" //FIXME Windows !!

#include "windowManager/sdl/sdlWindow.h"

class ImGuiCtrl : public GuiContainer
{
    typedef GuiContainer Parent;

protected:
    bool mInitialized;
    static bool smGlobalImGuiInitialized;
    U32 mListenerId;

public:
    ImGuiCtrl();
    virtual ~ImGuiCtrl();

    // Torque3D Object Requirements
    DECLARE_CONOBJECT(ImGuiCtrl);
    static void initPersistFields();

    // Core GUI Methods
    virtual bool onWake();
    virtual void onSleep();
    virtual void onRender(Point2I offset, const RectI &updateRect);

    // Input Redirection
    virtual bool onInputEvent(const InputEventInfo &event);
};

#endif
