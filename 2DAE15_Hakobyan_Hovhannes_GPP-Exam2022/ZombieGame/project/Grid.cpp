#include "stdafx.h"
#include "Grid.h"
#include "IExamInterface.h"

using namespace Elite;

Grid::Grid(int nrCols, int nrRows, const WorldInfo& worldInfo) :
	m_NrCols{ nrCols },
	m_NrRows{ nrRows }
{
	m_Width = worldInfo.Dimensions.x;
	m_Height = worldInfo.Dimensions.y;

	m_Cells.reserve(m_NrCols * m_NrRows);

	m_CellWidth = m_Width / nrCols;
	m_CellHeight = m_Height / nrRows;

	Elite::Vector2 leftBot{worldInfo.Center.x - m_Width / 2, worldInfo.Center.y - m_Height / 2 };
	InitializeGrid(leftBot);
}

Grid::~Grid()
{
	for (size_t i = 0; i < m_Cells.size(); i++)
	{
		SAFE_DELETE(m_Cells[i]);
	}

	for (size_t i = 0; i < m_Houses.size(); i++)
	{
		SAFE_DELETE(m_Houses[i]);
	}

	m_Cells.clear();
	m_Houses.clear();
}

void Grid::SortHouses(const Elite::Vector2& locToCompare)
{

	auto criteria = [locToCompare](const House* first, const House* second)
	{
		float dx1 = first->center.x - locToCompare.x;
		float dy1 = first->center.y - locToCompare.y;

		float dist1Sqr = dx1 * dx1 + dy1 * dy1;

		float dx2 = second->center.x - locToCompare.x;
		float dy2 = second->center.y - locToCompare.y;

		float dist2Sqr = dx2 * dx2 + dy2 * dy2;

		return dist1Sqr < dist2Sqr;
	};

	std::sort(m_Houses.begin(), m_Houses.end(), criteria);

}


void Grid::Update(float dt, const AgentInfo& agentInfo)
{
	int currentCellIdx{ GetCellIdxFromPos(agentInfo.Position) };

	UpdateCellQueue(currentCellIdx, agentInfo.Position);

	for (size_t i = 0; i < m_Houses.size(); i++)
	{
		m_Houses[i]->Update(dt);
	}

}

void Grid::Render(IExamInterface* pInterface) const
{
	const WorldInfo& wInfo = pInterface->World_GetInfo();

	std::vector<Elite::Vector2> worldPoints
	{
		Elite::Vector2{wInfo.Center.x - wInfo.Dimensions.x / 2, wInfo.Center.y - wInfo.Dimensions.y / 2},
		Elite::Vector2{wInfo.Center.x + wInfo.Dimensions.x / 2, wInfo.Center.y - wInfo.Dimensions.y / 2},
		Elite::Vector2{wInfo.Center.x + wInfo.Dimensions.x / 2, wInfo.Center.y + wInfo.Dimensions.y / 2},
		Elite::Vector2{wInfo.Center.x - wInfo.Dimensions.x / 2, wInfo.Center.y + wInfo.Dimensions.y / 2}
	};

	pInterface->Draw_Polygon(&worldPoints[0], 4, Elite::Vector3{ 1.f,.0f,.0f });

	Elite::Vector3 color{};

	std::vector<Elite::Vector2> poly
	{
		Elite::Vector2{},
		Elite::Vector2{},
		Elite::Vector2{},
		Elite::Vector2{}
	};

	for (size_t i = 0; i < m_Cells.size(); i++)
	{
		poly[0] = m_Cells[i]->center - Elite::Vector2{ m_CellWidth / 2,m_CellHeight / 2 };
		poly[1] = Elite::Vector2{ m_Cells[i]->center.x + m_CellWidth / 2, m_Cells[i]->center.y - m_CellHeight / 2 };
		poly[2] = m_Cells[i]->center + Elite::Vector2{ m_CellWidth / 2,m_CellHeight / 2 };
		poly[3] = Elite::Vector2{ m_Cells[i]->center.x - m_CellWidth / 2, m_Cells[i]->center.y + m_CellHeight / 2 };

		if(m_Cells[i]->isHouse)
		{
			color = Elite::Vector3{ 0.f,1.f,0.f };
		}
		else if (m_Cells[i] == m_Cells[m_CellsToVisitIdx.front()])
		{
			color = Elite::Vector3{ 0.f,1.f,1.f };
		}
		else if(m_Cells[i]->isVisited)
		{
			color = Elite::Vector3{ .5f,.5f,.5f };
		}
		else
		{
			color = m_Cells[i]->isMarkedToVisit ? Elite::Vector3{ 0.f,0.f,1.f } : Elite::Vector3{ 1.f,0.f,0.f };
		}
		
		//Blue = in the queue, should visit
		//Red = Not in the queue, not visited
		//Gray = Visited
		//Cyan if this is the next cell agent should visit
		//Green = house cell
		pInterface->Draw_Polygon(&poly[0], 4, color);

	}

	

}

