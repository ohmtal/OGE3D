//-----------------------------------------------------------------------------
//  License at: ohmtal/misc/ohmtalMIT.h
//-----------------------------------------------------------------------------
#include "scene/sceneRenderState.h"
#include "ClientTsStatic.h"

IMPLEMENT_CONOBJECT(ClientTsStatic);

bool ClientTsStatic::onAdd()
{

//   mNetFlags.clear(Ghostable | ScopeAlways);
   mNetFlags.set(IsGhost );

   return Parent::onAdd();
   
}

void ClientTsStatic::setSkinName(const char* name)
{
   if (name[0] != '\0')
   {
      // Use tags for better network performance
      // Should be a tag, but we'll convert to one if it isn't.
      if (name[0] == StringTagPrefixByte)
         mSkinNameHandle = NetStringHandle(U32(dAtoi(name + 1)));
      else
         mSkinNameHandle = NetStringHandle(name);
   }
   else
      mSkinNameHandle = NetStringHandle();

   reSkin();
}



void ClientTsStatic::prepRenderImage(SceneRenderState* state)
{
   // we have no game connection so shadows dont work so we only do the diffuse pass
   // prevent flicker in loadfiledlg dts preview :) NOT :(
   if (!state->isDiffusePass())
   {
      return;
   }
   Parent::prepRenderImage(state);
}
