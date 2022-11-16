#pragma once
#include <vector>
#include <iostream>
#include "framework/EliteMath/EMath.h"
#include "framework\EliteAI\EliteGraphs\ENavGraph.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphAlgorithms\EAStar.h"

namespace Elite
{
	class NavMeshPathfinding
	{
	public:
		static std::vector<Vector2> FindPath(Vector2 startPos, Vector2 endPos, NavGraph* pNavGraph, std::vector<Vector2>& debugNodePositions, std::vector<Portal>& debugPortals)
		{
			//Create the path to return
			std::vector<Vector2> finalPath{};

			//Get the start and endTriangle
			const Triangle* startTr = pNavGraph->GetNavMeshPolygon()->GetTriangleFromPosition(startPos);
			const Triangle* endTr = pNavGraph->GetNavMeshPolygon()->GetTriangleFromPosition(endPos);

			if (!startTr || !endTr)
				return finalPath;

			if (startTr == endTr)
			{
				finalPath.push_back(endPos);
				return finalPath;
			}

			//We have valid start/end triangles and they are not the same
			//=> Start looking for a path
			//Copy the graph
			auto graphClone = pNavGraph->Clone();
			IGraph<NavGraphNode, GraphConnection2D>* graphCopy = graphClone.get();

			//Create extra node for the Start Node (Agent's position
			NavGraphNode* startNode{ new NavGraphNode(graphCopy->GetNextFreeNodeIndex(),invalid_node_index,startPos) };
			graphCopy->AddNode(startNode);
			
			int currentConnectionIdx{ 0 };
			for (int lineIdx : startTr->metaData.IndexLines)
			{
				if (pNavGraph->GetNodeIdxFromLineIdx(lineIdx) != invalid_node_index)
				{					
					GraphConnection2D* connection{new GraphConnection2D( startNode->GetIndex(),pNavGraph->GetNodeIdxFromLineIdx(lineIdx)) };
					connection->SetCost(Distance(startNode->GetPosition(), graphCopy->GetNode(pNavGraph->GetNodeIdxFromLineIdx(lineIdx))->GetPosition()));

					graphCopy->AddConnection(connection);
				}
			}

			//Create extra node for the endNode
			NavGraphNode* endNode{ new NavGraphNode(graphCopy->GetNextFreeNodeIndex(),invalid_node_index,endPos) };
			graphCopy->AddNode(endNode);

			for (int lineIdx : endTr->metaData.IndexLines)
			{
				if (pNavGraph->GetNodeIdxFromLineIdx(lineIdx) != invalid_node_index)
				{
					GraphConnection2D* connection{ new GraphConnection2D( endNode->GetIndex(),pNavGraph->GetNodeIdxFromLineIdx(lineIdx)) };
					connection->SetCost(Distance(endNode->GetPosition(), graphCopy->GetNode(pNavGraph->GetNodeIdxFromLineIdx(lineIdx))->GetPosition()));
					graphCopy->AddConnection(connection);
				}
			}

			//Run A star on new graph
			auto pathfinder = AStar<NavGraphNode, GraphConnection2D>(graphCopy, HeuristicFunctions::Chebyshev);
			auto path = pathfinder.FindPath(graphCopy->GetNode(graphCopy->GetNrOfNodes() - 2), graphCopy->GetNode(graphCopy->GetNrOfNodes() - 1));
			
			//OPTIONAL BUT ADVICED: Debug Visualisation
			for (auto& node : path)
			{
				finalPath.push_back(node->GetPosition());
				debugNodePositions.push_back(node->GetPosition());
			}

			//Run optimiser on new graph, MAKE SURE the A star path is working properly before starting this section and uncommenting this!!!
			debugPortals = SSFA::FindPortals(path, pNavGraph->GetNavMeshPolygon());
			finalPath = SSFA::OptimizePortals(debugPortals);

			return finalPath;
		}
	};
}
