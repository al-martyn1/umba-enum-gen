/*! \file
    \brief Информация о приложении и его версии - название, описание, версия, время/дата, домашняя страница
 */

std::string appFullName   = "Umba Enum's Genertor";
std::string appVersion    = "1.01";
std::string appCommitHash;  //UNDONE
std::string appBuildDate  = __DATE__;
std::string appBuildTime  = __TIME__;

const char *appHomeUrl    = "https://github.com/al-martyn1/umba-enum-gen";
const char *appistrPath   = "";

#if defined(WIN32) || defined(_WIN32)
    const char *appSubPath    = "bin/umba-enum-gen.exe";
#else
    const char *appSubPath    = "bin/umba-enum-gen";
#endif

