#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDebug>
#include <QElapsedTimer>
#include <QColor>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_connect_disconnect_clicked();
    void readSerialData();


private:
    Ui::MainWindow *ui;
    QSerialPort *serialPort; // Declare the QSerialPort object
    QByteArray serialBuffer; // Buffer to accumulate received data

    void updateReceivedDataLabels(const QByteArray &data);
    void on_transmit();

    QElapsedTimer receiveTimer;
    qint64 lastReceiveTime;
};
#endif // MAINWINDOW_H



