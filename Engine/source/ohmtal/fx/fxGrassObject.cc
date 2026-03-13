//-----------------------------------------------------------------------------
// fxGrassObject
//
// look at T3D/fx/groundCover.* for VBO/Material and rendering!!! 
//
// 
// by T.Huehn 2010/2024
// Render a  GrassObject using VBO and Shaders
//
//
/*
   History (auteria):
      2010-07-18 Copy from Testobject, cleanup and named as fxGrassObject
      2024-02-17 Many enhancements :D * allow stuff, flat, flat on ground , Seed:D
      2024-02-23 search in worldbox only
   History (OGE3D):
      2024-02-23 updated OGE3D skeleton
      2026-03-13 started porting

----

      TODO:
      [ ] Vertex Logic: Implement the precise vertex/index assignment logic within the for loop in initBuffers << initVBO

      [ ] Networking: Update the packUpdate and unpackUpdate to properly serialize the new mMaterialName and handle the
      buffer regeneration flags if initBuffers is triggered on the client.

      [ ] Final Verification: Compile and test
      [ ] Cleanup code

----

Old shaders:


  ~~~~ vertex ~~~~

#version 110

uniform float time;
uniform float lod_metric;
uniform float swayMul;

void main(void)
{


	gl_TexCoord[0] = gl_MultiTexCoord0;

	vec4 vertex = gl_Vertex;
	vec4 vertexMV = gl_ModelViewMatrix * gl_Vertex;
	vec3 normalMV = gl_NormalMatrix * gl_Normal;

	if(swayMul > 0.0 && gl_Normal.z < 0.0 && length(vertexMV) < lod_metric) {
		normalMV = -normalMV;
		vertex.x += swayMul*cos(time) * cos(vertex.x) * sin(vertex.x);
		vertex.y += swayMul*sin(time) * cos(vertex.x) * sin(vertex.x);
	}

	gl_FrontColor.a = 1.0 - clamp(length(vertexMV)/lod_metric, 0.0, 1.0);


	gl_Position = gl_ModelViewProjectionMatrix * vertex;

	gl_TexCoord[1] = gl_TextureMatrix[0] * gl_Vertex;


}

 ~~~~ fragment ~~~~

#version 110

uniform sampler2D texDiffuse;
uniform vec4 sunAmbient;
uniform vec4 sunColor;

void main (void)
{
	vec4 cBase = texture2D(texDiffuse, gl_TexCoord[0].st);
	if(cBase.a < 0.4) discard;



   //made it a bit lighter in 2.24
   // added ambient in 2.30

   vec4 cAmbient = 0.5 * sunAmbient;
   vec4 cDiffuse = 0.5 * sunColor;

	//gl_FragColor = cAmbient * cBase + cDiffuse * cBase;



	vec4 color;
	color = gl_FrontLightModelProduct.sceneColor +
		 gl_FrontMaterial.ambient +
		 gl_FrontMaterial.diffuse ;

	color *= cAmbient * cBase + cDiffuse * cBase;


	gl_FragColor = color ;
	gl_FragColor.a = gl_Color.a;
}

*/
//-----------------------------------------------------------------------------
#include "scene/sceneRenderState.h"
#include "core/stream/bitStream.h"
#include "materials/sceneData.h"
#include "gfx/gfxDebugEvent.h"
#include "gfx/gfxTransformSaver.h"
#include "renderInstance/renderPassManager.h"
#include "gfx/gfxDrawUtil.h"

#include "console/consoleTypes.h"
#include "math/mathIO.h"
#include "console/simBase.h"

#include "ohmtal/ai/aiMath.h"
#include "core/util/safeDelete.h"

#include "fxGrassObject.h"

//------------------------------------------------------------------------------
extern bool gEditingMission;
#ifdef TGE_ENVCACHE
extern bool gDevelMode;
#endif
//------------------------------------------------------------------------------



IMPLEMENT_CO_NETOBJECT_V1(fxGrassObject);

//------------------------------------------------------------------------------
// Helper: _initMaterial
//------------------------------------------------------------------------------
void fxGrassObject::_initMaterial()
{
   if (mMatInst)
      return;

   // Load the material, using a default if none is specified.
   if (mMaterialName.isEmpty())
      mMaterialName = "GrassMaterial";

   Material *mat = static_cast<Material*>(Sim::findObject(mMaterialName));
   if (!mat)
   {
      Con::warnf("fxGrassObject::_initMaterial - Could not find material: %s", mMaterialName.c_str());
      return;
   }

   mMatInst = mat->createMatInstance();
   mMatInst->init(FeatureSet(), getGFXVertexFormat<GFXVertexPNT>());

   // Cache shader parameter handles
   mSwayParam = mMatInst->getMaterialParameterHandle("swayMulti");
   mTimeParam = mMatInst->getMaterialParameterHandle("time");
   mLodParam = mMatInst->getMaterialParameterHandle("lodMetric");
}

