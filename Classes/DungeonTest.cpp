#include "DungeonTest.h"
#include <time.h>
#include <stack>
#include <vector>
#include <hash_set>
#include <map>

#define random(x,y) (rand()%(y-x+1)+x)

USING_NS_CC;

const int BLOCK_BLANK		 =   -1;
const int BLOCK_DOOR         =   -2;
const int BLOCK_TEST         =   -3;
const int windingPercent     =   50;

const int EXTRA_CONNECT      =   2;

//Room
const int ROOM_COUNT = 50;
Room::Room()
{
}

Room::Room(IVec2 origin,IVec2 size)
{
	_origin = origin;
	_size = size;
}

Room::~Room()
{
}

bool Room::Overlap(Room* room)
{
	return (
		_origin.x + _size.x > room->getOrigin().x &&
		_origin.x < room->getOrigin().x + room->getRoomSize().x &&
		_origin.y < room->getOrigin().y + room->getRoomSize().y &&
		_origin.y + _size.y > room->getOrigin().y
		);
}

//DungeonTest
const int TILED_WIDTH = 10;
DungeonTest::DungeonTest()
{
	_currentRegion = -1;
	_regionRoomMax = -1;
	_regionCorridorMax = -1;
}
DungeonTest::~DungeonTest()
{
	for (int i = 0 ; i < _width ; i++)
	{
		delete [] _blocks[i];
	}

	for (int i = 0 ; i < _rooms.size(); i++)
	{
		delete _rooms[i];
	}
}

Scene* DungeonTest::createScene()
{	
	auto scene = Scene::create();	
	auto layer = DungeonTest::create();
	scene->addChild(layer);	
	return scene;
}


bool DungeonTest::init()
{
	if ( !Layer::init() )
	{
		return false;
	}
	srand((int)time(0));
	_drawnode = DrawNode::create();
	addChild(_drawnode);
	
	auto winSize = Director::getInstance()->getWinSize();
	_width = (int)winSize.width / TILED_WIDTH - 1;
	_height = (int)winSize.height / TILED_WIDTH - 1;
	
	//memory
	_blocks = new int*[_width];
	for (int i = 0 ; i < _width ; i++)
	{
		_blocks[i] = new int[_height];
		for (int j = 0 ; j < _height ; j++)
		{
			_blocks[i][j] = BLOCK_BLANK;
		}
	}

	createRooms();
	createCorridors();
	connect();
	removeDeadEnds();

	renderScene();

	int a = _regionRoomMax;
	int b = _regionCorridorMax;

	return true;
}

void DungeonTest::createRooms()
{
	for (int i = 0 ; i < ROOM_COUNT ; i++)
	{
		int size = random(1,3) * 2 + 1;
		int rec = random(0,size/2) * 2;

		int width = size;
		int height = size;
		if (random(0,1) == 1)
		{
			width += rec;
		}else
		{
			height += rec;
		}

		int x = random(0,(_width - width-1)/2) * 2 +1;
		int y = random(0,(_height - height-1)/2) * 2 +1;

		Room* room = new Room(IVec2(x,y),IVec2(width,height));
		bool overlaps = false;
		size_t len = _rooms.size();
		for (int t = 0 ; t < len ; t++)
		{
			if (room->Overlap(_rooms[t]))
			{
				overlaps = true;
				break;
			}
		}

		if (overlaps)
		{
			continue;
		}

		_rooms.push_back(room);
		_currentRegion++;
		for (int p = x; p < x+width ; p++)
		{
			for (int q = y; q < y+height ; q++)
			{
				_blocks[p][q] = _currentRegion;
			}
		}
	}

	_regionRoomMax = _currentRegion;
}

bool DungeonTest::isExist(int x,int y)
{
	return x>0 && y>0 && x < _width && y < _height;
}

bool DungeonTest::isEquel(int x,int y,int value)
{
	return  isExist(x,y) && (_blocks[x][y] == value);
}

bool DungeonTest::isUnmade(int x,int y)
{
	return isEquel(x,y,BLOCK_BLANK);
}

bool DungeonTest::inRegion(int x,int y)
{
	return isExist(x,y) && (_blocks[x][y] > BLOCK_BLANK);
}

const IVec2 DIR_UP    = IVec2(0,1);
const IVec2 DIR_DOWN  = IVec2(0,-1);
const IVec2 DIR_LEFT  = IVec2(-1,0);
const IVec2 DIR_RIGHT = IVec2(1,0);
const IVec2 DIR_INVID = IVec2(0,0);
void DungeonTest::createCorridors()
{
	stack<IVec2> blocks;
	vector<IVec2> unmadedirs;
	IVec2 lastdir;

	for (int i = 1 ; i < _width ; i += 2)
	{
		for (int j = 1; j < _height ; j += 2)
		{

			if (_blocks[i][j] != BLOCK_BLANK)
				continue;

			_currentRegion++;
			_blocks[i][j] = _currentRegion;
			blocks.push(IVec2(i,j));
			while (!blocks.empty())
			{
				IVec2 block = blocks.top();
				unmadedirs.clear();
				if (isUnmade(block.x+2,block.y))
					unmadedirs.push_back(DIR_RIGHT);
				if (isUnmade(block.x-2,block.y))
					unmadedirs.push_back(DIR_LEFT);
				if (isUnmade(block.x,block.y+2))
					unmadedirs.push_back(DIR_UP);
				if (isUnmade(block.x,block.y-2))
					unmadedirs.push_back(DIR_DOWN);

				if (!unmadedirs.empty())
				{
					vector<IVec2>::iterator it = find(unmadedirs.begin(),unmadedirs.end(),lastdir);

					IVec2 dir;
					if (it != unmadedirs.end() && random(0,100) > windingPercent)
					{
						dir = lastdir;
					}
					else
					{
						dir = unmadedirs[random(0,unmadedirs.size()-1)];
					}

					_blocks[block.x+dir.x][block.y+dir.y] = _currentRegion;
					_blocks[block.x+dir.x*2][block.y+dir.y*2] = _currentRegion;
					blocks.push(IVec2(block.x+dir.x*2,block.y+dir.y*2));
					lastdir = dir;

				}
				else
				{
					blocks.pop();
					lastdir = DIR_INVID;
				}

			}
		}
	}
	_regionCorridorMax = _currentRegion;
}

