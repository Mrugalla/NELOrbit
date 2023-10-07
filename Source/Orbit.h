#pragma once
#include <cmath>
#include <limits>
#include <array>
#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_data_structures/juce_data_structures.h>
#include "Constants.h"

namespace orbit
{
	struct StateIDs
	{
		juce::Identifier orbit{ "orbit" };
		juce::Identifier planet{ "planet" };
		juce::Identifier posX{ "posX" };
		juce::Identifier posY{ "posY" };
		juce::Identifier dirX{ "dirX" };
		juce::Identifier dirY{ "dirY" };
		juce::Identifier radius{ "radius" };
		juce::Identifier mass{ "mass" };
	};

	/********** struct Smooth **********/
	template<typename Float>
	struct Smooth
	{
		using numConst = constants::NumericConstants<Float>;

		void makeFromDecayInSamples(Float d) noexcept
		{
			const auto x = std::pow(numConst::E, static_cast<Float>(-1) / d);
			setX(x);
		}
		void makeFromDecayInSecs(Float d, Float Fs) noexcept
		{
			makeFromDecayInSamples(d * Fs);
		}
		void makeFromDecayInFc(Float fc) noexcept
		{
			const auto x = std::pow(numConst::E, -numConst::Tau * fc);
			setX(x);
		}
		void makeFromDecayInHz(Float d, Float Fs) noexcept
		{
			makeFromDecayInFc(d / Fs);
		}
		void makeFromDecayInMs(Float d, Float Fs) noexcept
		{
			makeFromDecayInSamples(d * Fs * static_cast<Float>(.001));
		}

		Smooth(const bool _snap = false, const Float _startVal = static_cast<Float>(0)) :
			a0(static_cast<Float>(1)),
			b1(static_cast<Float>(0)),
			y1(_startVal),
			eps(static_cast<Float>(0)),
			snap(_snap),
			startVal(_startVal)
		{}
		void reset()
		{
			a0 = static_cast<Float>(1);
			b1 = static_cast<Float>(0);
			y1 = startVal;
			eps = static_cast<Float>(0);
		}
		void setX(Float x) noexcept
		{
			a0 = static_cast<Float>(1) - x;
			b1 = x;
			eps = a0 * static_cast<Float>(1.5);
		}
		void operator()(Float* buffer, Float val, int numSamples) noexcept
		{
			if (buffer[0] == val)
				return juce::FloatVectorOperations::fill(buffer, val, numSamples);
			for (auto s = 0; s < numSamples; ++s)
				buffer[s] = processSample(val);
		}
		void operator()(Float* buffer, int numSamples) noexcept
		{
			for (auto s = 0; s < numSamples; ++s)
				buffer[s] = processSample(buffer[s]);
		}
		Float operator()(Float sample) noexcept
		{
			return processSample(sample);
		}
	private:
		Float a0, b1, y1, eps, startVal;
		const bool snap;

		Float processSample(Float x0) noexcept
		{
			if (snap && std::abs(y1 - x0) < eps)
				y1 = x0;
			else
				y1 = x0 * a0 + y1 * b1;
			return y1;
		}
	};

	template<typename Float>
	inline void prepareParam(Smooth<Float>& smooth, std::vector<Float>& buf, Float smoothLen, Float sampleRate, int blockSize)
	{
		smooth.makeFromDecayInMs(smoothLen, sampleRate);
		buf.resize(blockSize, static_cast<Float>(0));
	}

	/********** struct Vec **********/
	template<typename Float>
	struct Vec2D
	{
		Vec2D(Float _x = static_cast<Float>(0), Float _y = static_cast<Float>(0)) :
			x(_x),
			y(_y)
		{}

		bool operator==(const Vec2D<Float>& other) const noexcept
		{
			return x == other.x && y == other.y;
		}
		bool operator!=(const Vec2D<Float>& other) const noexcept
		{
			return x != other.x || y != other.y;
		}

		Vec2D<Float> operator+(const Vec2D<Float>& other) const noexcept
		{
			return { x + other.x, y + other.y };
		}
		Vec2D<Float> operator-(const Vec2D<Float>& other) const noexcept
		{
			return { x - other.x, y - other.y };
		}
		Vec2D<Float> operator*(const Vec2D<Float>& other) const noexcept
		{
			return { x * other.x, y * other.y };
		}
		Vec2D<Float> operator/(const Vec2D<Float>& other) const noexcept
		{
			return { x / other.x, y / other.y };
		}

