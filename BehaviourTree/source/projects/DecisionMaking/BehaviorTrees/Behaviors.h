/*=============================================================================*/
// Copyright 2020-2021 Elite Engine
/*=============================================================================*/
// Behaviors.h: Implementation of certain reusable behaviors for the BT version of the Agario Game
/*=============================================================================*/
#ifndef ELITE_APPLICATION_BEHAVIOR_TREE_BEHAVIORS
#define ELITE_APPLICATION_BEHAVIOR_TREE_BEHAVIORS
//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "framework/EliteMath/EMath.h"
#include "framework/EliteAI/EliteDecisionMaking/EliteBehaviorTree/EBehaviorTree.h"
#include "projects/Shared/Agario/AgarioAgent.h"
#include "projects/Shared/Agario/AgarioFood.h"
#include "projects/Movement/SteeringBehaviors/Steering/SteeringBehaviors.h"

//-----------------------------------------------------------------
// Behaviors
//-----------------------------------------------------------------

#define SEEK_RADIUS 20.0f

namespace BT_Actions
{
	Elite::BehaviorState ChangeToWander(Elite::Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent;

		if (!pBlackboard->GetData("Agent", pAgent) || !pAgent)
			return Elite::BehaviorState::Failure;

		pAgent->SetToWander();

		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState ChangeToSeekFood(Elite::Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent;
		Elite::Vector2 targetPos;

		if (!pBlackboard->GetData("Agent", pAgent) || !pAgent)
			return Elite::BehaviorState::Failure;

		if (!pBlackboard->GetData("Target", targetPos))
			return Elite::BehaviorState::Failure;

		pAgent->SetToSeek(targetPos);

		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState ChangeToFlee(Elite::Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent;
		AgarioAgent* pAgentToFlee;

		if (!pBlackboard->GetData("Agent", pAgent) || !pAgent)
			return Elite::BehaviorState::Failure;

		if (!pBlackboard->GetData("AgentFleeTarget", pAgentToFlee))
			return Elite::BehaviorState::Failure;

		pAgent->SetToFlee(pAgentToFlee->GetPosition());

		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState ChangeToChase(Elite::Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent;
		AgarioAgent* pAgentToChase;

		if (!pBlackboard->GetData("Agent", pAgent) || !pAgent)
			return Elite::BehaviorState::Failure;

		if (!pBlackboard->GetData("AgentChaseTarget", pAgentToChase))
			return Elite::BehaviorState::Failure;

		pAgent->SetToSeek(pAgentToChase->GetPosition());

		return Elite::BehaviorState::Success;
	}

}

//-----------------------------------------------------------------
// Conditions
//-----------------------------------------------------------------

namespace BT_Conditions
{
	bool IsFoodNearby(Elite::Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent;
		std::vector<AgarioFood*>* pFoodVec;

		if (!pBlackboard->GetData("Agent", pAgent) || !pAgent)
			return false;

		if (!pBlackboard->GetData("FoodVec", pFoodVec) || !pFoodVec)
			return false;

		if (pFoodVec->empty())
			return false;

		
		const float searchRadius{ pAgent->GetRadius() + SEEK_RADIUS };
		Elite::Vector2 agentPos{ pAgent->GetPosition() };

		DEBUGRENDERER2D->DrawCircle(agentPos, searchRadius, { 1.f,0.f,0.f,1.f }, DEBUGRENDERER2D->NextDepthSlice());

		float closestDistSqr{searchRadius * searchRadius};
		AgarioFood* pClosestFood{ nullptr };

		for (auto& pFood : *pFoodVec)
		{
			float distSqr = pFood->GetPosition().DistanceSquared(agentPos);

			if (distSqr < closestDistSqr)
			{
				closestDistSqr = distSqr;
				pClosestFood = pFood;
			}
		}

		if (pClosestFood)
		{
			pBlackboard->ChangeData("Target", pClosestFood->GetPosition());
			return true;
		}
		
		return false;

	}

	bool IsBiggerAgentNearby(Elite::Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent;

		if (!pBlackboard->GetData("Agent", pAgent) || !pAgent)
			return false;

		std::vector<AgarioAgent*>* pEnemiesVec;

		if (!pBlackboard->GetData("AgentsVec", pEnemiesVec) || !pEnemiesVec)
			return false;

		const float agentRadius = pAgent->GetRadius();
		const float fleeRadius = agentRadius + 10.f;
		Elite::Vector2 agentPos{ pAgent->GetPosition() };

		AgarioAgent* pTargetToFlee = nullptr;
		float currentFleeRad = fleeRadius * fleeRadius;


		for (const auto& pEnemy : *pEnemiesVec)
		{
			const float distanceSqr{ pEnemy->GetPosition().DistanceSquared(agentPos) };
			if (distanceSqr > currentFleeRad)
				continue;

			if (pEnemy->GetRadius() < agentRadius)
				continue;

			currentFleeRad = distanceSqr;
			pTargetToFlee = pEnemy;

		}

		if (!pTargetToFlee)
			return false;

		DEBUGRENDERER2D->DrawSegment(agentPos, pTargetToFlee->GetPosition(), { 1.f,0.f,0.f,1.f }, DEBUGRENDERER2D->NextDepthSlice());
		pBlackboard->ChangeData("AgentFleeTarget", pTargetToFlee);
		return true;
	}

	bool IsSmallerAgentNearby(Elite::Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent;

		if (!pBlackboard->GetData("Agent", pAgent) || !pAgent)
			return false;

		std::vector<AgarioAgent*>* pEnemiesVec;

		if (!pBlackboard->GetData("AgentsVec", pEnemiesVec) || !pEnemiesVec)
			return false;

		const float agentRadius = pAgent->GetRadius();
		const float chaseRadius = agentRadius + SEEK_RADIUS;
		Elite::Vector2 agentPos{ pAgent->GetPosition() };

		AgarioAgent* pTargetToChase = nullptr;
		float currentChaseRad = chaseRadius * chaseRadius;


		for (const auto& pEnemy : *pEnemiesVec)
		{
			const float distanceSqr{ pEnemy->GetPosition().DistanceSquared(agentPos) };
			if (distanceSqr > currentChaseRad)
				continue;

			if (pEnemy->GetRadius() + 5.f >= agentRadius)
				continue;

			currentChaseRad = distanceSqr;
			pTargetToChase = pEnemy;

		}

		if (!pTargetToChase)
			return false;

		DEBUGRENDERER2D->DrawSegment(agentPos, pTargetToChase->GetPosition(), { 0.f,1.f,0.f,1.f }, DEBUGRENDERER2D->NextDepthSlice());
		pBlackboard->ChangeData("AgentChaseTarget", pTargetToChase);
		return true;
	}
}








#endif