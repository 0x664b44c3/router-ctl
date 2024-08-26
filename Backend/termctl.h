#ifndef TERMCTL_H
#define TERMCTL_H
#include <QTextStream>

enum class TermCtl {
    Reset = 0,
    Red,
    Green,
    Blue,
    Cyan,
    Magenta,
    Yellow,
    Intensity
};

QTextStream& operator<<(QTextStream& out, const TermCtl);

#endif // TERMCTL_H
