#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QActionGroup>


#include <qextserialport/qextserialenumerator.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->statusBar->addWidget(&statusLabel);
    connect(&scanner, SIGNAL(deviceFound(QString)), this, SLOT(deviceFound(QString)));
    connect(&scanner, SIGNAL(barcodesReceieved(QStringList)), this, SLOT(barcodesReceieved(QStringList)));
    connect(&scanner, SIGNAL(commandTimeout(quint32)), this, SLOT(commandTimedout(quint32)));
    connect(&scanner, SIGNAL(errorHappened()), this, SLOT(scannerError()));

    ui->actionPlaceholder->deleteLater();

    QActionGroup *portActionsGroup = new QActionGroup(this);
    QString lastPortName = lastPort();
    bool lastFound = false;
    foreach (QextPortInfo info, QextSerialEnumerator::getPorts()) {
        QAction *action = new QAction(info.physName, this);
        action->setCheckable(true);
        portActionsGroup->addAction(action);
        if (info.physName == lastPortName) {
            action->setChecked(true);
            lastFound = true;
        }
        connect(action, SIGNAL(triggered()), this, SLOT(portChanged()));

        portActions.append(action);
        ui->menuPort->addAction(action);
    }

    if (!lastFound) {
        if (portActions.size())
            portActions[0]->setChecked(true);
    }
    portChanged();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButtonQueryDevice_clicked()
{
    statusLabel.clear();
    scanner.queryDevice();
}

void MainWindow::on_pushButtonReadBarcodes_clicked()
{
    scanner.readBarcodes();
}

void MainWindow::on_pushButtonClearBarcodes_clicked()
{
    scanner.clearBarcodes();
}

void MainWindow::on_pushButtonPowerDown_clicked()
{
    scanner.powerDownDevice();
}

void MainWindow::deviceFound(QString deviceName)
{
    statusLabel.setText(tr("Connected to %1 successfully!").arg(deviceName));
}

void MainWindow::barcodesReceieved( const QStringList &barcodes)
{
    if (barcodes.isEmpty()) {
        QMessageBox::warning(this, tr("No barcodes"), tr("The device does not contains any barcodes!"));
    }
    ui->listWidget->addItems(barcodes);
}

void MainWindow::on_pushButtonList_clicked()
{
    if (QMessageBox::question(this,
                              tr("Are you sure?"),
                              tr("Do you want to clear the readed barcodes?\nYour readed barcodes will be lost if you have not saved them yet!"),
                              QMessageBox::Yes,
                              QMessageBox::No) == QMessageBox::Yes) {
        ui->listWidget->clear();
    }
}

void MainWindow::commandTimedout(quint32 type)
{
    QMessageBox::warning(this,
                         tr("Timeout"),
                         tr("The %1 command timed out!\nPlease check that your scanner is turned on!\n(The LED should blinking green in ~every second)")
                         .arg(QWurthScanner::commandName((QWurthScanner::CommandType)type))
                         );
}

void MainWindow::on_pushButtonSaveList_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this,
                                 tr("Please select the file to save!"),
                                lastSaveDir(),
                                 tr("CSV files, (*.csv)"));
    if (!fileName.isEmpty()) {
        QFile out(fileName);
        if (out.open(QFile::WriteOnly)) {
            for (int i = 0; i<ui->listWidget->count(); i++) {
                out.write(QString(ui->listWidget->item(i)->text()+"\n").toLatin1());
            }
            out.close();

            QFileInfo fi(fileName);
            setLastSaveDir(fi.path());
        } else {
            QMessageBox::warning(this, tr("Error"), tr("Unable to open the output file!"));
        }
    }
}

void MainWindow::scannerError()
{
    QMessageBox::warning(this,
                         tr("Error!"),
                         scanner.errorString());
}

void MainWindow::portChanged()
{
    foreach (QAction *action, portActions) {
        if (action->isChecked()) {
            scanner.setPortName(action->text());
            setLastPort(action->text());
            break;
        }
    }
}

QString MainWindow::lastSaveDir()
{
    QString dir;
    settings.beginGroup("dirs");
    dir = settings.value("lastSaveDir", QDir::homePath()).toString();
    settings.endGroup();
    return dir;
}

void MainWindow::setLastSaveDir(const QString &dir)
{
    settings.beginGroup("dirs");
    settings.setValue("lastSaveDir", dir);
    settings.endGroup();
}

QString MainWindow::lastPort()
{
    QString port;
    settings.beginGroup("port");
    port = settings.value("lastPort", "/dev/ttyS0").toString(); // FIXME platform dependant
    settings.endGroup();
    return port;
}

void MainWindow::setLastPort(QString portName)
{
    settings.beginGroup("port");
    settings.setValue("lastPort", portName);
    settings.endGroup();
}

void MainWindow::on_actionQuit_triggered()
{
    if (QMessageBox::question(this,
                              tr("Are you sure"),
                              tr("Do you really want to quit from the program?"),
                              QMessageBox::Yes,
                              QMessageBox::No) == QMessageBox::Yes) {
        QCoreApplication::exit(0);
    }
}

void MainWindow::on_actionAbout_Qt_triggered()
{
    QApplication::aboutQt();
}

void MainWindow::on_actionAbout_QWurthScanner_triggered()
{
    aboutForm.show();
}
