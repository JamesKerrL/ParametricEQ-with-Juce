#pragma once
#include <JuceHeader.h>

#include <cmath>

class FilterTypeComboBoxLook : public juce::LookAndFeel_V4
{
public:
	void drawComboBox( juce::Graphics& g,
		int 	width,
		int 	height,
		bool 	isButtonDown,
		int 	buttonX,
		int 	buttonY,
		int 	buttonW,
		int 	buttonH,
		juce::ComboBox& box ) override
	{
		auto drawable = juce::Drawable::createFromImageData( BinaryData::lowpassicon_svg, BinaryData::lowpassicon_svgSize );
		juce::Path path = drawable->getOutlineAsPath();
		auto tr = path.getTransformToScaleToFit( juce::Rectangle<float>{ static_cast<float>( buttonX ), static_cast<float>( buttonY ), static_cast<float>( buttonW ), static_cast<float>( buttonH ) },
												 true, juce::Justification::centred );
		path.applyTransform( tr );
		g.setColour( juce::Colours::red );
		g.fillPath( path );
	}

private:

};