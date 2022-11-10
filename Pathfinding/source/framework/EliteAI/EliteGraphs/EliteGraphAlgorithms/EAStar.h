#pragma once

namespace Elite
{
	template <class T_NodeType, class T_ConnectionType>
	class AStar
	{
	public:
		AStar(IGraph<T_NodeType, T_ConnectionType>* pGraph, Heuristic hFunction);

		// stores the optimal connection to a node and its total costs related to the start and end node of the path
		struct NodeRecord
		{
			T_NodeType* pNode = nullptr;
			T_ConnectionType* pConnection = nullptr;
			float costSoFar = 0.f; // accumulated g-costs of all the connections leading up to this one
			float estimatedTotalCost = 0.f; // f-cost (= costSoFar + h-cost)

			bool operator==(const NodeRecord& other) const
			{
				return pNode == other.pNode
					&& pConnection == other.pConnection
					&& costSoFar == other.costSoFar
					&& estimatedTotalCost == other.estimatedTotalCost;
			};

			bool operator<(const NodeRecord& other) const
			{
				return estimatedTotalCost < other.estimatedTotalCost;
			};
		};

		std::vector<T_NodeType*> FindPath(T_NodeType* pStartNode, T_NodeType* pDestinationNode);

	private:
		float GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const;

		IGraph<T_NodeType, T_ConnectionType>* m_pGraph;
		Heuristic m_HeuristicFunction;
	};

	template <class T_NodeType, class T_ConnectionType>
	AStar<T_NodeType, T_ConnectionType>::AStar(IGraph<T_NodeType, T_ConnectionType>* pGraph, Heuristic hFunction)
		: m_pGraph(pGraph)
		, m_HeuristicFunction(hFunction)
	{
	}

	template <class T_NodeType, class T_ConnectionType>
	std::vector<T_NodeType*> AStar<T_NodeType, T_ConnectionType>::FindPath(T_NodeType* pStartNode, T_NodeType* pGoalNode)
	{
		std::vector<T_NodeType*> finalPath;
		std::vector<NodeRecord> openList;
		std::vector<NodeRecord> closedList;

		NodeRecord currentRecord = *pStartNode;
		currentRecord.pConnection = nullptr;
		currentRecord.estimatedTotalCost = GetHeuristicCost(pStartNode, pGoalNode);
		openList.push_back(currentRecord);


		while (!openList.empty())
		{
			currentRecord = *std::min_element(openList.begin(), openList.end());

			if (currentRecord == *pGoalNode)
				break;


			for (auto& connection : m_pGraph->GetNodeConnections(currentRecord.pNode))
			{
				NodeRecord nextNode = m_pGraph->GetNode(connection->GetTo());
				nextNode.pConnection = connection;
				nextNode.costSoFar = currentRecord.costSoFar + connection->getCost();
				nextNode.estimatedTotalCost = nextNode.costSoFar + GetHeuristicCost(nextNode.pNode, pGoalNode);

				if (std::find(closedList.begin(), closedList.end(), nextNode) != closedList.end())
				{
					if (currentRecord->costSoFar < nextNode.costSoFar)
					{
						continue;
					}
					else
					{
						closedList.erase(std::remove(closedList.begin(), closedList.end(), currentRecord));
					}
				}
				else if (std::find(openList.begin(),openList.end(),nextNode) != openList.end())
				{
					if (currentRecord->costSoFar < nextNode.costSoFar)
					{
						continue;
					}
					else
					{
						openList.erase(std::remove(openList.begin(), openList.end(), currentRecord));
					}
				}

				openList.push_back(nextNode);
						
			}
			currentRecord = *std::min_element(openList.begin(), openList.end());
			closedList.push_back(currentRecord);
			openList.erase(std::remove(openList.begin(), openList.end(), currentRecord));
			

		}

		while (currentRecord != *pStartNode)
		{
			finalPath.push_back(&currentRecord);

			for (auto& node : closedList)
			{
				if (currentRecord.pNode->GetIndex() == node.pConnection->GetFrom())
				{
					currentRecord = node;
					break;
				}
			}
			
		}
		finalPath.push_back(pStartNode);
		std::reverse(finalPath.begin(), finalPath.end());

		return finalPath;
	}

	template <class T_NodeType, class T_ConnectionType>
	float Elite::AStar<T_NodeType, T_ConnectionType>::GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const
	{
		Vector2 toDestination = m_pGraph->GetNodePos(pEndNode) - m_pGraph->GetNodePos(pStartNode);
		return m_HeuristicFunction(abs(toDestination.x), abs(toDestination.y));
	}
}