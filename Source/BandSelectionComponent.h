#pragma once
#include <JuceHeader.h>

#include <cmath>
#include "Constants.h"
#include "BandSelectorButtonLook.h"

class BandSelectionComponent : public juce::Component, public juce::AudioProcessorValueTreeState::Listener, public juce::Timer
{
public:
	BandSelectionComponent( ParametricEQAudioProcessor& processor,const int number_of_bands, std::function<void( int )> function ) : 
		mProcessor( processor ), mNumberOfBands( number_of_bands ), mOnChangedFunction( function )
	{
		for (int i = 0; i < number_of_bands; i++)
		{
			juce::String band_name;
			band_name << "Band " << (i + 1);
			auto local = std::make_unique<juce::TextButton>( band_name );
			local->setLookAndFeel( &mLook  );
			int index = i;
			local->setClickingTogglesState( false );
			local->setColour( juce::TextButton::textColourOffId, Constants::BAND_COLORS[i].darker( 0.3 ) );

			local->setColour( juce::TextButton::buttonColourId, Constants::BAND_COLORS[i].withAlpha(0.0f) );
			local->setColour( juce::TextButton::buttonOnColourId, Constants::BAND_COLORS[i].withAlpha( 0.2f ) );
			local->setColour( juce::TextButton::textColourOnId, Constants::BAND_COLORS[i].darker( 0.45 ) );
			local->onClick = [&, index]()
			{
				auto selected_band_parameter = mProcessor.GlobalStateTree.getParameter( "selectedBand" );
				float norm_value = selected_band_parameter->convertTo0to1( index );
				selected_band_parameter->sendValueChangedMessageToListeners( norm_value ); // TODO don't want this notifying host
				mOnChangedFunction( index );
				TurnOffUnselectedButtons( index );
				//mBandButtons[index]->setToggleState( true, juce::dontSendNotification );
			};
			mBandButtons.push_back( std::move( local ) );

			addAndMakeVisible( *mBandButtons.back() );
		}
		mProcessor.GlobalStateTree.addParameterListener( "selectedBand", this );
		startTimerHz( 60 );
	}

	~BandSelectionComponent()
	{
		mProcessor.GlobalStateTree.removeParameterListener( "selectedBand", this );
		for (int i = 0; i < mNumberOfBands; i++)
		{
			mBandButtons[i]->setLookAndFeel( nullptr );
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

	void parameterChanged( const juce::String& id, float newValue ) override
	{
		if (id.toStdString() == "selectedBand")
		{
			mUpdateSelectedBand.store( true );
		}
	}

	void timerCallback() override
	{
		if (mUpdateSelectedBand.load())
		{
			int selectedBand = static_cast<int>( mProcessor.GlobalStateTree.getRawParameterValue( "selectedBand" )->load() );
			SetSelectedButton( selectedBand );
			mOnChangedFunction( selectedBand );
			mUpdateSelectedBand.store( false );
		}
	}

private:
	ParametricEQAudioProcessor& mProcessor;
	std::atomic<bool> mUpdateSelectedBand{ false };
	int mNumberOfBands;
	std::function<void(int)> mOnChangedFunction;
	std::vector<std::unique_ptr<juce::TextButton>> mBandButtons = {};
	BandSelectorButtonLook mLook;
};