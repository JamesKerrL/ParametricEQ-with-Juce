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
		//g.setColour( juce::Colours::yellow );
		//g.fillAll();
		juce::File file = juce::File::getCurrentWorkingDirectory().getChildFile( "../../Source/Images/lowpass-icon.svg" );
		jassert( file.exists() );
		auto drawable = juce::Drawable::createFromImageFile( file );
		juce::Path path = drawable->getOutlineAsPath();
		auto tr = path.getTransformToScaleToFit( juce::Rectangle<float>{ static_cast<float>( buttonX ), static_cast<float>( buttonY ), static_cast<float>( buttonW ), static_cast<float>( buttonH ) },
												 true, juce::Justification::centred );
		path.applyTransform( tr );
		g.setColour( juce::Colours::red );
		g.fillPath( path );
	}

private:

};