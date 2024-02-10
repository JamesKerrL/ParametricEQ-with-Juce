#pragma once
#include <JuceHeader.h>

#include <cmath>
#include "BandMouseControlComponent.h"
#include "PluginParameters.h"
#include "nonblocking_call_queue.h"
#include "Constants.h"

class FilterControllersComponent : public juce::Component, public juce::AudioProcessorValueTreeState::Listener, public juce::Timer
{
public:
	FilterControllersComponent( juce::AudioProcessorValueTreeState& state_tree, const int number_of_bands ) : 
		mNumberBands( number_of_bands ), mStateTree( state_tree ), mCallQueue( 1000 )
	{
		for (int i = 0; i < number_of_bands; i++)
		{
			auto local = std::make_unique<BandMouseControlComponent>( Constants::BAND_COLORS[i] );
			mBandButtons.push_back( std::move( local ) );

			addAndMakeVisible( *mBandButtons.back() );

			mStateTree.addParameterListener( PluginParameters::GetCutOffParameterId( i ), this );
			mStateTree.addParameterListener( PluginParameters::GetGainParameterId( i ), this );
			mStateTree.addParameterListener( PluginParameters::GetResonanceParameterId( i ), this );
			mStateTree.addParameterListener( "selectedBand", this );
		}
		startTimerHz( 60 );
	}
	~FilterControllersComponent()
	{
		for (int i = 0; i < mNumberBands; i++)
		{
			mStateTree.removeParameterListener( PluginParameters::GetCutOffParameterId( i ), this );
			mStateTree.removeParameterListener( PluginParameters::GetGainParameterId( i ), this );
			mStateTree.removeParameterListener( PluginParameters::GetResonanceParameterId( i ), this );
			mStateTree.removeParameterListener( "selectedBand", this );
		}
	}

	void paint( juce::Graphics& g ) override
	{
	}

	void resized() override
	{
		for (int index = 0; index < mNumberBands; index++)
		{
			auto cutoff_parameter = mStateTree.getRawParameterValue( PluginParameters::GetCutOffParameterId( index ) );
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
				auto selected_band_parameter = mStateTree.getParameter( "selectedBand" );
				auto normalised_band = selected_band_parameter->convertTo0to1( draggedindex );
				selected_band_parameter->beginChangeGesture();
				selected_band_parameter->setValueNotifyingHost( normalised_band );
				selected_band_parameter->endChangeGesture();
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
				updateCuttoff( event.x, draggedindex );
			}
			if (new_position.getY() != event.y) {
				updateGain( event.y, draggedindex );
			}
		}
	}

	void mouseUp( const juce::MouseEvent& event ) override
	{
		draggedindex = -1;
	}

	void mouseWheelMove( const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) override
	{
		if (draggedindex >= 0)
		{
			auto resonance_parameter = mStateTree.getParameter( PluginParameters::GetResonanceParameterId( draggedindex ) );
			float resonance_value = mStateTree.getRawParameterValue( PluginParameters::GetResonanceParameterId( draggedindex ) )->load();
			float denormalized_value = resonance_value + (wheel.deltaY * 1.5 );
			float normalised_resonance = resonance_parameter->convertTo0to1( denormalized_value );

			resonance_parameter->beginChangeGesture();
			resonance_parameter->setValueNotifyingHost( normalised_resonance );
			resonance_parameter->endChangeGesture();
		}
	}

	void mouseDoubleClick( const juce::MouseEvent& event ) override
	{
		for (int index = 0; index < mNumberBands; index++)
		{
			auto pos = mBandButtons[index]->getLocalPoint( this, event.getPosition() );
			auto* p = mBandButtons[index].get();
			if (p->getLocalBounds().contains( pos ))
			{
				auto resonance_parameter = mStateTree.getParameter( PluginParameters::GetResonanceParameterId( index ) );
				auto cutoff_parameter = mStateTree.getParameter( PluginParameters::GetCutOffParameterId( index ) );
				auto gain_parameter = mStateTree.getParameter( PluginParameters::GetGainParameterId( index ) );
				resonance_parameter->beginChangeGesture();
				resonance_parameter->setValueNotifyingHost( resonance_parameter->getDefaultValue() );
				cutoff_parameter->setValueNotifyingHost( cutoff_parameter->getDefaultValue() );
				gain_parameter->setValueNotifyingHost( gain_parameter->getDefaultValue() );
				resonance_parameter->endChangeGesture();
				return;
			}
		}
		draggedindex = -1;
	}

	void updateCuttoff(int x, int index)
	{
		float frequency = juce::mapToLog10( static_cast<double>(x) / static_cast<double>(getWidth()), 20.0, 20000.0 );
		auto cutoff_parameter = mStateTree.getParameter( PluginParameters::GetCutOffParameterId( index ) );
		cutoff_parameter->beginChangeGesture();
		auto normalised_frquency = cutoff_parameter->convertTo0to1( frequency );
		cutoff_parameter->setValueNotifyingHost( normalised_frquency );
		cutoff_parameter->endChangeGesture();
	}

	void updateGain( int y, int index )
	{
		float gain = juce::jmap( static_cast<float>( y ) / getHeight(), -24.0f, 24.0f );

		auto gain_parameter = mStateTree.getParameter( PluginParameters::GetGainParameterId( index ) );
		gain_parameter->beginChangeGesture();
		auto normalised_gain = 1.0f - gain_parameter->convertTo0to1( gain );
		gain_parameter->setValueNotifyingHost( normalised_gain );
		gain_parameter->endChangeGesture();
	}

	void parameterChanged( const juce::String& id, float newValue ) override
	{
		juce::String index_string = id.substring( id.indexOf("_") + 1 );
		if (index_string.isEmpty())
		{
			return;
		}

		int index = index_string.getIntValue();
		if (id.toStdString() == PluginParameters::GetCutOffParameterId( index ))
		{
			mCallQueue.callf( std::bind( &FilterControllersComponent::setXOfBandButton, this, index, newValue ) );
		}
		else if (id.toStdString() == PluginParameters::GetGainParameterId( index ))
		{
			mCallQueue.callf( std::bind( &FilterControllersComponent::setYOfBandButton, this, index, newValue ) );
		}
		else if (id.toStdString() == PluginParameters::GetResonanceParameterId( index ))
		{
			mCallQueue.callf( std::bind( &FilterControllersComponent::setSizeOfBandButton, this, index, newValue ) );
		}
		else if (id.toStdString() == "selectedBand")
		{
			mCallQueue.callf( std::bind( &FilterControllersComponent::setSelectedBand, this, static_cast<int>( newValue ) ) );
		}
	}

