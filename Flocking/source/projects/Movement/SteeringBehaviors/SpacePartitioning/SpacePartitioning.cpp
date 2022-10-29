#include "stdafx.h"
#include "SpacePartitioning.h"
#include "projects\Movement\SteeringBehaviors\SteeringAgent.h"

using namespace Elite;
// --- Cell ---
// ------------
Cell::Cell(float left, float bottom, float width, float height)
{
	boundingBox.bottomLeft = { left, bottom };
	boundingBox.width = width;
	boundingBox.height = height;
}

std::vector<Elite::Vector2> Cell::GetRectPoints() const
{
	auto left = boundingBox.bottomLeft.x;
	auto bottom = boundingBox.bottomLeft.y;
	auto width = boundingBox.width;
	auto height = boundingBox.height;

	std::vector<Elite::Vector2> rectPoints =
	{
		{ left , bottom  },
		{ left , bottom + height  },
		{ left + width , bottom + height },
		{ left + width , bottom  },
	};

	return rectPoints;
}

bool Cell::Contains(SteeringAgent* pAgent) const
{
	Vector2 pos{ pAgent->GetPosition() };

	//Pos on Left or bottom edges, returns true
	return pos.x > boundingBox.bottomLeft.x
		&& pos.x < boundingBox.bottomLeft.x + boundingBox.width
		&& pos.y > boundingBox.bottomLeft.y
		&& pos.y < boundingBox.bottomLeft.y + boundingBox.height;
}

bool Cell::Intersects(Elite::Vector2 bottomLeft, float width, float height) const
{
	if (boundingBox.bottomLeft.y > bottomLeft.y + height || bottomLeft.y > boundingBox.bottomLeft.y +boundingBox.height)
		return false;

	if (boundingBox.bottomLeft.x + boundingBox.width < bottomLeft.x || bottomLeft.x + width < boundingBox.bottomLeft.x)
		return false;

	return true;
	return false;
}

// --- Partitioned Space ---
// -------------------------
CellSpace::CellSpace(float width, float height, int rows, int cols, int maxEntities)
	: m_SpaceWidth(width)
	, m_SpaceHeight(height)
	, m_NrOfRows(rows)
	, m_NrOfCols(cols)
	, m_Neighbors(maxEntities)
	, m_NrOfNeighbors(0)
{
	m_CellWidth = width / cols ;
	m_CellHeight = height / rows ;
	m_Neighbors.reserve(maxEntities);
	InitCells();

}

void CellSpace::AddAgent(SteeringAgent* agent)
{
	
	int idx{ PositionToIndex(agent->GetPosition()) };
	m_Cells[idx].agents.push_back(agent);	
}

void CellSpace::UpdateAgentCell(SteeringAgent* agent, Elite::Vector2 oldPos)
{
	int oldIdx{ PositionToIndex(oldPos) };
	int idx{ PositionToIndex(agent->GetPosition()) };
	
	if (oldIdx == idx)
		return;

	m_Cells[oldIdx].agents.remove(agent);
	AddAgent(agent);
}

void CellSpace::RegisterNeighbors(SteeringAgent* agent, float queryRadius, int& nrIterations)
{
	int startRowIdx{}, endRowIdx{}, startColIdx{}, endColIdx{};
	Vector2 targetPos{ agent->GetPosition() };
	GetNeighborhoodCells(targetPos, queryRadius, startRowIdx, endRowIdx, startColIdx, endColIdx);

	int currentIdx{};
	
	m_NrOfNeighbors = 0;	
	for ( int rows = startRowIdx; rows <= endRowIdx; ++rows)
	{
		for (int cols = startColIdx; cols <= endColIdx; ++cols)
		{
			currentIdx = rows * m_NrOfCols + cols;
			for (SteeringAgent* pAgent : m_Cells[currentIdx].agents)
			{ 
				++nrIterations;
				if (pAgent == nullptr || pAgent == agent)
				{
					continue;
				}
				float distance{ (targetPos - pAgent->GetPosition()).Magnitude()};
				if (distance < queryRadius)
				{
					m_Neighbors[m_NrOfNeighbors] = pAgent;
					++m_NrOfNeighbors;
				}
			}

		}
	}

}

void CellSpace::EmptyCells()
{
	for (Cell& c : m_Cells)
		c.agents.clear();
}

