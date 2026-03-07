//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// ImGuiCaller Base Class
//-----------------------------------------------------------------------------
#pragma once
#include "console/simObject.h"
#include "ohmtal/imgui/ImGuiCtrl.h"
#include "ohmtal/sim/DeferredDelete.h"


class ImGuiCaller: public SimObject {
private:
    typedef SimObject Parent;
    U32 mCallerId = 0;

    ImGuiCtrl* mImGuiCtrl = nullptr;

public:
    DECLARE_CONOBJECT(ImGuiCaller);

    ImGuiCtrl* getImGuiCtrl() const {  return mImGuiCtrl; }

    bool onAdd();
    void onRemove();
    bool Connect(ImGuiCtrl* ctrl);
    virtual void onImGuiRender(Point2I offset, const RectI& updateRect) {}

};
