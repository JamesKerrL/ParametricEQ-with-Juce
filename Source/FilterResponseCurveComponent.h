#pragma once
#define _USE_MATH_DEFINES


#include <JuceHeader.h>

#include <cmath>

class FilterResponseCurveComponent : public juce::Component
{
public:
	FilterResponseCurveComponent( )
	{

	}

	void SetMagnitudes(std::vector<float>& magnitudes)
	{
		mMagnitudes = magnitudes;
		repaint();
	}

	void paint( juce::Graphics& g ) override
	{
		if (mMagnitudes.empty()) 
		{
			return;
		}
		float bot = static_cast<float>(getHeight());
		float top = static_cast<float>(getY());

		juce::Path curve;
		float y1 = juce::jmap( mMagnitudes[0], -24.0f, 24.0f, bot, top );
		curve.startNewSubPath(0, y1);
		for (int i = 1; i < mMagnitudes.size(); i++)
		{
			float y = juce::jmap( mMagnitudes[i], -24.0f, 24.0f, bot, top );
			curve.lineTo( i, y );
		}
		g.setColour( juce::Colours::white );
		g.strokePath( curve, juce::PathStrokeType( 2.0f ) );
	}

	void resized() override
	{
	}
private:
	std::vector<float> mMagnitudes;
};