		Vec2D<Float> operator+(const Float other) const noexcept
		{
			return { x + other, y + other };
		}
		Vec2D<Float> operator-(const Float other) const noexcept
		{
			return { x - other, y - other };
		}
		Vec2D<Float> operator*(const Float other) const noexcept
		{
			return { x * other, y * other };
		}
		Vec2D<Float> operator/(const Float other) const noexcept
		{
			return { x / other, y / other };
		}

		void operator+=(const Vec2D<Float>& other) noexcept
		{
			x += other.x;
			y += other.y;
		}
		void operator-=(const Vec2D<Float>& other) noexcept
		{
			x -= other.x;
			y -= other.y;
		}
		void operator*=(const Vec2D<Float>& other) noexcept
		{
			x *= other.x;
			y *= other.y;
		}
		void operator/=(const Vec2D<Float>& other) noexcept
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

		Vec2D<Float> inv() const noexcept
		{
			return {
				static_cast<Float>(1) / x,
				static_cast<Float>(1) / y
			};
		}

		Float distSqr(const Vec2D<Float>& other) const noexcept
		{
			const auto x2 = other.x - x;
			const auto y2 = other.y - y;
			return x2 * x2 + y2 * y2;
		}
		Float dist(const Vec2D<Float>& other) const noexcept
		{
			return std::sqrt(distSqr(other));
		}
		Float angle(const Vec2D<Float>& other) const noexcept
		{
			return std::atan2(other.y - y, other.x - x);
		}

		Float x, y;
	};

	template<typename Float>
	inline bool operator==(const Vec2D<Float>& a, const Vec2D<Float>& b) noexcept
	{
		return a.x == b.x && a.y == b.y;
	}
	template<typename Float>
	inline bool operator!=(const Vec2D<Float>& a, const Vec2D<Float>& b) noexcept
	{
		return a.x != b.x || a.y != b.y;
	}

	/********** struct Move **********/
	template<typename Float, size_t NumEdges>
	struct Move
	{
		using numConst = constants::NumericConstants<Float>;
		static constexpr Float NumEdgesF = static_cast<Float>(NumEdges);

		Move() :
			sinBuf(),
			cosBuf()
		{
			const auto numEdgesInv = static_cast<Float>(1) / NumEdgesF;
			for (auto i = 0; i < NumEdges + 1; ++i)
			{
				const auto x = numConst::Tau * static_cast<Float>(i) * numEdgesInv - numConst::Pi;
				sinBuf[i] = std::sin(x);
				cosBuf[i] = std::cos(x);
			}
		}
		void operator()(Vec2D<Float>& vec, Float angle, Float mag) const noexcept
		{
			const auto idx = static_cast<size_t>(std::floor((angle + numConst::Pi) * numConst::TauInv * NumEdgesF));
			vec.x += cosBuf[idx] * mag;
			vec.y += sinBuf[idx] * mag;
		}
	private:
		std::array<Float, NumEdges + 1> sinBuf, cosBuf;
	};

	/********** struct Shared **********/
	/*
	template<typename Float>
	struct Shared
	{
		Shared() :
			move()
		{}

		const Move<Float, 64> move;
		static Shared<Float> shared;
	}; */

	// Singleton
	template<typename Float>
	struct Shared {
		const Move<Float, 64> move;

		static Shared<Float>& getInstance() {
			static Shared<Float> instance{};
			return instance;
		}

		Shared<Float>& operator=(Shared<Float> const& other) = delete;
		Shared<Float>(Shared<Float> const& other) = delete;

	private:
		Shared() : move() {}
	};

	/********** struct Downsample **********/
	template<typename Number>
	struct Downsample
	{
		Downsample(const int _order) :
			order(1 << _order)
		{

		}
		void prepare(Number sampleRate, int _blockSize) noexcept
		{
			const auto oInv = static_cast<Number>(1) / static_cast<Number>(order);
			Fs = sampleRate * oInv;
			blockSize = _blockSize * oInv;
		}
		bool doProcess() noexcept
		{
			++idx;
			if (idx == order)
			{
				idx = 0;
				return true;
			}
			return false;
		}
		Number Fs{ static_cast<Number>(48000) };

