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

void CellSpace::RegisterNeighbors(SteeringAgent* agent, float queryRadius)
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
