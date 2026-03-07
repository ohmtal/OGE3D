//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// To Test: ImGuiHud must be created!!
//  $testCaller = new ImGuiTestCaller(); $testCaller.connect(ImGuiHud);


#include "console/simObject.h"
#include "ImGuiCtrl.h"


class ImGuiTestCaller: public SimObject {
private:
    typedef SimObject Parent;
    U32 mCallerId = 0;

    ImGuiCtrl* mGuiCtrl = nullptr;

public:
    DECLARE_CONOBJECT(ImGuiTestCaller);

    bool onAdd();
    void onRemove();
    bool Connect(ImGuiCtrl* ctrl);
    void onImGuiRender(Point2I offset, const RectI& updateRect);

};
IMPLEMENT_CONOBJECT(ImGuiTestCaller);

bool ImGuiTestCaller::onAdd()
{
    if (!Parent::onAdd())
        return false;
    return true;
}

void ImGuiTestCaller::onRemove()
{
    if (mCallerId && mGuiCtrl) {
        mGuiCtrl->removeDrawCaller(mCallerId);
    }
    Parent::onRemove();
}

bool ImGuiTestCaller::Connect(ImGuiCtrl* ctrl) {
    if ( !ctrl ||  mCallerId > 0 ) return false;
    mGuiCtrl = ctrl;
    mCallerId = mGuiCtrl->addDrawCaller(
        [this](Point2I offset, const RectI& updateRect) {
            onImGuiRender(offset, updateRect);
        }
    );
    return true;
}

DefineEngineMethod(ImGuiTestCaller, connect, bool, (ImGuiCtrl* ctrl),,"Connect to a ImGuiCtrl") {
    return object->Connect(ctrl);
}


void ImGuiTestCaller::onImGuiRender(Point2I offset, const RectI& updateRect) {
    ImGui::PushID(this);
    ImGui::TextDisabled("Hello from TestCaller ....");
    static char nameBuf[64];
    ImGui::Text("Name");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(200);
    if (ImGui::InputText("##Name", nameBuf, sizeof(nameBuf))) {
    }
    ImGui::PopID();


}