	private:
		int blockSize{ 64 };
		int idx{ 0 }; // or -1?
		const int order;
	};

	template<typename Float>
	juce::String toString(Vec2D<Float> vec)
	{
		return static_cast<juce::String>(vec.x) << "; " << static_cast<juce::String>(vec.y);
	}

	/********** struct Planet **********/
	template<typename Float>
	struct Planet
	{
		using numConst = constants::NumericConstants<Float>;

		Planet() :
			pos(),
			dir(),
			mass(static_cast<Float>(1)),
			radius(static_cast<Float>(.005)),
			angle(static_cast<Float>(0)),
			mag(static_cast<Float>(0))
		{}
		Planet(Vec2D<Float>&& _pos, Vec2D<Float>&& _dir, Float _mass, Float _radius) :
			pos(_pos),
			dir(_dir),
			mass(_mass),
			radius(_radius),
			angle(static_cast<Float>(0)),
			mag(static_cast<Float>(0))
		{}
		const Float getX() const noexcept { return pos.x; }
		const Float getY() const noexcept { return pos.y; }

		void savePatch(juce::ValueTree& state, const StateIDs& ids) const
		{
			juce::ValueTree planetChild(ids.planet);
			planetChild.setProperty(ids.posX, pos.x, nullptr);
			planetChild.setProperty(ids.posY, pos.y, nullptr);
			planetChild.setProperty(ids.dirX, dir.x, nullptr);
			planetChild.setProperty(ids.dirX, dir.y, nullptr);
			planetChild.setProperty(ids.radius, radius, nullptr);
			planetChild.setProperty(ids.mass, mass, nullptr);
			state.appendChild(planetChild, nullptr);
		}
		void loadPatch(const juce::ValueTree& state, const StateIDs& ids) noexcept
		{
			pos.x = static_cast<Float>(state.getProperty(ids.posX));
			pos.y = static_cast<Float>(state.getProperty(ids.posY));
			dir.x = static_cast<Float>(state.getProperty(ids.dirX));
			dir.y = static_cast<Float>(state.getProperty(ids.dirY));
			radius = static_cast<Float>(state.getProperty(ids.radius));
			mass = static_cast<Float>(state.getProperty(ids.mass));
		}

		bool gravitate(const Planet<Float>& other, const Float G,
			Float spaceMudVal = static_cast<Float>(1),
			Float attraction = static_cast<Float>(1)) noexcept
		{
			const auto distSqr = pos.distSqr(other.pos);
			const auto dist = std::sqrt(distSqr);
			const auto rad2 = radius + other.radius;
			const auto rad2sq = rad2 * rad2;
			const auto distSqrInv = static_cast<Float>(1) / distSqr;
			const auto velo = std::sqrt(dir.x * dir.x + dir.y * dir.y);
			angle = pos.angle(other.pos);

			bool collides = false;
			if (distSqr < rad2sq)
			{
				const Float innerRepel = 10.f;
				mag = ((G * mass * other.mass * distSqrInv * std::abs(attraction))
					/ rad2sq - innerRepel) / (rad2sq * rad2);
				speedLimit(static_cast<Float>(10e+03));
				Shared<Float>::getInstance().move(dir, angle, std::tanh(mag));
				collides = true;
			}
			else
			{
				mag = G * mass * other.mass * distSqrInv * attraction;
				speedLimit(static_cast<Float>(10e+03));
				Shared<Float>::getInstance().move(dir, angle, std::tanh(mag));
			}
			spaceMud(spaceMudVal);
			return collides;
		}
		void update() noexcept
		{
			pos += dir;
		}

		Vec2D<Float> pos, dir;
		Float mass, radius, angle, mag;
	private:
		// experimental functions:
		void collideSlowDown(Float coeff) noexcept
		{
			dir *= coeff;
		}

		void spaceMud(Float coeff) noexcept
		{
			dir *= coeff;
		}

		void speedLimit(Float threshold = static_cast<Float>(10e+04)) noexcept
		{
			mag = std::tanh(mag * threshold) / threshold;
		}
	};

	/********** struct CelestialBuffer **********/
	template<typename Float>
	struct CelestialBuffer
	{
		using numConst = constants::NumericConstants<Float>;
		using Plnt = Planet<Float>;
		using Buf = std::vector<Float>;
		using Smth = Smooth<Float>;

