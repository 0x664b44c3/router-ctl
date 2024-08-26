#include "termctl.h"
#include <QVector>
QTextStream &operator<<(QTextStream &out, const TermCtl op)
{
    QVector<int> codes;
    QByteArray d;

    switch (op) {
    case TermCtl::Reset:
        codes << 0;
        break;
    case TermCtl::Red:
        codes << 31;
        break;
    case TermCtl::Green:
        codes << 32;
        break;
    case TermCtl::Blue:
        codes << 34;
        break;
    case TermCtl::Cyan:
        break;
    case TermCtl::Magenta:
        break;
    case TermCtl::Yellow:
        break;
    case TermCtl::Intensity:
        break;
    default:
        break;
    }
    for(auto n: std::as_const(codes))
    {
        if (d.size())
            d.append(";");
        d.append(QByteArray::number(n, 10));
    }
    return out << "\033[" << d<<"m";
}


/**
 *  0          reset all attributes to their defaults
       1          set bold
       2          set half-bright (simulated with color on a color display)
       3          set italic (since Linux 2.6.22; simulated with color on a color display)
       4          set underscore (simulated with color on a color display) (the colors
                  used to simulate dim or underline are set using ESC ] ...)
       5          set blink
       7          set reverse video
       10         reset selected mapping, display control flag, and toggle meta flag
                  (ECMA-48 says "primary font").
       11         select null mapping, set display control flag, reset toggle meta flag
                  (ECMA-48 says "first alternate font").
       12         select null mapping, set display control flag, set toggle meta flag
                  (ECMA-48 says "second alternate font").  The toggle meta flag causes the
                  high bit of a byte to be toggled before the mapping table translation is
                  done.
       21         set underline; before Linux 4.17, this value set normal intensity (as is
                  done in many other terminals)
       22         set normal intensity
       23         italic off (since Linux 2.6.22)
       24         underline off
       25         blink off
       27         reverse video off
       30         set black foreground
       31         set red foreground
       32         set green foreground
       33         set brown foreground
       34         set blue foreground
       35         set magenta foreground
       36         set cyan foreground
       37         set white foreground
       38         256/24-bit foreground color follows, shoehorned into 16 basic colors
                  (before Linux 3.16: set underscore on, set default foreground color)
       39         set default foreground color (before Linux 3.16: set underscore off, set
                  default foreground color)
       40         set black background
       41         set red background
       42         set green background
       43         set brown background
       44         set blue background
       45         set magenta background
       46         set cyan background
       47         set white background
       48         256/24-bit background color follows, shoehorned into 8 basic colors
       49         set default background color
       90..97     set foreground to bright versions of 30..37
       100..107   set background, same as 40..47 (bright not supported)
*/
