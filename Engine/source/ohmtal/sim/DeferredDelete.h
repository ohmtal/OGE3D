//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Example call:
// Sim::postEvent(this, new DeferredDeleteActionEvent(this), Sim::getCurrentTime()+100);
//-----------------------------------------------------------------------------
#pragma once
#include <console/simObject.h>
#include <console/sim.h>



class DeferredDeleteActionEvent : public SimEvent
{
    SimObject* mObject = nullptr ;
public:
    DeferredDeleteActionEvent(SimObject* object)
    {
        mObject = object;
    }
    void process(SimObject *object)
    {
        if (mObject) {
            mObject->deleteObject();
            mObject = nullptr;
        }
    }
};
