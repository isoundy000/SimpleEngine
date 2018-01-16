
#pragma once
#include "../defines.h"
#include "../defineTypes.h"
#include "Transform.h"

class Graphics2D;
class Object2D 
{
public:
	enum RenderType
	{
		None,
		Line,
		Rect,
		Circle,
		Image
	};

	Object2D(){};

	~Object2D(){};

	virtual int type(){return None;}
	virtual void Draw( Graphics2D& g2d)  {};

	Vec3& Color(){ return color;};
	Transform& T(){ return transform;};

private:
	Transform transform;
	Vec3 color;
};

class Line : Object2D
{
public:
	Vec2 v1;
	Vec2 v2;
	
	Line(Vec2 v1,Vec2 v2)
	{
		this->v1 = v1;
		this->v2 = v2;

		this->T().pos.x = v1.x;
		this->T().pos.y = v1.y;

		this->T().size.x = v2.x-v1.x;
		this->T().size.y = v2.y-v2.y;

	};
	~Line(){};
	virtual int type(){return Object2D::Line;}
	void Draw( Graphics2D& g2d ) ;
};


class Rect : Object2D
{
public:
	int left;
	int top;
	int right;
	int bottom;

	Rect(int l,int t,int r,int b)
	{
		left= l;
		top = t;
		right= r;
		bottom = b;

		this->T().pos.x = l;
		this->T().pos.y = t;

		this->T().size.x = r-l;
		this->T().size.y = b-t;
	}
	~Rect(){};
	virtual int type(){return Object2D::Rect;}
	void Draw( Graphics2D& g2d) ;
};


class Circle : Object2D
{
public:
	int raduis;
	
	Circle(int x,int y, int r)
	{
		this->T().pos.x = x-r;
		this->T().pos.y = y-r;

		this->T().center.x = x;
		this->T().center.y = y;

		this->T().size.x = 2*r;
		this->T().size.y = 2*r;

		raduis = r;
	}

	~Circle(){};
	virtual int type(){return Object2D::Circle;}
	void Draw(Graphics2D& g2d);
};



class Image : Object2D
{
public:

	String path;
	Image(String path,Vec2 pos,Vec2 size)
	{
		this->path = path;
		this->T().pos.x  = pos.x;
		this->T().pos.y  = pos.y;
		this->T().size.x = size.x;
		this->T().size.y = size.y;

	}
	~Image(){};
	virtual int type(){return Object2D::Image;}
	void Draw(Graphics2D& g2d);
};

class Text : Object2D
{

};