//------------------------------------------------------------------------------
// Implementation: initBuffers
//------------------------------------------------------------------------------
bool fxGrassObject::initBuffers()
{
   _initMaterial();

   U32 lVertexCountPerBlade = mFlat ? 4 : 12;
   U32 lIndexCountPerBlade = mFlat ? 6 : 18;
   U32 lGrassCount = (U32)(mCount * mRatio);

   U32 totalVerts = lGrassCount * lVertexCountPerBlade;
   U32 totalIndices = lGrassCount * lIndexCountPerBlade;

   mVertexBuffer.set(GFX, totalVerts, GFXBufferTypeStatic);
   mPrimitiveBuffer.set(GFX, totalIndices, totalVerts, GFXBufferTypeStatic);

   GFXVertexPNT *vb = mVertexBuffer.lock();

   U16 *ib;
   GFXPrimitive *prim;
   mPrimitiveBuffer.lock(&ib, &prim);

   RandomGen.setSeed(mSeed);

   mVertexBuffer.unlock();
   mPrimitiveBuffer.unlock();

   return true;
}


//------------------------------------------------------------------------------
// Class: fxGrassObject
//------------------------------------------------------------------------------

fxGrassObject::fxGrassObject()
{
      // Setup NetObject.
   mTypeMask |= StaticObjectType | StaticShapeObjectType;
   mNetFlags.set(Ghostable);

#ifdef TGE_ENVCACHE
   if (!gDevelMode)
      mNetFlags.clear(Ghostable);
#endif

   // Texture Handle.
   // mTextureHandle = NULL;
   mTextureName = StringTable->insert("");


   mIsSquare = false;
   mInnerRadX = mInnerRadY = 0.f;
   mOuterRadX = mOuterRadY = 80.f;

   mMinWidth = mMaxWidth = 2.5f;
   mMinHeight = mMaxHeight = 1.5f;

   mOnlyInBoxZ = false;

   mOffsetZ = 0;
   mRandomFlip = true;
   mFlat = false; //2.30 
   mFlatGround = false; //2.30 
   mSeed = Sim::getCurrentTime(); //  //2.30 


   mDistance = 120.f;
   mCount = 5000;

   mSwayMulti = 0.5f;
   mObjBoxZ = 20.f;


   mMaterialGrassIndex = 0;

   //2.30 >>>
   mAllowOnTerrain = true;
   mAllowOnInteriors = false;
   mAllowStatics = false;
   mAllowUnderWater = true; //on terrain under water
   mAllowOnlyUnderWater = false; //Only under water 
   mAllowWaterSurface = false;
   //<<< 2.30 


   mLastCheckSum = 0.f;

   mTerrain = NULL;


   mRatio = 1.f;
   //sucks   mLastRatioFloat = 0.f;
}

//------------------------------------------------------------------------------

fxGrassObject::~fxGrassObject()
{

  //FIXME  SAFE_DELETE(m_pVBO);
}

//------------------------------------------------------------------------------

void fxGrassObject::initPersistFields()
{
   // Initialise parents' persistent fields.
   Parent::initPersistFields();

   // Add out own persistent fields.
   addGroup("Grass");
   addField("Seed", TypeS32, Offset(mSeed, fxGrassObject));
   addField("GrassFile", TypeFilename, Offset(mTextureName, fxGrassObject));
   addField("GrassCount", TypeS32, Offset(mCount, fxGrassObject));
   addField("SwayMulti", TypeF32, Offset(mSwayMulti, fxGrassObject));
   addField("ViewDistance", TypeF32, Offset(mDistance, fxGrassObject));
   addField("IsSquareArea", TypeBool, Offset(mIsSquare, fxGrassObject));
   addField("RandomFlip", TypeBool, Offset(mRandomFlip, fxGrassObject));
   addField("FlatPlane", TypeBool, Offset(mFlat, fxGrassObject));
   addField("GroundFlatPlane", TypeBool, Offset(mFlatGround, fxGrassObject));

   endGroup("Grass");
   addGroup("Area");
   addField("InnerRadiusX", TypeF32, Offset(mInnerRadX, fxGrassObject));
   addField("InnerRadiusY", TypeF32, Offset(mInnerRadY, fxGrassObject));
   addField("OuterRadiusX", TypeF32, Offset(mOuterRadX, fxGrassObject));
   addField("OuterRadiusY", TypeF32, Offset(mOuterRadY, fxGrassObject));
   addField("OnlyInBoxZ", TypeBool, Offset(mOnlyInBoxZ, fxGrassObject));

   endGroup("Area");

   addGroup("Restrictions");
   addField("OnTerrain", TypeBool, Offset(mAllowOnTerrain, fxGrassObject));
   addField("OnInteriors", TypeBool, Offset(mAllowOnInteriors, fxGrassObject));
   addField("OnStatics", TypeBool, Offset(mAllowStatics, fxGrassObject));
   addField("UnderWater", TypeBool, Offset(mAllowUnderWater, fxGrassObject));
   addField("OnlyUnderWater", TypeBool, Offset(mAllowOnlyUnderWater, fxGrassObject));
   addField("OnWaterSurface", TypeBool, Offset(mAllowWaterSurface, fxGrassObject));
   addField("MaterialGrassIndex", TypeS32, Offset(mMaterialGrassIndex, fxGrassObject));
   endGroup("Restrictions");

   addGroup("Dimensions");
   addField("MinWidth", TypeF32, Offset(mMinWidth, fxGrassObject));
   addField("MaxWidth", TypeF32, Offset(mMaxWidth, fxGrassObject));
   addField("MinHeight", TypeF32, Offset(mMinHeight, fxGrassObject));
   addField("MaxHeight", TypeF32, Offset(mMaxHeight, fxGrassObject));
   addField("OffsetZ", TypeF32, Offset(mOffsetZ, fxGrassObject));
   addGroup("Dimensions");

}

