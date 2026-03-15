#include "../proxypython.h"

#include <filesystem>

#include <QCoreApplication>
#include <QDirIterator>
#include <QMessageBox>
#include <QWidget>
#include <QtPlugin>

#include <uibase/log.h>
#include <uibase/utility.h>
#include <uibase/versioninfo.h>

namespace fs = std::filesystem;
using namespace MOBase;

// retrieve the path to the folder containing the proxy DLL
fs::path getPluginFolder()
{
    wchar_t path[MAX_PATH];
    HMODULE hm = NULL;

    if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                              GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                          (LPCWSTR)&getPluginFolder, &hm) == 0) {
        return {};
    }
    if (GetModuleFileName(hm, path, sizeof(path)) == 0) {
        return {};
    }

    return fs::path(path).parent_path();
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
        DWORD error   = ::GetLastError();
        m_LoadFailure = FailureType::DLL_NOT_FOUND;
        log::error("failed to resolve Python proxy directory ({}): {}", error,
                   qUtf8Printable(windowsErrorString(::GetLastError())));
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

    // load the pythonrunner library, this is done in multiple steps:
    //
    // 1. we set the dlls/ subfolder (from the plugin) as the DLL directory so Windows
    // will look for DLLs in it, this is required to find the Python and libffi DLL, but
    // also the runner DLL
    //
    const auto dllPaths = pluginFolder / "dlls";
    if (SetDllDirectoryW(dllPaths.c_str()) == 0) {
        DWORD error   = ::GetLastError();
        m_LoadFailure = FailureType::DLL_NOT_FOUND;
        log::error("failed to add python DLL directory ({}): {}", error,
                   qUtf8Printable(windowsErrorString(::GetLastError())));
        return false;
    }

    // 2. we create the Python runner, we do not need to use ::LinkLibrary and
    // ::GetProcAddress because we use delayed load for the runner DLL (see the
    // CMakeLists.txt)
    //
    m_Runner = mo2::python::createPythonRunner();

    if (m_Runner) {
        const auto libpath = pluginFolder / "libs";
        const std::vector<fs::path> paths{
            libpath / "pythoncore.zip", libpath,
            std::filesystem::path{IOrganizer::getPluginDataPath().toStdWString()}};
        m_Runner->initialize(paths);
    }

    if (m_MOInfo) {
        m_MOInfo->setPersistent(name(), "tryInit", false);
    }

    // reset DLL directory
    SetDllDirectoryW(NULL);

    if (!m_Runner || !m_Runner->isInitialized()) {
        m_LoadFailure = FailureType::INITIALIZATION;
    }
    else {
        m_Runner->addDllSearchPath(pluginFolder / "dlls");
    }

    return true;
}
