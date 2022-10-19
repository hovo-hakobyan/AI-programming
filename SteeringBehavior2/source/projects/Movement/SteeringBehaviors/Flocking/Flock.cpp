#include "stdafx.h"
#include "Flock.h"

#include "../SteeringAgent.h"
#include "../Steering/SteeringBehaviors.h"
#include "../CombinedSteering/CombinedSteeringBehaviors.h"

using namespace Elite;

//Constructor & Destructor
Flock::Flock(
	int flockSize /*= 50*/, 
	float worldSize /*= 100.f*/, 
	SteeringAgent* pAgentToEvade /*= nullptr*/, 
	bool trimWorld /*= false*/)

	
	: m_WorldSize{ worldSize }
	, m_FlockSize{ flockSize }
	, m_TrimWorld { trimWorld }
	, m_pAgentToEvade{pAgentToEvade}
	, m_NeighborhoodRadius{ 15 }
	, m_NrOfNeighbors{0}
{
	m_Agents.resize(m_FlockSize);
	m_Neighbors.resize(m_FlockSize);

	m_pSeekBehavior = new Seek();
	m_pWanderBehavior = new Wander();
	m_pCohesionBehavior = new Cohesion(this);
	m_pVelMatchBehavior = new VelocityMatch(this);
	m_pSeparationBehavior = new Separation(this);
	m_pEvadeBehavior = new Evade();
	m_pEvadeBehavior->SetEvadeRadius(m_EvadeRadius);

	m_pAgentToEvade = new SteeringAgent();
	m_pAgentToEvade->SetSteeringBehavior(m_pSeekBehavior);
	m_pAgentToEvade->SetMaxLinearSpeed(150.0f);
	m_pAgentToEvade->SetAutoOrient(true);
	m_pAgentToEvade->SetPosition(randomVector2(m_WorldSize));
	m_pAgentToEvade->SetBodyColor({ 1.0f,0.0f,0.0f });

	

	m_pBlendedSteering = new BlendedSteering({ { m_pCohesionBehavior,0.0f }, {m_pSeekBehavior,0.0f}, {m_pVelMatchBehavior, 0.0f}, {m_pWanderBehavior,0.5f}, {m_pSeparationBehavior,0.5f} });
	m_pPrioritySteering = new PrioritySteering( {m_pEvadeBehavior,m_pBlendedSteering} );

	

	for (int i = 0; i < m_FlockSize; i++)
	{
		m_Agents[i] = new SteeringAgent();
		m_Agents[i]->SetSteeringBehavior(m_pPrioritySteering);
		m_Agents[i]->SetMaxLinearSpeed(randomFloat(20.0f,50.0f));
		m_Agents[i]->SetAutoOrient(true);
		m_Agents[i]->SetPosition(randomVector2(m_WorldSize));
		
	}
	
	
}

Flock::~Flock()
{
	

	SAFE_DELETE(m_pBlendedSteering);
	SAFE_DELETE(m_pPrioritySteering);
	SAFE_DELETE(m_pSeekBehavior);
	SAFE_DELETE(m_pCohesionBehavior);
	SAFE_DELETE(m_pVelMatchBehavior);
	SAFE_DELETE(m_pWanderBehavior);
	SAFE_DELETE(m_pPrioritySteering);
	SAFE_DELETE(m_pSeparationBehavior);
	SAFE_DELETE(m_pEvadeBehavior);
	SAFE_DELETE(m_pAgentToEvade);

	for(auto pAgent: m_Agents)
	{
		SAFE_DELETE(pAgent);
	}
	m_Agents.clear();
}

void Flock::Update(float deltaT)
{
	for (SteeringAgent* pAgent : m_Agents)
	{
		if (pAgent == nullptr)
		{
			continue;
		}

		RegisterNeighbors(pAgent);
		pAgent->SetRenderBehavior(m_CanDebugRender);
		pAgent->Update(deltaT);
		if (m_TrimWorld)
		{
			pAgent->TrimToWorld(m_WorldSize);
		}
		if (pAgent == m_Agents[0])
		{
			if (m_DebugNeighborhood)
			{
				for (int i = 0; i < m_NrOfNeighbors; i++)
				{
					DEBUGRENDERER2D->DrawSolidCircle(m_Neighbors[i]->GetPosition(), 2, { 1,0 }, { 0,0,1 },-1);
				}
				DEBUGRENDERER2D->DrawCircle(m_Agents[0]->GetPosition(), m_NeighborhoodRadius, { 1,1,1 }, 0);
			}
			
		}
	}
	if (m_TrimWorld)
	{
		m_pAgentToEvade->TrimToWorld(m_WorldSize);
	}

	TargetData evadeTarget{};
	evadeTarget.LinearVelocity = m_pAgentToEvade->GetLinearVelocity();
	evadeTarget.Position = m_pAgentToEvade->GetPosition();
	m_pEvadeBehavior->SetTarget(evadeTarget);
	m_pAgentToEvade->Update(deltaT);

	// TODO: update the flock
	// loop over all the agents
		// register its neighbors	(-> memory pool is filled with neighbors of the currently evaluated agent)
		// update it				(-> the behaviors can use the neighbors stored in the pool, next iteration they will be the next agent's neighbors)
		// trim it to the world
}