		CelestialBuffer() :
			phaseSmooth(), magSmooth(),
			phaseBuf(), magBuf()
		{}
		void prepare(Float sampleRate, int blockSize)
		{
			prepareParam(phaseSmooth, phaseBuf, static_cast<Float>(120), sampleRate, blockSize);
			prepareParam(magSmooth, magBuf, static_cast<Float>(20), sampleRate, blockSize);
		}
		void update(const Plnt& planet, int s) noexcept
		{
			magBuf[s] = std::tanh(planet.mag * static_cast<Float>(8000));
			phaseBuf[s] = planet.angle * planet.angle + planet.pos.x * planet.pos.y * numConst::Tau;
		}
		void makeSmooth(const float* depth, float ringBufferSize, int numSamples) noexcept
		{
			for (auto s = 0; s < numSamples; ++s)
			{
				phaseBuf[s] = ringBufferSize * depth[s] * (.5f * std::cos(phaseSmooth(phaseBuf[s])) + .5f);
				magBuf[s] = magSmooth(magBuf[s]);
			}
		}
		const Float* getPhaseBuf() const noexcept { return phaseBuf.data(); }
		const Float* getMagBuf() const noexcept { return magBuf.data(); }
	private:
		Smth phaseSmooth, magSmooth;
		Buf phaseBuf, magBuf;
	};

	/********** struct UniversalBuffer **********/
	template<typename Float, size_t NumPlanets>
	struct UniversalBuffer
	{
		using Plnt = Planet<Float>;
		using Celest = CelestialBuffer<Float>;
		using Buffer = std::array<Celest, NumPlanets>;
		using ParamBuf = std::vector<Float>;

		UniversalBuffer() :
			buffer(),
			depthSmooth(),
			depthBuf()
		{}
		void prepare(Float sampleRate, int blockSize)
		{
			for(auto& cb: buffer)
				cb.prepare(sampleRate, blockSize);
			prepareParam(depthSmooth, depthBuf, static_cast<Float>(20), static_cast<Float>(sampleRate), blockSize);
		}
		void update(const Plnt& planet, int p, int s) noexcept
		{
			buffer[p].update(planet, s);
		}
		void makeSmooth(float ringBufferSize, float depth, int numSamples, int numPlanets = NumPlanets) noexcept
		{
			depthSmooth(depthBuf.data(), depth, numSamples);
			for (auto c = 0; c < numPlanets; ++c)
			{
				auto& celest = buffer[c];
				celest.makeSmooth(depthBuf.data(), ringBufferSize, numSamples);
			}
		}
		const Celest& operator[](int p) const noexcept { return buffer[p]; }
	private:
		Buffer buffer;
		Smooth<Float> depthSmooth;
		ParamBuf depthBuf;
	};

	/********** struct Processor **********/
	template<typename Float, size_t NumPlanets>
	struct Processor
	{
		using numConst = constants::NumericConstants<Float>;

		static constexpr Float Gravity = static_cast<Float>(.001);
		static constexpr Float MinPos = static_cast<Float>(-1);
		static constexpr Float MaxPos = static_cast<Float>(1);
		static constexpr Float RangePos = MaxPos - MinPos;

		using Planets = std::array<Planet<Float>, NumPlanets>;
		using UniBuf = UniversalBuffer<Float, NumPlanets>;

		Processor(int _downsampleOrder) :
			planets(),
			sampleRate(static_cast<Float>(48000)),
			sampleRateInv(Gravity * static_cast<Float>(1) / sampleRate),
			downsample(_downsampleOrder)
		{}

		void savePatch(juce::ValueTree& state) const
		{
			StateIDs ids;
			auto orbitChild = state.getChildWithName(ids.orbit);
			if (!orbitChild.isValid())
			{
				orbitChild = juce::ValueTree(ids.orbit);
				state.appendChild(orbitChild, nullptr);
			}
			else
				orbitChild.removeAllChildren(nullptr);
			for (auto p = 0; p < planets.size(); ++p)
				planets[p].savePatch(orbitChild, ids);
		}
		void loadPatch(const juce::ValueTree& state) noexcept
		{
			StateIDs ids;
			auto orbitChild = state.getChildWithName(ids.orbit);
			if (!orbitChild.isValid())
				return;
			for (auto p = 0; p < planets.size(); ++p)
				planets[p].loadPatch(orbitChild.getChild(p), ids);
		}

