export module EasyGui.Tools.Misc;

import std.compat;

namespace EasyGui::Tools {
    template<std::integral T>
    T GenerateRandomIntegral() {
        static std::random_device rd;
        static std::mt19937 generator(rd());
        return std::uniform_int_distribution<T>(std::numeric_limits<T>::min(), std::numeric_limits<T>::max())(generator);
    }

    template<std::floating_point T>
    T GenerateRandomReal() {
        static std::random_device rd;
        static std::mt19937 generator(rd());
        return std::uniform_real_distribution<T>(std::numeric_limits<T>::min(), std::numeric_limits<T>::max())(generator);
    }
}