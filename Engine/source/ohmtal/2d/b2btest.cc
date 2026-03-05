//-----------------------------------------------------------------------------
// Ohmtal Game Engine

//-----------------------------------------------------------------------------
/**
 * Box2D Test 
 * @author T.Huehn
 * @since  2021-03-18

 $testWorld = new TestWorldB2B(){x=300; y=300; layer=1;};GameScreen.addRenderObject($testworld);

 $testWorld = new TestWorldB2B(){x=100; y=100; layer=1;};GameScreen.addRenderObject($testworld);


 $testWorld.delete();
 */



#include "platform/platform.h"
#include "console/console.h"
#include "console/engineAPI.h"
#include "core/util/safeDelete.h"
#include "gfx/gfxDevice.h"
#include "gfx/gfxDrawUtil.h"
#include "Box2D/Box2D.h"
#include "tom2DCtrl.h"
#include "b2btest.h"

namespace b2bTest
{
    IMPLEMENT_CONOBJECT(TestWorldB2B);

    TestWorldB2B::TestWorldB2B( void )
    {

//FIXME ME USE SCALE PIXEL TO METER!!

        //** could be created in tom2DCtrl
        //Creating a World
        mGravity = b2Vec2(0.f, 9.81f);
       // mWorld->SetGravity(mGravity);
        mWorld = new b2World(mGravity);

        //added to tom2dSprite ?!
        /*
        four kinds of different shapes:
          see also collision/Shapes ...
            class b2Shape;
            class b2CircleShape;
            class b2EdgeShape;
            class b2PolygonShape;
        
        */

        // Create edge around entire screen
        b2BodyDef groundBodyDef;
        groundBodyDef.position.Set(0, 0);

        winWidth = 200;
        winHeight = 200;

        b2Body* groundBody = mWorld->CreateBody(&groundBodyDef);
        b2EdgeShape groundEdge;
        b2FixtureDef boxShapeDef;
        boxShapeDef.shape = &groundEdge;

        // Wall definitions
        
        groundEdge.Set(b2Vec2(0, 0), b2Vec2(winWidth, 0));
        groundBody->CreateFixture(&boxShapeDef);

        groundEdge.Set(b2Vec2(0, 0), b2Vec2(0, winHeight));
        groundBody->CreateFixture(&boxShapeDef);

        groundEdge.Set(b2Vec2(0, winHeight), b2Vec2(winWidth, winHeight));
        groundBody->CreateFixture(&boxShapeDef);

        groundEdge.Set(b2Vec2(winWidth, winHeight), b2Vec2(winWidth, 0));
        groundBody->CreateFixture(&boxShapeDef);
        /*
        //Creating a Ground Box - static body
        mGroundBodyDef.position.Set(0.f, 100.f);
        mGroundBody = mWorld->CreateBody(&mGroundBodyDef);
        mGroundBox.SetAsBox(50.0f, 10.0f);
        mGroundBody->CreateFixture(&mGroundBox, 0.0f);
        */
        //Creating a Dynamic Body
        mBodyDef.type = b2_dynamicBody;
        mBodyDef.position.Set(50.0f, 50.0f);
        mBody = mWorld->CreateBody(&mBodyDef);
        mDynamicBox.SetAsBox(3.0f, 3.0f);
        mFixtureDef.shape = &mDynamicBox;
        mFixtureDef.density = 1.0f;
        mFixtureDef.friction = 0.01f;
        //power of bouncing back: 
        mFixtureDef.restitution = 12.f; //0.8

        mBody->CreateFixture(&mFixtureDef);

        //Creating a Dynamic Bouncing Ball => restitution is the answer
        
        b2BodyDef ballBodyDef;
        ballBodyDef.type = b2_dynamicBody;
        ballBodyDef.position.Set(100.f,  100.f);

      

        // link to rendered object:  ballBodyDef.userData = _ball;
        mBallBody = mWorld->CreateBody(&ballBodyDef);

        b2CircleShape mBallShape;
        mBallShape.m_radius = 5.0; 

        b2FixtureDef ballShapeDef;
        ballShapeDef.shape = &mBallShape;
        ballShapeDef.density = 1.0f;
        ballShapeDef.friction = 0.2f;
        //power of bouncing back: 
        ballShapeDef.restitution = 1.f; // 10.8f; //0.8
        mBallBody->CreateFixture(&ballShapeDef);

    }

    TestWorldB2B::~TestWorldB2B() {
        if (mScreen) {
            // mScreen->removeSprite(this);
            mScreen->removeRenderObject(this);
            mScreen = NULL;
        }

        SAFE_DELETE(mWorld);
    }

    void TestWorldB2B::onUpdate(F32 fDt)
    {
        U32 velocityIterations = 8;
        U32 positionIterations = 2;

        mWorld->Step(fDt, velocityIterations, positionIterations);
    }

