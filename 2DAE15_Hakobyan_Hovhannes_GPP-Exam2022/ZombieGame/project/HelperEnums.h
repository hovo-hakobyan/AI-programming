#pragma once
enum class ECellNeighbor
{
	NORTH, EAST, SOUTH, WEST
};

enum class ESteeringBehaviors
{
	SEEK, FACE
};

enum class EAgentState
{
	LOOKINGFORITEM, FREE, GOINGTOHOUSE , COMBAT
};