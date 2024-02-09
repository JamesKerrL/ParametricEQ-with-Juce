#pragma once


#include <JuceHeader.h>


class FifoBuffer
{
public:
	FifoBuffer( size_t buffer_size )
	{
		mAbstractFifo = std::make_unique< juce::AbstractFifo>( buffer_size );
		mStorage.ensureStorageAllocated( buffer_size );
		for (int index = 0; index < mStorage.size(); index++)
		{
			mStorage.set( index, 1.0f );
		}
	}

	void WriteTo( const float* data, int number_to_write )
	{
		int start1, start2, blockSize1, blockSize2;
		mAbstractFifo->prepareToWrite( number_to_write, start1, blockSize1, start2, blockSize2 );
		if (blockSize1 > 0)
		{
			juce::FloatVectorOperations::copy( mStorage.getRawDataPointer() + start1, data, blockSize1 );
		}
		if (blockSize2 > 0)
		{
			juce::FloatVectorOperations::copy( mStorage.getRawDataPointer() + start2, data + blockSize1, blockSize2 );
		}
		mAbstractFifo->finishedWrite( blockSize1+ blockSize2 );
	}

	void ReadFrom( float* data, int number_to_read )
	{
		int start1, start2, blockSize1, blockSize2;
		mAbstractFifo->prepareToRead( number_to_read, start1, blockSize1, start2, blockSize2 );
		if (blockSize1 > 0)
		{
			juce::FloatVectorOperations::copy( data, mStorage.getRawDataPointer() + start1, blockSize1 );
		}
		if (blockSize2 > 0)
		{
			juce::FloatVectorOperations::copy( data + blockSize1, mStorage.getRawDataPointer() + start2, blockSize2 );
		}
		mAbstractFifo->finishedRead( blockSize1 + blockSize2 );
	}

	int getNumReady()
	{
		return mAbstractFifo->getNumReady();
	}

private:
	std::unique_ptr<juce::AbstractFifo> mAbstractFifo;
	juce::Array<float> mStorage;
};