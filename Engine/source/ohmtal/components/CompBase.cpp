/**
   CompBase

   @since 2024-01-01
   @author XXTH

   License at: ohmtal/misc/ohmtalMIT.h
*/

#include "scene/sceneObject.h"
#include "console/consoleObject.h"

#include "CompBase.h"

void CompBase::initPersistFields()
{
   Parent::initPersistFields();
   removeField("scale");
   removeField("position");
   removeField("rotation");
   removeField("EnvCacheClientObject");
   removeField("isRenderEnabled");
   removeField("hidden");
}

void CompBase::onMount(SceneObject* obj, S32 node)
{
   obj->setDataField(mIdent, 0, this->getIdString());
   mMountParent = obj;
}

void CompBase::onUnmount(SceneObject* obj, S32 node)
{
   obj->setDataField(mIdent, 0, "");
   mMountParent = NULL;
}