//------------------------------------------------------------------------------

bool fxGrassObject::onAdd()
{
   if (!Parent::onAdd())
      return(false);

   if (isClientObject())
   {
      // Add the Replicator to the Replicator Set.
      SimSet* lSet = dynamic_cast<SimSet*>(Sim::findObject("fxGrassObjSet"));
      if (!lSet) {
         lSet = new SimSet();
         lSet->assignName("fxGrassObjSet");
      }
      lSet->addObject(this);

   }

   // Set initial bounding box.
   updateWorldBox();
   // Set the Render Transform.
   setRenderTransform(mObjToWorld);




   // Add to Scene.
   addToScene();



   // Return OK.
   return(true);
}
//--------------------------------------------------------------------------
void fxGrassObject::updateWorldBox()
{
   mObjBox.minExtents.set(-mOuterRadX, -mOuterRadY, -mObjBoxZ * 2.f);
   mObjBox.maxExtents.set(mOuterRadX, mOuterRadY, mObjBoxZ * 2.f);


   resetWorldBox();
}

//------------------------------------------------------------------------------
// bool fxGrassObject::initVBO()
// {
//
//
//    mObjBoxZ = 20.f; //2.30 sanity
//
//    if (!mAllowOnTerrain &&
//       !mAllowOnInteriors &&
//       !mAllowStatics &&
//       !mAllowUnderWater &&
//       !mAllowWaterSurface
//       )
//    {
//       // Problem ...
//       Con::warnf(ConsoleLogEntry::General, "fxGrassObject - Could not place Grass, All alloweds are off!");
//       // Return here.
//       return false;
//    }
//
//
//    S32 lVertexCount = 12;
//
//    //2.30 flat on Ground
//    if (mFlat) {
//       lVertexCount = 4;
//    }
//
//
//
//
//    updateWorldBox();
//
//    //Checksum
//    F32 lCheckSum = mOuterRadX + mOuterRadY + mInnerRadX + mInnerRadY
//       + mMinWidth + mMaxWidth + mMinHeight + mMaxHeight
//       + mOffsetZ + mRandomFlip + mCount + mDistance + mIsSquare + mSwayMulti
//       + mFlat + mSeed + mFlatGround + mAllowUnderWater + mAllowOnlyUnderWater
//       + mAllowOnTerrain + mAllowOnInteriors + mAllowStatics + mAllowWaterSurface
//       + mOnlyInBoxZ
//       + getTransform().toEuler().lenSquared() + getPosition().lenSquared();
//    ;
//
//
//    F32 lGrassRatio = Con::getFloatVariable("$pref::Grass::replicationRatio", 1.0f);
//
//    if (!force && m_pVBO && mLastCheckSum == lCheckSum && lCheckSum != 0.f)
//       return true;
//    mLastCheckSum = lCheckSum;
//
//    SAFE_DELETE(m_pVBO);
//
//
//
//    if (mCount * lVertexCount > 240000)
//    {
//       mCount = (S32)mFloor(240000 / lVertexCount);
//       Con::errorf("Count is to high ?!, cut to 240k Vertexes = %d  - it crashes with more no idea why!", mCount);
//    }
//
//
//    U32 lGrassCount = mFloor((F32)mCount * lGrassRatio);
//
//
//    const Point2F pos0(cosf(RADIANS(0.0f)), sinf(RADIANS(0.0f)));
//    const Point2F pos120(cosf(RADIANS(120.0f)), sinf(RADIANS(120.0f)));
//    const Point2F pos240(cosf(RADIANS(240.0f)), sinf(RADIANS(240.0f)));
//
//    //2.30 flat on Ground
//    const F32 lHalfPlane = 0.5f;
//    const Point3F tFlatVertices[] =
//    {
//       Point3F(-lHalfPlane, -lHalfPlane, 0.0f),
//       Point3F(-lHalfPlane, lHalfPlane, 0.0f),
//       Point3F(lHalfPlane, lHalfPlane, 0.0f),
//       Point3F(lHalfPlane, -lHalfPlane, 0.0f)
//    };
//
//
//
//    const Point3F tVertices[] = {
//       Point3F(-pos0.x, -pos0.y, 0.0f),		Point3F(-pos0.x, -pos0.y, 1.0f),		Point3F(pos0.x, pos0.y, 1.0f),		Point3F(pos0.x, pos0.y, 0.0f),
//       Point3F(-pos120.x, -pos120.y, 0.0f),	Point3F(-pos120.x, -pos120.y, 1.0f),	Point3F(pos120.x, pos120.y, 1.0f),	Point3F(pos120.x, pos120.y, 0.0f),
//       Point3F(-pos240.x, -pos240.y, 0.0f),	Point3F(-pos240.x, -pos240.y, 1.0f),	Point3F(pos240.x, pos240.y, 1.0f),	Point3F(pos240.x, pos240.y, 0.0f)
//    };
//
//
//
//    const Point2F tTexcoords[] = {
//       Point2F(0.0f, 1.f), Point2F(0.0f, 0.0f), Point2F(1.0f, 0.0f), Point2F(1.0f, 1.0f),
//       Point2F(0.0f, 1.f), Point2F(0.0f, 0.0f), Point2F(1.0f, 0.0f), Point2F(1.0f, 1.0f),
//       Point2F(0.0f, 1.f), Point2F(0.0f, 0.0f), Point2F(1.0f, 0.0f), Point2F(1.0f, 1.0f)
//    };
//
//    m_pVBO = new VertexBufferObject();
//
//    m_pVBO->getPosition().reserve(lGrassCount * lVertexCount);
//    m_pVBO->getNormal().reserve(lGrassCount * lVertexCount);
//    m_pVBO->getTexcoord().reserve(lGrassCount * lVertexCount);
//
//
//    F32				HypX, HypY;
//    F32				Angle;
//
//
//    bool lRaydidHit = false;
//    Point3F lLoc;
//    Point3F ln;
//
//
//    Point3F randomShapePosLocal;
//    Point3F			FoliageStart;
//    Point3F			FoliageEnd;
//    Point3F			FoliagePosition;
//    RayInfo			RayEvent;
//
//    Point3F			ltmpvert;
//
//    F32				lterrZ = AIMath::getTerrainHeight(getPosition());
//
//
//
//    F32 lWidth = mMaxWidth;
//    F32 lHeight = mMaxHeight;
//
//
//    RandomGen.setSeed(mSeed);
//
//    bool lIamHappyOnWater = false;
//    bool lWaterHit = false;
//
//    S32  lFinalCount = 0;
// #ifdef TORQUE_DEBUG
//    Con::printf("=================================================");
//    Con::printf("Grass VBOinit Pos=%0.f,%0.f,%0.f Count = %d, Seed: %d, Ratio is: %f",
//       getPosition().x, getPosition().y, getPosition().z, lGrassCount, mSeed, mRatio);
// #endif // TORQUE_DEBUG
//
//    for (S32 i = 0; i < lGrassCount; i++)
//    {
//       if (mMinWidth < mMaxWidth)
//          lWidth = RandomGen.randF(mMinWidth, mMaxWidth);
//       if (mMinHeight < mMaxHeight)
//          lHeight = RandomGen.randF(mMinHeight, mMaxHeight);
//
//       if (mIsSquare)
//       {
//          // ensure we are not within the inner radius
//          bool tryAgain;
//          S32  letmeout = 1000;
//
//          do
//          {
//             // Incase we don't find a proper place.
//             tryAgain = false;
//
//             HypX = RandomGen.randF(0, mOuterRadX * 2) - (mOuterRadX);
//             HypY = RandomGen.randF(0, mOuterRadY * 2) - (mOuterRadY);
//
//             if (HypX < (mInnerRadX) && HypY < (mInnerRadY)
//                && HypX > -(mInnerRadX) && HypY > -(mInnerRadY))
//             {
//                tryAgain = true;
//             }
//             else
//             {
//                randomShapePosLocal.x = HypX;
//                randomShapePosLocal.y = HypY;
//             }
//
//             letmeout--;
//
//          } while (tryAgain || letmeout < 0);
//          if (letmeout < 0)
//             continue;
//       }
//       else {
//          HypX = RandomGen.randF(mInnerRadX, mOuterRadX);
//          HypY = RandomGen.randF(mInnerRadY, mOuterRadY);
//          Angle = RandomGen.randF(0, M_2PI);
//
//          // Calculate the new position.
//          randomShapePosLocal.x = HypX * mCos(Angle);
//          randomShapePosLocal.y = HypY * mSin(Angle);
//
//       }
//       randomShapePosLocal.z = 0;
//       // Transform into world space coordinates
//       Point3F shapePosWorld;
//       MatrixF objToWorld = getRenderTransform();
//       objToWorld.mulP(randomShapePosLocal, &shapePosWorld);
//
//
//       if (!mOnlyInBoxZ)
//       {
//          shapePosWorld.z = AIMath::getTerrainHeight(shapePosWorld);
//       }
//       else {
//          shapePosWorld.z = getPosition().z;
//       }
//       FoliagePosition = shapePosWorld;
//       // Initialise RayCast Search Start/End Positions.
//       FoliageStart = FoliageEnd = FoliagePosition;
//
//
//       if (mOnlyInBoxZ)
//       {
//          FoliageStart.z += 40.f * getScale().z;
//          FoliageEnd.z -= 40.f * getScale().z;
//       }
//       else {
//
//          FoliageStart.z += 1000.f;
//          FoliageEnd.z -= 50.f;
//       }
//
//
//
//       //water handling ....
//       lIamHappyOnWater = false;
//       lWaterHit = false;
//
//
//       if (gClientContainer.castRay(FoliageStart, FoliageEnd
//          , WaterObjectType | TerrainObjectType | InteriorObjectType | StaticObjectType
//          , &RayEvent)
//          ) //if  ...
//       {
//          if (RayEvent.object->getTypeMask() & WaterObjectType) {
//             if (mAllowWaterSurface)
//                lIamHappyOnWater = true;
//             lWaterHit = true;
//          }
//
//       }
//
//
//       if (!lIamHappyOnWater && gClientContainer.castRay(FoliageStart, FoliageEnd
//          , TerrainObjectType | InteriorObjectType | StaticObjectType
//          , &RayEvent))
//       {
//          if (mAllowOnTerrain && (RayEvent.object->getTypeMask() & TerrainObjectType))
//          {
//
//             if (lWaterHit) {
//                if (!mAllowUnderWater && !mAllowOnlyUnderWater)
//                   continue;
//             }
//             else if (mAllowOnlyUnderWater) {
//                continue;
//             }
//
//          }
//          else if (mAllowOnInteriors && (RayEvent.object->getTypeMask() & InteriorObjectType))
//          {
//             if (lWaterHit) {
//                if (!mAllowUnderWater && !mAllowOnlyUnderWater)
//                   continue;
//             }
//             else if (mAllowOnlyUnderWater) {
//                continue;
//             }
//
//          }
//          else if (mAllowStatics && ((RayEvent.object->getTypeMask() & StaticTSObjectType) || (RayEvent.object->getTypeMask() & StaticShapeObjectType)))
//          {
//             if (lWaterHit) {
//                if (!mAllowUnderWater && !mAllowOnlyUnderWater)
//                   continue;
//             }
//             else if (mAllowOnlyUnderWater) {
//                continue;
//             }
//          }
//          else {
//             //no match
//             continue;
//          }
//
//
//       }
//       else { //castRay2
//          if (!lIamHappyOnWater)
//             continue;
//       }
//
//
//
//       lFinalCount++;
//
//
//       if (mMaterialGrassIndex != 0 && (RayEvent.object->getTypeMask() & TerrainObjectType))
//       {
//          if (!mTerrain) //caching
//             mTerrain = dynamic_cast<TerrainBlock*>(Sim::findObject("Terrain"));
//
//
//
//          if (mTerrain)
//          {
//             S32 mapIndex = mTerrain->getTerrainMapIndex(RayEvent.point);
//             if (mapIndex != -1)
//             {
//                if (!mMatMap) //Caching!
//                   mMatMap = static_cast<MaterialPropertyMap*>(Sim::findObject("MaterialPropertyMap"));
//                const MaterialPropertyMap::MapEntry* pEntry = mMatMap->getMapEntryFromIndex(mapIndex);
//                if (pEntry->fxGrassIndex < 0 || (mMaterialGrassIndex != 0 && pEntry->fxGrassIndex != mMaterialGrassIndex))
//                   continue; //we dont want the grass here!
//             }
//             else {
//                //texture is not defined lets check if we have a index set
//                if (mMaterialGrassIndex != 0)
//                   continue;
//             }
//          }
//       }//MaterialGrassIndex
//
//
//       ln = RayEvent.normal;
//       ln.normalize();
//
//       lLoc = RayEvent.point;
//
//       if (mFlat && mFlatGround) //z-fighting!
//       {
//          lLoc.z += RandomGen.randF(mOffsetZ - 0.1f, mOffsetZ + 0.1f);
//       }
//       else {
//          lLoc.z += mOffsetZ;
//       }
//
//       if (mAbs(lLoc.z - getPosition().z) > mObjBoxZ)
//          mObjBoxZ = mAbs(lLoc.z - getPosition().z);
//
//
//       F32 lRandAngle = RandomGen.randF(0.f, M_2PI);
//
//       for (U32 j = 0; j < lVertexCount; j++)
//       {
//          //flat on ground 2.30
//          if (mFlat && mFlatGround)
//          {
//             ltmpvert = tFlatVertices[j];
//          }
//          else {
//             ltmpvert = tVertices[j];
//          }
//
//
//          if (mRandomFlip)
//             ltmpvert = AIMath::RotateAroundOrigin(ltmpvert, lRandAngle);
//
//
//
//
//          ltmpvert.x *= lWidth;
//          ltmpvert.y *= lWidth;
//          ltmpvert.z *= lHeight;
//
//          if (mFlat && mFlatGround) //conformToGround
//          {
//             Point3F  x, ltmpN = -ln;
//             mCross(ltmpvert, ltmpN, &x);
//             //bad idea	x.normalize();
//             mCross(ltmpN, x, &ltmpvert);
//          }
//
//          Point3F data = ltmpvert + lLoc;
//
//          m_pVBO->getPosition().push_back(data);
//          m_pVBO->getNormal().push_back(tTexcoords[j].len() < 0.2f ? -ln : ln);
//          m_pVBO->getTexcoord().push_back(tTexcoords[j]);
//       }
//    } //for
//
//    //2.30 sanity!
//    if (lFinalCount < 1)
//    {
// #ifdef TORQUE_DEBUG
//       Con::printf("Final Grass Count = %d ", lFinalCount);
//       Con::printf("~~~~~~~~~~~~~~ cancel nothing match ~~~~~~~~~~~~~");
//       Con::printf("=================================================");
// #endif // TORQUE_DEBUG
//
//       SAFE_DELETE(m_pVBO);
//       return false;
//    }
//
//    m_pVBO->getPosition().compact();
//    m_pVBO->getNormal().compact();
//    m_pVBO->getTexcoord().compact();
//
// #ifdef TORQUE_DEBUG
//
//    Con::printf("Final Grass Count = %d ", lFinalCount);
//    Con::printf("VBO Position Size = %d ", m_pVBO->getPosition().size());
//    Con::printf("=================================================");
// #endif // TORQUE_DEBUG
//
//
//    bool result = m_pVBO->Create(GL_STATIC_DRAW_ARB);
//    if (!result)
//       mLastCheckSum = 0.f; //reset!
//    return result;
//    // return m_pVBO->Create(GL_STATIC_DRAW_ARB);
//
// }

