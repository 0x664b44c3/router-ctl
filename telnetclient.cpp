#include "telnetclient.h"
#include "telnet-options.h"
#include <QDebug>
#include <QTimer>
#define DEBUG
TelnetClient::TelnetClient(QObject *parent)
    : QObject{parent}, mSocket(new QTcpSocket(this)), mDecodeTelnet(true), mPort(23),mAutoReconnect(false)
{
    connect(mSocket, &QTcpSocket::connected, this, &TelnetClient::onConnect);
    connect(mSocket, &QTcpSocket::readyRead, this, &TelnetClient::onReadyRead);
    connect(mSocket, &QTcpSocket::disconnected, this, &TelnetClient::onDisconnect);
    qInfo()<<"Telnet client created";
}

TelnetClient::~TelnetClient()
{
    qInfo()<<"Telnet client d'tor";
}

void TelnetClient::connectToHost(QHostAddress addr, quint16 port)
{
    mPort = port;
    mHost.clear();
    mSocket->connectToHost(addr, port);
}

void TelnetClient::disconnect()
{
    mHost.clear();
    mSocket->close();
}

QByteArray TelnetClient::getData()
{
    auto copy = mBuffer;
    mBuffer.clear();
    return copy;
}

bool TelnetClient::atEnd() const
{
    return mBuffer.isEmpty();
}

bool TelnetClient::decodeTelnet() const
{
    return mDecodeTelnet;
}

void TelnetClient::setDecodeTelnet(bool newDecodeTelnet)
{
    if (mDecodeTelnet == newDecodeTelnet)
        return;
    mDecodeTelnet = newDecodeTelnet;
    emit decodeTelnetChanged();
}

QTcpSocket *TelnetClient::ioDev() const
{
    return mSocket;
}

void TelnetClient::flush()
{
    mSocket->flush();
}


void TelnetClient::onReadyRead()
{
    while(!mSocket->atEnd())
        mProcBuffer.append(mSocket->readAll());
    processRx();
}

void TelnetClient::onConnect()
{
    mHost = mSocket->peerAddress();
    qDebug()<<"Connected to remote host"<<mHost;
}

void TelnetClient::onDisconnect()
{
    qDebug()<<"connection closed."<<mSocket->errorString();
    emit disconnected();
    if ((mAutoReconnect) && (!mHost.isNull()))
    {
        QTimer::singleShot(1000, this, [&]() {
        qDebug()<<"trying to reconnect to remote host"<<mHost.toString()<<mPort;
        mSocket->connectToHost(mHost, mPort);
        });
    }
}

bool TelnetClient::autoReconnect() const
{
    return mAutoReconnect;
}

void TelnetClient::setAutoReconnect(bool newAutoReconnect)
{
    if (mAutoReconnect == newAutoReconnect)
        return;
    mAutoReconnect = newAutoReconnect;
    emit autoReconnectChanged();
}

void TelnetClient::processRx()
{
    int i = 0;
    while(i<mProcBuffer.size())
    {
        int newStart = i;
        int j = mProcBuffer.indexOf(static_cast<char>(Telnet::Ctl::IAC), i);
        if (j<0)
        { //we have no more telnet sequences
            mBuffer.append(mProcBuffer.mid(i));
            i = mProcBuffer.size();
            break;
        }
        //append everything up to the iac code to the output buffer
        mBuffer.append(mProcBuffer.mid(i, j - i));
        i = j; //always safe, we never run over the end here

        if (j >= mProcBuffer.size() - 1) // IAC was at end of input, we will take care of it next time we get some data
            break;

        if (mProcBuffer[j+1] == static_cast<char>(Telnet::Ctl::IAC))
        {
            i=j+2; // scan over <IAC IAC>, add one IAC char to the output buffer
            //if we ran over the end of the input this is fine, no more data to process
            mBuffer.append(static_cast<char>(Telnet::Ctl::IAC)); //this was an escaped IAC character
            continue;
        }

        if (mProcBuffer[j+1] == static_cast<char>(Telnet::Ctl::SB)) //subnegotiation
        {
            QByteArray subNegotiationTerminator;
            subNegotiationTerminator.append(static_cast<char>(Telnet::Ctl::IAC));
            subNegotiationTerminator.append(static_cast<char>(Telnet::Ctl::SE));
            int k = mProcBuffer.indexOf(subNegotiationTerminator, i+1);
            if (k<0)
                break;
            else
            {
                qDebug()<<"skip subnegotiation";
                i = k+2; //skip subnegotiation
                continue;
            }
        } else {
            newStart = j+3; // Normal telnet option sequence
            // qDebug()<<"skip telnet option sequence" << mProcBuffer.mid(i, newStart - i);
            if (newStart<=mProcBuffer.size())
            {
                unsigned char cmdCode = mProcBuffer[j+1];
                unsigned char optionCode = mProcBuffer[j+2];
                qDebug().noquote()<<QString::asprintf("<IAC> %02x %02x", cmdCode, optionCode);
                i = newStart;
            }
            else
                break;
        }
    }

    if (i<mProcBuffer.size())
        mProcBuffer = mProcBuffer.mid(i);
    else
        mProcBuffer.clear();

    if (mBuffer.size())
        emit readyRead();

}