		void randomizePlanetPositions() noexcept
		{
			juce::Random rand;
			for (auto& p : planets)
			{
				p.pos.x = static_cast<Float>(rand.nextDouble() * 2. - 1.);
				p.pos.y = static_cast<Float>(rand.nextDouble() * 2. - 1.);
			}
		}
		void giveBirthToPlanet(Planet<Float>&& pl, int p) noexcept
		{
			planets[p] = pl;
		}
		void giveBirthWithRandomProperties(int p) noexcept
		{
			juce::Random rand;
			planets[p].pos.x = static_cast<Float>(rand.nextDouble() * 2. - 1.);
			planets[p].pos.y = static_cast<Float>(rand.nextDouble() * 2. - 1.);
			planets[p].mass = static_cast<Float>(.3 + rand.nextDouble() * (.9 - .3));
			planets[p].radius = static_cast<Float>(.001 + rand.nextDouble() * (.01 - .001));
		}

		void prepare(Float _sampleRate, int _blockSize)
		{
			downsample.prepare(_sampleRate, _blockSize);
			sampleRate = downsample.Fs;
			sampleRateInv = static_cast<Float>(1) / sampleRate;
		}

		void processBlock(UniBuf& uniBuf, int numSamples,
			int _numPlanets = NumPlanets,
			Float gravity = Gravity,
			Float spaceMud = 1.f,
			Float attraction = 1.f) noexcept
		{
			numPlanets.store(_numPlanets);

			for (auto s = 0; s < numSamples; ++s)
			{
				if (downsample.doProcess())
					processSample(_numPlanets, gravity, spaceMud, attraction);
				for (auto i = 0; i < _numPlanets; ++i)
					uniBuf.update(planets[i], i, s);
			}
		}

		const Planets& getPlanets() const noexcept
		{
			return planets;
		}
		int getNumPlanets() const noexcept { return numPlanets.load(); }
	private:
		Planets planets;
		double sampleRate, sampleRateInv;
		Downsample<Float> downsample;
		std::atomic<int> numPlanets;

		void processSample(int _numPlanets, Float gravity, Float spaceMud, Float attraction) noexcept
		{
			const auto numPlanetsInv = static_cast<Float>(1) / static_cast<Float>(_numPlanets);

			bool needBigBang = true;
			for (auto i = 0; i < _numPlanets; ++i)
			{
				auto& p0 = planets[i];
				for (auto j = 0; j < _numPlanets; ++j)
				{
					const bool samePlanet = i == j;
					if (!samePlanet)
						if (!p0.gravitate(
							planets[j],
							sampleRateInv * numPlanetsInv * gravity,
							spaceMud,
							attraction
						))
						{
							needBigBang = false;
						}
				}
				p0.update();
			}
			topologyBillard(_numPlanets);
			if(needBigBang)
				bigBang(_numPlanets);
		}

		void topologyBillard(int _numPlanets)
		{
			static constexpr auto Repell = static_cast<Float>(-1);

			for (auto i = 0; i < _numPlanets; ++i)
			{
				auto& planet = planets[i];

				if (planet.pos.x < MinPos || planet.pos.x > MaxPos)
					planet.dir.x *= Repell;
				if (planet.pos.y < MinPos || planet.pos.y > MaxPos)
					planet.dir.y *= Repell;
			}
		}
		/*
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
		} */

