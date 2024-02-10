#pragma once


#include <JuceHeader.h>


namespace
{
}

namespace Constants
{
	constexpr int NUMBER_OF_BANDS = 5;

	static const std::vector<juce::Colour> BAND_COLORS{ 
		juce::Colour( 0xffFF6C22 ),
		juce::Colour( 0xffC16991 ), 
		juce::Colour( 0xff9585B6 ), 
		juce::Colour( 0xff22B5FF ), 
		juce::Colour( 0xff69C199 ),
		// Just a duplicate of first set of colors
		juce::Colour( 0xffFF6C22 ),
		juce::Colour( 0xffC16991 ),
		juce::Colour( 0xff9585B6 ),
		juce::Colour( 0xff22B5FF ),
		juce::Colour( 0xff69C199 )
	};

	constexpr int FFT_SIZE = 2048 * 2;
	constexpr int FFT_ORDER = 12;

	/// These are frequencies that visually used indicate the frequency position within the spectrum to the user
	static const std::vector<float> FREQUENCIES_OF_INTEREST{ 20.f, 30.f, 40.f, 60.f, 80.f, 100.f,
													         200.f, 300.f, 400.f, 600.f, 800.f, 1000.f, 2000.f, 3000.f, 4000.f, 6000.f, 8000.f, 10000.f, 20000.f };

	namespace DraggableFilterControl
	{
		constexpr int WIDTH = 5;
		constexpr int HEIGHT = 5;
	};
}