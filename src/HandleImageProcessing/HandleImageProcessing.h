#pragma once
#include "HighResolution.h"
#include "CycleganAPI.h"
#include <Poco/FIFOEvent.h>

namespace AI::ImageProcessing{

	template <typename T>
	class HandleImageProcessing
	{
	public:
		Poco::FIFOEvent<T> processEvent;
		
		void processImage(T& args);
	};

	template class HandleImageProcessing<AI::ImageProcessingUnit2::InputArgList>;
	template class HandleImageProcessing<AI::ImageProcessingUnit::InputArg>;
}