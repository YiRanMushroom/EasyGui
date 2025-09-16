export module EasyGui.std_extensions;

import std;

namespace EasyGui {
    template<auto & _Fn, typename _Tp>
    struct _Wrap_Fn_Impl {
        static_assert(false, "Not a function type");
    };

    template<typename _Ret, typename... _Args, _Ret(&_Fn)(_Args...)>
    struct _Wrap_Fn_Impl<_Fn, _Ret(_Args...)> {
        static_assert(std::is_same_v<decltype(_Fn), _Ret(&)(_Args...)>, "Function signature mismatch");
        using reference = _Ret(&)(_Args...);

        constexpr static reference value = _Fn;

        constexpr static _Ret operator()(_Args... args) noexcept(noexcept(_Fn)) {
            return _Fn(std::forward<_Args>(args)...);
        }

        constexpr static _Ret Invoke(_Args... args) noexcept(noexcept(_Fn)) {
            return _Fn(std::forward<_Args>(args)...);
        }
    };

    export template<auto & _Fn>
    constexpr inline auto WrapFnOf = _Wrap_Fn_Impl<_Fn, std::remove_reference_t<decltype(_Fn)>>{};

    export template<auto& _Fn>
    using WrapFnTypeOf = decltype(WrapFnOf<_Fn>);
}
