#ifndef _B2BTEST_H_
#define _B2BTEST_H_


#ifndef _CONSOLEOBJECT_H_
#include "console/consoleObject.h"
#endif


#ifndef _MPOINT_H_
#include "math/mPoint3.h"
#endif

#ifndef _TOM2DRENDEROBJ_H_
#include "tom2DRenderObject.h"
#endif


#ifndef BOX2D_H
#include "Box2D/Box2D.h"
#endif




namespace b2bTest
{

	class TestWorldB2B : public tom2DRenderObject
	{
	private:
		typedef tom2DRenderObject Parent;
	
	protected:

		F32 winWidth;
		F32 winHeight;

		b2Vec2 mGravity;
		b2World* mWorld;
		//b2BodyDef mGroundBodyDef;
		//b2Body* mGroundBody;
		//b2PolygonShape mGroundBox;
		b2BodyDef mBodyDef;
		b2Body* mBody;
		b2PolygonShape mDynamicBox;
		b2FixtureDef mFixtureDef;


		b2CircleShape mBallShape;
		b2Body* mBallBody;


	public:
		DECLARE_CONOBJECT(TestWorldB2B);

		/**
  		 * 
		 **/
		TestWorldB2B();

		/**
		 * Deletes any allocated memory.
		 **/
		virtual ~TestWorldB2B();


		virtual void onRender(U32 dt, Point3F lOffset);
		virtual void onUpdate(F32 fDt);
		


	};



}
#endif // !_B2BTEST_H_