void CellSpace::RenderCells() const
{
	for (size_t i = 0; i < m_Cells.size(); i++)
	{
		DEBUGRENDERER2D->DrawPolygon(&m_Cells[i].GetRectPoints()[0],4,{1,0,0},0);
		DEBUGRENDERER2D->DrawString(m_Cells[i].GetRectPoints()[1], std::to_string(m_Cells[i].agents.size()).c_str());
	}
}

int CellSpace::PositionToIndex(const Elite::Vector2 pos) const
{
	float x{ pos.x };
	float y{ pos.y };
	//makes sure we only get valid cell indexes
	if (x < 0)
	{
		x = 0;
	}
	if (x >= m_SpaceWidth)
	{
		x = m_SpaceWidth - 1.f;
	}
	if (y >= m_SpaceHeight)
	{
		y = m_SpaceHeight - 1.f;
	}
	if (y < 0)
	{
		y = 0;
	}
		

	return floorf(x/ m_CellWidth) + floorf(y/ m_CellHeight) * m_NrOfCols ;
}

void CellSpace::InitCells()
{
	m_Cells.reserve(m_NrOfRows * m_NrOfCols);
	for (int rows = 0; rows < m_NrOfRows; rows++)
	{
		for (int cols = 0; cols < m_NrOfCols; cols++)
		{
			m_Cells.push_back(Cell{ cols * m_CellWidth,rows * m_CellHeight,m_CellWidth,m_CellHeight });

		}
	}
}

void CellSpace::GetNeighborhoodCells(const Vector2& centerPos, const float queryRadius, int& startRowIdx, int& endRowIdx, int& startColIdx, int& endColIdx)
{
	Vector2 bottomLeft{ centerPos.x - queryRadius,centerPos.y - queryRadius };
	Vector2 topRight{ centerPos.x + queryRadius, centerPos.y + queryRadius };

	int firstIdx{ PositionToIndex(bottomLeft) };
	int lastIdx{ PositionToIndex(topRight) };

	startRowIdx =  firstIdx / m_NrOfCols;
	endRowIdx = lastIdx / m_NrOfCols ;
	startColIdx = firstIdx % m_NrOfCols;
	endColIdx = lastIdx % m_NrOfCols;
}

void CellSpace::RenderNeighborhoodCells(SteeringAgent* agent, float queryRadius) const
{
	int startRowIdx{}, endRowIdx{}, startColIdx{}, endColIdx{};
	Vector2 targetPos{ agent->GetPosition() };
	Vector2 bottomLeft{ targetPos.x - queryRadius,targetPos.y - queryRadius };
	Vector2 topRight{ targetPos.x + queryRadius, targetPos.y + queryRadius };

	int firstIdx{ PositionToIndex(bottomLeft) };
	int lastIdx{ PositionToIndex(topRight) };

	startRowIdx = firstIdx / m_NrOfCols;
	endRowIdx = lastIdx / m_NrOfCols;
	startColIdx = firstIdx % m_NrOfCols;
	endColIdx = lastIdx % m_NrOfCols;

	int currentIdx{};

	for (int rows = startRowIdx; rows <= endRowIdx; ++rows)
	{
		for (int cols = startColIdx; cols <= endColIdx; ++cols)
		{
			currentIdx = rows * m_NrOfCols + cols;
			DEBUGRENDERER2D->DrawPolygon(&m_Cells[currentIdx].GetRectPoints()[0], 4, { 0,1,0 }, -1);

		}
	}

}

#define BOTTOMLEFT 0
#define BOTTOMRIGHT 1
#define TOPLEFT 2
#define TOPRIGHT 3


//Quadtree
QuadTree::QuadTree(Cell boundingCell, int maxEntities, int depth) :
	m_Cell(boundingCell),
	m_MaxEntities{maxEntities},
	m_DepthLevel{depth},
	m_IsDivided{false},
	m_IsLeaf{true},
	m_Sections(4)
{

}


