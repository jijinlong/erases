#pragma once
#include "vector"
#include "cocos2d.h"
#include <algorithm>
#include "xmlScript.h"
NS_CC_BEGIN
/**
 * 六边形的网格系统
 * 每横向格子数量相等
 * 每偶数横向有右凹进
 */
class GridIndex{
public:
	enum{
		 STATIC_BLOCK = 1,
		 MONSTER_BLOCK= 1 << 1,
		 HORIZONTAL_BLOCK = 1 << 2, // 横向移动阻挡
		 VERTICAL_BLOCK = 1 << 3, // 纵向移动阻挡
	};
	short x;
	short y;
	GridIndex(short x,short y):x(x),y(y)
	{}
	GridIndex()
	{
		x = y = -1;
	}
	GridIndex & operator=(const GridIndex &index)
	{
		x = index.x;
		y = index.y;
		return *this;
	}
	bool isValid()
	{
		return x != -1 && y != -1;
	}
	bool equal(const GridIndex &index)
	{
		return x == index.x && y == index.y;
	}
	const GridIndex &operator + (const GridIndex &index)
	{
		x += index.x;
		y += index.y;
		return *this;
	}
};
/**
 * 搜索逻辑
 **/
struct stSearchLogic{
public:
	virtual bool getNext(GridIndex &index) const
	{
		return false;
	}
};
/**
 * 回调 针对单元格的处理
 */
struct stExecEach{
public:
	virtual void exec(const GridIndex& index)
	{
		
	}
}; 
/**
 * 六边形网格系统
 */
template<typename CELLOBJECT>
class HexagonGrids{
public:
	/**
	 * 默认划分的格子
	 */
	HexagonGrids(unsigned int width,unsigned int height,int cellWidth):width(width),height(height),cellWidth(cellWidth)
	{
		cells.resize(height);
		for (int i = 0; i < height;i++)
		{
			cells[i].resize(width);
		}
	}
	/**
	 * 获取地图上的实际位置
	 */
	virtual CCPoint getPointByIndex(const GridIndex& index)
	{
		int offsetx = 0;
		if (index.y % 2 == 1) offsetx = cellWidth /2;
		return ccp(offsetx + index.x * cellWidth,index.y * cellWidth);
	}
	virtual GridIndex getIndexByPoint(const CCPoint &point)
	{
		GridIndex index;
		index.y = point.y / cellWidth;
		int offsetx = 0;
		if (index.y % 2 == 1) offsetx = cellWidth /2;
		index.x = (point.x - offsetx) / cellWidth;
		return index;
	}
	/**
	 * 遍历元素
	 * searchType 搜索类型
	 * execEach 每个元素会被执行
	 */
	void exec(const GridIndex& index,const stSearchLogic &searchType, stExecEach *execEach)
	{
		GridIndex next = index;
		while (searchType.getNext(next)) // 获取下一个点
		{ 
			execEach->exec(next); // 执行该节点
		}
	}
	/**
	 * 获取元素
	 */
	CELLOBJECT * getObjectByIndex(const GridIndex &index)
	{
		if (index.x >= 0 && index.y >= 0 && index.x < cells.size() && index.y < cells[index.x].size())
		{
			return &cells[index.x][index.y];
		}
		return NULL;
	}
	/**
	 * 针对单元格进行处理
	 */
	void execOne(const GridIndex &index, stExecEach *execEach)
	{
		execEach->exec(index); // 执行该节点
	}
	/**
	 * 遍历所有的网格
	 */
	void execAll( stExecEach *execEach)
	{
		for (int i = 0; i < width;i++)
			for (int j = 0; j < height;j++)
			{
				GridIndex index(i,j);
				execEach->exec(index); // 执行该节点
			}
	}
public:
	int cellWidth;
	unsigned int width;
	unsigned int height;
	std::vector<std::vector<CELLOBJECT> > cells;
	typedef typename std::vector<std::vector<CELLOBJECT> > CELLS_ITER;
};

