#ifndef TELNETOPTIONS_H
#define TELNETOPTIONS_H

namespace Telnet {
enum class Option {

    Binary = 0, // [RFC856]
    Echo = 1, // [RFC857]
    Reconnection = 2, // [NIC 15391 of 1973]
    SuppressGoAhead = 3, // [RFC858]
    ApproxMessageSizeNegotiation = 4, // [NIC 15393 of 1973]
    Status = 5, // [RFC859]
    TimingMark = 6, // [RFC860]
    RemoteControlledTransandEcho = 7, // [RFC726]
    OutputLineWidth = 8, // [NIC 20196 of August 1978]
    OutputPageSize = 9, // [NIC 20197 of August 1978]
    OutputCarriageReturnDisposition = 10, // [RFC652]
    OutputHorizontalTabStops = 11, // [RFC653]
    OutputHorizontalTabDisposition = 12, // [RFC654]
    OutputFormfeedDisposition = 13, // [RFC655]
    OutputVerticalTabstops = 14, // [RFC656]
    OutputVerticalTabDisposition = 15, // [RFC657]
    OutputLinefeedDisposition = 16, // [RFC658]
    ExtendedASCII = 17, // [RFC698]
    Logout = 18, // [RFC727]
    ByteMacro = 19, // [RFC735]
    DataEntryTerminal = 20, // [RFC1043][RFC732]
    SUPDUP = 21, // [RFC736][RFC734]
    SUPDUP_Output = 22, // [RFC749]
    SendLocation = 23, // [RFC779]
    TerminalType = 24, // [RFC1091]
    EndofRecord = 25, // [RFC885]
    TACACSUserIdentification = 26, // [RFC927]
    OutputMarking = 27, // [RFC933]
    TerminalLocationNumber = 28, // [RFC946]
    Telnet3270Regime = 29, // [RFC1041]

};

enum class Ctl {
    SE                = 240, //End of subnegotiation parameters.
    NOP               = 241, //No operation.
    DataMark          = 242, //The data stream portion of a Synch. This should always be accompanied by a TCP Urgent notification.
    Break             = 243, //NVT character BRK.
    InterruptProcess  = 244, //The function IP.
    AbortOutput       = 245, //The function AO.
    AreYouThere       = 246, //The function AYT.
    EraseCharacter    = 247, //The function EC.
    EraseLine         = 248, //The function EL.
    Goahead           = 249, //The GA signal.
    SB                = 250, //Indicates that what follows is subnegotiation of the indicated option.
    WILL              = 251, //Indicates the desire to begin performing, or confirmation that you are now performing, the indicated option.
    WONT              = 252, //Indicates the refusal to perform, or continue performing, the indicated option.
    DO                = 253, //Indicates the request that the other party perform, or confirmation that you are expecting
    DONT              = 254, //Indicates the demand that the other party stop performing, or confirmation that you are no longer expecting the other party to perform, the indicated option.
    IAC               = 255
};

}
#endif
