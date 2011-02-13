#include "shared.h"

const std::string qStringToUtf8(const QString& qString)
{
    return std::string(qString.toUtf8().data());
}

/** Convert a std::string of UTF-8 to QString. */
const QString utf8ToQString(const std::string& sString)
{
    return QString::fromUtf8(sString.c_str());
}
