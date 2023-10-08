/*
  ==============================================================================

    Constants.h
    Created: 5 Oct 2023 6:44:32am
    Author:  christoph

  ==============================================================================
*/

#pragma once

namespace constants {
    template<typename Float>
    struct NumericConstants {
        static constexpr Float Pi = static_cast<Float>(3.14159265359);
        static constexpr Float Tau = static_cast<Float>(6.28318530718f);
        static constexpr Float PiHalf = static_cast<Float>(1.57079632679f);
        static constexpr Float PiQuart = static_cast<Float>(.785398163397f);
        static constexpr Float PiInv = static_cast<Float>(1.f / Pi);
        static constexpr Float E = static_cast<Float>(2.71828182846);

        static constexpr Float TauInv = static_cast<Float>(1.0f/Tau);
    };

} // namespace constants