void Flock::Render(float deltaT)
{
	
	for (SteeringAgent* pagent: m_Agents)
	{
		if (pagent == nullptr)
			continue;
		pagent->Render(deltaT);
	}
	m_pAgentToEvade->Render(deltaT);
}

void Flock::UpdateAndRenderUI()
{
	//Setup
	int menuWidth = 235;
	int const width = DEBUGRENDERER2D->GetActiveCamera()->GetWidth();
	int const height = DEBUGRENDERER2D->GetActiveCamera()->GetHeight();
	bool windowActive = true;
	ImGui::SetNextWindowPos(ImVec2((float)width - menuWidth - 10, 10));
	ImGui::SetNextWindowSize(ImVec2((float)menuWidth, (float)height - 20));
	ImGui::Begin("Gameplay Programming", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
	ImGui::PushAllowKeyboardFocus(false);

	//Elements
	ImGui::Text("CONTROLS");
	ImGui::Indent();
	ImGui::Text("LMB: place target");
	ImGui::Text("RMB: move cam.");
	ImGui::Text("Scrollwheel: zoom cam.");
	ImGui::Unindent();

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();
	ImGui::Spacing();

	ImGui::Text("STATS");
	ImGui::Indent();
	ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
	ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
	ImGui::Unindent();

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	ImGui::Text("Flocking");
	ImGui::Spacing();

	ImGui::Checkbox("Debug Render steering", &m_CanDebugRender);
	ImGui::Checkbox("Debug Render neighborhood", &m_DebugNeighborhood);
	ImGui::Checkbox("Trim World", &m_TrimWorld);
	if (m_TrimWorld)
	{
		ImGui::SliderFloat("Trim Size", &m_WorldSize, 0.f, 500.f, "%1.");
	}
	ImGui::Spacing();
	ImGui::Spacing();
	ImGui::Spacing();

	ImGui::Text("Behavior Weights");
	ImGui::Spacing();

	//maakt sliders
	ImGui::SliderFloat("Cohesion", &m_pBlendedSteering->GetWeightedBehaviorsRef()[0].weight, 0.f, 1.f, "%.5");
	ImGui::SliderFloat("Seek", &m_pBlendedSteering->GetWeightedBehaviorsRef()[1].weight, 0.f, 1.f, "%.2");
	ImGui::SliderFloat("Alignment", &m_pBlendedSteering->GetWeightedBehaviorsRef()[2].weight, 0.f, 1.f, "%.2");
	ImGui::SliderFloat("Wander", &m_pBlendedSteering->GetWeightedBehaviorsRef()[3].weight, 0.f, 1.f, "%.2");
	ImGui::SliderFloat("Separation", &m_pBlendedSteering->GetWeightedBehaviorsRef()[4].weight, 0.f, 1.f, "%.2");

	//End
	ImGui::PopAllowKeyboardFocus();
	ImGui::End();
	
}

void Flock::RegisterNeighbors(SteeringAgent* pAgent)
{
	m_NrOfNeighbors = 0;
	Vector2 targetPos(pAgent->GetPosition());
	for (SteeringAgent* pCurrentAgent : m_Agents)
	{
		if (pAgent == pCurrentAgent || pCurrentAgent == nullptr)
		{
			continue;
		}
		float distance{ (targetPos - pCurrentAgent->GetPosition()).Magnitude() };
		if (distance < m_NeighborhoodRadius)
		{
			m_Neighbors[m_NrOfNeighbors] = pCurrentAgent;
			++m_NrOfNeighbors;
		}
	}
}

Elite::Vector2 Flock::GetAverageNeighborPos() const
{
	Vector2 averagePos{};
	for (int i{ 0 }; i < m_NrOfNeighbors; ++i)
	{
		averagePos += m_Neighbors[i]->GetPosition();
	}
	if (m_NrOfNeighbors >0)
	{
		averagePos /= m_NrOfNeighbors;
	}
	return averagePos;
}

Elite::Vector2 Flock::GetAverageNeighborVelocity() const
{
	Vector2 averageVel{};
	for (int i{ 0 }; i < m_NrOfNeighbors; ++i)
	{
		averageVel += m_Neighbors[i]->GetLinearVelocity();
	}
	if (m_NrOfNeighbors > 0)
	{
		averageVel /= m_NrOfNeighbors;
		averageVel.Normalize();
	}
	return averageVel;
}



void Flock::SetTarget_Seek(TargetData target)
{
	m_pSeekBehavior->SetTarget(target);
}


float* Flock::GetWeight(ISteeringBehavior* pBehavior) 
{
	if (m_pBlendedSteering)
	{
		auto& weightedBehaviors = m_pBlendedSteering->GetWeightedBehaviorsRef();
		auto it = find_if(weightedBehaviors.begin(),
			weightedBehaviors.end(),
			[pBehavior](BlendedSteering::WeightedBehavior el)
			{
				return el.pBehavior == pBehavior;
			}
		);

		if(it!= weightedBehaviors.end())
			return &it->weight;
	}

	return nullptr;
}
