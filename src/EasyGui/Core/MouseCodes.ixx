export module EasyGui.Core.MouseCodes;

import std.compat;

import "../Lib/Lib_SDL3.hpp";

namespace Mouse {
    export using MouseCode = uint16_t;

    export enum : uint16_t {
        ButtonLeft = SDL_BUTTON_LEFT,
        ButtonRight = SDL_BUTTON_RIGHT,
        ButtonMiddle = SDL_BUTTON_MIDDLE,
    };
}
