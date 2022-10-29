/*=============================================================================*/
// Copyright 2019-2020
// Authors: Yosha Vandaele
/*=============================================================================*/
// SpacePartitioning.h: Contains Cell and Cellspace which are used to partition a space in segments.
// Cells contain pointers to all the agents within.
// These are used to avoid unnecessary distance comparisons to agents that are far away.

// Heavily based on chapter 3 of "Programming Game AI by Example" - Mat Buckland
/*=============================================================================*/

#pragma once
#include <list>
#include <vector>
#include <map>
#include <iterator>
#include "framework\EliteMath\EVector2.h"
#include "framework\EliteGeometry\EGeometry2DTypes.h"

class SteeringAgent;

// --- Cell ---
// ------------
struct Cell
{
	Cell(float left, float bottom, float width, float height);

	std::vector<Elite::Vector2> GetRectPoints() const;
	bool Contains(SteeringAgent* pAgent) const;
	
	// all the agents currently in this cell
	std::list<SteeringAgent*> agents;
	Elite::Rect boundingBox;

	bool Intersects(Elite::Vector2 leftBottom, float width, float height) const;
};

// --- Partitioned Space ---
// -------------------------
class CellSpace
{
public:
	CellSpace(float width, float height, int rows, int cols, int maxEntities);

	void AddAgent(SteeringAgent* agent);
	void UpdateAgentCell(SteeringAgent* agent, Elite::Vector2 oldPos);

	void RegisterNeighbors(SteeringAgent* agent, float queryRadius, int& nrIterations);
	const std::vector<SteeringAgent*>& GetNeighbors() const { return m_Neighbors; }
	int GetNrOfNeighbors() const { return m_NrOfNeighbors; }

	//empties the cells of entities
	void EmptyCells();

	void RenderCells() const;
	void RenderNeighborhoodCells(SteeringAgent* agent, float queryRadius) const;

private:
	// Cells and properties
	std::vector<Cell> m_Cells;

	float m_SpaceWidth;
	float m_SpaceHeight;

	int m_NrOfRows;
	int m_NrOfCols;

	float m_CellWidth;
	float m_CellHeight;

	// Members to avoid memory allocation on every frame
	std::vector<SteeringAgent*> m_Neighbors;
	int m_NrOfNeighbors;


	// Helper functions
	int PositionToIndex(const Elite::Vector2 pos) const;
	void InitCells();
	void GetNeighborhoodCells(const Elite::Vector2& centerPos, const float queryRadius, int& startRowIdx, int& endRowIdx, int& startColIdx, int& endColIdx);
	
};

class QuadTree
{
public:


	QuadTree(Cell boundingCell, int maxEntities, int depth);
	~QuadTree() = default;

	void AddAgent(SteeringAgent* pAgent);
	void Query(SteeringAgent* agent, float queryRadius, std::vector<SteeringAgent*>& neighbors, int& nrNeighbors, int& nrIterations);

	void Render() const;
	void RenderNeighborhoodCells(SteeringAgent* agent, float queryRadius) const;

	void CleanUp();
private:
	Cell m_Cell;

	int m_MaxEntities; //amount of entities per section. Used to subdevide
	int m_DepthLevel;

	bool m_IsDivided; //We want to subdivide this only once
	bool m_IsLeaf;	//True means this is the final depth level

	std::vector<QuadTree*> m_Sections;

	const static int s_MaxDepth = 10;

	//Methods
	void SubdivideSpace();
	
};