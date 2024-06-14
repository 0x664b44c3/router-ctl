#include "panelconnection.h"
#include <QTimer>
#include <QByteArray>
#include <QTcpSocket>
#include <QString>
#include <QStringList>
#include "panelserver.h"
#include <iostream>
namespace BMD {

PanelConnection::PanelConnection(QTcpSocket *sock, PanelServer *parent)
    : mSock(sock), QObject{parent}
{
    mCurrentRun=0;
    QTimer::singleShot(0, this, &PanelConnection::sendBanner);
    mNumInputs = 128;
    mNumOutputs = 128;
    QObject::connect(sock, &QTcpSocket::readyRead, this, &PanelConnection::onDataReady);
    QTimer * tmr = new QTimer(this);
    tmr->start(100);
    connect(tmr, &QTimer::timeout, this, [&]() {
        QStringList status;
        status << "VIDEO OUTPUT ROUTING:"
               << QString::asprintf("2 %d", mCurrentRun++)
               << ""
               << "ACK"
               << ""
               << "";
        if (mCurrentRun>=20)
            mCurrentRun=0;
        queueOutput(status);
        mSock->flush();
    });
    // mSock->write("PING\n\n");});
}

void PanelConnection::queueOutput(QStringList lines)
{
    QByteArray d = lines.join("\n").toLatin1();
    mOutputBuffer.append(d);
    trySend();
}

void PanelConnection::trySend()
{
    if (mOutputBuffer.isEmpty())
        return;
    if (mInputBuffer.isEmpty())
    {
        mSock->write(mOutputBuffer);
        mOutputBuffer.clear();
    }
}

QStringList PanelConnection::_deviceInfo() const
{
    return QStringList()
           << "VIDEOHUB DEVICE:"
           << "Device present: true"
           << "Model name: Blackmagic Smart Videohub"
           << "Video Inputs: "  + QString::number(mNumInputs)
           << "Video Outputs: " + QString::number(mNumOutputs)
           << "Video Monitoring Outputs: 0"
           << "Serial Ports: 0"
           << "";
}

QStringList PanelConnection::_statusDump() const
{
    QStringList ret;
    ret << "INPUT LABELS:";
    for (int i=0;i< mNumInputs;++i)
        ret << QString::number(i) + " " + "Label";
    ret.append("");
    ret << "OUTPUT LABELS:";
    for (int i=0;i< mNumOutputs;++i)
        ret << QString::number(i) + " " + "Label";
    ret.append("");

    //pretend a 1:1 routing for now
    ret << "OUTPUT ROUTING:";
    for (int i=0;i< mNumOutputs;++i)
        ret << QString::number(i) + " " + QString::number(i);
    ret.append("");

    //no lock support r/n
    ret << "VIDEO OUTPUT LOCKS:";
    for (int i=0;i< mNumOutputs;++i)
        ret << QString::number(i) + " U";

    ret.append("");

    return ret;
}

void PanelConnection::processData()
{
    int start= 0;
    int end_of_block = mInputBuffer.indexOf("\n\n");
    while (end_of_block>=0)
    {
        QByteArray block = mInputBuffer.mid(start, end_of_block - start + 1);
        QStringList cmds = QString::fromLatin1(block).split("\n", Qt::KeepEmptyParts);
        if (cmds.size()>1)
        {
            QString cmd =cmds.takeFirst();
            processCmd(cmd, cmds);
        }
        start = end_of_block + 2;
        end_of_block = mInputBuffer.indexOf("\n\n", start);
    }
    mInputBuffer.remove(0, start);
    // qDebug()<<mInputBuffer.size()<<"bytes unprocessed.";
    if (mInputBuffer.isEmpty())
        trySend();
}

void PanelConnection::processCmd(QString cmd, QStringList args)
{
    cmd = cmd.toUpper();
    if (!cmd.endsWith(':'))
    {
        mSock->write("NAK\n\n");
    }
    cmd.chop(1);
    qDebug()<<"BMD Panel CMD: "<<cmd<<args;
    if (cmd=="PING")
    {
        mSock->write("ACK\n\n");
    }
    else if (cmd == "VIDEO OUTPUT ROUTING")
    {
        mSock->write("ACK\n\n");
    }
}

void PanelConnection::onDisconnect()
{
    deleteLater();
}

} // namespace BMD

void BMD::PanelConnection::onDataReady()
{
    while(!mSock->atEnd())
        mInputBuffer.append(mSock->readAll());
    processData();
}

void BMD::PanelConnection::sendBanner()
{
    QStringList response;
    response << "PROTOCOL PREAMBLE";
    response << "Version: 2.3";
    response << "";
    response << _deviceInfo();
    response << _statusDump();
    response <<"ACK"
             <<"";

    queueOutput(response);

}
