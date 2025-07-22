export module EasyGui.Core.KeyCodes;

import std.compat;

import "../Lib/Lib_SDL3.hpp";

namespace Key {
    export using KeyCode = uint32_t;

    export enum : uint32_t {
        // From SDL3.h
        Space = SDLK_SPACE,
        Apostrophe = SDLK_APOSTROPHE, /* ' */
        Comma = SDLK_COMMA, /* , */
        Minus = SDLK_MINUS, /* - */
        Period = SDLK_PERIOD, /* . */
        Slash = SDLK_SLASH, /* / */

        D0 = SDLK_0, /* 0 */
        D1 = SDLK_1, /* 1 */
        D2 = SDLK_2, /* 2 */
        D3 = SDLK_3, /* 3 */
        D4 = SDLK_4, /* 4 */
        D5 = SDLK_5, /* 5 */
        D6 = SDLK_6, /* 6 */
        D7 = SDLK_7, /* 7 */
        D8 = SDLK_8, /* 8 */
        D9 = SDLK_9, /* 9 */

        Semicolon = SDLK_SEMICOLON, /* ; */
        Equal = SDLK_EQUALS, /* = */

        A = SDLK_A, /* A */
        B = SDLK_B, /* B */
        C = SDLK_C, /* C */
        D = SDLK_D, /* D */
        E = SDLK_E, /* E */
        F = SDLK_F, /* F */
        G = SDLK_G, /* G */
        H = SDLK_H, /* H */
        I = SDLK_I, /* I */
        J = SDLK_J, /* J */
        K = SDLK_K, /* K */
        L = SDLK_L, /* L */
        M = SDLK_M, /* M */
        N = SDLK_N, /* N */
        O = SDLK_O, /* O */
        P = SDLK_P, /* P */
        Q = SDLK_Q, /* Q */
        R = SDLK_R, /* R */
        S = SDLK_S, /* S */
        T = SDLK_T, /* T */
        U = SDLK_U, /* U */
        V = SDLK_V, /* V */
        W = SDLK_W, /* W */
        X = SDLK_X, /* X */
        Y = SDLK_Y, /* Y */
        Z = SDLK_Z, /* Z */

        LeftBracket = SDLK_LEFTBRACKET, /* [ */
        Backslash = SDLK_BACKSLASH, /* \ */
        RightBracket = SDLK_RIGHTBRACKET, /* ] */
        GraveAccent = SDLK_GRAVE, /* ` */

        // World1 = 161, /* non-US #1 */
        // World2 = 162, /* non-US #2 */

        /* Function keys */
        Escape = SDLK_ESCAPE,
        Enter = SDLK_RETURN,
        Tab = SDLK_TAB,
        Backspace = SDLK_BACKSPACE,
        // Insert = 260,
        Insert = SDLK_INSERT,
        // Delete = 261,
        Delete = SDLK_DELETE,
        // Right = 262,
        // Left = 263,
        // Down = 264,
        // Up = 265,
        Right = SDLK_RIGHT,
        Left = SDLK_LEFT,
        Down = SDLK_DOWN,
        Up = SDLK_UP,
        // PageUp = 266,
        // PageDown = 267,
        // Home = 268,
        // End = 269,
        PageUp = SDLK_PAGEUP,
        PageDown = SDLK_PAGEDOWN,
        Home = SDLK_HOME,
        End = SDLK_END,
        // CapsLock = 280,
        // ScrollLock = 281,
        // NumLock = 282,
        // PrintScreen = 283,
        // Pause = 284,
        CapsLock = SDLK_CAPSLOCK,
        ScrollLock = SDLK_SCROLLLOCK,
        NumLock = SDLK_NUMLOCKCLEAR,
        PrintScreen = SDLK_PRINTSCREEN,
        Pause = SDLK_PAUSE,
        // F1 = 290,
        // F2 = 291,
        // F3 = 292,
        // F4 = 293,
        // F5 = 294,
        // F6 = 295,
        // F7 = 296,
        // F8 = 297,
        // F9 = 298,
        // F10 = 299,
        // F11 = 300,
        // F12 = 301,
        // F13 = 302,
        // F14 = 303,
        // F15 = 304,
        // F16 = 305,
        // F17 = 306,
        // F18 = 307,
        // F19 = 308,
        // F20 = 309,
        // F21 = 310,
        // F22 = 311,
        // F23 = 312,
        // F24 = 313,
        // F25 = 314,
        // all the way to F24
        F1 = SDLK_F1,
        F2 = SDLK_F2,
        F3 = SDLK_F3,
        F4 = SDLK_F4,
        F5 = SDLK_F5,
        F6 = SDLK_F6,
        F7 = SDLK_F7,
        F8 = SDLK_F8,
        F9 = SDLK_F9,
        F10 = SDLK_F10,
        F11 = SDLK_F11,
        F12 = SDLK_F12,
        F13 = SDLK_F13,
        F14 = SDLK_F14,
        F15 = SDLK_F15,
        F16 = SDLK_F16,
        F17 = SDLK_F17,
        F18 = SDLK_F18,
        F19 = SDLK_F19,
        F20 = SDLK_F20,
        F21 = SDLK_F21,
        F22 = SDLK_F22,
        F23 = SDLK_F23,
        F24 = SDLK_F24,

        /* Keypad */
        KP0 = SDLK_KP_0,
        KP1 = SDLK_KP_1,
        KP2 = SDLK_KP_2,
        KP3 = SDLK_KP_3,
        KP4 = SDLK_KP_4,
        KP5 = SDLK_KP_5,
        KP6 = SDLK_KP_6,
        KP7 = SDLK_KP_7,
        KP8 = SDLK_KP_8,
        KP9 = SDLK_KP_9,
        KPDecimal = SDLK_KP_DECIMAL,
        KPDivide = SDLK_KP_DIVIDE,
        KPMultiply = SDLK_KP_MULTIPLY,
        KPSubtract = SDLK_KP_MINUS,
        KPAdd = SDLK_KP_PLUS,
        KPEnter = SDLK_KP_ENTER,
        KPEqual = SDLK_KP_EQUALS,

        LeftShift = SDLK_LSHIFT,
        LeftControl = SDLK_LCTRL,
        LeftAlt = SDLK_LALT,
        LeftSuper = SDLK_LGUI,
        RightShift = SDLK_RSHIFT,
        RightControl = SDLK_RCTRL,
        RightAlt = SDLK_RALT,
        RightSuper = SDLK_RGUI,
        Menu = SDLK_MENU,
    };
}
