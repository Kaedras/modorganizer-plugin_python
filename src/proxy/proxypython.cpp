/*
Copyright (C) 2022 Sebastian Herbord & MO2 Team. All rights reserved.

This file is part of python proxy plugin for MO

python proxy plugin is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Python proxy plugin is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with python proxy plugin.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "proxypython.h"

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

ProxyPython::ProxyPython()
    : m_MOInfo{nullptr}, m_RunnerLib{nullptr}, m_Runner{nullptr},
      m_LoadFailure(FailureType::NONE)
{
}

QString ProxyPython::name() const
{
    return "Python Proxy";
}

QString ProxyPython::localizedName() const
{
    return tr("Python Proxy");
}

QString ProxyPython::author() const
{
    return "AnyOldName3, Holt59, Silarn, Tannin";
}

QString ProxyPython::description() const
{
    return tr("Proxy Plugin to allow plugins written in python to be loaded");
}

VersionInfo ProxyPython::version() const
{
    return VersionInfo(3, 0, 0, VersionInfo::RELEASE_FINAL);
}

QList<PluginSetting> ProxyPython::settings() const
{
    return {};
}

QStringList ProxyPython::pluginList(const QDir& pluginPath) const
{
    QDir dir(pluginPath);
    dir.setFilter(dir.filter() | QDir::NoDotAndDotDot);
    QDirIterator iter(dir);

    // Note: We put python script (.py) and directory names, not the __init__.py
    // files in those since it is easier for the runner to import them.
    QStringList result;
    while (iter.hasNext()) {
        QString name   = iter.next();
        QFileInfo info = iter.fileInfo();

        if (info.isFile() && name.endsWith(".py")) {
            result.append(name);
        }
        else if (info.isDir() && QDir(info.absoluteFilePath()).exists("__init__.py")) {
            result.append(name);
        }
    }

    return result;
}

QList<QObject*> ProxyPython::load(const QString& identifier)
{
    if (!m_Runner) {
        return {};
    }
    return m_Runner->load(identifier);
}

void ProxyPython::unload(const QString& identifier)
{
    if (m_Runner) {
        return m_Runner->unload(identifier);
    }
}

std::vector<unsigned int> ProxyPython::activeProblems() const
{
    auto failure = m_LoadFailure;

    // don't know how this could happen but wth
    if (m_Runner && !m_Runner->isInitialized()) {
        failure = FailureType::INITIALIZATION;
    }

    if (failure != FailureType::NONE) {
        return {static_cast<std::underlying_type_t<FailureType>>(failure)};
    }

    return {};
}

QString ProxyPython::shortDescription(unsigned int key) const
{
    switch (static_cast<FailureType>(key)) {
    case FailureType::SEMICOLON:
        return tr("ModOrganizer path contains a semicolon");
    case FailureType::DLL_NOT_FOUND:
        return tr("Python DLL not found");
    case FailureType::INVALID_DLL:
        return tr("Invalid Python DLL");
    case FailureType::INITIALIZATION:
        return tr("Initializing Python failed");
    default:
        return tr("invalid problem key %1").arg(key);
    }
}

QString ProxyPython::fullDescription(unsigned int key) const
{
    switch (static_cast<FailureType>(key)) {
    case FailureType::SEMICOLON:
        return tr("The path to Mod Organizer (%1) contains a semicolon.<br>"
                  "While this is legal on NTFS drives, many applications do not "
                  "handle it correctly.<br>"
                  "Unfortunately MO depends on libraries that seem to fall into that "
                  "group.<br>"
                  "As a result the python plugin cannot be loaded, and the only "
                  "solution we can offer is to remove the semicolon or move MO to a "
                  "path without a semicolon.")
            .arg(QCoreApplication::applicationDirPath());
    case FailureType::DLL_NOT_FOUND:
        return tr("The Python plugin DLL was not found, maybe your antivirus deleted "
                  "it. Re-installing MO2 might fix the problem.");
    case FailureType::INVALID_DLL:
        return tr(
            "The Python plugin DLL is invalid, maybe your antivirus is blocking it. "
            "Re-installing MO2 and adding exclusions for it to your AV might fix the "
            "problem.");
    case FailureType::INITIALIZATION:
        return tr("The initialization of the Python plugin DLL failed, unfortunately "
                  "without any details.");
    default:
        return tr("invalid problem key %1").arg(key);
    }
}

bool ProxyPython::hasGuidedFix(unsigned int) const
{
    return false;
}

void ProxyPython::startGuidedFix(unsigned int) const {}
