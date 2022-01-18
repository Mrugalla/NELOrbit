#pragma once
#include "GUIBasics.h"

namespace orbit
{
	namespace gui
	{
		template<typename Float, size_t NumPlanets, size_t FPS>
		struct Editor :
			juce::Component,
			juce::Timer
		{
			using Orbit = Processor<Float, NumPlanets>;
			using Planets = std::array<Planet<Float>, NumPlanets>;

			Editor(Orbit& _orbit) :
				juce::Component(),
				orbit(_orbit),
				planets(orbit.getPlanets()),
				centre(),
				minDimen(static_cast<Float>(1)),
				planetCols()
			{
				for (auto p = 0; p < NumPlanets; ++p)
				{
					const auto x = static_cast<Float>(p) / static_cast<Float>(NumPlanets);
					planetCols[p] = juce::Colour(0xffff0000).withRotatedHue(x);
				}

				startTimerHz(FPS);
			}
			void paint(juce::Graphics& g) override
			{
				for (auto p = 0; p < NumPlanets; ++p)
				{
					g.setColour(planetCols[p]);
					const auto& planet = planets[p];
					const auto pos = mapToBounds(planet.pos);
					const auto rad = planet.radius * minDimen;
					const auto diamtr = static_cast<float>(rad * static_cast<Float>(2));
					const auto d = diamtr > 1.f ? diamtr : 1.f;
					if (inBounds(pos))
					{
						const auto x = static_cast<float>(pos.x - rad);
						const auto y = static_cast<float>(pos.y - rad);
						g.fillEllipse(x, y, d, d);
					}	
				}
			}
		protected:
			Orbit& orbit;
			const Planets& planets;
			Vec<Float> bounds, centre;
			Float minDimen;
			std::array<juce::Colour, NumPlanets> planetCols;

			void resized() override
			{
				const auto width = static_cast<Float>(getWidth());
				const auto height = static_cast<Float>(getHeight());
				bounds.x = width;
				bounds.y = height;
				centre.x = bounds.x * static_cast<Float>(.5);
				centre.y = bounds.y * static_cast<Float>(.5);
				minDimen = std::min(width, height);
			}

			void timerCallback() override
			{
				repaint();
			}

			Vec<Float> mapToBounds(const Vec<Float>& vec) const noexcept
			{
				return
				{
					(vec.x + static_cast<Float>(1)) * centre.x,
					(vec.y + static_cast<Float>(1)) * centre.y
				};
			}

			bool inBounds(const Vec<Float>& vec) const noexcept
			{
				return
					vec.x >= static_cast<Float>(0) &&
					vec.x <= bounds.x &&
					vec.y >= static_cast<Float>(0) &&
					vec.y <= bounds.y;
			}
		};
	}
}