void DungeonTest::connect()
{
	map<IVec2,hash_set<int>> connector;

	for (int i = 0 ; i < _width ; i++)
	{
		for (int j = 0 ; j < _height ; j++)
		{
			if (_blocks[i][j] != BLOCK_BLANK)
				continue;

			hash_set<int> regions;
			regions.clear();
			if (inRegion(i-1,j))
				regions.insert(_blocks[i-1][j]);
			if (inRegion(i+1,j))
				regions.insert(_blocks[i+1][j]);
			if (inRegion(i,j+1))
				regions.insert(_blocks[i][j+1]);
			if (inRegion(i,j-1))
				regions.insert(_blocks[i][j-1]);

			if (regions.size() < 2)
				continue;
			
			//_blocks[i][j] =  BLOCK_DOOR;
			connector[IVec2(i,j)] = regions;
			

		}
	}

	vector<int> merged;
	list<int> openRegions;
	for (int i = 0 ; i < _currentRegion+1 ; i++)
	{		
		merged.push_back(i);
		openRegions.push_back(i);
	}
	
	while (openRegions.size() > 1)
	{
		int ranNum = random(0,connector.size() - 1);
		map<IVec2,hash_set<int>>::iterator it = connector.begin();
		while (ranNum != 0)
		{
			++it;
			ranNum --;
		}

		IVec2 pos = IVec2((it->first).x,(it->first).y);
		_blocks[pos.x][pos.y] = BLOCK_DOOR;
		int dest = merged[*((it->second).begin())];
		int source = merged[*((it->second).rbegin())];

		//merged[source] = dest;
		for(int i = 0 ; i < _currentRegion+1 ; i++)
		{
			if(merged[i] == source)
				merged[i] = dest;
		}

		list<int>::iterator list_it = openRegions.begin();
		for( ; list_it != openRegions.end() ;)
		{
			if (*list_it == source)
			{
				openRegions.erase(list_it++);
				break;
			}
			else
			{
				++list_it;
			}
		}

		for (it = connector.begin() ; it != connector.end() ;)
		{
			int first = *((it->second).begin());
			int second = *((it->second).rbegin());

			if(merged[first] == dest && merged[second] == dest)
			{
				if (random(0,100) <= EXTRA_CONNECT && pos.dis(IVec2((it->first).x,(it->first.y))) > 2)
					_blocks[(it->first).x][(it->first).y] = BLOCK_DOOR;
				connector.erase(it++);
			}
				
			else
				++it;
		}
	}
}

void DungeonTest::removeDeadEnds()
{

	bool done = false;
	while (!done)
	{
		done = true;

		for (int i = 0 ; i < _width ; i++)
		{
			for (int j = 0 ; j < _height ; j++)
			{
				//must corridors
				if (_blocks[i][j] <= _regionRoomMax)
					continue;

				int count = 0;
				if (!isExist(i+1,j) || isUnmade(i+1,j))
					count ++;
				if (!isExist(i-1,j) || isUnmade(i-1,j))
					count ++;
				if (!isExist(i,j+1) || isUnmade(i,j+1))
					count ++;
				if (!isExist(i,j-1) || isUnmade(i,j-1))
					count ++;

				if (count != 3)
					continue;

				done = false;
				_blocks[i][j] = BLOCK_BLANK;
			}
		}
	}

	
}

void DungeonTest::renderScene()
{
	Vec2 tiled = Vec2(TILED_WIDTH,TILED_WIDTH);

	for (int i = 0 ; i < _width ; i++)
	{
		for (int j = 0 ; j < _height ; j++)
		{
			//_blocks[i][j] = 1;
			Vec2 origin = Vec2(i*TILED_WIDTH,j*TILED_WIDTH);
			int block = _blocks[i][j];
			if (block == BLOCK_BLANK)
			{
				continue;
			}
			else if(block == BLOCK_DOOR)
			{
				_drawnode->drawSolidRect(origin,origin+tiled,Color4F::RED);
			}
			else if(block == BLOCK_TEST)
			{
				_drawnode->drawSolidRect(origin,origin+tiled,Color4F::GREEN);
			}
			else if(block <= _regionRoomMax)
			{			
				_drawnode->drawRect(origin,origin+tiled,Color4F::RED);
				_drawnode->drawSolidRect(origin,origin+tiled,Color4F::BLUE);
			}
			else if(block <= _regionCorridorMax)
			{
				_drawnode->drawSolidRect(origin,origin+tiled,Color4F::GRAY);
			}
			

		}
	}
}