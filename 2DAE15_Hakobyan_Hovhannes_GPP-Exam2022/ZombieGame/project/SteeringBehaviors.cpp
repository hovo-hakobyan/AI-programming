//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "SteeringBehaviors.h"


using namespace Elite; 
//SEEK
//****
SteeringPlugin_Output Seek::CalculateSteering(float deltaT,  AgentInfo_Extended& agentInfo)
{
	
	SteeringPlugin_Output steering = {};
	
	steering.LinearVelocity = m_Target.Position - agentInfo.Position;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= agentInfo.MaxLinearSpeed;
	steering.AutoOrient = true;
	steering.RunMode = agentInfo.RunMode;

	return steering;
}

SteeringPlugin_Output Face::CalculateSteering(float deltaT,  AgentInfo_Extended& agentInfo)
{
	SteeringPlugin_Output steering{};
	steering.LinearVelocity = { 0.f,0.f };

	const Vector2 viewVector{OrientationToVector(agentInfo.Orientation) };
	const Vector2 finalVector{ m_Target.Position - agentInfo.Position };

	float angleBetween{ AngleBetween(viewVector,finalVector) };
	
	
	if (abs(angleBetween) < 0.1f)
	{
		steering.AngularVelocity = 0.0f;
		steering.AutoOrient = true;
		agentInfo.isDoneFacing = true;
		
	}
	else
	{
		agentInfo.isDoneFacing = false;
		if (angleBetween > 0.0f)
		{
			steering.AngularVelocity = agentInfo.MaxAngularSpeed;
		}
		else
		{
			steering.AngularVelocity = -agentInfo.MaxAngularSpeed;
		}
	}

	steering.AutoOrient = false;
	return steering;
}

