/*=============================================================================*/
// Copyright 2020-2021 Elite Engine
// Authors: Andries Geens
/*=============================================================================*/
// EDecisionMaking.h: General Decision making header that includes all Decision making structures and blackboard data
/*=============================================================================*/

/* --- Data --- */
//General Interface


namespace Elite
{
	class IDecisionMaking
	{
	public:
		IDecisionMaking() = default;
		virtual ~IDecisionMaking() = default;

		virtual void Update(float deltaT) = 0;

	};
}

	

