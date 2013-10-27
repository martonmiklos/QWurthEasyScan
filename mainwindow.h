#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSignalMapper>
#include <QSettings>
#include <QLabel>

#include "qwurthscanner.h"
#include "formabout.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
private slots:
    void on_pushButtonQueryDevice_clicked();

    void on_pushButtonReadBarcodes_clicked();

    void on_pushButtonClearBarcodes_clicked();

    void on_pushButtonPowerDown_clicked();

    void deviceFound(QString);

    void barcodesReceieved(const QStringList &barcodes);

    void on_pushButtonList_clicked();

    void commandTimedout(quint32 type);

    void on_pushButtonSaveList_clicked();

    void scannerError();

    void portChanged();

    void on_actionQuit_triggered();

    void on_actionAbout_Qt_triggered();

    void on_actionAbout_QWurthScanner_triggered();

private:
    Ui::MainWindow *ui;
    QWurthScanner scanner;
    QLabel statusLabel;
    QSettings settings;

    QString lastSaveDir();
    void setLastSaveDir(const QString &dir);

    QString lastPort();
    void setLastPort(QString portName);

    QList<QAction*> portActions;
    FormAbout aboutForm;
};

#endif // MAINWINDOW_H
