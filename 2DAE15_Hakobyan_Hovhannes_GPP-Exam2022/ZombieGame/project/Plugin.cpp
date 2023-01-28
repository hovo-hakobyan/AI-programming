#include "stdafx.h"
#include "Plugin.h"
#include "IExamInterface.h"
#include "SteeringBehaviors.h"

using namespace std;
using namespace Elite;


//Called only once, during initialization
void Plugin::Initialize(IBaseInterface* pInterface, PluginInfo& info)
{
	//Retrieving the interface
	//This interface gives you access to certain actions the AI_Framework can perform for you
	m_pInterface = static_cast<IExamInterface*>(pInterface);

	//Bit information about the plugin
	//Please fill this in!!
	info.BotName = "MinionExam";
	info.Student_FirstName = "Hovhannes";
	info.Student_LastName = "Hakobyan";
	info.Student_Class = "2DAE15";

	WorldInfo worldInfo = m_pInterface->World_GetInfo();
	m_pWorldGrid = new Grid{ 15,15,worldInfo };

	m_pSeek = new Seek();
	m_pFace = new Face();
	m_pSteeringBehavior = nullptr;

	m_pItemManager = new ItemManager{ m_pInterface };
	m_AgentState = EAgentState::FREE;
	InitBT();
}

//Called only once
void Plugin::DllInit()
{
	//Called when the plugin is loaded
	
}

//Called only once
void Plugin::DllShutdown()
{
	//Called wheb the plugin gets unloaded

	SAFE_DELETE(m_pWorldGrid);
	SAFE_DELETE(m_pBehaviorTree);
	SAFE_DELETE(m_pItemManager);
	
	SAFE_DELETE(m_pSeek);
	SAFE_DELETE(m_pFace);

}

//Called only once, during initialization
void Plugin::InitGameDebugParams(GameDebugParams& params)
{
	params.AutoFollowCam = true; //Automatically follow the AI? (Default = true)
	params.RenderUI = true; //Render the IMGUI Panel? (Default = true)
	params.SpawnEnemies = true; //Do you want to spawn enemies? (Default = true)
	params.EnemyCount = 20; //How many enemies? (Default = 20)
	params.GodMode = false; //GodMode > You can't die, can be useful to inspect certain behaviors (Default = false)
	params.LevelFile = "GameLevel.gppl";
	params.AutoGrabClosestItem = true; //A call to Item_Grab(...) returns the closest item that can be grabbed. (EntityInfo argument is ignored)
	params.StartingDifficultyStage = 1;
	params.InfiniteStamina = true;
	params.SpawnDebugPistol = true;
	params.SpawnDebugShotgun = true;
	params.SpawnPurgeZonesOnMiddleClick = true;
	params.PrintDebugMessages = true;
	params.ShowDebugItemNames = true;
	params.SpawnZombieOnRightClick = true;
	params.Seed = 69420;
	
}

//Only Active in DEBUG Mode
//(=Use only for Debug Purposes)
void Plugin::Update(float dt)
{

}

//Update
//This function calculates the new SteeringOutput, called once per frame
SteeringPlugin_Output Plugin::UpdateSteering(float dt)
{
	UpdateEntitiesInFOV();
	UpdateHousesInFOV();

	m_AgentInfo = m_pInterface->Agent_GetInfo();

	m_pWorldGrid->Update(dt, m_AgentInfo);
	m_pItemManager->UpdateItems();

	for (size_t i = 0; i < m_HousesInFOV.size(); i++)
	{
		m_pWorldGrid->AddHouse(m_HousesInFOV[i].Center);
	}

	m_pBehaviorTree->Update(dt);
	m_HousesInFOV.clear();
	m_EntitiesInFOV.clear();
	
	

	switch (m_AgentInfo.SteeringBehavior)
	{
	case ESteeringBehaviors::SEEK:
		m_pSteeringBehavior = m_pSeek;
		m_AgentInfo.TargetData.Position = m_pInterface->NavMesh_GetClosestPathPoint(m_AgentInfo.TargetData.Position);
		break;
	case ESteeringBehaviors::FACE:
		m_pSteeringBehavior = m_pFace;
		break;
	}
	
	
	m_pSteeringBehavior->SetTarget(m_AgentInfo.TargetData);

	auto steering = m_pSteeringBehavior->CalculateSteering(dt,m_AgentInfo);
	
	return steering;
}

