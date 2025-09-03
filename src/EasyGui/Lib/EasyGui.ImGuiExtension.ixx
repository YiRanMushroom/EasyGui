export module EasyGui.ImGuiExtension;

import EasyGui.LibBasic;
import std.compat;

namespace ImGui {
    export template<typename... Args>
    void TextFmt(std::format_string<const Args &...> fmt, const Args &... args) {
        ImGui::Text("%s", std::format(fmt, args...).c_str());
    }

    export void BeginChild(std::source_location location = std::source_location::current()) {
        const char buffer[1024]{};
        std::snprintf(const_cast<char *>(buffer), sizeof(buffer), "Child_%s_%d_%d", location.file_name(),
                      location.line(), location.column());
        ImGui::BeginChild(buffer);
    }
}
