#pragma once
#include <JuceHeader.h>

#include <cmath>
#include "BandMouseControlComponent.h"

class FilterControllersComponent : public juce::Component
{
public:
	FilterControllersComponent( const int number_of_bands )
	{
		for (int i = 0; i < number_of_bands; i++)
		{
			auto local = std::make_unique<BandMouseControlComponent>();
			mBandButtons.push_back( std::move( local ) );

			addAndMakeVisible( *mBandButtons.back() );
		}
	}

	void paint( juce::Graphics& g ) override
	{
	}

	void resized() override
	{

	}

private:
	std::vector<std::unique_ptr<BandMouseControlComponent>> mBandButtons = {};
};