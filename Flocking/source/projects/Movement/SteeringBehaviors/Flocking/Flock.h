#pragma once
#include "../SteeringHelpers.h"
#include "FlockingSteeringBehaviors.h"

class ISteeringBehavior;
class SteeringAgent;
class BlendedSteering;
class PrioritySteering;
class CellSpace;
class QuadTree;

class Flock final
{
public:
	Flock(
		int flockSize = 50, 
		float worldSize = 100.f, 
		SteeringAgent* pAgentToEvade = nullptr, 
		bool trimWorld = false);

	~Flock();

	void Update(float deltaT);
	void UpdateAndRenderUI() ;
	void Render(float deltaT) const;

	void RegisterNeighbors(SteeringAgent* pAgent);
	int GetNrOfNeighbors() const { return m_NrOfNeighbors; }
	const std::vector<SteeringAgent*>& GetNeighbors() const { return m_Neighbors; }
	float GetWorldTrimSize() const { return m_WorldSize; };

	Elite::Vector2 GetAverageNeighborPos() const;
	Elite::Vector2 GetAverageNeighborVelocity() const;

	void SetTarget_Seek(TargetData target);
	void SetWorldTrimSize(float size) { m_WorldSize = size; }

private:
	//Datamembers
	int m_FlockSize = 0;
	int m_NrIterations = 0;
	std::vector<SteeringAgent*> m_Agents;
	std::vector<SteeringAgent*> m_Neighbors;

	bool m_TrimWorld = false;
	float m_WorldSize = 0.f;
	bool m_CanDebugRender = false;
	bool m_DebugNeighborhood = false;
	bool m_UsePartitioning = false;
	bool m_UseQuadTree = true;

	float m_NeighborhoodRadius = 1.f;
	int m_NrOfNeighbors = 0;
	float m_EvadeRadius = 30.0f;

	SteeringAgent* m_pAgentToEvade = nullptr;
	
	//Steering Behaviors
	Seek* m_pSeekBehavior = nullptr;
	Separation* m_pSeparationBehavior = nullptr;
	Cohesion* m_pCohesionBehavior = nullptr;
	VelocityMatch* m_pVelMatchBehavior = nullptr;
	Wander* m_pWanderBehavior = nullptr;
	Evade* m_pEvadeBehavior = nullptr;

	BlendedSteering* m_pBlendedSteering = nullptr;
	PrioritySteering* m_pPrioritySteering = nullptr;

	float* GetWeight(ISteeringBehavior* pBehaviour);

	//SpatialPartitioning
	CellSpace* m_pCellSpace;

	//QuadTree
	QuadTree* m_pQuadTree;
	
	void RenderBoundingBox(const Elite::Vector2& center) const;
private:
	Flock(const Flock& other);
	Flock& operator=(const Flock& other);
};