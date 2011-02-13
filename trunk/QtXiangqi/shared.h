#ifndef __QtXiangqi_SHARED_H__
#define __QtXiangqi_SHARED_H__

#include <QString>
#include <string>

/** Convert from QString to std::string of UTF-8. */
const std::string qStringToUtf8(const QString& qString);

/** Convert a std::string of UTF-8 to QString. */
const QString utf8ToQString(const std::string& sString);

#endif // __QtXiangqi_SHARED_H__
