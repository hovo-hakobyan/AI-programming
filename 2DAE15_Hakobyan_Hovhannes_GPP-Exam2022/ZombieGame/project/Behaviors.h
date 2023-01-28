#pragma once
/*=============================================================================*/
// Copyright 2020-2021 Elite Engine
/*=============================================================================*/
// Behaviors.h: Implementation of certain reusable behaviors for the BT version of the Agario Game
/*=============================================================================*/

//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "EBehaviorTree.h"
#include "EBlackboard.h"
#include "Grid.h"
#include"ItemManager.h"
#include "IExamInterface.h"


using namespace Elite;

//-----------------------------------------------------------------
// Behaviors
//-----------------------------------------------------------------



namespace BT_Actions
{
	Elite::BehaviorState Turn(Elite::Blackboard* pBlackboard)
	{
		AgentInfo_Extended* pAgentInfo;
		if (!pBlackboard->GetData("AgentInfo", pAgentInfo) || !pAgentInfo)
			return Elite::BehaviorState::Failure;

		Vector2 behindAgent{ZeroVector2};

		if(!pBlackboard->GetData("AgentBehindLocFixed",behindAgent))
			return Elite::BehaviorState::Failure;

		if (behindAgent == ZeroVector2)
		{
			Vector2 viewVec{ OrientationToVector(pAgentInfo->Orientation) * 10.f };
			behindAgent =pAgentInfo->Position -viewVec;
			pBlackboard->ChangeData("AgentBehindLocFixed", behindAgent);
		}

		EAgentState* pAgentState;
		if (!pBlackboard->GetData("AgentState", pAgentState))
			return Elite::BehaviorState::Failure;

		pAgentInfo->TargetData.Position = behindAgent;
		pAgentInfo->SteeringBehavior = ESteeringBehaviors::FACE;
		*pAgentState = EAgentState::COMBAT;

		if (pAgentInfo->isDoneFacing)
		{
			pBlackboard->ChangeData("AgentBehindLocFixed",	ZeroVector2);
			pBlackboard->ChangeData("WasAgentBitten", false);
			*pAgentState = EAgentState::FREE;
			
		}
		return Elite::BehaviorState::Success;
		
		
	}

