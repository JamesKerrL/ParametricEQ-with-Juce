#pragma once
#include <JuceHeader.h>

#include <cmath>
#include "FilterResponseCurveComponent.h"
#include "FifoBuffer.h"
#include "Constants.h"

class SpectrumComponent : public juce::Component, public juce::Timer
{
public:
	SpectrumComponent( FifoBuffer& fifo_buffer, double sample_rate ) :
		forwardFFT( Constants::FFT_ORDER ),
		window( Constants::FFT_SIZE, juce::dsp::WindowingFunction<float>::hann ),
		mFifoBuffer( fifo_buffer ),
		mSampleRate( sample_rate )
	{
		juce::FloatVectorOperations::fill( mInternalBuffer.data(), 0.0f, mInternalBuffer.size() );
		juce::FloatVectorOperations::fill( mRenderBuffer.data(), 0.0f, mRenderBuffer.size() );
		startTimerHz( 60 );
	}

	void paint( juce::Graphics& g ) override
	{
		g.fillAll( juce::Colours::black );
		g.setColour( juce::Colours::white.withAlpha(0.1f) );
		int idx = 0;
		for (auto freq : Constants::FREQUENCIES_OF_INTEREST)
		{
			int x = std::round( juce::mapFromLog10( freq, 20.0f, 20000.0f ) * getWidth() );
			g.fillRect( juce::Rectangle<int>{ x, 0, 1, getHeight() } );
			idx++;
		}

		g.setColour( juce::Colours::orange );
		if (mPath) {
			g.strokePath( *mPath, juce::PathStrokeType( 2.0f, juce::PathStrokeType::curved ) );
			juce::Path fill_path = *mPath;
			fill_path.lineTo( getWidth(), getHeight() );
			fill_path.lineTo( 0, getHeight() );
			fill_path.closeSubPath();
			juce::Colour color{ juce::Colours::orange };
			g.setColour( color.withAlpha( 0.3f ) );
			g.fillPath( fill_path );
		}
	}

	void resized() override
	{

	}

	void timerCallback() override 
	{
		if (mFifoBuffer.getNumReady() + 1 >= Constants::FFT_SIZE && mSampleRate > 0) // TODO Fix the + 1
		{
			mFifoBuffer.ReadFrom( mInternalBuffer.data(), Constants::FFT_SIZE );

			window.multiplyWithWindowingTable( mInternalBuffer.data(), Constants::FFT_SIZE );
			forwardFFT.performFrequencyOnlyForwardTransform( mInternalBuffer.data() );
			auto fsize = forwardFFT.getSize();

			juce::FloatVectorOperations::multiply( mRenderBuffer.data(), 0.7f, mRenderBuffer.size() );
			for (auto i = 0; i < mRenderBuffer.size(); ++i)
			{
				if (mInternalBuffer[i] > mRenderBuffer[i])
					mRenderBuffer[i] = mInternalBuffer[i];
			}

			juce::Path path;
			int index = 0; // 0 index is DC frequency
			float frequency = 0;

			float y0 = juce::jmap<float>( juce::Decibels::gainToDecibels( mRenderBuffer[1] / Constants::FFT_SIZE ), -78.0, 0.0, getBottom(), getY() );
			path.startNewSubPath( 0, y0 * 1.2 ); // 1.2 is arbitrarily chosen to make a smooth line going from first frequency to off the sceen.

			while (frequency < 20)
			{
				index++;
				frequency = (index * mSampleRate) / Constants::FFT_SIZE;
			}

			while (frequency <= 20000 && index < Constants::FFT_SIZE)
			{
				float y = juce::jmap<float>( juce::Decibels::gainToDecibels( mRenderBuffer[index] / Constants::FFT_SIZE ), -78.0, 0.0, getBottom(), getY() );
				float x = juce::mapFromLog10( frequency, 20.0f, 20000.0f ) * getWidth();

				path.lineTo( x, y );
				index++;
				frequency = (index * mSampleRate) / Constants::FFT_SIZE;
			}
			mPath = std::make_unique<juce::Path>( path.createPathWithRoundedCorners( 5 ) );
			repaint();
		}
	}

	void setSampleRate( double sample_rate )
	{
		mSampleRate = sample_rate;
	}

private:
	std::unique_ptr<juce::Path> mPath;
	juce::dsp::FFT forwardFFT;
	juce::dsp::WindowingFunction<float> window;
	FifoBuffer& mFifoBuffer;
	std::array<float, Constants::FFT_SIZE * 2> mInternalBuffer;
	std::array<float, Constants::FFT_SIZE * 2> mRenderBuffer;
	double mSampleRate;
};