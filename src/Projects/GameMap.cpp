        #include "GameMap.h"
#include "Astar.h"
#include <SOIL/SOIL.h>
#include <random>
#include <iostream>
#include "Config.h"
#include "Environment.h"
#include "Demo.h"

#include "Astar/include/InputHandler.h"



GameMap::GameMap(uint32 mapId)
{ 	/*读取配置*/
	Config config(Environment::GetAbsPath("Resource/Table/config.txt"));
	config.Print();
	std::string fileName = config.GetMapPath(config.mDict["map"][mapId]);

	mXyqMap = new NetEase::MAP(fileName);

	mMapWidth = mXyqMap->m_MapWidth;
	mMapHeight = mXyqMap->m_MapHeight;

	mWidth = mXyqMap->m_Width;
	mHeight = mXyqMap->m_Height;
	
	mRow = mXyqMap->m_RowCount;
	mCol = mXyqMap->m_ColCount;
	printf("初始化GameMap %d %d ", mRow, mCol);

	mMapTiles.clear();
	
	for (int i = 0; i<mXyqMap->m_UnitSize; i++)
	{
		mXyqMap->ReadUnit(i);
		
		mMapTiles[i] = (new Texture(320,240, false,(uint8*)(mXyqMap->m_MapUnits)[i].BitmapRGB24) );
	//	 delete[] mXyqMap->m_MapUnits[i].BitmapRGB24;
//		mXyqMap->m_MapUnits[i].BitmapRGB24 = nullptr;
	}
	


	mMaskTiles.clear();
	for (int i = 0; i<mXyqMap->m_MaskSize; i++)
	{
		mXyqMap->ReadMask(i);

		mMaskTiles.push_back(new Texture(mXyqMap->m_MaskInfos[i].Width,
			mXyqMap->m_MaskInfos[i].Height,true, (uint8*)(mXyqMap->m_MaskInfos)[i].Data ));

//		 delete[] mXyqMap->m_MaskInfos[i].Data;
//		mXyqMap->m_MaskInfos[i].Data = nullptr;
	}

	delete[] mXyqMap->m_MapPixelsRGB24;
	mXyqMap->m_MapPixelsRGB24 = nullptr;
	
	mCellWidth = mCol * 16;
	mCellHeight = mRow * 12;
	mCell = new int*[mCellWidth];
	for (int i = 0; i < mCellWidth; i++)
	{
		mCell[i] = new int[mCellHeight];
	}

	int x = 0, y = 0 ,p,q;
	for (int i = 0; i < mRow; i++)
	{
		for (int j = 0; j < mCol; j++)
		{
			y = 12 * i;
			for (p = 0; p < 12; p++)
			{
				x = 16 * j;
				for (q = 0; q < 16; q++)
				{
					mCell[x++][y] = (mXyqMap->m_MapUnits)[i*mCol + j].Cell[p * 16 + q];
				}
				y++;
			}
		}
	}
	//mAstar = new Astar(mCellWidth, mCellHeight, mCell);

	// std::ofstream outfile("a.map");
	// for (int y = 0; y < mCellHeight; y++) 
	// {
	// 	for (int x = 0; x < mCellWidth; x++) {
	// 		//printf("%d", mMap[x][y]);
	// 		outfile << mCell[x][y];
	// 	}
	// 	outfile << '\n';
	// 	//printf("\n");
	// }
	// outfile.close();

	

	mCellPic = new Texture(Environment::GetAssetsPath("X.png"));

}

void GameMap::clamp(int val, int min, int max)
{
	if (val < min) val = min;
	if (val > max) val = max;
}

NetEase::MAP* GameMap::GetMapPtr()
{
	return mXyqMap;
}