//------------------------------------------------------------------------------

void fxGrassObject::onRemove()
{
   // Remove from Scene.
   removeFromScene();

   // Do Parent.
   Parent::onRemove();
}

//------------------------------------------------------------------------------

void fxGrassObject::inspectPostApply()
{
   // Set Parent.
   Parent::inspectPostApply();

   if (mInnerRadX >= mOuterRadX)
      mInnerRadX = 0;
   if (mInnerRadY >= mOuterRadY)
      mInnerRadY = 0;

   if (mMinWidth > mMaxWidth)
      mMinWidth = mMaxWidth;
   if (mMinHeight > mMaxHeight)
      mMinHeight = mMaxHeight;


   // Set fxPortal Mask.
   setMaskBits(fxGrassObjectMask);
}

//------------------------------------------------------------------------------

void fxGrassObject::onEditorEnable()
{
}

//------------------------------------------------------------------------------

void fxGrassObject::onEditorDisable()
{
}

//------------------------------------------------------------------------------

// void fxGrassObject::prepRenderImage(SceneRenderState* state)
// {
//
//    if (!state->isDiffusePass() || !isRenderEnabled())
//    {
//       return;
//    }
// /* FIXME
//    if (!m_pVBO || !m_pVBO->getOK())
//       return false;
//
//    // Return if we don't have a texture.
//    if (!mTextureHandle)
//       return false;
//
//
// #ifdef TGE_WATER
//    // XXTH added to deny rendering on reflect pass which is really bad !
//    // since the this eat fps like hell :P
//    if (gClientSceneGraph->mReflectPass)
//       return false;
// #endif
//
// #ifdef TGE_SHADOWMAP
//    if (gClientSceneGraph->mShadowPass)
//       return false;
// #endif
//
//
//    // Return if last state.
//    if (isLastState(state, stateKey))
//       return false;
//
//
//    // Set Last State.
//    setLastState(state, stateKey);
//
//
//
//    // Is Object Rendered?
//    if (state->isObjectRendered(this))
//    {
//       // calc distance cap
//       Point3F cameraOffset;
//       getRenderTransform().getColumn(3, &cameraOffset);
//       cameraOffset -= state->getCameraPosition();
//       F32 dist = cameraOffset.len();
//       //check visible distance ..
//       F32 lCap = mDistance + getWorldSphere().radius;
//       if (dist > lCap)
//          return false;
//
//       // Yes, so get a SceneRenderImage.
//       SceneRenderImage* image = new SceneRenderImage;
//       // Populate it.
//       image->obj = this;
//
//       image->isTranslucent = false;
//       image->sortType = SceneRenderImage::Normal;
//
//
//       // Insert it into the scene images.
//       state->insertRenderImage(image);
//    }
//
//    return false;
// */
// }

