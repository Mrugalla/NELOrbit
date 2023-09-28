/*
  ==============================================================================

    GUIBasics.cpp
    Created: 28 Sep 2023 6:29:44am
    Author:  christoph

  ==============================================================================
*/

#include "GUIBasics.h"

namespace ui {
    Layout::Layout(const std::vector<float> xBounds, const std::vector<float> yBounds) : 
        rXRaw(),
        rYRaw(),
        rX(),
        rY() 
    { 
        rXRaw.resize(xBounds.size() + 1, 0.f);
        rYRaw.resize(yBounds.size() + 1, 0.f);
        
        auto sum = 0.f;
        for (auto x = 0; x < xBounds.size(); ++x)
        {
            rXRaw[x + 1] = xBounds[x];
            sum += xBounds[x];
        }
        auto totalXInv = 1.f / sum;
        for (auto& x : rXRaw) x *= totalXInv;

        sum = 0.f;
        for (auto y = 0; y < yBounds.size(); ++y)
        {
            rYRaw[y + 1] = yBounds[y];
            sum += yBounds[y];
        }

        auto totalYInv = 1.f / sum;
        for (auto& y : rYRaw) y *= totalYInv;

        for (auto x = 1; x < rXRaw.size(); ++x)
            rXRaw[x] += rXRaw[x - 1];
        for (auto y = 1; y < rYRaw.size(); ++y)
            rYRaw[y] += rYRaw[y - 1];
            
        rX.resize(rXRaw.size(), 0.f);
        rY.resize(rYRaw.size(), 0.f); 
    }

    void Layout::setBounds(const juce::Rectangle<float>& bounds) noexcept
    {
        for (auto x = 0; x < rX.size(); ++x)
            rX[x] = rXRaw[x] * bounds.getWidth();
        for (auto y = 0; y < rY.size(); ++y)
            rY[y] = rYRaw[y] * bounds.getHeight();
        for (auto& x : rX) x += bounds.getX();
        for (auto& y : rY) y += bounds.getY();
    }

    void Layout::place(juce::Component& comp, int posX, int posY, int width, int height, float padding, bool isQuad)
    {
        const auto cBounds = this->operator()(posX, posY, width, height);
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

    juce::Rectangle<float> Layout::operator()() const noexcept
    {
        return
        {
            rX[0],
            rY[0],
            rX[rX.size() - 1] - rX[0],
            rY[rY.size() - 1] - rY[0]
        };
    }

    juce::Rectangle<float> Layout::operator()(int x, int y, int width, int height, float padding, 
        bool isQuad) const noexcept
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

    juce::Rectangle<float> Layout::bottomBar() const noexcept
    {
        return
        {
            rX[0],
            rY[rY.size() - 2],
            rX[rX.size() - 1] - rX[0],
            rY[rY.size() - 1] - rY[rY.size() - 2]
        };
    }
    juce::Rectangle<float> Layout::topBar() const noexcept
    {
        return
        {
            rX[0],
            rY[0],
            rX[rX.size() - 1] - rX[0],
            rY[1] - rY[0]
        };
    }
    juce::Rectangle<float> Layout::exceptBottomBar() const noexcept
    {
        return
        {
            rX[0],
            rY[0],
            rX[rX.size() - 1] - rX[0],
            rY[rY.size() - 2] - rY[0]
        };
    }
} // namespace ui