bool GameMap::CanArriveDirect(Pos src,Pos dest)
{
	int dx = dest.x - src.x;
	int dy = dest.y - src.y;
	if(dx == 0)
	{
		int opt = dy < 0 ? -1 : 1;
		for(int i=0;i<dy*opt;i++)
		{
			int cellX = src.x;
			int cellY = src.y + i*opt;
			if(mCell[cellX][cellY]>=1)
				return false;
		}
		return true;
	}

	if(dy == 0)
	{
		int opt = dx < 0 ? -1 : 1;
		for(int i=0;i<dx*opt;i++)
		{
			int cellX = src.x + i*opt;
			int cellY = src.y;
			if(mCell[cellX][cellY]>=1)
				return false;
		}
		return true;
	}

	double ratio = dy / dx;
	int opt = dx < 0 ? -1 : 1;
	for (int i = 0; i < dx * opt; ++i)
	{
		int cellX = src.x + opt*i;
		int cellY = src.y + ratio*i*opt;
		if(mCell[cellX][cellY]>=1)
			return false;
	}

	ratio = dx / dy;
	opt = dy < 0 ? -1 : 1;
	for (int i = 0; i < dy * opt; ++i)
	{
		int cellX = src.x + ratio*opt*i;
		int cellY = src.y + i*opt;
		if(mCell[cellX][cellY]>=1)
			return false;
	}

	return true;

}

std::list<Pos> GameMap::Move(int sx, int sy, int ex, int ey)
{
	//printf(" GameMap:sx %d sy %d ex %d ey %d\n", sx, sy, ex, ey);
	if (sx < 0 || sy < 0 || ex < 0 || ey < 0) return{};
	if (sx > mCellWidth || ex > mCellWidth || sy > mCellHeight || ey > mCellHeight) return{};
	if (sx == ex && sy == ey) return{};
	if (mCell[sx][sy] > 0 || mCell[ex][ey] > 0) return{};

	// 	13 ，35
	// 115，20
	// if (mAstar->PathFinding(sx, sy, ex, ey))
	// {
	// 	return mAstar->GetMoveList();
	// }
	// else {
	// 	return{ };
	// }
	int dimension = mCellHeight<mCellWidth? mCellWidth:mCellHeight;
	SquareGraph graph(dimension);
	for (int y = 0; y < dimension; y++)
	{	
		for (int x = 0; x < dimension; x++)
		{
			if(x>=mCellWidth || y >= mCellHeight)
			{
				graph.setCellValue(std::make_pair(y,x),'X');
				continue;
			}
			if (mCell[x][y] >= 1)
			{
				graph.setCellValue(std::make_pair(y,x),'X');
			}else
			{
				graph.setCellValue(std::make_pair(y,x),' ');
			}
		}
	}
	graph.setFirstRobotPos(std::make_pair(sy,sx));
	graph.setSecondRobotPos(std::make_pair(ey,ex));
	std::vector<Node> path = graph.executeAStar();
	//graph.printPath(path);

	std::vector<Pos> moveList;
	moveList.clear();
	for(auto i=path.begin(); i != path.end(); i++){
		Node node = *i;
		//cout << "node : (" << node.x << "," << node.y << ")" << endl;
        moveList.push_back(Pos(node.y,node.x));
	}

    // Smooth move List
    /*
     算法：
     1.第一个节点为当前节点，查看下一个节点
     2.如果可以无障碍直接到达，则下一个节点为第一个节点的后继
     3.如果不可到达，当前节点为后继节点
     */
    int currentNode = 0;
    int nextNode = 1;
    std::list<Pos> smoothMoveList;
    smoothMoveList.push_back(moveList[currentNode]);
    while(nextNode!=moveList.size())
    {
    	int lastNextNode = nextNode;
        while(nextNode!=moveList.size() && CanArriveDirect(moveList[currentNode],moveList[nextNode]) )
        {
        	lastNextNode = nextNode;
        	nextNode++;
        }
        currentNode=lastNextNode;
        smoothMoveList.push_back(moveList[currentNode]);
    }

    for(auto i=smoothMoveList.begin(); i != smoothMoveList.end(); i++){
		Pos node = *i;
		cout << "smoothNode : (" << node.x << "," << node.y << ")" << endl;
        
	}

	return smoothMoveList;

}

