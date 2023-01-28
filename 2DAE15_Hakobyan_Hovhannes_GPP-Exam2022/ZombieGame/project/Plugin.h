#pragma once
#include "IExamPlugin.h"
#include "Exam_HelperStructs.h"
#include "Behaviors.h"


class IBaseInterface;
class IExamInterface;
class Grid;
class ItemManager;

class ISteeringBehavior;
class Seek;
class Flee;
class Face;

class Plugin :public IExamPlugin
{
public:
	Plugin() {};
	virtual ~Plugin() {};

	void Initialize(IBaseInterface* pInterface, PluginInfo& info) override;
	void DllInit() override;
	void DllShutdown() override;

	//Deze functie wordt gefired enkel in debug. maar examen gaan ze release ook checken
	void InitGameDebugParams(GameDebugParams& params) override;

	void Update(float dt) override;
	
	SteeringPlugin_Output UpdateSteering(float dt) override;
	void Render(float dt) const override;

private:
	//Interface, used to request data from/perform actions with the AI Framework
	IExamInterface* m_pInterface = nullptr;

	void UpdateHousesInFOV() ;
	void UpdateEntitiesInFOV();
	

	//
	ItemManager* m_pItemManager;
	Grid* m_pWorldGrid;
	AgentInfo_Extended m_AgentInfo;
	EAgentState m_AgentState;

	Elite::Blackboard* m_pBB;
	Elite::BehaviorTree* m_pBehaviorTree;

	std::vector<HouseInfo> m_HousesInFOV;
	std::vector<EntityInfo> m_EntitiesInFOV;

	ISteeringBehavior* m_pSteeringBehavior;
	Seek* m_pSeek;
	Face* m_pFace;

	void InitBT();
};

//ENTRY
//This is the first function that is called by the host program
//The plugin returned by this function is also the plugin used by the host program
extern "C"
{
	__declspec (dllexport) IPluginBase* Register()
	{
		return new Plugin();
	}
}