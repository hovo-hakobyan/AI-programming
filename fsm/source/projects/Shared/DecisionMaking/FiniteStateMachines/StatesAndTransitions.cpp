#include "stdafx.h"
#include "StatesAndTransitions.h"

using namespace Elite;
using namespace FSMStates;
using namespace FSMConditions;

#define FOOD_RADIUS 20.0f
#define EVADE_RADIUS 10.0f

#pragma region States

void WanderState::OnEnter(Blackboard* pBlackboard)
{
	AgarioAgent* pAgent;

	if (!pBlackboard->GetData("Agent", pAgent) || !pAgent)
		return;

	pAgent->SetToWander();
}

void SeekFoodState::OnEnter(Blackboard* pBlackboard)
{
	AgarioAgent* pAgent;

	AgarioFood* pNearestFood;
	if (!pBlackboard->GetData("Agent", pAgent) || !pAgent)
		return;

	if (!pBlackboard->GetData("NearestFood", pNearestFood) || !pNearestFood)
		return;

	pAgent->SetToSeek(pNearestFood->GetPosition());
}


void FleeBiggerAgentState::OnEnter(Elite::Blackboard* pBlackboard)
{
	AgarioAgent* pAgent;
	AgarioAgent* pAgentToEvade;

	if (!pBlackboard->GetData("Agent", pAgent) || !pAgent)
		return;

	if (!pBlackboard->GetData("EvadeAgent", pAgentToEvade) || !pAgentToEvade)
		return;

	pAgent->SetToFlee(pAgentToEvade->GetPosition());
}

#pragma endregion States

#pragma region Conditions
bool FoodNearbyCondition::Evaluate(Blackboard* pBlackboard) const
{
	AgarioAgent* pAgent = nullptr;
	std::vector<AgarioFood*>* pFoodVec;

	if (!pBlackboard->GetData("Agent", pAgent) || !pAgent)
		return false;

	if (!pBlackboard->GetData("FoodVec", pFoodVec) || pFoodVec == nullptr)
		return false;

	const float radius{ pAgent->GetRadius() + FOOD_RADIUS};
	Vector2 agentPos{ pAgent->GetPosition() };

	DEBUGRENDERER2D->DrawCircle(agentPos, radius, Color{ 1.f,0.f,0.f,1.f }, DEBUGRENDERER2D->NextDepthSlice());

	//Lambda
	auto isCloser = [agentPos](AgarioFood* pFood1, AgarioFood* pFood2) 
	{
		float dist1 = pFood1->GetPosition().DistanceSquared(agentPos);
		float dist2 = pFood2->GetPosition().DistanceSquared(agentPos);
		return dist1 < dist2;
	};

	auto closestElementIt = std::min_element(pFoodVec->begin(), pFoodVec->end(), isCloser);
	
	if (closestElementIt == pFoodVec->end())
		return false;

	AgarioFood* closestFood = *closestElementIt;
	
	if (closestFood->GetPosition().DistanceSquared(agentPos) <= radius * radius)
	{
		pBlackboard->ChangeData("NearestFood", closestFood);
		return true;
	}
		
	return false;
}



bool FSMConditions::SeekFoodToWanderCondition::Evaluate(Elite::Blackboard* pBlackboard) const
{
	AgarioAgent* pAgent = nullptr;
	std::vector<AgarioFood*>* pFoodVec;
	AgarioFood* closestFood{nullptr};
	pBlackboard->GetData("NearestFood", closestFood);
	if (!pBlackboard->GetData("Agent", pAgent) || !pAgent)
		return false;

	if (!pBlackboard->GetData("FoodVec", pFoodVec) || pFoodVec == nullptr)
		return true;
	
	if (std::find(pFoodVec->begin(), pFoodVec->end(), closestFood) == pFoodVec->end())
	{
		return true;
	}

	return false;
}

bool EvadeAgentCondition::Evaluate(Elite::Blackboard* pBlackboard) const
{
	AgarioAgent* pAgent = nullptr;

	if (!pBlackboard->GetData("Agent", pAgent) || !pAgent)
		return false;

	std::vector<AgarioAgent*>* pAgentVec;

	if (!pBlackboard->GetData("AgentVec", pAgentVec) || !pAgentVec)
		return false;

	AgarioAgent* pEvadeAgent = nullptr;


	const float radius{ pAgent->GetRadius() + EVADE_RADIUS };
	Vector2 agentPos{ pAgent->GetPosition() };

	DEBUGRENDERER2D->DrawCircle(agentPos, radius, Color{ 1.f,1.f,0.f,1.f }, DEBUGRENDERER2D->NextDepthSlice());

	for (auto& pOtherAgent : *pAgentVec)
	{
		if (pOtherAgent->GetPosition().DistanceSquared(agentPos) <radius * radius)
		{
			if (pOtherAgent->GetRadius() > pAgent->GetRadius())
			{
				pEvadeAgent = pOtherAgent;
			}
			
		}
		
	}

	if (!pEvadeAgent)
		return false;

	pBlackboard->ChangeData("EvadeAgent", pEvadeAgent);

	return true;


}

bool EvadeToWanderCondition::Evaluate(Elite::Blackboard* pBlackboard) const
{
	AgarioAgent* pAgent = nullptr;

	if (!pBlackboard->GetData("Agent", pAgent) || !pAgent)
		return false;

	std::vector<AgarioAgent*>* pAgentVec;

	if (!pBlackboard->GetData("AgentVec", pAgentVec) || !pAgentVec)
		return true;

	AgarioAgent* pEvadeAgent = nullptr;


	const float radius{ pAgent->GetRadius() + EVADE_RADIUS };
	Vector2 agentPos{ pAgent->GetPosition() };

	DEBUGRENDERER2D->DrawCircle(agentPos, radius, Color{ 1.f,1.f,0.f,1.f }, DEBUGRENDERER2D->NextDepthSlice());

	float currentMaxRadius{ radius * radius };

	for (auto& pOtherAgent : *pAgentVec)
	{
		if (pOtherAgent->GetPosition().DistanceSquared(agentPos) < radius * radius)
		{
			if (pOtherAgent->GetRadius() > pAgent->GetRadius())
			{
				pEvadeAgent = pOtherAgent;
			}
		}
	}

	if (!pEvadeAgent)
		return true;


	return false;

}
#pragma endregion Conditions


