//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// ImGuiCaller Base Class
//-----------------------------------------------------------------------------
// * ImGuiCaller automaticly deleted when the ImGuiCtrl is deleted!
//-----------------------------------------------------------------------------

// #include "console/simObject.h"
// #include "ohmtal/imgui/ImGuiCtrl.h"
// #include "ohmtal/sim/DeferredDelete.h"
#include "ImGuiCaller.h"

IMPLEMENT_CONOBJECT(ImGuiCaller);

bool ImGuiCaller::onAdd()
{
    if (!Parent::onAdd())
        return false;
    return true;
}

void ImGuiCaller::onRemove()
{
    if (mCallerId && mImGuiCtrl) {
        mImGuiCtrl->removeDrawCaller(mCallerId);
    }
    Parent::onRemove();
}

bool ImGuiCaller::Connect(ImGuiCtrl* ctrl) {
    if ( !ctrl ||  mCallerId > 0 ) return false;
    mImGuiCtrl = ctrl;
    mCallerId = mImGuiCtrl->addDrawCaller(
        [this](Point2I offset, const RectI& updateRect) {
            onImGuiRender(offset, updateRect);
        },
        [this]() {

            //deferred
            Sim::postEvent(this, new DeferredDeleteActionEvent(this), Sim::getCurrentTime()+100);
        }
    );
    return true;
}

DefineEngineMethod(ImGuiCaller, connect, bool, (ImGuiCtrl* ctrl),,"Connect to a ImGuiCtrl") {
    return object->Connect(ctrl);
}


