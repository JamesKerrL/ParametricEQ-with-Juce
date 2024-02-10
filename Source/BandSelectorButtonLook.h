#pragma once
#include <JuceHeader.h>

#include <cmath>

class BandSelectorButtonLook : public juce::LookAndFeel_V4
{
public:
	void drawButtonBackground( juce::Graphics & g, juce::Button & button, const juce::Colour &backgroundColour, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown ) override
	{
		g.setColour( backgroundColour );
		g.fillAll();
	}

private:

};