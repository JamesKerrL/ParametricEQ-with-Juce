#pragma once
#include <JuceHeader.h>

#include <cmath>
#include "FilterResponseCurveComponent.h"
#include "FifoBuffer.h"

class SpectrumComponent : public juce::Component, public juce::Timer
{
public:
	SpectrumComponent( FifoBuffer& fifo_buffer, double sample_rate ) :
		forwardFFT( 9 ),
		window( 512, juce::dsp::WindowingFunction<float>::hann ),
		mFifoBuffer( fifo_buffer ),
		mSampleRate( sample_rate )
	{
		juce::FloatVectorOperations::fill( mInternalBuffer.data(), 0.0f, mInternalBuffer.size() );
		startTimerHz( 30 );
	}

	void paint( juce::Graphics& g ) override
	{
		g.fillAll( juce::Colours::black );

		window.multiplyWithWindowingTable( mInternalBuffer.data(), 512 );
		forwardFFT.performFrequencyOnlyForwardTransform( mInternalBuffer.data() );
		auto fsize = forwardFFT.getSize();
		
		juce::Path path;
		float y0 = juce::jmap<float>( juce::Decibels::gainToDecibels( mInternalBuffer[0] / 512 ), -98.0, 0.0, getBottom(), getY() );
		path.startNewSubPath( 0, y0 );
		for (int i = 1; i < getWidth(); i++)
		{
			// pixel to frequency
			float frq = juce::mapToLog10( static_cast<double>(i) / static_cast<double>(getWidth()), 20.0, 20000.0 );

			// frequency to buffer index
			auto index = ((frq * 512) / mSampleRate);

			float y = juce::jmap<float>( juce::Decibels::gainToDecibels( mInternalBuffer[index]/512), -98.0, 0.0, getBottom(), getY());

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
	double mSampleRate;
};