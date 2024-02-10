#pragma once
#include <JuceHeader.h>

#include <cmath>
#include "Constants.h"

class BandMouseControlComponent : public juce::Component
{
public:
	BandMouseControlComponent( juce::Colour band_color ) : mBandColor( band_color )
	{
		setInterceptsMouseClicks( false, false );
		glow.setGlowProperties( 2, mBandColor );
	}

	~BandMouseControlComponent()
	{
	}

	void paint( juce::Graphics& g ) override
	{
		g.setColour( mBandColor );
		g.drawEllipse( 5, 5, getWidth() - 10, getHeight() - 10, 2 );
		g.setColour( mBandColor.withAlpha( mAlpha ) );
		g.fillEllipse( 5, 5, getWidth() - 10, getHeight() - 10 );
	}

	void resized() override
	{
	}

	void setAlpha( float alpha )
	{
		mAlpha = juce::jmap( alpha, 0.0f, 0.7f );
		repaint();
	}

	void setGlow( bool on )
	{
		if (on)
		{
			setComponentEffect( &glow );
		}
		else
		{
			setComponentEffect( nullptr );
		}
		repaint();
	}

private:
	juce::Colour mBandColor;
	float mAlpha = 0.1f;
	juce::GlowEffect glow;
};