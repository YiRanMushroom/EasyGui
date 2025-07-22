export module EasyGui.SoLoud;

export import SoLoud;
import std.compat;

namespace EasyGui {
    export class SoLoadLibrary {
    public:
        static SoLoadLibrary Create() {
            return SoLoadLibrary();
        }

        SoLoud::Soloud& GetSoLoudInstance() const {
            return *m_SoLoudInstance;
        }

        SoLoud::Soloud* operator->() const {
            return m_SoLoudInstance.get();
        }

        void Release() {
            m_SoLoudInstance->deinit();
            m_SoLoudInstance.reset();
        }

        explicit operator bool() const {
            return m_SoLoudInstance != nullptr;
        }

        bool IsValid() const {
            return m_SoLoudInstance != nullptr;
        }

        ~SoLoadLibrary() {
            m_SoLoudInstance->deinit();
        }

    private:
        SoLoadLibrary() : m_SoLoudInstance(std::make_unique<SoLoud::Soloud>()) {
            m_SoLoudInstance->init();
        }

        std::unique_ptr<SoLoud::Soloud> m_SoLoudInstance;
    };
}