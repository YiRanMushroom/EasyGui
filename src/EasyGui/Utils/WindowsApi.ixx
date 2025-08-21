export module EasyGui.Utils.WindowsApi;

import std.compat;
export import <Windows.h>;
import <shlobj.h>;
import <direct.h>;

#undef MAX_PATH

constexpr size_t MAX_PATH = 1024;

namespace EasyGui::Windows {
    export class PreserveWorkingDirectory {
    public:
        PreserveWorkingDirectory() {
            _wgetcwd(m_OriginalDirectory, MAX_PATH);
        }

        ~PreserveWorkingDirectory() {
            _wchdir(m_OriginalDirectory);
        }

    private:
        wchar_t m_OriginalDirectory[MAX_PATH];
    };

    export std::wstring Utf8ToUtf16(std::string_view utf8) {
        int len = MultiByteToWideChar(CP_UTF8, 0, utf8.data(), (int) utf8.size(), nullptr, 0);
        std::wstring wstr(len, 0);
        MultiByteToWideChar(CP_UTF8, 0, utf8.data(), (int) utf8.size(), wstr.data(), len);
        return wstr;
    }

    export std::string Utf16ToUtf8(const std::wstring &wstr) {
        int len = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int) wstr.size(), nullptr, 0, nullptr, nullptr);
        std::string str(len, 0);
        WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int) wstr.size(), str.data(), len, nullptr, nullptr);
        return str;
    }

    export std::optional<std::string> OpenFileDialog(std::wstring_view filter = L"All Files\0*.*\0") {
        PreserveWorkingDirectory preserveDir;
        wchar_t filename[MAX_PATH] = L"";

        OPENFILENAMEW ofn;
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = NULL;
        ofn.lpstrFilter = filter.data();
        ofn.lpstrFile = filename;
        ofn.nMaxFile = MAX_PATH;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

        std::optional<std::string> result;
        if (GetOpenFileNameW(&ofn)) {
            result = Utf16ToUtf8(filename);
        }

        return result;
    }

    export std::optional<std::wstring> OpenDirectoryDialog(std::wstring_view title = L"Select a folder") {
        PreserveWorkingDirectory preserveDir;
        wchar_t path[MAX_PATH] = L"";

        BROWSEINFOW bi = {0};
        bi.lpszTitle = title.data();
        bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
        LPITEMIDLIST pidl = SHBrowseForFolderW(&bi);

        if (pidl && SHGetPathFromIDListW(pidl, path)) {
            CoTaskMemFree(pidl);
            return path;
        }
        if (pidl) CoTaskMemFree(pidl);
        return {};
    }

    export struct ProcessOutput {
        std::wstring stdout_str;
        std::wstring stderr_str;
    };

    // ShowErrorMessage: UTF-8 support
    export void ShowErrorMessage(std::wstring_view message, std::wstring_view title = L"Error") {
        MessageBoxW(NULL, message.data(), title.data(), MB_OK | MB_ICONERROR);
    }

    export void SetClipboardContent(std::wstring_view content) {
        if (OpenClipboard(NULL)) {
            EmptyClipboard();

            size_t size = (content.length() + 1) * sizeof(wchar_t);
            HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, size);
            if (hGlobal != nullptr) {
                wchar_t *buffer = static_cast<wchar_t *>(GlobalLock(hGlobal));
                if (buffer != nullptr) {
                    memcpy(buffer, content.data(), size);
                    GlobalUnlock(hGlobal);

                    SetClipboardData(CF_UNICODETEXT, hGlobal);
                }
            }
            CloseClipboard();
        }
    }

    // RunProcessWithOutput: UTF-8 support
    export std::optional<ProcessOutput> RunProcessWithOutput(const std::wstring_view cmd) {
        HANDLE outRead, outWrite, errRead, errWrite;
        SECURITY_ATTRIBUTES sa{sizeof(sa), NULL, TRUE};

        if (!CreatePipe(&outRead, &outWrite, &sa, 0)) return std::nullopt;
        if (!CreatePipe(&errRead, &errWrite, &sa, 0)) {
            CloseHandle(outRead);
            CloseHandle(outWrite);
            return std::nullopt;
        }

        STARTUPINFOW si = {sizeof(si)};
        si.dwFlags = STARTF_USESTDHANDLES;
        si.hStdOutput = outWrite;
        si.hStdError = errWrite;
        si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);

        PROCESS_INFORMATION pi{};

        BOOL success = CreateProcessW(
            NULL, const_cast<wchar_t *>(cmd.data()), NULL, NULL, TRUE,
            CREATE_NO_WINDOW, NULL, NULL, &si, &pi
        );

        CloseHandle(outWrite);
        CloseHandle(errWrite);

        if (!success) {
            CloseHandle(outRead);
            CloseHandle(errRead);
            ShowErrorMessage(
                L"Failed to create process. Please check if all required files are present and the command is correct.");
            return std::nullopt;
        }

        std::string outStr, errStr;
        char buf[4096];
        DWORD read;

        while (ReadFile(outRead, buf, sizeof(buf), &read, NULL) && read)
            outStr.append(buf, read);

        while (ReadFile(errRead, buf, sizeof(buf), &read, NULL) && read)
            errStr.append(buf, read);

        CloseHandle(outRead);
        CloseHandle(errRead);

        WaitForSingleObject(pi.hProcess, INFINITE);
        DWORD exitCode = 0;
        GetExitCodeProcess(pi.hProcess, &exitCode);

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        if (exitCode != 0) {
            ShowErrorMessage(
                std::format(L"Process exited abnormally. Exit code: {}. Please report this issue to the vendor.",
                            exitCode).c_str());
        }

        return ProcessOutput{
            Utf8ToUtf16(outStr),
            Utf8ToUtf16(errStr)
        };
    }
}