//------------------------------------------------------------------------------
// void fxGrassObject::renderObject(SceneRenderState* state)
// {
// /* FIXME
//    // Check we are in Canonical State.
//    AssertFatal(dglIsInCanonicalState(), "Error, GL not in canonical state on entry");
//    if (!g_GrassShader || !m_pVBO || !m_pVBO->getOK()) //2.30 sanity + 2.31 extra sanity
//       return;
//
//
//    // Save state.
//    RectI viewport;
//
//
//
//    // Save Projection Matrix so we can restore Canonical state at exit.
//    glMatrixMode(GL_PROJECTION);
//    glPushMatrix();
//
//    // Save Viewport so we can restore Canonical state at exit.
//    dglGetViewport(&viewport);
//
//    //2.30 added light
//    //fail gClientSceneGraph->getLightManager()->sgSetupLights(this);
//
//    // Setup the projection to the current frustum.
//    //
//    // NOTE:-	You should let the SceneGraph drive the frustum as it
//    //			determines portal clipping etc.
//    //			It also leaves us with the MODELVIEW current.
//    //
//    state->setupBaseProjection();
//
//
//    // Save ModelView Matrix so we can restore Canonical state at exit.
//    glPushMatrix();
//
//
//    glEnable(GL_BLEND);
//    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//    g_GrassShader->Activate();
//
//
//    glActiveTextureARB(GL_TEXTURE0_ARB);
//    glEnable(GL_TEXTURE_2D);
//    glBindTexture(GL_TEXTURE_2D, mTextureHandle.getGLName());
//
//
//    g_GrassShader->UniformTexture("texDiffuse", 0);
//
//    F32 thetimeis = (F32)Platform::getRealMilliseconds() / 1000;
//
//    g_GrassShader->Uniform("time", thetimeis);
//    g_GrassShader->Uniform("swayMul", mSwayMulti);
//
//    g_GrassShader->Uniform("lod_metric", getMin(state->getFogDistance(), mDistance * 2));
//
//
//
// #ifdef TGE_DAYNIGHT
//    if (gCelestials)
//    {
//       g_GrassShader->Uniform("sunColor", gCelestials->getCurrentColor()); //multiplied by 1.5 / 2022-02-24! and removed again 2022-03-03
//       g_GrassShader->Uniform("sunAmbient", gCelestials->getCurrentColor()); //2.30
//    }
//    else
// #endif
//    {
//       LightInfo* sunLight = gClientSceneGraph->getLightManager()->sgGetSpecialLight(LightManager::sgSunLightType);
//
//
//       //2.30 sucks g_GrassShader->Uniform("sunColor",sunLight->mAmbient+sunLight->mColor);
//       g_GrassShader->Uniform("sunColor", sunLight->mColor * 2.5f);
//       g_GrassShader->Uniform("sunAmbient", sunLight->mAmbient * 2.5f); //2.30
//
//    }
//
//    //new 2.30
//    //fail S32 lLightCount = (S32)gClientSceneGraph->getLightManager()->getBestLightCount();
//    //fail g_GrassShader->Uniform("lightCount", lLightCount);
//
//
//    //VBO!
//    glPushAttrib(GL_POLYGON_BIT);
//    glDisable(GL_CULL_FACE);
//    m_pVBO->Enable();
//
//
//    U32 indices_count = (U32)(mRatio * m_pVBO->getPosition().size());
//    indices_count -= indices_count % 4;
//
//
//
//    glDrawArrays(GL_QUADS, 0, indices_count);
//
//    m_pVBO->Disable();
//    glPopAttrib();
//
//    glActiveTextureARB(GL_TEXTURE0_ARB);
//    glBindTexture(GL_TEXTURE_2D, 0);
//
//
//    g_GrassShader->Deactivate();
//    glDisable(GL_BLEND);
//    glDisable(GL_TEXTURE_2D);
//
//
//    // Restore our canonical matrix state.
//    glPopMatrix();
//    glMatrixMode(GL_PROJECTION);
//    glPopMatrix();
//
//
//    glMatrixMode(GL_MODELVIEW);
//
//    //2.30
//    //fail gClientSceneGraph->getLightManager()->sgResetLights();
//
//    // Restore our canonical viewport state.
//    dglSetViewport(viewport);
//
//    // Check we have restored Canonical State.
//    AssertFatal(dglIsInCanonicalState(), "Error, GL not in canonical state on exit");
// */
// }
//