bool Grid::AreAnyKnownHouses() const
{
	return m_Houses.size() !=0;
}

bool Grid::AreAnyAvailableHouses() const
{
	if (m_Houses.empty())
		return false;

	for (size_t i = 0; i < m_Houses.size(); i++)
	{
		if (m_Houses[i]->isAvailable)
			return true;
	}

	return false;
	
}

Elite::Vector2 Grid::GetNextFreeCellPosition() const
{
	return m_Cells[m_CellsToVisitIdx.front()]->center;
}

void Grid::AddHouse(const Elite::Vector2& houseLoc)
{
	int idx{ GetCellIdxFromPos(houseLoc) };

	//Makes sure we add each house once
	if (!m_Cells[idx]->isHouse)
	{
		m_Cells[idx]->isHouse = true;
		m_Houses.push_back(new House{ houseLoc, idx });
	}
}

bool Grid::IsAgentInHouse(const Elite::Vector2& agentLoc, const House* pHouse)
{
	float errorRadius{ 5.f };

	float dx = agentLoc.x - pHouse->center.x;
	float dy = agentLoc.y - pHouse->center.y;
	float distanceSquared = dx * dx + dy * dy;


	return distanceSquared < errorRadius* errorRadius;
}

House* Grid::GetClosestAvailableHouse(const Elite::Vector2& locToCompare)
{
	if (m_Houses.empty())
		return nullptr;


	//sorts the houses based on distance
	SortHouses(locToCompare);

	////Get the closest house that is available to visit
	for (size_t i = 0; i < m_Houses.size(); i++)
	{
		if (!m_Houses[i]->isAvailable)
			continue;

		return m_Houses[i];
	}

	////If no house is available, return zero vector;
	return nullptr;
}

House* Grid::GetClosestHouse(const Elite::Vector2& locToCompare)
{
	if (m_Houses.empty())
		return nullptr;

	////sorts the houses based on distance
	SortHouses(locToCompare);

	////Returst closest house
	return m_Houses[0];
}

int Grid::GetHouseIndex(const Elite::Vector2& loc)
{
	return GetCellIdxFromPos(loc);
}

void Grid::InitializeGrid(const Elite::Vector2& leftBot)
{
	Elite::Vector2 center{leftBot.x + m_CellWidth / 2, leftBot.y + m_CellHeight / 2};

	for (int row = 0; row < m_NrRows; ++row)
	{
		for (int col = 0; col < m_NrCols; col++)
		{
			int idx{ row * m_NrCols + col };

			m_Cells.push_back(new Cell());

			m_Cells[idx]->xIdx = col;
			m_Cells[idx]->yIdx = row;
			m_Cells[idx]->center = center;

			center.x += m_CellWidth;
		}

		center.x = leftBot.x + m_CellWidth / 2;
		center.y += m_CellHeight;
	}
}