		void bigBang(int _numPlanets) noexcept
		{
			for (auto p = 0; p < _numPlanets; ++p)
			{
				auto& planet = planets[p];
				planet.angle = static_cast<Float>(p) / static_cast<Float>(_numPlanets) * numConst::Tau - numConst::Pi;
				planet.mag = .01f;
				Shared<Float>::getInstance().move(planet.dir, planet.angle, std::tanh(planet.mag));
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

	/********** struct WriteHead **********/
	struct WriteHead
	{
		using Buffer = std::vector<int>;

		WriteHead() :
			buffer(),
			wHead(-1)
		{}
		void prepare(int blockSize)
		{
			buffer.resize(blockSize, 0);
		}
		void operator()(int numSamples, int ringBufferSize) noexcept
		{
			for (auto s = 0; s < numSamples; ++s)
			{
				++wHead;
				if (wHead == ringBufferSize)
					wHead = 0;
				buffer[s] = wHead;
			}
		}
		Buffer buffer;
	private:
		int wHead;
	};

	template<typename Float>
	inline Float cubicHermiteSpline(const Float* buffer, const Float readHead, const int size) noexcept
	{
		const auto iFloor = std::floor(readHead);
		auto i1 = static_cast<int>(iFloor);
		auto i0 = i1 - 1;
		auto i2 = i1 + 1;
		auto i3 = i1 + 2;
		if (i3 >= size) i3 -= size;
		if (i2 >= size) i2 -= size;
		if (i0 < 0) i0 += size;

		const auto t = readHead - iFloor;
		const auto v0 = buffer[i0];
		const auto v1 = buffer[i1];
		const auto v2 = buffer[i2];
		const auto v3 = buffer[i3];

		const auto c0 = v1;
		const auto c1 = static_cast<Float>(.5) * (v2 - v0);
		const auto c2 = v0 - static_cast<Float>(2.5) * v1 + static_cast<Float>(2) * v2 - static_cast<Float>(.5) * v3;
		const auto c3 = static_cast<Float>(1.5) * (v1 - v2) + static_cast<Float>(.5) * (v3 - v0);

		return ((c3 * t + c2) * t + c1) * t + c0;
	}

	/********** struct Delay **********/
	template<typename Float>
	struct Delay
	{

		using RingBuffer = std::array<std::vector<Float>, 2>;
		using WHeadBuf = std::vector<int>;
		using CelestialBuf = CelestialBuffer<Float>;
		using Samples = std::array<std::vector<Float>, 2>;

		Delay() :
			ringBuffer(),
			ringBufferSizeF(static_cast<Float>(1)),
			ringBufferSize(1)
		{}
		void prepare(double sampleRate, int blockSize)
		{
			ringBufferSize = static_cast<int>(sampleRate * .04); // 40ms
			ringBufferSizeF = static_cast<Float>(ringBufferSize);
			for(auto& r: ringBuffer)
				r.resize(ringBufferSize + 4);
		}
		void operator()(Samples& samples, const int* wHead, const CelestialBuf& celestial,
			int numChannels, int numSamples) noexcept
		{
			const auto phase = celestial.getPhaseBuf();
			const auto mag = celestial.getMagBuf();

			for (auto ch = 0; ch < numChannels; ++ch)
			{
				auto ring = ringBuffer[ch].data();
				auto smpls = samples[ch].data();
				for (auto s = 0; s < numSamples; ++s)
				{
					const auto w = wHead[s];
					
					auto r = static_cast<Float>(w) - phase[s];
					if (r < static_cast<Float>(0))
						r += ringBufferSize;

					ring[w] = ring[w] * -mag[s] + smpls[s];
					smpls[s] = cubicHermiteSpline(ring, r, ringBufferSize);
				}
			}
		}
		int getRingBufferSize() const noexcept { return ringBufferSize; }
		Float getRingBufferSizeF() const noexcept { return ringBufferSizeF; }
	private:
		RingBuffer ringBuffer;
		Float ringBufferSizeF;
		int ringBufferSize;
	};

	/********** struct Delays **********/
	template<typename Float, size_t NumPlanets>
	struct Delays
	{
		using Delay = Delay<Float>;
		using DelayBuf = std::array<Delay, NumPlanets>;
		using UniBuf = UniversalBuffer<Float, NumPlanets>;
		using Samples = std::array<std::array<std::vector<Float>, 2>, NumPlanets>;

		Delays() :
			wHead(),
			delays()
		{}
		void prepare(double sampleRate, int blockSize)
		{
			wHead.prepare(blockSize);
			for (auto& delay : delays)
				delay.prepare(sampleRate, blockSize);
		}
		void operator()(Samples& samples, const UniBuf& uniBuf,
			int numPlanets, int numChannels, int numSamples) noexcept
		{
			wHead(numSamples, delays[0].getRingBufferSize());
			for (auto d = 0; d < numPlanets; ++d)
			{
				auto& delay = delays[d];
				delay(
					samples[d],
					wHead.buffer.data(),
					uniBuf[d],
					numChannels,
					numSamples
				);
			}
		}
		Float getRingBufferSizeF() const noexcept { return delays[0].getRingBufferSizeF(); }
	private:
		WriteHead wHead;
		DelayBuf delays;
	};
}