/**
* \brief 路径坐标点
*/
struct PathPoint
{
	/**
	* \brief 坐标
	*/
	GridIndex pos;
	/**
	* \brief 当前距离
	*/
	int cc;
	/**
	* \brief 路径上一个结点指针
	*/
	PathPoint *father;
};

/**
* \brief 路径头
*/
struct PathQueue
{
	/**
	* \brief 路径节点头指针
	*/
	PathPoint *node;
	/**
	* \brief 路径消耗距离
	*/
	int cost;
	/**
	* \brief 构造函数
	* \param node 初始化的路径节点头指针
	* \param cost 当前消耗距离
	*/
	PathQueue(PathPoint *node,int cost)
	{
		this->node = node;
		this->cost = cost;
	}
	/**
	* \brief 拷贝构造函数
	* \param queue 待拷贝的源数据
	*/
	PathQueue(const PathQueue &queue)
	{
		node = queue.node;
		cost = queue.cost;
	}
	/**
	* \brief 赋值操作符号
	* \param queue 待赋值的源数据
	* \return 返回结构的引用
	*/
	PathQueue & operator= (const PathQueue &queue)
	{
		node = queue.node;
		cost = queue.cost;
		return *this;
	}
};

struct stCheckMoveAble{
	virtual bool exec(const GridIndex &index) = 0;
};

/**
 * 在六边形网格中全地图搜寻路径
 */
template<typename CELLOBJECT>
class AStarSeachInHexagonGrids:public HexagonGrids<CELLOBJECT>{
public:
	std::vector<GridIndex> adjust;
	AStarSeachInHexagonGrids()
	{
		minRadius = 12;
		initCircles();
	}
	AStarSeachInHexagonGrids(unsigned int width,unsigned int height,int cellWidth):HexagonGrids<CELLOBJECT>(width,height,cellWidth)
	{
		cells.resize(width * height);
		minRadius = 12;
		initCircles();
	}
	virtual void initCircles()
	{
		adjust.clear();
		adjust.push_back(GridIndex(-1,0));
		adjust.push_back(GridIndex(0,1));
		adjust.push_back(GridIndex(0,-1));
		adjust.push_back(GridIndex(1,0));
		adjust.push_back(GridIndex(1,1));
		adjust.push_back(GridIndex(1,-1));
	}
	int minRadius;
	/**
	* \brief 定义所有路径的链表
	*/
	typedef std::list<PathQueue> PathQueueHead;
	typedef typename PathQueueHead::iterator iterator;
	typedef typename PathQueueHead::reference reference;
	/**
	 * 记录close 表
	 */
	std::set<int> mapCloses;
	int getGridIndexValue(const GridIndex &index)
	{
		return index.y << 16 + index.x;
	}
	const GridIndex getGridIndexByValue(const int & index)
	{
		GridIndex index;
		index.x = index & 0xffff;
		index.y = (index >> 16) & 0xffff;
		return index;
	}
	/**
	 * 获取下一个可以移动的格子
	 */
	bool getNextGridIndex(const GridIndex &src,const GridIndex &dest,GridIndex &out, stExecEach *execEach = NULL,stCheckMoveAble*check = NULL)
	{
		//DisMap是以destPos为中心的边长为2 * radius + 1 的正方形
		const int width = (2 * minRadius + 1);
		const int height = (2 * minRadius + 1);
		const int MaxNum = width * height ;
		//把所有路径距离初始化为最大值
		std::vector<int> pDisMap(MaxNum,MaxNum);
		std::vector<PathPoint> stack(MaxNum * 8 + 1);//在堆栈中分配内存
		PathQueueHead queueHead;

		//从开始坐标进行计算
		PathPoint *root = &stack[MaxNum * 8];
		root->pos = src;
		root->cc = 0;
		root->father = NULL;
		enter_queue(queueHead,root,root->cc + judge(root->pos,dest));

		int Count = 0;
		//无论如何,循环超过MaxNum次则放弃
		while(Count < MaxNum)
		{
			root = exit_queue(queueHead);
			if (NULL == root)
			{
				//目标点不可达
				return false;
			}

			if (abs(root->pos.x -  dest.x) <= 0 && abs(root->pos.y - dest.y) <= 0)
			{
				//找到到达目的地的路径
				break;
			}
			std::random_shuffle(adjust.begin(),adjust.end());
			for(int i = 0; i < adjust.size(); i++)
			{
				//分别对周围8个格点进行计算路径
				bool bCanWalk = true;
				GridIndex tempPos = root->pos;
				tempPos.x += adjust[i].x;
				tempPos.y += adjust[i].y;
				if (moveable(tempPos,check))
				{
					//对路径进行回溯
					PathPoint *p = root;
					while(p)
					{
						if (p->pos.x == tempPos.x && p->pos.y == tempPos.y)
						{
							//发现坐标点已经在回溯路径中，不能向前走
							bCanWalk = false;
							break;
						}
						p = p->father;
					}
					/*
					std::set<int>::iterator iter = mapCloses.find(getGridIndexValue(tempPos));
					if (iter != mapCloses.end())
					{
						bCanWalk = false;
					}
					*/
					//如果路径回溯成功，表示这个点是可行走的
					if (bCanWalk)
					{
						int distance = 10 * (abs(adjust[i].x) + abs(adjust[i].y));
						int cost = root->cc + distance;
						int f = cost + judge(tempPos,dest);
						int index = (tempPos.y - dest.y + minRadius) * width + (tempPos.x - dest.x + minRadius);
						if (index >= 0
							&& index < MaxNum
							&& f < pDisMap[index])
						{
							//这条路径比上次计算的路径还要短，需要加入到最短路径队列中
							pDisMap[index] = f;
							PathPoint *pNewEntry = &stack[Count * 8 + i];
							pNewEntry->pos = tempPos;
							pNewEntry->cc = cost;
							pNewEntry->father = root;
							enter_queue(queueHead,pNewEntry,f);
						}
					}
				}
			}

			Count++;
		}
		if (Count < MaxNum)
		{
			//最终路径在PointHead中,但只走一步
			while(root)
			{
				//倒数第二个节点
				if (root->father != NULL
					&& root->father->father == NULL)
				{
					out = root->pos;
					if (execEach)
					execEach->exec(root->pos);
					return true;
				}
				if (execEach)
					execEach->exec(root->pos);
				root = root->father;
			}
		}

		return false;
	}
	/**
	* \brief 估价函数
	* \param midPos 中间临时坐标点
	* \param endPos 最终坐标点
	* \return 估算出的两点之间的距离
	*/
	int judge(const GridIndex &midPos,const GridIndex &endPos)
	{
		int distance = 10 * abs((long)(midPos.x - endPos.x)) + abs((long)(midPos.y - endPos.y));
		return distance;
	}

