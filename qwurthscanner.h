#ifndef QWURTHSCANNER_H
#define QWURTHSCANNER_H

#include <QObject>
#include <QDebug>
#include <QStringList>
#include <QTimer>
#include <qextserialport/qextserialport.h>

class QWurthScanner : public QObject
{
    Q_OBJECT
public:
    enum CommandType {
        QueryDevice,
        PowerDownDevice,
        ReadBarcodes,
        ClearBarodes,
        Idle
    };

    explicit QWurthScanner(QObject *parent = 0);
    ~QWurthScanner();
    void setPortName(QString port);
    QString errorString() const {return mErrorString;}

    void readBarcodes();
    void queryDevice();
    void powerDownDevice();
    void clearBarcodes();
    static QString commandName(CommandType type)
    {
        switch (type) {
        case ClearBarodes:
            return tr("Clear barcodes");
        case ReadBarcodes:
            return tr("Read barcodes");
        case QueryDevice:
            return tr("Query device");
        case PowerDownDevice:
            return tr("Power down device");
        default:
            return tr("Invalid command: %1").arg(type);
        }
    }

signals:
    void deviceFound(QString deviceName);
    void barcodesReceieved(QStringList barcodes);
    void barcodesCleared();
    void commandTimeout(quint32 type);
    void errorHappened();

public slots:
    void deviceDataReceieved();
    void writeNextChar();
    void commandTimeOut();

private:
    void startSendPackets();

    enum BarcodeReadState {
        ReadHeader,
        ReadCodes
    };

    enum ClearState {
        SendC,
        SendP
    };

    enum CommandType;
    struct Command {
        CommandType type;
        QByteArray data;
    };

    QextSerialPort port;
    QList<Command> commandsToSend;
    QTimer sendTimer, timeoutTimer;
    Command currentCommand;
    QByteArray receieveBuffer;
    QStringList readedBarcodes;
    int barcodesToReceieve;
    BarcodeReadState barcodeReadState;
    void dumpData(QByteArray data);
    void createCommand(CommandType type, QByteArray data);
    QString mErrorString;
};

#endif // QWURTHSCANNER_H
