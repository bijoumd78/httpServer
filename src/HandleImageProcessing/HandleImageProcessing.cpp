#include "HandleImageProcessing.h"

namespace AI::ImageProcessing
{
	template <typename T>
	void HandleImageProcessing<T>::processImage(T& args)
	{
		processEvent(this, args);
		//processEvent.notify(this, args);
	}
}