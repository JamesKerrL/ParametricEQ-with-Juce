#pragma once
#include <JuceHeader.h>

#include <cmath>
#include "BandMouseControlComponent.h"

class FilterControllersComponent : public juce::Component
{
public:
	FilterControllersComponent( juce::AudioProcessorValueTreeState& state_tree, const int number_of_bands ) : 
		mNumberBands( number_of_bands ), mStateTree( state_tree )
	{
		for (int i = 0; i < number_of_bands; i++)
		{
			auto local = std::make_unique<BandMouseControlComponent>();
			mBandButtons.push_back( std::move( local ) );

			addAndMakeVisible( *mBandButtons.back() );
		}
	}
	~FilterControllersComponent()
	{
	}

	void paint( juce::Graphics& g ) override
	{
	}

	void resized() override
	{
		const std::string CUTOFF_PARAMETER_PREFIX = "cutoff";
		for (int index = 0; index < mNumberBands; index++)
		{
			auto cutoff_parameter = mStateTree.getRawParameterValue( CUTOFF_PARAMETER_PREFIX + "_" + std::to_string( index ) );
			float cutoff = cutoff_parameter->load();

			float x = juce::mapFromLog10( cutoff, 20.0f, 20000.0f ) * getWidth();
			float y = getHeight() / 2;

			mBandButtons[index]->setBounds( x - 15, y - 15, 30, 30 );
		}
	}

	void mouseDown( const juce::MouseEvent& event ) override
	{
		for (int index = 0; index < mNumberBands; index++)
		{
			auto pos = mBandButtons[index]->getLocalPoint( this, event.getPosition() );
			auto* p =mBandButtons[index].get();
			if (p->getLocalBounds().contains( pos ))
			{
				draggedindex = index;
				return;
			}
		}
		draggedindex = -1;
	}

	void mouseDrag( const juce::MouseEvent& event ) override
	{
		if (draggedindex >= 0)
		{
			juce::Rectangle<int> new_position = mBandButtons[draggedindex]->getBounds();
			if (new_position.getX() != event.x) {
				new_position.setX( event.x );
				mBandButtons[draggedindex]->setBounds( new_position );

				updateCuttoff( event.x, draggedindex );
			}
		}
	}

	void mouseUp( const juce::MouseEvent& event ) override
	{
		draggedindex = -1;
	}

	void updateCuttoff(int x, int index)
	{
		float frequency = juce::mapToLog10( static_cast<double>(x) / static_cast<double>(getWidth()), 20.0, 20000.0 );
		const std::string CUTOFF_PARAMETER_PREFIX = "cutoff";
		auto cutoff_parameter = mStateTree.getRawParameterValue( CUTOFF_PARAMETER_PREFIX + "_" + std::to_string( index ) );
		cutoff_parameter->store( frequency );
	}

private:
	std::vector<std::unique_ptr<BandMouseControlComponent>> mBandButtons = {};
	const int mNumberBands;
	juce::AudioProcessorValueTreeState& mStateTree;
	int draggedindex = -1;
};