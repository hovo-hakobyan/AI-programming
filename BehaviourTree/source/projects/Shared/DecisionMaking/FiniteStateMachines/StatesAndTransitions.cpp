#include "stdafx.h"
#include "StatesAndTransitions.h"

using namespace Elite;
using namespace FSMStates;
using namespace FSMConditions;

void WanderState::OnEnter(Blackboard* pBlackboard)
{
	AgarioAgent* pAgent;
	bool isValid = pBlackboard->GetData("Agent",pAgent);

	if (pAgent == nullptr || !isValid)
		return;

	pAgent->SetToWander();
}

void SeekFoodState::OnEnter(Blackboard* pBlackboard)
{
	AgarioAgent* pAgent;
	bool isValid = pBlackboard->GetData("Agent", pAgent);

	if (pAgent == nullptr || !isValid)
		return;

	AgarioFood* nearestFood;
	if (pBlackboard->GetData("NearestFood", nearestFood) || nearestFood == nullptr)
		return;
	pAgent->SetToSeek(nearestFood->GetPosition());
}

bool FoodNearbyCondition::Evaluate(Blackboard* pBlackboard) const
{
	AgarioAgent* pAgent = nullptr;
	std::vector<AgarioFood*>* pFoodVec;

	if (pAgent == nullptr || !pBlackboard->GetData("Agent", pAgent))
		return false;

	if (!pBlackboard->GetData("FoodVec", pFoodVec) || pFoodVec == nullptr)
		return false;

	const float radius{ 10.f };

	Vector2 agentPos = pAgent->GetPosition();

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
	if (closestFood->GetPosition().DistanceSquared(agentPos) < radius * radius)
	{
		pBlackboard->ChangeData("NearestFood", closestFood->GetPosition());
		return true;
	}
		
	return false;
}