//This function should only be used for rendering debug elements
void Plugin::Render(float dt) const
{
	//This Render function should only contain calls to Interface->Draw_... functions
	EntityInfo* target;
	if (m_pBB->GetData("ItemUnderInspection",target) && target)
	{
		m_pInterface->Draw_SolidCircle(target->Location, 7.f, { 0,0 }, { 1, 0, 0 });	
	}
	
	m_pInterface->Draw_SolidCircle(m_AgentInfo.TargetData.Position, .7f, { 0,0 }, { 1, 0, 0 });
	m_pWorldGrid->Render(m_pInterface);

}

void Plugin::UpdateHousesInFOV() 
{
	
	HouseInfo hi = {};
	for (int i = 0;; ++i)
	{
		if (m_pInterface->Fov_GetHouseByIndex(i, hi))
		{
			m_HousesInFOV.push_back(hi);
			continue;
		}
		break;
	}
	
}

void  Plugin::UpdateEntitiesInFOV()
{
	EntityInfo ei = {};
	for (int i = 0;; ++i)
	{
		if (m_pInterface->Fov_GetEntityByIndex(i, ei))
		{

			m_EntitiesInFOV.push_back(ei);
			continue;
		}

		break;
	}

}

void Plugin::InitBT()
{

	m_pBB = new Blackboard();
	m_pBB->AddData("Interface", m_pInterface);
	m_pBB->AddData("World", m_pWorldGrid);
	m_pBB->AddData("ItemManager", m_pItemManager);
	m_pBB->AddData("HousesInFOV", &m_HousesInFOV);
	m_pBB->AddData("EntitiesInFOV", &m_EntitiesInFOV);
	m_pBB->AddData("EnemyInFOV", static_cast<EntityInfo*>(nullptr));
	m_pBB->AddData("ItemUnderInspection", static_cast<EntityInfo*>(nullptr));
	m_pBB->AddData("CurrentItemLoc", ZeroVector2);
	m_pBB->AddData("AgentInfo", &m_AgentInfo);
	m_pBB->AddData("AgentState", &m_AgentState);
	m_pBB->AddData("CurrentHouse", static_cast<House*>(nullptr));
	m_pBB->AddData("AgentBehindLocFixed", ZeroVector2);
	m_pBB->AddData("WasAgentBitten", false);


	
	m_pBehaviorTree = new BehaviorTree(m_pBB, new BehaviorSelector(
		{
			new BehaviorSequence({
				new BehaviorConditional(BT_Conditions::HasFood),
				new BehaviorConditional(BT_Conditions::NeedsToConsumeFood),
				new BehaviorAction(BT_Actions::ConsumeFood)
				}),

			new BehaviorSequence({
				new BehaviorConditional(BT_Conditions::HasMedkit),
				new BehaviorConditional(BT_Conditions::NeedsToUseMedkit),
				new BehaviorAction(BT_Actions::UseMedkit)
				}),

			new BehaviorSequence({
				new BehaviorConditional(BT_Conditions::HasAmmo),
				new BehaviorSelector({
						new BehaviorSequence({
								new BehaviorConditional(BT_Conditions::IsEnemyInFOV),
								new BehaviorAction(BT_Actions::FaceEnemy),
								new BehaviorAction(BT_Actions::Shoot)
							}),
						new BehaviorSequence({
								new BehaviorConditional(BT_Conditions::WasBitten),
								new BehaviorAction(BT_Actions::Turn)
							}),
					}),

			}),

			new BehaviorSequence({
				new BehaviorConditional(BT_Conditions::IsItemInFOV),
				new BehaviorAction(BT_Actions::GoToItem),	
				new BehaviorSequence({
						new BehaviorConditional(BT_Conditions::IsItemInGrabRange),
						new BehaviorAction(BT_Actions::ProcessItem)
						}),

			}),

			new BehaviorSequence({
				new BehaviorConditional(BT_Conditions::AreThereAvailableHouses),
				new BehaviorAction(BT_Actions::GoToClosestAvailableHouse),
			}),
			
			new BehaviorSequence({
				new BehaviorAction(BT_Actions::Explore)
			}),
			

		}
	));
}
