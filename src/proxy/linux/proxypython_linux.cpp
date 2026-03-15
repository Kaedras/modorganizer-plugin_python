#include "../proxypython.h"

#include <QCoreApplication>
#include <QMessageBox>

#include <filesystem>
#include <uibase/log.h>

namespace fs = std::filesystem;
using namespace MOBase;

// retrieve the path to the folder containing the proxy DLL
fs::path getPluginFolder()
{
#warning STUB
    return {};
}

bool ProxyPython::init(IOrganizer* moInfo)
{
#warning STUB
    m_MOInfo = moInfo;
    return true;
}