	Elite::BehaviorState Explore(Elite::Blackboard* pBlackboard)
	{

		AgentInfo_Extended* pAgentInfo;

		if (!pBlackboard->GetData("AgentInfo", pAgentInfo) || !pAgentInfo)
			return Elite::BehaviorState::Failure;

		EAgentState* pAgentState;
		if (!pBlackboard->GetData("AgentState", pAgentState))
			return Elite::BehaviorState::Failure;

		if(*pAgentState !=EAgentState::FREE)
			return Elite::BehaviorState::Failure;

		Grid* pGridWorld;

		if (!pBlackboard->GetData("World", pGridWorld) || !pGridWorld)
			return Elite::BehaviorState::Failure;

		pAgentInfo->TargetData.Position = pGridWorld->GetNextFreeCellPosition();
		pAgentInfo->SteeringBehavior = ESteeringBehaviors::SEEK;

		if (pAgentInfo->Stamina == 0.0f)
		{
			pAgentInfo->RunMode = false;
		}
		else if (pAgentInfo->Stamina == 10.f)
		{
			pAgentInfo->RunMode = true;
		}

		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState GoToClosestAvailableHouse(Elite::Blackboard* pBlackboard)
	{
		Grid* pGrid;
		if (!pBlackboard->GetData("World", pGrid) || !pGrid)
			return Elite::BehaviorState::Failure;

		AgentInfo_Extended* pAgentInfo;
		if (!pBlackboard->GetData("AgentInfo", pAgentInfo) || !pAgentInfo)
			return Elite::BehaviorState::Failure;

		House* pHouse = pGrid->GetClosestAvailableHouse(pAgentInfo->Position);

		if (!pHouse)
			return Elite::BehaviorState::Failure;


		pAgentInfo->TargetData.Position = pHouse->center;
		pAgentInfo->SteeringBehavior = ESteeringBehaviors::SEEK;


		pBlackboard->ChangeData("CurrentHouse", pHouse);
		pAgentInfo->RunMode = false;

		if (pGrid->IsAgentInHouse(pAgentInfo->Position,pHouse))
		{
			EAgentState* pAgentState;
			if (!pBlackboard->GetData("AgentState", pAgentState))
				return Elite::BehaviorState::Failure;

			*pAgentState = EAgentState::FREE;
			pHouse->isAvailable = false;
			return Elite::BehaviorState::Success;
		}

		return Elite::BehaviorState::Running;
	}

	Elite::BehaviorState ProcessItem(Elite::Blackboard* pBlackboard)
	{

		ItemManager* pItemManager{};

		if (!pBlackboard->GetData("ItemManager", pItemManager) || !pItemManager)
			return Elite::BehaviorState::Failure;

		IExamInterface* pInterface;
		if (!pBlackboard->GetData("Interface", pInterface) || !pInterface)
			return Elite::BehaviorState::Failure;

		EntityInfo* pItem;

		if(!pBlackboard->GetData("ItemUnderInspection", pItem) || !pItem)
			return Elite::BehaviorState::Failure;

		EAgentState* pAgentState;
		if (!pBlackboard->GetData("AgentState", pAgentState))
			return Elite::BehaviorState::Failure;

		ItemInfo itemInfo{};


		if (!pInterface->Item_GetInfo(*pItem, itemInfo))
			return Elite::BehaviorState::Failure;

		

		if (pItemManager->IsItemNeeded(itemInfo))
		{
			if (!pInterface->Item_Grab(*pItem, itemInfo))
				return Elite::BehaviorState::Failure;	
			pItemManager->AddItem(itemInfo);
		}
		else
		{
			pInterface->Item_Destroy(*pItem);
		}
		
		pBlackboard->ChangeData("ItemUnderInspection", nullptr);
		pBlackboard->ChangeData("CurrentItemLoc", ZeroVector2);
		
		*pAgentState = EAgentState::FREE;
		return Elite::BehaviorState::Success;

	}

	Elite::BehaviorState GoToItem(Elite::Blackboard* pBlackboard)
	{
		Vector2 loc{};

		if (!pBlackboard->GetData("CurrentItemLoc", loc))
			return Elite::BehaviorState::Failure;

		if (loc == ZeroVector2)
		{
			return Elite::BehaviorState::Failure;
		}

		AgentInfo_Extended* pAgentInfo;
		if (!pBlackboard->GetData("AgentInfo", pAgentInfo) || !pAgentInfo)
			return Elite::BehaviorState::Failure;

		EAgentState* pAgentState;
		if (!pBlackboard->GetData("AgentState", pAgentState))
			return Elite::BehaviorState::Failure;


		*pAgentState = EAgentState::LOOKINGFORITEM;

		pAgentInfo->TargetData.Position = loc;
		pAgentInfo->SteeringBehavior = ESteeringBehaviors::SEEK;
		pAgentInfo->RunMode = false;
		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState FaceEnemy(Elite::Blackboard* pBlackboard)
	{
		AgentInfo_Extended* pAgentInfo;
		if (!pBlackboard->GetData("AgentInfo", pAgentInfo) || !pAgentInfo)
			return Elite::BehaviorState::Failure;

		EntityInfo* enemyInFOV;

		if(!pBlackboard->GetData("EnemyInFOV", enemyInFOV) || !enemyInFOV)
			return Elite::BehaviorState::Failure;

		IExamInterface* pInterface;

		if (!pBlackboard->GetData("Interface", pInterface) || !pInterface)
			return Elite::BehaviorState::Failure;

		EnemyInfo enemyInfo{};



		if (pInterface->Enemy_GetInfo(*enemyInFOV,enemyInfo))
		{
			
			pAgentInfo->TargetData.Position = enemyInFOV->Location;
			pAgentInfo->SteeringBehavior = ESteeringBehaviors::FACE;

			if (pAgentInfo->isDoneFacing)
			{
				
				return Elite::BehaviorState::Success;
			}

			return Elite::BehaviorState::Running;
			
		}

		return Elite::BehaviorState::Failure;
	}

	Elite::BehaviorState Shoot(Elite::Blackboard* pBlackboard)
	{
		ItemManager* pItemManager;

		if (!pBlackboard->GetData("ItemManager", pItemManager) || !pItemManager)
			return Elite::BehaviorState::Failure;

		EAgentState* pAgentState;
		if (!pBlackboard->GetData("AgentState", pAgentState))
			return Elite::BehaviorState::Failure;

		if (pItemManager->HasItemInInventory(eItemType::SHOTGUN))
		{
			if (pItemManager->UseItem(eItemType::SHOTGUN))
			{
				*pAgentState = EAgentState::FREE;
				return Elite::BehaviorState::Success;
			}
			
		}
			
		if (pItemManager->HasItemInInventory(eItemType::PISTOL))
		{
			if (pItemManager->UseItem(eItemType::PISTOL))
			{
				*pAgentState = EAgentState::FREE;
				return Elite::BehaviorState::Success;
			}
		}

		return Elite::BehaviorState::Failure;
	}

	Elite::BehaviorState ConsumeFood(Elite::Blackboard* pBlackboard)
	{
		ItemManager* pItemManager;

		if (!pBlackboard->GetData("ItemManager", pItemManager) || !pItemManager)
			return Elite::BehaviorState::Failure;

		if(!pItemManager->UseItem(eItemType::FOOD))
			return Elite::BehaviorState::Failure;


		return Elite::BehaviorState::Success;

	}

	Elite::BehaviorState UseMedkit(Elite::Blackboard* pBlackboard)
	{
		ItemManager* pItemManager;

		if (!pBlackboard->GetData("ItemManager", pItemManager) || !pItemManager)
			return Elite::BehaviorState::Failure;

		if (!pItemManager->UseItem(eItemType::MEDKIT))
			return Elite::BehaviorState::Failure;

		return Elite::BehaviorState::Success;
	}
}

//-----------------------------------------------------------------
// Conditions
//-----------------------------------------------------------------

namespace BT_Conditions
{
	bool IsEnemyInFOV(Elite::Blackboard* pBlackboard)
	{
		std::vector<EntityInfo>* pEntityInfoVec;

		if (!pBlackboard->GetData("EntitiesInFOV", pEntityInfoVec) || !pEntityInfoVec)
			return false;

		IExamInterface* pInterface;
		if (!pBlackboard->GetData("Interface", pInterface) || !pInterface)
			return false;

		EAgentState* pAgentState;
		if (!pBlackboard->GetData("AgentState", pAgentState))
			return false;

		for (size_t i = 0; i < pEntityInfoVec->size(); i++)
		{
			//If the entity is not an enemy, go to next item
			if ((*pEntityInfoVec)[i].Type != eEntityType::ENEMY)
				continue;

			//If the entity is enemy, add it to blackboard and return true
			EnemyInfo enemyInfo{};
			if (pInterface->Enemy_GetInfo((*pEntityInfoVec)[i], enemyInfo))
			{
				*pAgentState = EAgentState::COMBAT;
				pBlackboard->ChangeData("EnemyInFOV", &(*pEntityInfoVec)[i]);
				return true;
			}
		}

		return false;
	}

	bool IsItemInFOV(Elite::Blackboard* pBlackboard)
	{
		EAgentState* pAgentState;
		if (!pBlackboard->GetData("AgentState", pAgentState))
			return false;

		if (*pAgentState == EAgentState::COMBAT)
			return false;

		std::vector<EntityInfo>* pEntityInfoVec;
		if (!pBlackboard->GetData("EntitiesInFOV", pEntityInfoVec) || !pEntityInfoVec)
			return false;

		if (pEntityInfoVec->empty())
			return false;
			
		EntityInfo* pNeededItem = nullptr;

		if (!pBlackboard->GetData("ItemUnderInspection", pNeededItem))
			return false;

		Vector2 loc{};

		if (!pBlackboard->GetData("CurrentItemLoc", loc))
			return false;
		
	
		for (size_t i = 0; i < pEntityInfoVec->size(); i++)
		{
			if (loc == ZeroVector2)
			{
				//If the entity is not an item, go to next element
				if ((*pEntityInfoVec)[i].Type != eEntityType::ITEM)
					continue;

				
				pBlackboard->ChangeData("ItemUnderInspection", &(*pEntityInfoVec)[i]);
				pBlackboard->ChangeData("CurrentItemLoc", (*pEntityInfoVec)[i].Location);
				return true;
			}
			else
			{
				if ((*pEntityInfoVec)[i].Location == loc)
				{
			
					pBlackboard->ChangeData("ItemUnderInspection", &(*pEntityInfoVec)[i]);
					return true;
				}
			}
		}
	
		return false;
	}

	bool HasAmmo(Elite::Blackboard* pBlackboard)
	{
		ItemManager* pItemManager;

		if (!pBlackboard->GetData("ItemManager", pItemManager) || !pItemManager)
			return false;

		if (pItemManager->HasItemInInventory(eItemType::SHOTGUN))
			return true;

		if (pItemManager->HasItemInInventory(eItemType::PISTOL))
			return true;

		return false;
	}

	bool AreThereAvailableHouses(Elite::Blackboard* pBlackboard)
	{
		EAgentState* pAgentState;
		if (!pBlackboard->GetData("AgentState", pAgentState))
			return false;

		switch (*pAgentState)
		{
		case EAgentState::LOOKINGFORITEM:
		case EAgentState::COMBAT:
			return false;
		}

		Grid* pGrid;
		if (!pBlackboard->GetData("World", pGrid) || !pGrid)
			return false;

		if (!pGrid->AreAnyKnownHouses())
			return false;

		if (pGrid->AreAnyAvailableHouses())
		{
			*pAgentState = EAgentState::GOINGTOHOUSE;
			return true;
		}
		return false;
	}

	bool IsItemInGrabRange(Elite::Blackboard* pBlackboard)
	{
		AgentInfo_Extended* pAgentInfo;
		if (!pBlackboard->GetData("AgentInfo", pAgentInfo) || !pAgentInfo)
			return false;

		EntityInfo* pItemUnderInspection;

		if (!pBlackboard->GetData("ItemUnderInspection", pItemUnderInspection) || !pItemUnderInspection)
			return false;

		Vector2 itemLoc{ pItemUnderInspection->Location };
		float radius{ pAgentInfo->GrabRange };
		float distanceSqr = itemLoc.DistanceSquared(pAgentInfo->Position);

		if (distanceSqr <= radius * radius)
		{
			return true;
		}
		return false;
		
	}

	bool WasBitten(Elite::Blackboard* pBlackboard)
	{
		AgentInfo_Extended* pAgentInfo;
		if (!pBlackboard->GetData("AgentInfo", pAgentInfo) || !pAgentInfo)
			return false;
		
		bool wasBitten = false;

		if (!pBlackboard->GetData("WasAgentBitten", wasBitten))
			return false;

		if (pAgentInfo->WasBitten)
		{
			wasBitten = true;
			pBlackboard->ChangeData("WasAgentBitten", wasBitten);
		}

		return wasBitten;
	}

	bool HasFood(Elite::Blackboard* pBlackboard)
	{
		ItemManager* pItemManager;

		if (!pBlackboard->GetData("ItemManager", pItemManager) || !pItemManager)
			return false;

		if (pItemManager->HasItemInInventory(eItemType::FOOD))
			return true;

	
		return false;
	}

	bool HasMedkit(Elite::Blackboard* pBlackboard)
	{
		ItemManager* pItemManager;

		if (!pBlackboard->GetData("ItemManager", pItemManager) || !pItemManager)
			return false;

		if (pItemManager->HasItemInInventory(eItemType::MEDKIT))
			return true;

		return false;
	}

	bool NeedsToConsumeFood(Elite::Blackboard* pBlackboard)
	{
		ItemManager* pItemManager;

		if (!pBlackboard->GetData("ItemManager", pItemManager) || !pItemManager)
			return false;

		AgentInfo_Extended* pAgentInfo;
		if (!pBlackboard->GetData("AgentInfo", pAgentInfo) || !pAgentInfo)
			return false;

		int maxEnergy = 10;
		int inventoryFood = pItemManager->GetFoodEnergy();

		
		if (inventoryFood == -1)
			return false;

		if (pAgentInfo->Energy + inventoryFood > maxEnergy)
			return false;

		return true;
	}

	bool NeedsToUseMedkit(Elite::Blackboard* pBlackboard)
	{
		ItemManager* pItemManager;

		if (!pBlackboard->GetData("ItemManager", pItemManager) || !pItemManager)
			return false;

		AgentInfo_Extended* pAgentInfo;
		if (!pBlackboard->GetData("AgentInfo", pAgentInfo) || !pAgentInfo)
			return false;

		int maxHealth = 10;
		int inventoryHealth = pItemManager->GetMedkitHealth();

		if (inventoryHealth == -1)
			return false;

		if (pAgentInfo->Health + inventoryHealth > maxHealth)
			return false;

		return true;
	}
}
