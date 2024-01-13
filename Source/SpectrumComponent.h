#pragma once
#include <JuceHeader.h>

#include <cmath>
#include "FilterResponseCurveComponent.h"
#include "FifoBuffer.h"

class SpectrumComponent : public juce::Component, public juce::Timer
{
public:
	SpectrumComponent( FifoBuffer& fifo_buffer ) : 
		forwardFFT( 9 ),
		window( 512, juce::dsp::WindowingFunction<float>::hann ),
		mFifoBuffer( fifo_buffer )
	{
		juce::FloatVectorOperations::fill( mInternalBuffer.data(), 0.0f, mInternalBuffer.size() );
		startTimerHz( 30 );
	}

	void paint( juce::Graphics& g ) override
	{
		g.fillAll( juce::Colours::black );

		window.multiplyWithWindowingTable( mInternalBuffer.data(), 512 );
		forwardFFT.performFrequencyOnlyForwardTransform( mInternalBuffer.data() );

		
		juce::Path path;
		float y0 = juce::jmap<float>( juce::Decibels::gainToDecibels( mInternalBuffer[0] / 512 ), -98.0, 0.0, getBottom(), getY() );
		path.startNewSubPath( 0, y0 );
		for (int i = 1; i < getWidth(); i++)
		{
			auto skewedProportionX = 1.0f - std::exp( std::log( 1.0f - (float)i / (float)getWidth() ) * 0.2f );
			auto fftDataIndex = juce::jlimit( 0, 512 / 2, (int)(skewedProportionX * (float)512 * 0.5f) );
			float y = juce::jmap<float>( juce::Decibels::gainToDecibels( mInternalBuffer[fftDataIndex]/512), -98.0, 0.0, getBottom(), getY());
			path.lineTo( i, y );
		}
		g.setColour( juce::Colours::orange );
		g.strokePath( path, juce::PathStrokeType( 2.0f ) );
	}

	void resized() override
	{

	}

	void timerCallback() override 
	{
		if (mFifoBuffer.getNumReady() >= mInternalBuffer.size())
		{
			mFifoBuffer.ReadFrom( mInternalBuffer.data(), mInternalBuffer.size() );
			repaint();
		}
	}

private:
	juce::dsp::FFT forwardFFT;
	juce::dsp::WindowingFunction<float> window;
	FifoBuffer& mFifoBuffer;
	std::array<float, 1024> mInternalBuffer;
};