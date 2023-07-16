#include "MultithreadingHelper.h"

MultithreadingHelper::MultithreadingHelper(uint32_t chunkSize, uint32_t chunkCount)
{
	_chunkSize = chunkSize;
	_chunkCurrent = 0;
	_chunkCount = chunkCount;
}

bool MultithreadingHelper::tryReservingChunk(uint32_t& chunkOffset)
{	
	bool ret = true;

	_semaphore.acquire();

	if (_chunkCurrent >= _chunkCount) {
		ret = false;
	}	
	else
	{
		chunkOffset = _chunkSize * _chunkCurrent;
		_chunkCurrent++;
	}

	_semaphore.release();

	return ret;
}
