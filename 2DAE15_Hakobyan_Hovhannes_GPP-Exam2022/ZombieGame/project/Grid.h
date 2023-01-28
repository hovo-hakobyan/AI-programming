#pragma once

#include <vector>
#include <queue>

#include "Exam_HelperStructs.h"
#include "HelperEnums.h"
class IExamInterface;

struct Cell final
{
	int xIdx;
	int yIdx;

	Elite::Vector2 center;

	bool isMarkedToVisit = false;
	bool shouldCheckNeighbors = true;
	bool isVisited = false;

	bool isHouse = false; 
};

struct House final
{
	House()
	{
		idx = -1;
		center = Elite::ZeroVector2;
		cooldown = 240.f;
		currentCooldown = 0.f;
	}

	House(const Elite::Vector2& _center, int _idx)
	{
		idx = _idx;
		center = _center;
		cooldown = 240.f;
		currentCooldown = 0.f;
	}

	House(const Elite::Vector2& _center, int _idx, float _cooldown)
	{
		idx = _idx;
		center = _center;
		cooldown = _cooldown;
		currentCooldown = 0.f;
	}

	void Update(float dt)
	{
		if (isAvailable)
			return;

		currentCooldown += dt;

		if (currentCooldown < cooldown)
			return;

		isAvailable = true;
		currentCooldown = 0.f;

	}

	int idx;
	Elite::Vector2 center;
	float cooldown;
	float currentCooldown;
	bool isAvailable = true;

};

class Grid final
{
public:
	Grid(int nrCols, int nrRows, const WorldInfo& worldInfo);

	~Grid();
	Grid(const Grid& other) = delete;
	Grid& operator=(const Grid& other) = delete;
	Grid(Grid&& other) = delete;
	Grid& operator =(Grid&& other) = delete;

	void Update(float dt, const AgentInfo& agentInfo);
	void Render(IExamInterface* pInterface) const;


	void AddHouse(const Elite::Vector2& houseLoc);
	bool IsAgentInHouse(const Elite::Vector2& agentLoc, const House* pHouse);

	bool AreAnyKnownHouses()const;
	bool AreAnyAvailableHouses()const;
	Elite::Vector2 GetNextFreeCellPosition() const;
	House* GetClosestAvailableHouse(const Elite::Vector2& locToCompare);	//Sorts m_Houses first
	House* GetClosestHouse(const Elite::Vector2& locToCompare);				//Sorts m_Houses first
	int GetHouseIndex(const Elite::Vector2& loc);

private:
	int m_NrCols;
	int m_NrRows;

	float m_Width;
	float m_Height;

	float m_CellWidth;
	float m_CellHeight;

	std::vector<Cell*> m_Cells;
	std::queue<int> m_CellsToVisitIdx;

	std::vector<House*> m_Houses;
	

	void InitializeGrid(const Elite::Vector2& leftBot);
	int GetCellIdxFromPos(const Elite::Vector2& pos);

	void UpdateCellQueue(int currentCellIdx, const Elite::Vector2 agentPos);

	void SortHouses(const Elite::Vector2& locToCompare);
	bool IsNeighborValidToBeAdded(const ECellNeighbor& cellNeighbor, int currentIdx, int& neigbhorIdx);
	bool ShouldCellBeMarkedVisited(const Elite::Vector2 agentPos);

};

