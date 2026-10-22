#include "parsercommon.h"

namespace ParserCommon
{
QString toUtf8String(const QByteArray &data)
{
    return QString::fromUtf8(data);
}
}
