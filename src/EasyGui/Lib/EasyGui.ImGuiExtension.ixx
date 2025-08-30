export module EasyGui.ImGuiExtension;

import EasyGui.LibBasic;
import std.compat;

namespace ImGui {
    export template<typename... Args>
    void TextFmt(std::format_string<const Args&...> fmt, const Args&... args) {
        ImGui::Text("%s", std::format(fmt, args...).c_str());
    }
}