	/**
	* \brief 进入路径队列
	* \param queueHead 路径队列头
	* \param pPoint 把路径节点添加到路径中
	* \param currentCost 路径的估算距离
	*/
	void enter_queue(PathQueueHead &queueHead,PathPoint *pPoint,int currentCost)
	{
		PathQueue pNew(pPoint,currentCost);
		if (!queueHead.empty())
		{
			for(iterator it = queueHead.begin(); it != queueHead.end(); it++)
			{
				//队列按cost由小到大的顺序排列
				if ((*it).cost > currentCost)
				{
					queueHead.insert(it,pNew);
					return;
				}
			}
		}
		queueHead.push_back(pNew);
	}

	/**
	* \brief 从路径链表中弹出最近距离
	* \param queueHead 路径队列头
	* \return 弹出的最近路径
	*/
	PathPoint *exit_queue(PathQueueHead &queueHead)
	{
		PathPoint *ret = NULL;
		if (!queueHead.empty())
		{
			reference ref = queueHead.front();
			ret = ref.node;
			queueHead.pop_front();
			mapCloses.insert(getGridIndexValue(ret->pos));
		}
		return ret;
	}
	virtual bool moveable(const GridIndex &dest,stCheckMoveAble*check = NULL)
	{
		if (dest.x < 0 || dest.y < 0) return false; 
		if (dest.x > width || dest.y >= this->height) return false;
		if (check && !check->exec(dest)) return false;
		return true;
	}
};

