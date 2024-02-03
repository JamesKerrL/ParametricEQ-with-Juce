#pragma once
#include <JuceHeader.h>

#include <cmath>

class BandMouseControlComponent : public juce::Component
{
public:
	BandMouseControlComponent()
	{
		setInterceptsMouseClicks( false, false );
	}

	~BandMouseControlComponent()
	{
	}

	void paint( juce::Graphics& g ) override
	{
		g.setColour( juce::Colours::white );
		g.drawEllipse( 0, 0, getWidth(), getHeight(), 3 );
	}

	void resized() override
	{
	}

private:
};