#pragma once
#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_gui_extra/juce_gui_extra.h>

namespace ui
{
    template<typename Float>
    inline juce::Rectangle<Float> maxQuadIn(const juce::Rectangle<Float>& b) noexcept
    {
        const auto minDimen = std::min(b.getWidth(), b.getHeight());
        const auto x = b.getX() + static_cast<Float>(.5) * (b.getWidth() - minDimen);
        const auto y = b.getY() + static_cast<Float>(.5) * (b.getHeight() - minDimen);
        return { x, y, minDimen, minDimen };
    }

    class Layout
    {
    public:
        Layout(const std::vector<float> xxx, const std::vector<float> yyy) :
            rXRaw(),
            rYRaw(),
            rX(),
            rY()
        {
            rXRaw.resize(xxx.size() + 1, 0.f);
            rYRaw.resize(yyy.size() + 1, 0.f);
            auto sum = 0.f;
            for (auto x = 0; x < xxx.size(); ++x)
            {
                rXRaw[x + 1] = xxx[x];
                sum += xxx[x];
            }
            auto gain = 1.f / sum;
            for (auto& x : rXRaw) x *= gain;

            sum = 0.f;
            for (auto y = 0; y < yyy.size(); ++y)
            {
                rYRaw[y + 1] = yyy[y];
                sum += yyy[y];
            }
            gain = 1.f / sum;
            for (auto& y : rYRaw) y *= gain;

            for (auto x = 1; x < rXRaw.size(); ++x)
                rXRaw[x] += rXRaw[x - 1];
            for (auto y = 1; y < rYRaw.size(); ++y)
                rYRaw[y] += rYRaw[y - 1];

            rX.resize(rXRaw.size(), 0.f);
            rY.resize(rYRaw.size(), 0.f);
        }
        // SET
        void setBounds(const juce::Rectangle<float>& bounds) noexcept
        {
            for (auto x = 0; x < rX.size(); ++x)
                rX[x] = rXRaw[x] * bounds.getWidth();
            for (auto y = 0; y < rY.size(); ++y)
                rY[y] = rYRaw[y] * bounds.getHeight();
            for (auto& x : rX) x += bounds.getX();
            for (auto& y : rY) y += bounds.getY();
        }
        // PROCESS
        void place(juce::Component& comp, int x, int y, int width = 1, int height = 1, float padding = 0.f, bool isQuad = false)
        {
            const auto cBounds = this->operator()(x, y, width, height);
            if (!isQuad)
                if (padding != 0.f)
                    comp.setBounds(cBounds.reduced(padding).toNearestInt());
                else
                    comp.setBounds(cBounds.toNearestInt());
            else
                if (padding != 0.f)
                    comp.setBounds(maxQuadIn(cBounds).reduced(padding).toNearestInt());
                else
                    comp.setBounds(maxQuadIn(cBounds).toNearestInt());
        }
        void place(juce::Component* comp, int x, int y, int width = 1, int height = 1, float padding = 0.f, bool isQuad = false)
        {
            if (comp == nullptr) return;
            place(*comp, x, y, width, height, padding, isQuad);
        }
        juce::Rectangle<float> operator()() const noexcept
        {
            return
            {
                rX[0],
                rY[0],
                rX[rX.size() - 1] - rX[0],
                rY[rY.size() - 1] - rY[0]
            };
        }
        juce::Rectangle<float> operator()(int x, int y, int width = 1, int height = 1, float padding = 0.f, bool isQuad = false) const noexcept
        {
            juce::Rectangle<float> nBounds(rX[x], rY[y], rX[x + width] - rX[x], rY[y + height] - rY[y]);
            if (!isQuad)
                if (padding == 0.f)
                    return nBounds;
                else
                    return nBounds.reduced(padding);
            else
                if (padding == 0.f)
                    return maxQuadIn(nBounds);
                else
                    return maxQuadIn(nBounds).reduced(padding);
        }
        juce::Rectangle<float> bottomBar() const noexcept
        {
            return
            {
                rX[0],
                rY[rY.size() - 2],
                rX[rX.size() - 1] - rX[0],
                rY[rY.size() - 1] - rY[rY.size() - 2]
            };
        }
        juce::Rectangle<float> topBar() const noexcept
        {
            return
            {
                rX[0],
                rY[0],
                rX[rX.size() - 1] - rX[0],
                rY[1] - rY[0]
            };
        }
        juce::Rectangle<float> exceptBottomBar() const noexcept
        {
            return
            {
                rX[0],
                rY[0],
                rX[rX.size() - 1] - rX[0],
                rY[rY.size() - 2] - rY[0]
            };
        }
    private:
        std::vector<float> rXRaw, rYRaw, rX, rY;
    };
}