//------------------------------------------------------------------------------

U32 fxGrassObject::packUpdate(NetConnection* con, U32 mask, BitStream* stream)
{
   // Pack Parent.
   U32 retMask = Parent::packUpdate(con, mask, stream);

   // Write fxPortal Mask Flag.
   if (stream->writeFlag(mask & fxGrassObjectMask))
   {
      // Write Object Transform.
      stream->writeAffineTransform(mObjToWorld);
      // Write Texture Name.
      stream->writeString(mTextureName);

      stream->write(mOuterRadX);
      stream->write(mOuterRadY);
      stream->write(mInnerRadX);
      stream->write(mInnerRadY);

      stream->write(mMinWidth);
      stream->write(mMaxWidth);
      stream->write(mMinHeight);
      stream->write(mMaxHeight);

      stream->write(mOffsetZ);
      stream->write(mRandomFlip);
      stream->write(mFlat); //2.30 
      stream->write(mFlatGround); //2.30 
      stream->write(mSeed); //2.30  Replicator Seed.

      stream->write(mCount);
      stream->write(mDistance);
      stream->write(mIsSquare);

      stream->write(mSwayMulti);

      stream->write(mMaterialGrassIndex);

      stream->writeFlag(mAllowOnTerrain);					// Allow on Terrain.
      stream->writeFlag(mAllowOnInteriors);				// Allow on Interiors.
      stream->writeFlag(mAllowStatics);					// Allow on Statics.
      stream->writeFlag(mAllowUnderWater);			    // Allow under Water.
      stream->writeFlag(mAllowOnlyUnderWater);			 // Allow under Water ONLY
      stream->writeFlag(mAllowWaterSurface);				// Allow on Water Surface.

      stream->writeFlag(mOnlyInBoxZ);				// restrict raycast Z


   }

   updateWorldBox();
   // Were done ...
   return(retMask);
}

