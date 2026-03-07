//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// To Test: ImGuiHud must be created!!
//  $testCaller = new ImGuiTestCaller(); $testCaller.connect(ImGuiHud);


// #include "console/simObject.h"
// #include "ohmtal/imgui/ImGuiCtrl.h"
// #include "ohmtal/sim/DeferredDelete.h"
#include "ohmtal/imgui/ImGuiCaller.h"


class ImGuiTestCaller: public ImGuiCaller {
    typedef ImGuiCaller Parent;
public:
    DECLARE_CONOBJECT(ImGuiTestCaller);
    void onImGuiRender(Point2I offset, const RectI& updateRect);

};
IMPLEMENT_CONOBJECT(ImGuiTestCaller);


void ImGuiTestCaller::onImGuiRender(Point2I offset, const RectI& updateRect) {
    ImGui::PushID(this);
    ImGui::Begin("Test Caller");
    ImGui::TextDisabled("Hello from TestCaller ....");
    static char nameBuf[64];
    ImGui::Text("Name");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(200);
    if (ImGui::InputText("##Name", nameBuf, sizeof(nameBuf))) {
    }
    if (ImGui::Button("DELETE TEST !! DANGERZONE") ){
        Sim::postEvent(this, new DeferredDeleteActionEvent(getImGuiCtrl()), Sim::getCurrentTime()+100);
    }

    if (ImGui::Button("Call ShowCase") ){
        Con::evaluatef("$testCaller = new imGuiTest2(); $testCaller.connect(ImGuiHud);");
    }


    // $testCaller = new imGuiTest2(); $testCaller.connect(ImGuiHud);
    ImGui::End();
    ImGui::PopID();


}