private:
	void setXOfBandButton( int index, float new_frequency )
	{
		jassert( juce::MessageManager::getInstance()->isThisTheMessageThread() );

		float x = juce::mapFromLog10( new_frequency, 20.0f, 20000.0f ) * getWidth();
		float y = mBandButtons[index]->getY();

		mBandButtons[index]->setBounds( x - 15, y, 30, 30 );
	}

	void setYOfBandButton( int index, float new_gain )
	{
		jassert( juce::MessageManager::getInstance()->isThisTheMessageThread() );

		auto gain_parameter = mStateTree.getParameter( PluginParameters::GetGainParameterId( index ) );
		auto normalised_gain = 1.0f - gain_parameter->convertTo0to1( new_gain );
		float x = mBandButtons[index]->getX();
		float y = juce::jmap( normalised_gain, static_cast<float>(0), static_cast<float>(getHeight()) );
		mBandButtons[index]->setBounds( x, y - 15, 30, 30 );
	}

	void setSizeOfBandButton( int index, float new_resonance )
	{
		jassert( juce::MessageManager::getInstance()->isThisTheMessageThread() );
		auto resonance_parameter = mStateTree.getParameter( PluginParameters::GetResonanceParameterId( index ) );
		auto normalised_resonance = resonance_parameter->convertTo0to1( new_resonance );
		mBandButtons[index]->setAlpha( normalised_resonance );
	}

	void setSelectedBand( int index )
	{
		jassert( juce::MessageManager::getInstance()->isThisTheMessageThread() );

		for (int i = 0; i < mNumberBands; i++)
		{
			if (index == i)
			{
				mBandButtons[index]->setGlow( true );
			}
			else
			{
				mBandButtons[i]->setGlow( false );
			}
		}
	}

	void timerCallback() override
	{
		mCallQueue.synchronize();
	}

	float mapResonanceToPixels( int index, float resonance )
	{
		auto resonance_parameter = mStateTree.getParameter( PluginParameters::GetResonanceParameterId( index ) );
		auto normalised_resonance = resonance_parameter->convertTo0to1( resonance );

		return juce::jmap( normalised_resonance, 5.0f, 30.0f );
	}
	std::vector<std::unique_ptr<BandMouseControlComponent>> mBandButtons = {};
	const int mNumberBands;
	juce::AudioProcessorValueTreeState& mStateTree;
	int draggedindex = -1;
	LockFreeCallQueue mCallQueue; // Used to sync threads without blocking
};