#pragma once
#include <cmath>
#include <limits>
#include <array>
#include <juce_core/juce_core.h>
//juce_core » maths

namespace orbit
{
	template<typename Float>
	struct Vec
	{
		Vec(Float _x = static_cast<Float>(0), Float _y = static_cast<Float>(0)) :
			x(_x),
			y(_y)
		{}

		Vec<Float> operator+(const Vec<Float>& other) const noexcept
		{
			return { x + other.x, y + other.y };
		}
		Vec<Float> operator-(const Vec<Float>& other) const noexcept
		{
			return { x - other.x, y - other.y };
		}
		Vec<Float> operator*(const Vec<Float>& other) const noexcept
		{
			return { x * other.x, y * other.y };
		}
		Vec<Float> operator/(const Vec<Float>& other) const noexcept
		{
			return { x / other.x, y / other.y };
		}

		Vec<Float> operator+(const Float other) const noexcept
		{
			return { x + other, y + other };
		}
		Vec<Float> operator-(const Float other) const noexcept
		{
			return { x - other, y - other };
		}
		Vec<Float> operator*(const Float other) const noexcept
		{
			return { x * other, y * other };
		}
		Vec<Float> operator/(const Float other) const noexcept
		{
			return { x / other, y / other };
		}

		void operator+=(const Vec<Float>& other) noexcept
		{
			x += other.x;
			y += other.y;
		}
		void operator-=(const Vec<Float>& other) noexcept
		{
			x -= other.x;
			y -= other.y;
		}
		void operator*=(const Vec<Float>& other) noexcept
		{
			x *= other.x;
			y *= other.y;
		}
		void operator/=(const Vec<Float>& other) noexcept
		{
			x /= other.x;
			y /= other.y;
		}

		void operator+=(const Float other) noexcept
		{
			x += other;
			y += other;
		}
		void operator-=(const Float other) noexcept
		{
			x -= other;
			y -= other;
		}
		void operator*=(const Float other) noexcept
		{
			x *= other;
			y *= other;
		}
		void operator/=(const Float other) noexcept
		{
			x /= other;
			y /= other;
		}

		bool isZero() const noexcept
		{
			static constexpr auto eps = std::numeric_limits<Float>::epsilon();
			return x < eps || y < eps;
		}

		Vec<Float> inv() const noexcept
		{
			return {
				static_cast<Float>(1) / x,
				static_cast<Float>(1) / y
			};
		}

		Float distSqr(const Vec<Float>& other) const noexcept
		{
			const auto x2 = other.x - x;
			const auto y2 = other.y - y;
			return x2 * x2 + y2 * y2;
		}
		Float dist(const Vec<Float>& other) const noexcept
		{
			return std::sqrt(distSqr(other));
		}
		Float angle(const Vec<Float>& other) const noexcept
		{
			return std::atan2(other.y - y, other.x - x);
		}

		double x, y;
	};

	template<typename Float, size_t NumEdges>
	struct Move
	{
		static constexpr Float Pi = static_cast<Float>(3.14159265359);
		static constexpr Float Tau = Pi * static_cast<Float>(2);
		static constexpr Float TauInv = static_cast<Float>(1) / Tau;
		static constexpr Float NumEdgesF = static_cast<Float>(NumEdges);

		Move() :
			sinBuf(),
			cosBuf()
		{
			const auto numEdgesInv = static_cast<Float>(1) / NumEdgesF;
			for (auto i = 0; i < NumEdges; ++i)
			{
				const auto x = Tau * static_cast<Float>(i) * numEdgesInv - Pi;
				sinBuf[i] = std::sin(x);
				cosBuf[i] = std::cos(x);
			}
		}
		void operator()(Vec<Float>& vec, Float angle, Float mag) const noexcept
		{
			const auto idx = static_cast<size_t>(std::floor((angle + Pi) * TauInv * NumEdgesF));
			vec.x += cosBuf[idx] * mag;
			vec.y += sinBuf[idx] * mag;
		}
	protected:
		std::array<Float, NumEdges> sinBuf, cosBuf;
	};

	template<typename Float>
	struct Shared
	{
		Shared() :
			move()
		{}

		const Move<Float, 64> move;
		static Shared<Float> shared;
	};

	template<typename Type>
	struct Downsample
	{
		Downsample(const int _order) :
			Fs(static_cast<Type>(48000)),
			blockSize(64),
			idx(-1),
			order(1 << _order)
		{

		}
		void prepare(Type sampleRate, int _blockSize) noexcept
		{
			const auto oInv = static_cast<Type>(1) / static_cast<Type>(order);
			Fs = sampleRate * oInv;
			blockSize = _blockSize / order;
		}
		bool process() noexcept
		{
			++idx;
			if (idx == order)
			{
				idx = 0;
				return true;
			}
			return false;
		}

		Type Fs;
		int blockSize;
	protected:
		int idx;
		const int order;
	};

