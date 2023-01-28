#pragma once
#include "Exam_HelperStructs.h"

class IExamInterface;

class ItemManager final
{
public:
	ItemManager(IExamInterface* Interface);

	~ItemManager() = default;
	ItemManager(const ItemManager& other) = delete;
	ItemManager& operator=(const ItemManager& other) = delete;
	ItemManager(ItemManager&& other) = delete;
	ItemManager& operator =(ItemManager&& other) = delete;

	void UpdateItems();

	int GetCapacity() const;
	bool IsItemNeeded(ItemInfo& item) const;
	bool HasItemInInventory(const eItemType& type) const;
	int GetFoodEnergy() const;
	int GetMedkitHealth() const;



	void AddItem(ItemInfo& item);
	bool UseItem(const eItemType& type);

private:
	IExamInterface* m_pInterface;

	std::vector<eItemType> m_DesiredSlotItems;
	std::vector<eItemType> m_Items;


	
};

