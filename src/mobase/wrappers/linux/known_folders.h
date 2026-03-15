#pragma once

#include <QStandardPaths>

namespace mo2::python {

    struct KnownFolder {
        const char* name;
        QStandardPaths::StandardLocation id;
    };

    constexpr std::array<KnownFolder, 23> KNOWN_FOLDERS{
        {{"Desktop", QStandardPaths::DesktopLocation},
         {"Documents", QStandardPaths::DocumentsLocation},
         {"Fonts", QStandardPaths::FontsLocation},
         {"Applications", QStandardPaths::ApplicationsLocation},
         {"Music", QStandardPaths::MusicLocation},
         {"Movies", QStandardPaths::MoviesLocation},
         {"Pictures", QStandardPaths::PicturesLocation},
         {"Temp", QStandardPaths::TempLocation},
         {"Home", QStandardPaths::HomeLocation},
         {"AppLocalData", QStandardPaths::AppLocalDataLocation},
         {"Cache", QStandardPaths::CacheLocation},
         {"GenericData", QStandardPaths::GenericDataLocation},
         {"Runtime", QStandardPaths::RuntimeLocation},
         {"Config", QStandardPaths::ConfigLocation},
         {"Download", QStandardPaths::DownloadLocation},
         {"GenericCache", QStandardPaths::GenericCacheLocation},
         {"GenericConfig", QStandardPaths::GenericConfigLocation},
         {"AppData", QStandardPaths::AppDataLocation},
         {"AppConfig", QStandardPaths::AppConfigLocation},
         {"PublicShare", QStandardPaths::PublicShareLocation},
         {"Templates", QStandardPaths::TemplatesLocation},
         {"State", QStandardPaths::StateLocation},
         {"GenericState", QStandardPaths::GenericStateLocation}}};

}  // namespace mo2::python
