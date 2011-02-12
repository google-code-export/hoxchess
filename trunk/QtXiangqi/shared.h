#ifndef SHARED_H
#define SHARED_H

#include <QString>
#include <string>

/** Convert from QString to std::string of UTF-8. */
const std::string qStringToUtf8(const QString& qString)
{
    return std::string(qString.toUtf8().data());
}

/** Convert a std::string of UTF-8 to QString. */
const QString utf8ToQstring(const std::string& sString)
{
    return QString::fromUtf8(sString.c_str());
}

#endif // SHARED_H