	template<typename Float>
	juce::String toString(Vec<Float> vec)
	{
		return static_cast<juce::String>(vec.x) << "; " << static_cast<juce::String>(vec.y);
	}

	template<typename Float>
	struct Planet
	{
		static constexpr Float Pi = static_cast<Float>(3.14159265359);
		static constexpr Float Tau = Pi * static_cast<Float>(2);

		Planet() :
			pos(),
			dir(),
			mass(static_cast<Float>(1)),
			radius(static_cast<Float>(.004)),
			angle(static_cast<Float>(0)),
			mag(static_cast<Float>(0))
		{}
		Planet(Vec<Float>&& _pos, Vec<Float>&& _dir, Float _mass, Float _radius) :
			pos(_pos),
			dir(_dir),
			mass(_mass),
			radius(_radius),
			angle(static_cast<Float>(0)),
			mag(static_cast<Float>(0))
		{}
		const Float getX() const noexcept { return pos.x; }
		const Float getY() const noexcept { return pos.y; }

		void gravitate(const Planet<Float>& other, const Float G) noexcept
		{
			const auto distSqr = pos.distSqr(other.pos);
			const auto rad2 = radius + other.radius;
			if (distSqr < rad2 * rad2)
			{
				//collideSlowDown(1 - G * Tau);
			}
			else
			{
				const auto distSqrInv = static_cast<Float>(1) / distSqr;
				angle = pos.angle(other.pos);
				mag = G * mass * other.mass * distSqrInv;
				Shared<Float>::shared.move(dir, angle, mag);
			}
			spaceMud(.999);
		}
		void update() noexcept
		{
			pos += dir;
		}

		Vec<Float> pos, dir;
		Float mass, radius, angle, mag;
	protected:
		void collideSlowDown(Float coeff) noexcept
		{
			dir *= coeff;
		}

		void spaceMud(Float coeff) noexcept
		{
			dir *= coeff;
		}
	};

	template<typename Float, size_t NumPlanets>
	struct Processor
	{
		static constexpr Float Gravity = static_cast<Float>(.005);
		static constexpr Float MinPos = static_cast<Float>(-1);
		static constexpr Float MaxPos = static_cast<Float>(1);
		static constexpr Float RangePos = MaxPos - MinPos;

		using Planets = std::array<Planet<Float>, NumPlanets>;

		Processor(int _downsampleOrder) :
			planets(),
			sampleRate(static_cast<Float>(48000)),
			sampleRateInv(Gravity * static_cast<Float>(1) / sampleRate),
			downsample(_downsampleOrder)
		{}

		void randomizePlanetPositions() noexcept
		{
			juce::Random rand;
			for (auto& p : planets)
			{
				p.pos.x = static_cast<double>(rand.nextDouble() * 2. - 1.);
				p.pos.y = static_cast<double>(rand.nextDouble() * 2. - 1.);
			}
		}

		void prepare(Float _sampleRate, int _blockSize)
		{
			downsample.prepare(_sampleRate, _blockSize);
			sampleRate = downsample.Fs;
			sampleRateInv = Gravity * static_cast<Float>(1) / sampleRate / static_cast<Float>(NumPlanets);
		}

		void processBlock(int numSamples) noexcept
		{
			for (auto s = 0; s < numSamples; ++s)
				if(downsample.process())
					processSample();
		}

		const Planets& getPlanets() const noexcept
		{
			return planets;
		}
	protected:
		Planets planets;
		double sampleRate, sampleRateInv;
		Downsample<Float> downsample;

		void processSample() noexcept
		{
			for (auto i = 0; i < NumPlanets; ++i)
			{
				auto& p0 = planets[i];
				for (auto j = 0; j < NumPlanets; ++j)
				{
					const bool samePlanet = i == j;
					if (!samePlanet)
						p0.gravitate(planets[j], sampleRateInv);
				}
				p0.update();
			}
			topologyBillard();
		}

		void topologyBillard()
		{
			static constexpr auto Repell = static_cast<Float>(-1);

			for (auto& planet : planets)
			{
				if (planet.pos.x < MinPos || planet.pos.x > MaxPos)
					planet.dir.x *= Repell;
				if (planet.pos.y < MinPos || planet.pos.y > MaxPos)
					planet.dir.y *= Repell;
			}
		}
		void topologyTorus()
		{
			
			for (auto& planet : planets)
			{
				while (planet.pos.x < MinPos)
					planet.pos.x += RangePos;
				while (planet.pos.x > MaxPos)
					planet.pos.x -= RangePos;
				while (planet.pos.y < MinPos)
					planet.pos.y += RangePos;
				while (planet.pos.y > MaxPos)
					planet.pos.y -= RangePos;
			}
		}

	public:
		void dbg(int i = 0)
		{
			switch (i)
			{
			case 0:
				juce::String s;
				for (auto& p : planets)
					s += toString(p.pos) + "\n";
				DBG(s);
				return;
			}
		}
	};
}