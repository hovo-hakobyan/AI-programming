#include "stdafx.h"
#include "ItemManager.h"

#include "IExamInterface.h"


ItemManager::ItemManager(IExamInterface* pInterface):
	m_pInterface{pInterface}
{
	m_DesiredSlotItems.reserve(5);
	m_DesiredSlotItems.push_back(eItemType::PISTOL);
	m_DesiredSlotItems.push_back(eItemType::SHOTGUN);
	m_DesiredSlotItems.push_back(eItemType::MEDKIT);
	m_DesiredSlotItems.push_back(eItemType::FOOD);
	m_DesiredSlotItems.push_back(eItemType::FOOD);

	m_Items.resize(5, eItemType::RANDOM_DROP);
}

void ItemManager::UpdateItems()
{
	for (size_t i = 0; i < m_Items.size(); i++)
	{
		if (m_Items[i] == eItemType::RANDOM_DROP)
			continue;

		ItemInfo itemInfo;

		if (!m_pInterface->Inventory_GetItem(static_cast<UINT>(i), itemInfo))
			continue;

		if ( m_Items[i] == eItemType::SHOTGUN  ||m_Items[i] == eItemType::PISTOL)
		{
			int ammo{ m_pInterface->Weapon_GetAmmo(itemInfo) };
			if (ammo !=0)
				continue;
		}
		else if (m_Items[i] == eItemType::FOOD)
		{
			int energy{ m_pInterface->Food_GetEnergy(itemInfo) };
			if (energy != 0)
				continue;
		}
		else if (m_Items[i] == eItemType::MEDKIT)
		{
			int health{ m_pInterface->Medkit_GetHealth(itemInfo) };

			if (health != 0)
				continue;
		}

		m_pInterface->Inventory_RemoveItem(static_cast<UINT>(i));
		m_Items[i] = eItemType::RANDOM_DROP;
		
	}
}

int ItemManager::GetCapacity() const
{
	return static_cast<int>( m_pInterface->Inventory_GetCapacity());
	
}

bool ItemManager::IsItemNeeded(ItemInfo& item) const
{
	for (size_t i = 0; i < m_DesiredSlotItems.size(); i++)
	{
		if (item.Type == m_DesiredSlotItems[i])
		{
			if (m_Items[i]== eItemType::RANDOM_DROP)
			{
				return true;
			}
		}
	}
	return false;

}

bool ItemManager::HasItemInInventory(const eItemType& type) const
{
	for (size_t i = 0; i < m_Items.size(); i++)
	{
		if (m_Items[i] != type)
			continue;
		
		return true;
	}
	return false;
}

int ItemManager::GetFoodEnergy() const
{
	for (size_t i = 0; i < m_DesiredSlotItems.size(); i++)
	{
		if (m_DesiredSlotItems[i] != eItemType::FOOD)
			continue;

		if (m_Items[i] == eItemType::RANDOM_DROP)
			continue;
		
		ItemInfo itemInfo;
		if (!m_pInterface->Inventory_GetItem(static_cast<UINT>(i), itemInfo))
			continue;

		int amount{ m_pInterface->Food_GetEnergy(itemInfo) };
		if (amount == -1)
			continue;

		return amount;
	}

	return -1;
}

int ItemManager::GetMedkitHealth() const
{
	for (size_t i = 0; i < m_DesiredSlotItems.size(); i++)
	{
		if (m_DesiredSlotItems[i] != eItemType::MEDKIT)
			continue;

		if (m_Items[i] == eItemType::RANDOM_DROP)
			continue;

		ItemInfo itemInfo;
		if (!m_pInterface->Inventory_GetItem(static_cast<UINT>(i), itemInfo))
			continue;

		int amount{ m_pInterface->Medkit_GetHealth(itemInfo) };
		if (amount == -1)
			continue;

		return amount;
	}

	return -1;
}





void ItemManager::AddItem(ItemInfo& item)
{
	for (size_t i = 0; i < m_DesiredSlotItems.size(); i++)
	{
		if (item.Type == m_DesiredSlotItems[i])
		{
			if (m_Items[i] != item.Type)
			{
				if (m_pInterface->Inventory_AddItem(static_cast<UINT>(i), item))
				{
					m_Items[i] = item.Type;
				}
			}
		}
	}
}

bool ItemManager::UseItem(const eItemType& type)
{
	for (size_t i = 0; i < m_Items.size(); i++)
	{
		if (m_Items[i] != type)
			continue;

		if (m_pInterface->Inventory_UseItem(static_cast<UINT>(i)))
			return true;		
	}

	return false;
}

	


