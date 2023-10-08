#pragma once
#include "GUIBasics.h"

#define PLANET_STARTING_COLOR 0xffff0000

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
				minDimen(static_cast<Float>(1))
			{
				for (auto p = 0; p < NumPlanets; ++p)
				{
					auto mass = planets[p].mass;
					planetCols[p] = juce::Colour(PLANET_STARTING_COLOR).withRotatedHue(mass);
				}

				startTimerHz(FPS);
			}
			void paint(juce::Graphics& g) override
			{
				for (auto p = 0; p < orbit.getNumPlanets(); ++p)
				{
					g.setColour(planetCols[p]);
					const auto& planet = planets[p];
					const auto pos = mapPlanetPosToBounds(planet.pos);
					const auto rad = planet.radius * minDimen;
					const auto diamtr = static_cast<float>(rad * static_cast<Float>(2));
					const auto d = diamtr > 1.f ? diamtr : 1.f;
					if (inBounds(pos, 2.0))
						//if (true)
					{
						const auto x = static_cast<float>(pos.x - rad);
						const auto y = static_cast<float>(pos.y - rad);
						g.fillEllipse(x, y, d, d);
					}
				}
			}
			void timerCallback() override
			{
				repaint();
			}

			void resized() override
			{
				if (getWidth() != getHeight())
				{
					setBounds(ui::maxQuadIn(getLocalBounds().toFloat()).toNearestInt());
					return;
				}

				const auto width = static_cast<Float>(getWidth());
				const auto height = static_cast<Float>(getHeight());
				bounds.x = width;
				bounds.y = height;
				centre.x = bounds.x * static_cast<Float>(.5);
				centre.y = bounds.y * static_cast<Float>(.5);
				minDimen = std::min(width, height);
			}

		private:
			Orbit& orbit;
			const Planets& planets;
			Vec2D<Float> bounds{}, centre{};
			Float minDimen;
			std::array<juce::Colour, NumPlanets> planetCols{};

			/*
			* The relative position of a planet is between -1 and 1, where 0 is the centre of the coord. system.
			*/
			Vec2D<Float> mapPlanetPosToBounds(const Vec2D<Float>& vec) const noexcept
			{
				return
				{
					(vec.x + static_cast<Float>(1)) * centre.x,
					(vec.y + static_cast<Float>(1)) * centre.y
				};
			}

			bool inBounds(const Vec2D<Float>& vec, const Float& edge) const noexcept
			{
				return
					vec.x > -edge &&
					vec.x < (bounds.x + edge) &&
					vec.y > -edge &&
					vec.y < bounds.y + edge;
			}
		};
	}
}