    void TestWorldB2B::onRender(U32 dt, Point3F lOffset)
    {
        //dglDrawText(mScreen->getProfile()->mFont, Point2I(mX, mY), "Box2D Test!!");
        GFX->getDrawUtil()->drawText(mScreen->getProfile()->mFont, Point2I(mX, mY), "Box2D Test!!");

        b2Vec2 position = mBody->GetPosition();
        //dglDrawLine(Point2I(mX, mY), Point2I(mRayPos.x, mRayPos.y), ColorF(1.0f, 0.20f, 0.20f, 0.75f));

        //mDynamicBox
        F32 lRadius = 3; //mDynamicBox.m_radius * 1000;
        RectI lBox = RectI(
               (U32)(mX + position.x - lRadius), (U32)(mY + position.y - lRadius),
               (U32)( lRadius * 2), (U32)(lRadius * 2)
            );
        GFX->getDrawUtil()->drawRectFill(lBox, ColorI(52, 52, 230, 197));
         
        float angle = mBody->GetAngle();
        //Con::printf("%4.2f %4.2f %4.2f\n", position.x, position.y, angle);


        position = mBallBody->GetPosition();
        //mDynamicBox
        lRadius = 5; // mBallShape.m_radius; //FIXME!! mBallShape.m_radius * 1000;
        lBox = RectI(
            (U32)(mX + position.x - lRadius), (U32)(mY + position.y - lRadius),
            (U32)(lRadius * 2), (U32)(lRadius * 2)
        );
        GFX->getDrawUtil()->drawRectFill(lBox, ColorI(0, 255, 0, 197));

        angle = mBallBody->GetAngle();
        //Con::printf("%4.2f %4.2f %4.2f\n", position.x, position.y, angle);

        
       // ~~~

        /* 
        position = mGroundBody->GetPosition();
       
        lBox = RectI(
            (U32)(mX + position.x - 50), (U32)(mY + position.y - 10),
            (U32)(100), (U32)(20)
        );
        dglDrawRectFill(lBox, ColorF(1.0f, 0.20f, 0.20f, 0.75f));
        */
        ColorI linecolor = ColorI(255, 255, 255, 255);
        GFX->getDrawUtil()->drawLine(Point2I(mX, mY), Point2I(winWidth + mX, mY), linecolor);
        GFX->getDrawUtil()->drawLine(Point2I(winWidth + mX, mY), Point2I(winWidth + mX, winHeight + mY), linecolor);
        GFX->getDrawUtil()->drawLine(Point2I(winWidth + mX, winHeight + mY), Point2I(mX, winHeight + mY), linecolor);
        GFX->getDrawUtil()->drawLine(Point2I(mX, winHeight + mY), Point2I(mX, mY), linecolor);


        
    }


    //-------------------------------------------------------------------------------------------------------
    //-------------------------------------------------------------------------------------------------------
    // https://box2d.org/documentation/md__d_1__git_hub_box2d_docs_hello.html
    void hello()
    {
        //Creating a World
        b2Vec2 gravity(0.0f, -10.0f);
        b2World world(gravity);

        //Creating a Ground Box
        b2BodyDef groundBodyDef;
        groundBodyDef.position.Set(0.0f, -10.0f);
        b2Body* groundBody = world.CreateBody(&groundBodyDef);
        b2PolygonShape groundBox;
        groundBox.SetAsBox(50.0f, 10.0f);
        groundBody->CreateFixture(&groundBox, 0.0f);
        
        //Creating a Dynamic Body
        b2BodyDef bodyDef;
        bodyDef.type = b2_dynamicBody;
        bodyDef.position.Set(0.0f, 4.0f);
        b2Body* body = world.CreateBody(&bodyDef);

        b2PolygonShape dynamicBox;
        dynamicBox.SetAsBox(1.0f, 1.0f);

        b2FixtureDef fixtureDef;
        fixtureDef.shape = &dynamicBox;
        fixtureDef.density = 1.0f;
        fixtureDef.friction = 0.3f;
        body->CreateFixture(&fixtureDef);

        // Simulating the World
        F32 timeStep = 1.0f / 60.0f;
        U32 velocityIterations = 6;
        U32 positionIterations = 2;
        for (U32 i = 0; i < 60; ++i)
        {
            world.Step(timeStep, velocityIterations, positionIterations);
            b2Vec2 position = body->GetPosition();
            float angle = body->GetAngle();
            Con::printf("%4.2f %4.2f %4.2f\n", position.x, position.y, angle);
        }



    }

} //namespace b2bTest


//===========================================================================
/* What this for ?
DefineEngineStringlyVariadicFunction(b2bTest_Hello, void, 1, 1, "")
{
    b2bTest::hello();
}


//Example:
//function fooDings(){ long %i = 74005544; return %i;}
DefineEngineStringlyVariadicFunction(b2bTest_ochjo, void, 1, 1, "")
{
    const char* dings = Con::executef( "fooDings" );
    Con::errorf("DINGS IS: %s", dings);
    F32 fdings = dAtof(dings);
    Con::errorf("DINGS IS: %s float is: %f", dings, fdings);
}
*/
