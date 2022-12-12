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

	const Vector2 viewVector{OrientationToVector(pAgent->GetRotation()) };
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
	const Vector2 circleCenter{ pAgent->GetPosition() + pAgent->GetDirection() * m_OffsetDistance };

	m_WanderAngle += float(rand()) / RAND_MAX * m_MaxAngleChange * 2 - m_MaxAngleChange;

	m_Target = circleCenter + Vector2{ cosf(m_WanderAngle), sinf(m_WanderAngle) } *m_Radius;

	SteeringOutput steering{ Seek::CalculateSteering(deltaT, pAgent) };

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawCircle(Vector2(m_Target.Position.x,m_Target.Position.y), 1.0f, {1.0f, 0.0f, 0.0F}, 0.0f);
		DEBUGRENDERER2D->DrawCircle(circleCenter, m_Radius, { 0.0f, 0.0f, 1.0f }, 0.0f);
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

	// newCode for priority steering
	if (vectorToTarget.Magnitude() > m_EvadeRadius)
	{
		steering.IsValid = false;
		return steering;
	}

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
