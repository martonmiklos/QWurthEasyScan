#include "qwurthscanner.h"
#include <unistd.h>

QWurthScanner::QWurthScanner(QObject *parent) :
    QObject(parent)
{
    sendTimer.setInterval(3000);
    sendTimer.setSingleShot(true);
    connect(&sendTimer, SIGNAL(timeout()), this, SLOT(writeNextChar()));
    currentCommand.type = Idle;
    barcodeReadState = ReadHeader;
    connect(&timeoutTimer, SIGNAL(timeout()), this, SLOT(commandTimeOut()));
    connect(&port, SIGNAL(readyRead()), this, SLOT(deviceDataReceieved()));
}

QWurthScanner::~QWurthScanner()
{
    if (port.isOpen())
        port.close();
}

void QWurthScanner::setPortName(QString portName)
{
    mErrorString.clear();
    if (port.isOpen())
        port.close();

    port.setPortName(portName);
    port.setBaudRate(BAUD9600);
    port.setQueryMode(QextSerialPort::EventDriven);
    if (!port.open(QextSerialPort::ReadWrite)) {
        mErrorString = tr("Unable to open the %1 port").arg(portName);
        emit errorHappened();
    }
}

void QWurthScanner::queryDevice()
{
    unsigned char dataRaw[4] = {0x1B, '@', 'I', 0x12};
    QByteArray data;
    data.setRawData((char*)dataRaw, 4);

    createCommand(QueryDevice, data);
}

void QWurthScanner::powerDownDevice()
{
    unsigned char dataRaw[4] = {0x1B, '@', 'P', 0x0B};
    QByteArray data;
    data.setRawData((char*)dataRaw, 4);

    createCommand(PowerDownDevice, data);
}

void QWurthScanner::readBarcodes()
{
    unsigned char dataRaw[4] = {0x1B, '@', 'U', 0x0E};
    QByteArray data;
    data.setRawData((char*)dataRaw, 4);
    createCommand(ReadBarcodes, data);
}

void QWurthScanner::clearBarcodes()
{
    unsigned char dataRaw[4] = {0x1B, '@', 'C', 0x18};
    QByteArray data;
    data.setRawData((char*)dataRaw, 4);
    createCommand(ClearBarodes, data);
}

void QWurthScanner::writeNextChar()
{
    if (currentCommand.data.length()) {
        port.putChar(currentCommand.data.at(0));
        currentCommand.data = currentCommand.data.mid(1, currentCommand.data.length()-1);
        sendTimer.start(2);
    }
}

void QWurthScanner::commandTimeOut()
{
    emit commandTimeout(currentCommand.type);
    startSendPackets();
}

void QWurthScanner::deviceDataReceieved()
{
    receieveBuffer.append(port.readAll());
    //dumpData(receieveBuffer);
    switch (currentCommand.type) {
    case ReadBarcodes:
        if (barcodeReadState == ReadHeader) {
            if (receieveBuffer.length() >= 11) {
                if (receieveBuffer.mid(2, 7) == "@MemoSc") {
                    barcodesToReceieve = ((unsigned char)receieveBuffer.at(9) << 8) + (unsigned char)receieveBuffer.at(10);
                    barcodeReadState = ReadCodes;
                    receieveBuffer = receieveBuffer.mid(11); //strip out header
                    //qWarning() << "receieve %1 barcodes" << barcodesToReceieve;
                } else {
                    qWarning() << tr("Malformed readbarcode response: %1").arg(QString(receieveBuffer.mid(2, 7)));
                }
            } else if (receieveBuffer.length() == 1) {
                if (receieveBuffer.at(0) == 5) { // no stored barcodes
                    emit barcodesReceieved(readedBarcodes);
                    //qWarning() << "no barcodes";
                    startSendPackets();
                }
            }
        }

        if (barcodeReadState == ReadCodes) {
            while (receieveBuffer.length()) {
                unsigned char codeLength = receieveBuffer.at(0);
                if (receieveBuffer.length() >= codeLength+1) {
                    readedBarcodes.append(receieveBuffer.mid(1, codeLength));
                    receieveBuffer = receieveBuffer.mid(codeLength+1);
                    barcodesToReceieve--;
                    if (barcodesToReceieve == 0) {
                        emit barcodesReceieved(readedBarcodes);
                        readedBarcodes.clear();
                        barcodeReadState = ReadHeader;
                        if (receieveBuffer.length() > 1) {
                            qWarning() << tr("%1 characters left after parsing barcodes").arg(receieveBuffer.length()-1);
                        }
                        startSendPackets();
                        break;
                    }
                } else {
                    break;
                }
            }
        }
        break;
    case ClearBarodes:
        if (receieveBuffer.length() >= 5) {
            QString response = "\x02\x1b@C\x1a";
            if (receieveBuffer.mid(0, 5) == response) {
                currentCommand.type = Idle;
            } else {
                qWarning() << "clearbuffer malformed answer!" << receieveBuffer.mid(0, 5) << response;
            }
            timeoutTimer.stop();
            receieveBuffer.clear();
            startSendPackets();
        }
        break;
    case QueryDevice:
        if (receieveBuffer.length() >= 25) {
            QString deviceName = receieveBuffer.mid(5);
            emit deviceFound(deviceName);
            timeoutTimer.stop();
            receieveBuffer.clear();
            startSendPackets();
        }
        break;
    case PowerDownDevice:
        if (receieveBuffer.length() >= 5) {
            QString answer = receieveBuffer.mid(0, 5);
            if (answer == "\x02\x1b@P\x09") {
                timeoutTimer.stop();
                receieveBuffer.clear();
                startSendPackets();
            } else {
                qWarning() << "malformed answer came to the powerdown query:" << answer;
            }
        }
        break;
    case Idle:
        qWarning() << "data receieved in idle state...";
        break;
    }
}

void QWurthScanner::startSendPackets()
{
    timeoutTimer.stop();
    receieveBuffer.clear();
    if (commandsToSend.size()) {
        currentCommand = commandsToSend.takeFirst();
        writeNextChar();
    } else {
        currentCommand.type = Idle;
    }
}

void QWurthScanner::dumpData(QByteArray data)
{
    qWarning() << "DUMP";
    for (int i = 0; i<data.length(); i++) {
        qWarning("%02x", (unsigned char)data[i]);
    }
}

void QWurthScanner::createCommand(CommandType type, QByteArray data)
{
    Command cmd;
    cmd.data = data;
    cmd.type = type;
    commandsToSend.append(cmd);

    if (currentCommand.type == Idle) {
        startSendPackets();
    }
    timeoutTimer.start(2000);
}
