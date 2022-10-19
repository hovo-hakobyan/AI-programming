//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "SteeringBehaviors.h"
#include "../SteeringAgent.h"
#include "../Obstacle.h"
#include "framework\EliteMath\EMatrix2x3.h"

using namespace Elite;
//SEEK
//****
SteeringOutput Seek::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};
	Vector2 desiredVelocity{ m_Target.Position - pAgent->GetPosition() };
	
	steering.LinearVelocity = desiredVelocity - pAgent->GetLinearVelocity();
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, { 0,1,0 });
	}

	return steering;
}

SteeringOutput Flee::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};
	Vector2 desiredVelocity{ -(m_Target.Position - pAgent->GetPosition()) };
	
	steering.LinearVelocity = desiredVelocity - pAgent->GetLinearVelocity();
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();
	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, { 0,1,0 });
	}
	return steering;
}

SteeringOutput Arrive::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};
	const float slowRadius{15.0f};
	const float stopRadius{4.0f};
	steering.LinearVelocity = m_Target.Position - pAgent->GetPosition();
	float distance = steering.LinearVelocity.Magnitude();
	steering.LinearVelocity.Normalize();
	if (distance < stopRadius)
	{
		steering.LinearVelocity = { 0.f,0.f };
		
	}
	else if (distance < slowRadius)
	{
		steering.LinearVelocity *= pAgent->GetMaxLinearSpeed() * (distance / slowRadius);
	}
	else
	{
		steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();
	}
	

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawCircle(pAgent->GetPosition(), slowRadius, { 1,0,0 }, 0);
		DEBUGRENDERER2D->DrawCircle(pAgent->GetPosition(), stopRadius, { 0,0,1 }, 0);
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, steering.LinearVelocity.Magnitude(), {0,1,0});
	}
	return steering;
}

SteeringOutput Face::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};
	steering.LinearVelocity = { 0.f,0.f };
	pAgent->SetAutoOrient(false);

	const Vector2 viewVector{OrientationToVector(pAgent->GetRotation() + ToRadians(90.0f)) };
	const Vector2 finalVector{ m_Target.Position - pAgent->GetPosition() };

	float angleBetween{ AngleBetween(viewVector,finalVector) };
	
	
	if (abs(angleBetween) < 0.1f)
	{
		steering.AngularVelocity = 0.0f;
	}
	else
	{
		if (angleBetween > 0.0f)
		{
			steering.AngularVelocity = pAgent->GetMaxAngularSpeed();
		}
		else
		{
			steering.AngularVelocity = -pAgent->GetMaxAngularSpeed();
		}
	}

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(),viewVector, 5, { 0,1,0 });
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), finalVector, 5, { 1,0,0 });
	}

	return steering;
}

SteeringOutput Wander::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering;
	
	float charRotation{ pAgent->GetRotation() +ToRadians(90.0f) };
	Vector2 dir{ OrientationToVector(charRotation)};
	Vector2 agentPos{ pAgent->GetPosition() };
	Vector2 circleCenter{agentPos + dir * m_OffsetDistance };
	Vector2 agentToCircle{ circleCenter - agentPos };
	m_WanderAngle += randomFloat(-m_MaxAngleChange, m_MaxAngleChange);
	Vector2 angleVector{OrientationToVector(charRotation+ m_WanderAngle) };
	angleVector.Normalize();
	Vector2 pointOnCircle{ circleCenter + angleVector * m_Radius };
	Vector2 newVelocity{ pointOnCircle - agentPos };

	steering.LinearVelocity = newVelocity;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();


	if (pAgent->CanRenderBehavior())
	{
		
		DEBUGRENDERER2D->DrawCircle(circleCenter, m_Radius, { 1,0,0 }, 0);
		DEBUGRENDERER2D->DrawCircle(circleCenter, 0.3, { 1,0,0 }, 0);
		DEBUGRENDERER2D->DrawCircle(pointOnCircle, 0.7, { 0,1,0 }, 0);
		DEBUGRENDERER2D->DrawDirection(agentPos, agentToCircle, agentToCircle.Magnitude(), {0,0,0});
		DEBUGRENDERER2D->DrawDirection(agentPos, newVelocity, newVelocity.Magnitude(), {0,1,0});
	}

	return steering;
}

SteeringOutput Pursuit::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};
	
	Vector2 targetPoint{ m_Target.Position };
	Vector2 vectorToTarget{ targetPoint - pAgent->GetPosition() };
	Vector2 targetVel{ m_Target.LinearVelocity };

	if (vectorToTarget.Magnitude() >= targetVel.Magnitude())
	{
		targetPoint += targetVel;
	}
	else
	{
		if (vectorToTarget.Magnitude()>0.0f)
		{
			//the closer we get to the target, the less distance we have to predict
			targetPoint += targetVel * vectorToTarget / targetVel.Magnitude();
		}
		
	}

	steering.LinearVelocity = targetPoint - pAgent->GetPosition();
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawPoint(targetPoint,4,{0.2f,0.6f,0.2f});
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, steering.LinearVelocity.Magnitude(), { 1,0,0 });
	}

	return steering;
}

SteeringOutput Evade::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};

	Vector2 targetPoint{ m_Target.Position };
	Vector2 vectorToTarget{ targetPoint - pAgent->GetPosition() };
	Vector2 targetVel{ m_Target.LinearVelocity };

	if (vectorToTarget.Magnitude() >= targetVel.Magnitude())
	{
		targetPoint += targetVel;
	}
	else
	{
		if (vectorToTarget.Magnitude() > 0.0f)
		{
			//the closer we get to the target, the less distance we have to predict
			targetPoint += targetVel * vectorToTarget / targetVel.Magnitude();
		}

	}
	steering.LinearVelocity = -(targetPoint - pAgent->GetPosition());
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	return steering;
}
