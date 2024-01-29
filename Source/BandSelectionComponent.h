#pragma once
#include <JuceHeader.h>

#include <cmath>
#include "Constants.h"

class BandSelectionComponent : public juce::Component
{
public:
	BandSelectionComponent( const int number_of_bands, std::function<void( int )> function ) : mNumberOfBands( number_of_bands ), mOnChangedFunction( function )
	{
		for (int i = 0; i < number_of_bands; i++)
		{
			juce::String band_name;
			band_name << "Band " << (i + 1);
			auto local = std::make_unique<juce::TextButton>( band_name );
			int index = i;
			local->setClickingTogglesState( true );
			local->setColour( juce::TextButton::buttonColourId, Constants::BAND_COLORS[i] );
			local->setColour( juce::TextButton::buttonOnColourId, Constants::BAND_COLORS[i].darker( 0.85 ) );
			local->onClick = [&, index]()
			{
				mOnChangedFunction( index );
				TurnOffUnselectedButtons( index );
			};
			mBandButtons.push_back( std::move( local ) );

			addAndMakeVisible( *mBandButtons.back() );
		}
	}

	void paint( juce::Graphics& g ) override
	{
		g.fillAll( juce::Colours::black );
	}

	void resized() override
	{
		auto band_width = getWidth() / mBandButtons.size();
		for (int index = 0; index < mBandButtons.size(); index++)
		{
			mBandButtons[index]->setBounds( index * band_width, 0, band_width, getHeight() );
		}
	}

	void TurnOffUnselectedButtons( int currently_selected_index )
	{
		for (int index = 0; index < mBandButtons.size(); index++)
		{
			if (currently_selected_index == index)
			{
				continue;
			}
			mBandButtons[index]->setToggleState( false, juce::dontSendNotification );
		}
	}

	void SetSelectedButton( int index )
	{
		mBandButtons[index]->setToggleState( true, juce::dontSendNotification );
		TurnOffUnselectedButtons( index );
	}

private:
	int mNumberOfBands;
	std::function<void(int)> mOnChangedFunction;
	std::vector<std::unique_ptr<juce::TextButton>> mBandButtons = {};
};