void QuadTree::AddAgent(SteeringAgent* pAgent)
{
	if (!m_Cell.Contains(pAgent))
		return;
	
	if (m_IsLeaf)
	{
		//Can we add agents?
		if (static_cast<int>(m_Cell.agents.size()) < m_MaxEntities)
		{
			m_Cell.agents.emplace_back(pAgent);
			return;
		}

		//Can we subdevide?
		if (m_DepthLevel < s_MaxDepth)
		{
			
			m_IsLeaf = false;
			if (!m_IsDivided)
				SubdivideSpace();

			//Check in which subsection the new agent belongs
			for (int i = 0; i < 4; ++i)
			{
				m_Sections[i]->AddAgent(pAgent);
			}

			//Move the existing agents to subsections
			std::list<SteeringAgent*>::iterator it;
			for (it = m_Cell.agents.begin(); it !=m_Cell.agents.end(); ++it)
			{
				for (int j = 0; j < 4; j++)
				{
					m_Sections[j]->AddAgent(*it);
				}
			}

			m_Cell.agents.clear();
		
		}
	}
	else //there are already child sections
	{
		//we directly insert into child sections
		for (int i = 0; i < 4; ++i)
		{
			m_Sections[i]->AddAgent(pAgent);
		}
	}

}

void QuadTree::SubdivideSpace()
{
	float left{ m_Cell.boundingBox.bottomLeft.x };
	float bottom{ m_Cell.boundingBox.bottomLeft.y };
	float width{ m_Cell.boundingBox.width };
	float height{ m_Cell.boundingBox.height };

	Cell bottomLeft{ left, bottom, width / 2 , height / 2 };
	m_Sections[BOTTOMLEFT] = new QuadTree(bottomLeft,m_MaxEntities, m_DepthLevel + 1);

	Cell bottomRight{ left + width / 2, bottom, width / 2 , height / 2 };
	m_Sections[BOTTOMRIGHT] =  new QuadTree(bottomRight,m_MaxEntities, m_DepthLevel + 1);

	Cell topLeft{ left, bottom + height / 2, width / 2 , height / 2 };
	m_Sections[TOPLEFT] =  new QuadTree(topLeft, m_MaxEntities, m_DepthLevel + 1);

	Cell topRight{ left +width / 2, bottom + height / 2, width / 2 , height / 2 };
	m_Sections[TOPRIGHT] = new QuadTree(topRight,m_MaxEntities, m_DepthLevel + 1) ;

	m_IsDivided = true;
}

void QuadTree::Query(SteeringAgent* pAgent, float queryRadius, std::vector<SteeringAgent*>& neighbors, int& nrNeighbors, int& nrIterations)
{

	Vector2 bottomLeft{ pAgent->GetPosition().x - queryRadius,pAgent->GetPosition().y - queryRadius };
	if (!m_Cell.Intersects(bottomLeft, queryRadius*2, queryRadius*2))
		return;

	if (m_IsLeaf)
	{
		for (SteeringAgent* pCurrentAgent : m_Cell.agents)
		{
			++nrIterations;
			if (pCurrentAgent == nullptr || pCurrentAgent == pAgent)
			{
				continue;
			}
			neighbors[nrNeighbors] = pCurrentAgent;
			++nrNeighbors;
		}
	}
	else
	{
		for (int i = 0; i < 4; i++)
		{
			m_Sections[i]->Query(pAgent, queryRadius, neighbors, nrNeighbors,nrIterations);
		}
	}

}

void QuadTree::Render() const
{
	DEBUGRENDERER2D->DrawPolygon(&m_Cell.GetRectPoints()[0], 4, { 1,0,0 }, 0);
	if (!m_IsLeaf)
	{
		for (int i = 0; i < 4; i++)
		{
			m_Sections[i]->Render();
		}
	}
}

void QuadTree::RenderNeighborhoodCells(SteeringAgent* agent, float queryRadius) const
{
	Vector2 bottomLeft{ agent->GetPosition().x - queryRadius,agent->GetPosition().y - queryRadius };
	if (!m_Cell.Intersects(bottomLeft, queryRadius*2, queryRadius*2))
		return;

	if (m_IsLeaf)
	{
		DEBUGRENDERER2D->DrawPolygon(&m_Cell.GetRectPoints()[0], 4, { 0,1,0 }, -1);
	}
	else
	{
		for (int i = 0; i < 4; i++)
		{
			m_Sections[i]->RenderNeighborhoodCells(agent, queryRadius);
		}
	}
	
}

void QuadTree::CleanUp()
{
	if (m_IsDivided)
	{
		for (int i = 0; i < 4; i++)
		{
			if (m_Sections[i] == nullptr)
				continue;

			m_Sections[i]->CleanUp();
			delete m_Sections[i];
		}
		m_IsDivided = false;
	}
	else
	{
		for (size_t i = 0; i < m_Cell.agents.size(); i++)
		{
			m_Cell.agents.pop_back();
		}
	}

	m_IsLeaf = true;
}

