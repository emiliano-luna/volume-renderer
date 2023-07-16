#pragma once
#ifndef VOLUMERENDERER_MULTITHREADINGHELPER
#define VOLUMERENDERER_MULTITHREADINGHELPER

#include <cstdint>
#include <semaphore>

class MultithreadingHelper
{
public:
	MultithreadingHelper(uint32_t chunkSize, uint32_t chunkCount);
	bool tryReservingChunk(uint32_t& chunkOffset);
private: 
	std::binary_semaphore _semaphore{ 1 };
	uint32_t _chunkSize;
	uint32_t _chunkCurrent;
	uint32_t _chunkCount;
};

#endif