template<typename CELLOBJECT>
class AStarSeachInGrids:public AStarSeachInHexagonGrids<CELLOBJECT>{
public:
	AStarSeachInGrids(unsigned int width,unsigned int height,int cellWidth):AStarSeachInHexagonGrids<CELLOBJECT>(width,height,cellWidth)
	{
		initCircles();
	}
	virtual void initCircles()
	{
		adjust.clear();
		adjust.push_back(GridIndex(0,-1));
		adjust.push_back(GridIndex(-1,0));
		adjust.push_back(GridIndex(1,0));
		adjust.push_back(GridIndex(0,1));
		adjust.push_back(GridIndex(1,-1));
		adjust.push_back(GridIndex(-1,-1));
		adjust.push_back(GridIndex(-1,1));
		adjust.push_back(GridIndex(1,1));
	}
	/**
	 * 获取地图上的实际位置
	 */
	CCPoint getPointByIndex(const GridIndex& index)
	{
		int offsetx = 0;
		return ccp(offsetx + index.x * cellWidth,index.y * cellWidth);
	}
	/**
	 * 根据点获取引索
	 */
	GridIndex getIndexByPoint(const CCPoint &point)
	{
		GridIndex index;
		index.y = point.y / cellWidth;
		int offsetx = 0;
		index.x = (point.x - offsetx) / cellWidth;
		return index;
	}
};
/**
 * 遍历一圈
 */
class XmlSearch:public stSearchLogic{
public:
	virtual bool getNext(GridIndex &index) const
	{
		if (startIndex >= indexs.size()) return false;
		index.x += indexs[startIndex].x;
		index.y += indexs[startIndex].y;
		(*(int*)&startIndex) ++;
		return true;
	}
	XmlSearch()
	{
		startIndex = 0;
	}
	void takeNode(script::tixmlCodeNode *node)
	{
		script::tixmlCodeNode offsetNode = node->getFirstChildNode("point");
		while (offsetNode.isValid())
		{
			GridIndex index;
			index.x = offsetNode.getInt("x");
			index.y = offsetNode.getInt("y");
			indexs.push_back(index);
			offsetNode = offsetNode.getNextNode("point");
		}
	}
	void reset()
	{
		startIndex = 0;
	}
	int startIndex;
	std::vector<GridIndex> indexs;
};
/**
 * 搜寻策略管理器
 */
class SerachTypeManager:public script::tixmlCode{
public:
	static SerachTypeManager &getMe()
	{
		static SerachTypeManager stm;
		return stm;
	}
	SerachTypeManager()
	{
		init();
	}
	std::map<std::string,XmlSearch> searches;
	XmlSearch nullLogic;
	typedef std::map<std::string,XmlSearch>::iterator SEARCHES_ITER;
	void init()
	{
		std::string startui = CCFileUtils::sharedFileUtils()->fullPathFromRelativePath("searchtypes.xml");
		unsigned long nSize = 0;
		unsigned char * buffer = CCFileUtils::sharedFileUtils()->getFileData(startui.c_str(),"rb",&nSize);
		if (!nSize)return;
		initFromString((char*)buffer);
	}
	void takeNode(script::tixmlCodeNode *node)
	{
		if (node && node->equal("Config"))
		{
			script::tixmlCodeNode typeNode = node->getFirstChildNode("search");
			while (typeNode.isValid())
			{
				std::string name = typeNode.getAttr("name");
				XmlSearch search;
				search.takeNode(&typeNode);
				searches[name] = search;
				typeNode = typeNode.getNextNode("search");
			}
		}
	}
	XmlSearch & getSeachByName(const std::string &name)
	{
		SEARCHES_ITER iter = searches.find(name);
		if (iter != searches.end())
		{
			iter->second.reset();
			return iter->second;
		}
		return nullLogic;
	}
};

#define SEARCH_TYPE(name) SerachTypeManager::getMe().getSeachByName(name)
NS_CC_END