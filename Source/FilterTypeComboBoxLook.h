#pragma once
#include <JuceHeader.h>

#include <cmath>

namespace {

}
class FilterTypeComboBoxLook : public juce::LookAndFeel_V4
{
public:
	struct Item
	{
		std::string path;
		int size;
	};
	FilterTypeComboBoxLook()
	{
		images[1] = { std::string{ BinaryData::lp_svg }, BinaryData::lp_svgSize };
		images[2] = { std::string{ BinaryData::hp_svg }, BinaryData::hp_svgSize };
		images[3] = { std::string{ BinaryData::notch_svg }, BinaryData::notch_svgSize };
		images[4] = { std::string{ BinaryData::peak_svg }, BinaryData::peak_svgSize };
	}
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
		int id = box.getSelectedId();
		auto drawable = juce::Drawable::createFromImageData( images[id].path.c_str(), images[id].size );
		juce::Path path = drawable->getOutlineAsPath();
		auto tr = path.getTransformToScaleToFit( juce::Rectangle<float>{ static_cast<float>( buttonX ), static_cast<float>( buttonY ), static_cast<float>( buttonW ), static_cast<float>( buttonH ) },
												 true, juce::Justification::centred );
		path.applyTransform( tr );
		g.setColour( juce::Colours::red );
		g.fillPath( path );
	}

	std::unordered_map<int, Item>
	GetImages()
	{
		return images;
	}

private:
	std::unordered_map<int, Item> images; // id to image
};