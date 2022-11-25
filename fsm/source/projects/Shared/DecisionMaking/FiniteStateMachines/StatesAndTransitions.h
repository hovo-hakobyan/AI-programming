/*=============================================================================*/
// Copyright 2020-2021 Elite Engine
/*=============================================================================*/
// StatesAndTransitions.h: Implementation of the state/transition classes
/*=============================================================================*/
#ifndef ELITE_APPLICATION_FSM_STATES_TRANSITIONS
#define ELITE_APPLICATION_FSM_STATES_TRANSITIONS

#include "projects/Shared/Agario/AgarioAgent.h"
#include "projects/Shared/Agario/AgarioFood.h"
#include "projects/Movement/SteeringBehaviors/Steering/SteeringBehaviors.h"
#include "framework/EliteAI/EliteData/EBlackboard.h"

//------------
//---STATES---
//------------

namespace FSMStates
{
	class WanderState : public Elite::FSMState
	{
	public:
		WanderState() : FSMState() {};
		virtual void OnEnter(Elite::Blackboard* pBlackboard) override;
	};

	class SeekFoodState : public Elite::FSMState
	{
	public:
		SeekFoodState() : FSMState() {};
		virtual void OnEnter(Elite::Blackboard* pBlackboard) override;
	};

	class FleeBiggerAgentState : public Elite::FSMState
	{
	public:
		FleeBiggerAgentState() : FSMState() {};
		virtual void OnEnter(Elite::Blackboard* pBlackboard) override;
	};
}

//-----------------
//---TRANSITIONS---
//-----------------

namespace FSMConditions
{
	class FoodNearbyCondition : public Elite::FSMCondition
	{
	public:
		FoodNearbyCondition() : FSMCondition() {};
		
		// Inherited via FSMCondition
		virtual bool Evaluate(Elite::Blackboard* pBlackboard) const override;

	};

	class SeekFoodToWanderCondition : public Elite::FSMCondition
	{
	public:
		SeekFoodToWanderCondition() : FSMCondition() {};

		// Inherited via FSMCondition
		virtual bool Evaluate(Elite::Blackboard* pBlackboard) const override;

	};

	class EvadeAgentCondition : public Elite::FSMCondition
	{
	public:
		EvadeAgentCondition() : FSMCondition() {};

		// Inherited via FSMCondition
		virtual bool Evaluate(Elite::Blackboard* pBlackboard) const override;

	};

	class EvadeToWanderCondition : public Elite::FSMCondition
	{
	public:
		EvadeToWanderCondition() : FSMCondition() {};

		// Inherited via FSMCondition
		virtual bool Evaluate(Elite::Blackboard* pBlackboard) const override;

	};
}

#endif