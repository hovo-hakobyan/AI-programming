#include "stdafx.h"
#include "SandboxAgent.h"

using namespace Elite;

SandboxAgent::SandboxAgent(): BaseAgent()
{
	m_Target = GetPosition();
}

void SandboxAgent::Update(float dt)
{
	const float maxSpeed{ 50.0f };
	const float arrivalRadius{ 1.f };
	const float slowRadius{ 15.0f };

	Vector2 toTarget{ m_Target - GetPosition() };
	const float distance = toTarget.Magnitude();
	if (distance < arrivalRadius)
	{
		SetLinearVelocity({ 0.f,0.f });
		return;
	}

	toTarget.Normalize();
	if (distance < slowRadius)
	{
		toTarget *= maxSpeed * (distance / slowRadius);
		std::cout << toTarget.Magnitude() << std::endl;
	}
	else
	{
		toTarget *= maxSpeed;
	}
	
	DEBUGRENDERER2D->DrawDirection(GetPosition(), toTarget, 5, { 1,0,0 });
	SetLinearVelocity(toTarget);


	//Orientation
	AutoOrient();
}

void SandboxAgent::Render(float dt)
{
	BaseAgent::Render(dt); //Default Agent Rendering
}

void SandboxAgent::AutoOrient()
{
	//Determine angle based on direction
	Vector2 velocity = GetLinearVelocity();
	if (velocity.Magnitude() > 0)
	{
		velocity.Normalize();
		SetRotation(atan2(velocity.y, velocity.x) + E_PI_2);
	}

	SetRotation(GetRotation() + E_PI_2);
}