GameMap::~GameMap()
{
	if (mXyqMap != nullptr) {
		delete mXyqMap;
		mXyqMap = nullptr;
	}
}
void GameMap::Draw(SpriteRenderer* renderer,int playerX,int playerY)
{
	int screenWidth = Demo::GetScreenWidth();
	int screenHeight = Demo::GetScreenHeight();
	int halfScreenWidth = screenWidth / 2;
	int halfScreenHeight = screenHeight / 2;

	int mapOffsetX = halfScreenWidth - playerX;
	int mapOffsetY = halfScreenHeight - playerY;

	mapOffsetX = GMath::Clamp(mapOffsetX, -mWidth + screenWidth, 0);
	mapOffsetY = GMath::Clamp(mapOffsetY, -mHeight + screenHeight, 0);

	// mGameMap->Draw(m_RendererPtr, mapOffsetX, mapOffsetY);
    int startRow , endRow,startCol,endCol ;
    startRow = playerY/240-3;
    startRow = startRow<0?0:startRow;
    endRow = playerY/240+3;
    endRow = endRow>mRow?mRow:endRow;

    startCol = playerX/320-3;
    startCol = startCol<0?0:startCol;
    endCol = playerX/320+3;
    endCol = endCol>mCol?mCol:endCol;


	for (int i = startRow; i<endRow; i++) {
		for (int j = startCol; j<endCol; j++) {
            int unit = i*mCol+j;


            if(mMapTiles.find(unit) == mMapTiles.end())
            {
            	mXyqMap->ReadUnit(unit);
                //mXyqMap->SaveUnit(unit);

                mMapTiles[unit] = new Texture(320,240, false,(uint8*)(mXyqMap->m_MapUnits)[unit].BitmapRGB24);
                mXyqMap->m_MapUnits[unit].BitmapRGB24 = nullptr;
            }

			renderer->DrawSprite(mMapTiles[i*mCol + j],
				glm::vec2(j * 320 + mapOffsetX, i * 240 + mapOffsetY),
				glm::vec2(320, 240),
				0.0f,
				glm::vec3(1.0f, 1.0f, 1.0f)
			);
		}
	}
}


void GameMap::DrawCell(SpriteRenderer* renderer, int cur_x, int cur_y)
{
    int minx,maxx,miny,maxy;
    int posx,posy;
    posx = -cur_x + 400;
    posy = -cur_y + 300;
    minx = posx - 410 ;
    minx = minx<0?0:minx;
    maxx = posx + 410;
    maxx = maxx>mMapWidth?mMapWidth:maxx;

    miny = posy - 310 ;
    miny = miny<0?0:miny;
    maxy = posy + 310;
    maxy = maxy>mMapHeight?mMapHeight:maxy;


	//for (int y = 0; y < mCellHeight; y++)
    for (int y = miny/20; y < maxy/20; y++)
	{
		//for (int x = 0; x < mCellWidth; x++)
        for (int x = minx/20; x < maxx/20; x++)
		{
			if (mCell[x][y] >= 1)
			{
				 renderer->DrawSprite(mCellPic,
				 	glm::vec2( x*20 + cur_x,
				 		y*20 + cur_y),
				 	glm::vec2(20, 20),
				 	0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
			}
		}
	}
}


void GameMap::DrawMask(SpriteRenderer* renderer, int playerX, int playerY)
{

	int screenWidth = Demo::GetScreenWidth();
	int screenHeight = Demo::GetScreenHeight();
	int halfScreenWidth = screenWidth / 2;
	int halfScreenHeight = screenHeight / 2;

	int mapOffsetX = halfScreenWidth - playerX;
	int mapOffsetY = halfScreenHeight - playerY;

	mapOffsetX = GMath::Clamp(mapOffsetX, -mWidth + screenWidth, 0);
	mapOffsetY = GMath::Clamp(mapOffsetY, -mHeight + screenHeight, 0);


	for (int m = 0; m < mXyqMap->m_MaskSize; m++)
	{
		NetEase::MaskInfo& info = mXyqMap->m_MaskInfos[m];
		renderer->DrawSprite(
			mMaskTiles[m],
			glm::vec2(info.StartX + mapOffsetX,
			info.StartY + mapOffsetY),
			glm::vec2(info.Width, info.Height),
			0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
	}
}

