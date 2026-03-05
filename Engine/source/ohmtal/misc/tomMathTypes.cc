//-----------------------------------------------------------------------------
// Ohmtal Game Engine 
// Copyright (C) ohmtal game studio
//-----------------------------------------------------------------------------
//  License at: ohmtal/misc/ohmtalMIT.h
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// math Objects/Functions
// 
// FIXME  DefineEngineMethod
// 
//-----------------------------------------------------------------------------
#include "console/engineAPI.h"
#include "console/consoleObject.h"
#include "math/mRect.h"
#include "math/mathTypes.h"

#include "tomMathTypes.h"

//-----------------------------------------------------------------------------
// RectF Class 
//-----------------------------------------------------------------------------
IMPLEMENT_CONOBJECT(tomRectF);

tomRectF::tomRectF()
{
	mRectF = RectF(0.f, 0.f, 0.f,0.f);

	
}
void tomRectF::initPersistFields()
{
	Parent::initPersistFields();
	addField("rect", TypeRectF, Offset(mRectF, tomRectF));
}

ConsoleMethod(tomRectF, get, const char*, 2, 2, "return RectF (point, extent)")
{
	char* rbuf = Con::getReturnBuffer(256);
	dSprintf(rbuf, 255, "%f %f %f %f", 
		object->mRectF.point.x
		, object->mRectF.point.y
		, object->mRectF.extent.x
		, object->mRectF.extent.y
	);

	return rbuf;
}
ConsoleMethod(tomRectF, getAsInt, const char*, 2, 2, "return RectI (point, extent)")
{
	char* rbuf = Con::getReturnBuffer(256);
	dSprintf(rbuf, 255, "%d %d %d %d",
		(S32) object->mRectF.point.x
		, (S32)object->mRectF.point.y
		, (S32)object->mRectF.extent.x
		, (S32)object->mRectF.extent.y
	);

	return rbuf;
}

ConsoleMethod(tomRectF, getAbsolute, const char*, 2, 2, "return RectF (point1, point2)")
{
	char* rbuf = Con::getReturnBuffer(256);
	dSprintf(rbuf, 255, "%f %f %f %f",
		object->mRectF.point.x
		, object->mRectF.point.y
		, object->mRectF.point.x + object->mRectF.extent.x
		, object->mRectF.point.y + object->mRectF.extent.y
	);

	return rbuf;
}

ConsoleMethod(tomRectF, getAbsoluteAsInt, const char*, 2, 2, "return RectI (point1, point2)")
{
	char* rbuf = Con::getReturnBuffer(256);
	dSprintf(rbuf, 255, "%d %d %d %d",
		(S32)object->mRectF.point.x
		, (S32)object->mRectF.point.y
		, (S32)object->mRectF.point.x + object->mRectF.extent.x
		, (S32)object->mRectF.point.y + object->mRectF.extent.y
	);

	return rbuf;
}


ConsoleMethod(tomRectF, getLeftTop, const char*, 2, 2, "return Point2F (point1)")
{
	char* rbuf = Con::getReturnBuffer(256);
	dSprintf(rbuf, 255, "%f %f",
		object->mRectF.point.x
		, object->mRectF.point.y
	);

	return rbuf;
}

ConsoleMethod(tomRectF, getRightBottom, const char*, 2, 2, "return Point2F (point1)")
{
	char* rbuf = Con::getReturnBuffer(256);
	dSprintf(rbuf, 255, "%f %f",
		object->mRectF.point.x + object->mRectF.extent.x
		, object->mRectF.point.y + object->mRectF.extent.y
	);

	return rbuf;
}


ConsoleMethod(tomRectF, left, F32, 2, 2, "return left(x)")
{
	return object->mRectF.point.x;
}

ConsoleMethod(tomRectF, top, F32, 2, 2, "return top(y)")
{
	return object->mRectF.point.y;
}

ConsoleMethod(tomRectF, right, F32, 2, 2, "return right(x)")
{
	return object->mRectF.point.x + object->mRectF.extent.x;
}

ConsoleMethod(tomRectF, bottom, F32, 2, 2, "return bottom(y)")
{
	return object->mRectF.point.y + +object->mRectF.extent.y;
}

ConsoleMethod(tomRectF, width, F32, 2, 2, "return len_x")
{
	return object->mRectF.len_x();
}

ConsoleMethod(tomRectF, height, F32, 2, 2, "return len_y")
{
	return object->mRectF.len_y();
}
ConsoleMethod(tomRectF, halfwidth, F32, 2, 2, "return len_x")
{
	return object->mRectF.len_x() / 2;
}
ConsoleMethod(tomRectF, halfheight, F32, 2, 2, "return len_y")
{
	return object->mRectF.len_y() / 2;
}

ConsoleMethod(tomRectF, centerX, F32, 2, 2, "return centreX")
{
	return object->mRectF.centre().x;
}

ConsoleMethod(tomRectF, centerY, F32, 2, 2, "return centreY")
{
	return object->mRectF.centre().y;
}

ConsoleMethod(tomRectF, center, const char*, 2, 2, "return centre")
{
	char* rbuf = Con::getReturnBuffer(256);
	Point2F lCentre = object->mRectF.centre();
	dSprintf(rbuf, 255, "%f %f",  lCentre.x, lCentre.y);

	return rbuf;
}
ConsoleMethod(tomRectF, intersect, bool, 3, 3, "param tomRectf id, return len_y")
{
	SimObject* obj = Sim::findObject(argv[1]);
	if (obj) {
		tomRectF* lRect = static_cast<tomRectF*>(obj);
		if (lRect) {
			RectF lDummy; 
			lDummy.point = object->mRectF.point;
			lDummy.extent = object->mRectF.extent;
			return lDummy.intersect(lRect->mRectF);
		}
	}
	return false;
}

ConsoleMethod(tomRectF, pointInRect, bool, 3, 3, "param Point2F pt")
{
	const Point2F pt; 
	dSscanf(argv[2], "%g %g", &pt.x, &pt.y);
	return object->mRectF.pointInRect(pt);

}


//bool contains(const RectI& R) const;
//void inset(S32 x, S32 y);