//------------------------------------------------------------------------------

void fxGrassObject::unpackUpdate(NetConnection* con, BitStream* stream)
{
   // Unpack Parent.
   Parent::unpackUpdate(con, stream);


   // Read fxPortal Mask Flag.
   if (stream->readFlag())
   {
      MatrixF		ObjectMatrix;

      // Read Object Transform.
      stream->readAffineTransform(&ObjectMatrix);
      // Read Texture Name.
      mTextureName = StringTable->insert(stream->readSTString());


      stream->read(&mOuterRadX);
      stream->read(&mOuterRadY);
      stream->read(&mInnerRadX);
      stream->read(&mInnerRadY);

      stream->read(&mMinWidth);
      stream->read(&mMaxWidth);
      stream->read(&mMinHeight);
      stream->read(&mMaxHeight);

      stream->read(&mOffsetZ);
      stream->read(&mRandomFlip);
      stream->read(&mFlat);
      stream->read(&mFlatGround);
      stream->read(&mSeed);

      stream->read(&mCount);
      stream->read(&mDistance);
      stream->read(&mIsSquare);

      stream->read(&mSwayMulti);

      stream->read(&mMaterialGrassIndex);

      mAllowOnTerrain = stream->readFlag();				// Allow on Terrain.
      mAllowOnInteriors = stream->readFlag();				// Allow on Interiors.
      mAllowStatics = stream->readFlag();					// Allow on Statics.
      mAllowUnderWater = stream->readFlag();				// Allow on Water.
      mAllowOnlyUnderWater = stream->readFlag();			// Allow on Water. ONLY
      mAllowWaterSurface = stream->readFlag();			// Allow on Water Surface.

      mOnlyInBoxZ = stream->readFlag();			// restrict raycast Z to box

      // Set Transform.
      setTransform(ObjectMatrix);

      // Reset our previous texture handle.
       // mTextureHandle = NULL;
      // Load the texture (if we've got one)
/* FIXME load/bind material !!!!
      if (*mTextureName) mTextureHandle = TextureHandle(mTextureName, BitmapTexture, true);
*/

      // does not help against flicker if (*mTextureName) mTextureHandle = TextureHandle(mTextureName, MeshTexture);





      //FIXME
      // if (gEditingMission) //2.30 replaced mClientReplicationStarted)
      //    initVBO();

      // Set the Render Transform.
      setRenderTransform(mObjToWorld);
   }
}


