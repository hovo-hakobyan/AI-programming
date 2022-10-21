#include "stdafx.h"
#include "FlockingSteeringBehaviors.h"
#include "Flock.h"
#include "../SteeringAgent.h"
#include "../SteeringHelpers.h"

using namespace Elite;
//*******************
//COHESION (FLOCKING)
SteeringOutput Cohesion::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	if (m_pFlock->GetNrOfNeighbors() >0)
	{
		m_Target = m_pFlock->GetAverageNeighborPos();
	}
	
	return Seek::CalculateSteering(deltaT,pAgent);
}

//*********************
//SEPARATION (FLOCKING)
SteeringOutput Separation::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	Vector2 target{};
	Vector2 result{};
	int nrNeighbors{ m_pFlock->GetNrOfNeighbors() };
	if (nrNeighbors>0)
	{
		for (int i = 0; i < nrNeighbors; i++)
		{
			target = (m_pFlock->GetNeighbors()[i]->GetPosition() - pAgent->GetPosition());
			result += target.GetNormalized() / target.Magnitude();
		}
		result *= -1;
		result.Normalize();
		result *= pAgent->GetMaxLinearSpeed();
		m_Target = pAgent->GetPosition() + result;
		
	}
	return Seek::CalculateSteering(deltaT,pAgent);
	
}

//*************************
//VELOCITY MATCH (FLOCKING)
SteeringOutput VelocityMatch::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};
	steering.LinearVelocity = m_pFlock->GetAverageNeighborVelocity();
	steering.LinearVelocity *= m_pFlock->GetAverageNeighborVelocity().Magnitude();
	return steering;
}