int Grid::GetCellIdxFromPos(const Elite::Vector2& pos)
{
	//Convert pos from center of the world to leftBot
	Elite::Vector2 convertedPos{pos.x + m_Width / 2, pos.y + m_Height / 2};

	int cellIdx_X{ static_cast<int>(convertedPos.x / m_CellWidth) };
	int cellIdx_Y{ static_cast<int>(convertedPos.y / m_CellHeight) };
	int cellIdx = cellIdx_Y * m_NrCols + cellIdx_X;

	return cellIdx;

}

void Grid::UpdateCellQueue(int currentCellIdx, const Elite::Vector2 agentPos)
{

	//If this cell and neighbors are/were in the queue, no point to recheck
	if (!m_Cells[currentCellIdx]->shouldCheckNeighbors && m_Cells[currentCellIdx]->isMarkedToVisit && m_Cells[currentCellIdx]->isVisited)
		return;

	if (!m_CellsToVisitIdx.empty())
	{
		if (!(m_Cells[currentCellIdx] == m_Cells[m_CellsToVisitIdx.front()]))
			return;
		else
		{
			//If this cell is the first element in our queue, we pop it and mark it as visited
			
			if (ShouldCellBeMarkedVisited(agentPos))
			{
				m_Cells[currentCellIdx]->isVisited = true;
				m_CellsToVisitIdx.pop();
			}	
		}
	}
	
	//Mark the cell to visit and add it to the queue
	if (!m_Cells[currentCellIdx]->isMarkedToVisit)
	{
		m_CellsToVisitIdx.push(currentCellIdx);
		m_Cells[currentCellIdx]->isMarkedToVisit = true;
	}
	

	//Check neigbhors
	int neighborIdx{};
	for (auto neighbor : { ECellNeighbor::NORTH,ECellNeighbor::EAST,ECellNeighbor::SOUTH,ECellNeighbor::WEST })
	{
		//If this neigbhor is marked or visited, go to next neigbhor
		if (IsNeighborValidToBeAdded(neighbor, currentCellIdx, neighborIdx))
			continue;

		//Add this neighbor to the queue and mark 
		m_CellsToVisitIdx.push(neighborIdx);
		m_Cells[neighborIdx]->isMarkedToVisit = true;
	}

	//We don't want to check wheter the neigbhors are visited more than 1 time
	m_Cells[currentCellIdx]->shouldCheckNeighbors = false;
	
}

bool Grid::IsNeighborValidToBeAdded(const ECellNeighbor& cellNeighbor, int currentIdx, int& neigbhorIdx)
{
	int x{}, y{};

	switch (cellNeighbor)
	{
	case ECellNeighbor::NORTH:
		x = m_Cells[currentIdx]->xIdx;
		y = m_Cells[currentIdx]->yIdx + 1;
		break;
	case ECellNeighbor::EAST:
		x = m_Cells[currentIdx]->xIdx + 1;
		y = m_Cells[currentIdx]->yIdx;
		break;
	case ECellNeighbor::SOUTH:
		x = m_Cells[currentIdx]->xIdx;
		y = m_Cells[currentIdx]->yIdx - 1;
		break;
	case ECellNeighbor::WEST:
		x = m_Cells[currentIdx]->xIdx - 1;
		y = m_Cells[currentIdx]->yIdx;
		break;
	}

	neigbhorIdx = y * m_NrCols + x;;
	return m_Cells[neigbhorIdx]->isMarkedToVisit;
}

bool Grid::ShouldCellBeMarkedVisited(const Elite::Vector2 agentPos)
{
	float errorRadius{ 3.f };

	float dx = agentPos.x - m_Cells[m_CellsToVisitIdx.front()]->center.x;
	float dy = agentPos.y - m_Cells[m_CellsToVisitIdx.front()]->center.y;
	float distanceSquared = dx * dx + dy * dy;

	return distanceSquared < errorRadius * errorRadius;
}

