#ifndef __DUNGEONTEST_SCENE_H__
#define __DUNGEONTEST_SCENE_H__

#include "cocos2d.h"
#include <stdio.h>

USING_NS_CC;
using namespace std;

template<class T>
struct DisableCompare :public std::binary_function<T,T,bool>
{
    bool operator()(T lhs,T rhs)  const
    {
        return true;
     }
};


struct IVec2
{
	IVec2()
	{
		x = 0;
		y = 0;
	};
	IVec2(int _x, int _y)
	{
		x = _x;
		y = _y;
	};
	IVec2(const IVec2 &ivec)
	{
		x = ivec.x;
		y = ivec.y;
	};

	inline int dis(const IVec2 &vec)
	{
		return abs(x-vec.x) + abs(y-vec.y);	
	}

	inline IVec2& operator+(const IVec2 &vec)
	{
		x += vec.x;
		y += vec.y;

		return *this;
	};

	inline bool operator==(const IVec2 &vec) const
	{
		return x == vec.x && y == vec.y;
	}	
	
	inline bool operator<(const IVec2 &vec) const
	{
		if (x < vec.x)
		{
			return true;
		}
		else if(x == vec.x)
		{
			return y < vec.y;
		}

		return false;
	}

	int x;
	int y;
};

class Room
{
public:
	Room();
	Room(IVec2 origin,IVec2 size);
	~Room();

	CC_SYNTHESIZE(IVec2,_origin,Origin);
	CC_SYNTHESIZE(IVec2,_size,RoomSize);

	bool Overlap(Room* room);
};

class DungeonTest : public cocos2d::Layer
{
public:

	DungeonTest();
	~DungeonTest();

	static cocos2d::Scene* createScene();
	virtual bool init();
	CREATE_FUNC(DungeonTest);

public:

	void createRooms();
	void createCorridors();
	void connect();
	void removeDeadEnds();

	void renderScene();

private:

	bool isUnmade(int x,int y);
	bool isEquel(int x,int y,int value);
	bool isExist(int x,int y);
	bool inRegion(int x,int y);

	vector<Room*> _rooms;
	DrawNode* _drawnode;
	int _width;
	int _height;
	int** _blocks;

	int _currentRegion;
	int _regionRoomMax;
	int _regionCorridorMax;

};

#endif // __DUNGEONTEST_SCENE_H__