//------------------------------------------------------------------------------------------------------
DefineEngineFunction(StartGrassObjReplication, void, (), , "@brief StartGrassObjReplication()")
{
   // Find the Replicator Set.
   SimSet* fxGrassSet = dynamic_cast<SimSet*>(Sim::findObject("fxGrassObjSet"));

   // Return if Error.
   if (!fxGrassSet)
   {
      // Console Warning.
      Con::warnf("fxGrassOBJReplicator - Cannot locate the 'fxGrassObjSet', this is bad!");
      // Return here.
      return;
   }

   //#ifdef TORQUE_MULTITHREAD //2.30 extra thread !!
   //   if (!gFxGrasObjectMutex)
   //	   gFxGrasObjectMutex = Mutex::createMutex();
   //   Mutex::lockMutex(gFxGrasObjectMutex, false);
   //#endif



      // Parse Replication Object(s).
   for (SimSetIterator itr(fxGrassSet); *itr; ++itr)
   {
      // Fetch the Replicator Object.
      fxGrassObject* Replicator = static_cast<fxGrassObject*>(*itr);
      // Start Client Objects Only.
      if (Replicator->isClientObject())
      {
         //FIXME Replicator->initVBO();
         //V2.23 added:
         // OGE3D not available ?! Platform::process(); //screen refresh !
      }
   }
   //#ifdef TORQUE_MULTITHREAD //2.30 extra thread !!
   //	Mutex::unlockMutex(gFxGrasObjectMutex);
   //	Mutex::destroyMutex(gFxGrasObjectMutex);
   //	gFxGrasObjectMutex = NULL;
   //#endif


      // Info ...
   Con::printf("fxGrassObj - Client Grass Replication Startup is complete.");
}

void fxGrassObject::prepRenderImage(SceneRenderState* state)
{
      if (!mMatInst) _initMaterial();
      if (!state->isDiffusePass() || !mMatInst) return;

      Point3F cameraOffset = getPosition() - state->getCameraPosition();
      if (cameraOffset.len() > (mDistance + getWorldSphere().radius)) return;

      ObjectRenderInst *ri = state->getRenderPass()->allocInst<ObjectRenderInst>();
      ri->renderDelegate.bind(this, &fxGrassObject::renderObject);
      ri->type = RenderPassManager::RIT_Object;
      ri->userData = this; // Store context
      state->getRenderPass()->addInst(ri);
}

void fxGrassObject::renderObject(ObjectRenderInst *ri, SceneRenderState *state, BaseMatInstance *overrideMat)
{
      fxGrassObject* obj = static_cast<fxGrassObject*>(ri->userData);
      if (!obj || !obj->mVertexBuffer.isValid() || !obj->mMatInst) return;

      SceneData sgData;
      sgData.init(state, SceneData::RegularBin);

      MaterialParameters* params = obj->mMatInst->allocMaterialParameters();
      obj->mMatInst->setMaterialParameters(params);
      if (obj->mMatInst->setupPass(state, sgData))
      {
            GFX->setVertexBuffer(obj->mVertexBuffer);
            GFX->setPrimitiveBuffer(obj->mPrimitiveBuffer);
            GFX->drawIndexedPrimitive(GFXTriangleList, 0, 0, obj->mVertexBuffer->mNumVerts, 0, obj->mPrimitiveBuffer->mIndexCount / 3);
      }
}
