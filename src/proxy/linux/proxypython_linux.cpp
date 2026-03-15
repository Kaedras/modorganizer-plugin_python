#include "../proxypython.h"

#include <QCoreApplication>
#include <QMessageBox>

#include <dlfcn.h>
#include <filesystem>
#include <uibase/log.h>

namespace fs = std::filesystem;
using namespace MOBase;

// retrieve the path to the folder containing the proxy DLL
fs::path getPluginFolder()
{
    Dl_info info;
    if (dladdr((void*)getPluginFolder, &info)) {
        return fs::path(info.dli_fname).parent_path();
    }
    return {};
}

bool ProxyPython::init(IOrganizer* moInfo)
{
    m_MOInfo = moInfo;

    if (m_MOInfo && !m_MOInfo->isPluginEnabled(this)) {
        return false;
    }

    if (QCoreApplication::applicationDirPath().contains(';')) {
        m_LoadFailure = FailureType::SEMICOLON;
        return true;
    }

    const auto pluginFolder = getPluginFolder();

    if (pluginFolder.empty()) {
        const int error = errno;
        m_LoadFailure   = FailureType::DLL_NOT_FOUND;
        log::error("failed to resolve Python proxy directory ({}): {}", error,
                   qUtf8Printable(strerror(error)));
        return false;
    }

    if (m_MOInfo && m_MOInfo->persistent(name(), "tryInit", false).toBool()) {
        m_LoadFailure = FailureType::INITIALIZATION;
        if (QMessageBox::question(
                parentWidget(), tr("Python Initialization failed"),
                tr("On a previous start the Python Plugin failed to initialize.\n"
                   "Do you want to try initializing python again (at the risk of "
                   "another crash)?\n "
                   "Suggestion: Select \"no\", and click the warning sign for further "
                   "help.Afterwards you have to re-enable the python plugin."),
                QMessageBox::Yes | QMessageBox::No,
                QMessageBox::No) == QMessageBox::No) {
            // we force enabled here (note: this is a persistent settings since MO2 2.4
            // or something), plugin
            // usually should not handle enabled/disabled themselves but this is a base
            // plugin so...
            m_MOInfo->setPersistent(name(), "enabled", false, true);
            return true;
        }
    }

    if (m_MOInfo) {
        m_MOInfo->setPersistent(name(), "tryInit", true);
    }
    /*
    const auto dllPaths = pluginFolder / "dlls";
    if (SetDllDirectoryW(dllPaths.c_str()) == 0) {
        DWORD error   = ::GetLastError();
        m_LoadFailure = FailureType::DLL_NOT_FOUND;
        log::error("failed to add python DLL directory ({}): {}", error,
                   qUtf8Printable(windowsErrorString(::GetLastError())));
        return false;
    }
    */

    m_Runner = mo2::python::createPythonRunner();

    if (m_Runner) {
        const auto libpath = pluginFolder / "libs";
        const std::vector paths{
            libpath / "pythoncore.zip", libpath,
            std::filesystem::path{IOrganizer::getPluginDataPath().toStdWString()}};
        m_Runner->initialize(paths);
    }

    if (m_MOInfo) {
        m_MOInfo->setPersistent(name(), "tryInit", false);
    }

    if (!m_Runner || !m_Runner->isInitialized()) {
        m_LoadFailure = FailureType::INITIALIZATION;
    }
    else {
        m_Runner->addDllSearchPath(pluginFolder / "lib");
    }

    return true;
}
