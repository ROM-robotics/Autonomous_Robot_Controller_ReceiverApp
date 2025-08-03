#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    serialPort = new QSerialPort(this);

    connect(serialPort, &QSerialPort::readyRead, this, &MainWindow::readSerialData);
    connect(ui->connect_btn, &QPushButton::clicked, this, &MainWindow::on_connect_btn_clicked);

    ui->connect_btn->setText("Connect");
    ui->comboBox_transmit_ldr->setCurrentIndex(70);
    ui->comboBox_transmit_rdr->setCurrentIndex(70);

    ui->label_debug->setStyleSheet("color: #FF0000;");
    ui->label_checksum->setStyleSheet("color: red");

    lastReceiveTime = 0;
}

MainWindow::~MainWindow()
{
    if (serialPort->isOpen()) {
        serialPort->close();
    }
    receiveTimer.invalidate(); // Stop the timer
    delete serialPort;
    delete ui;
}


void MainWindow::on_connect_btn_clicked()
{
        // If disconnected, try to connect
    QString port_name = ui->textEdit->toPlainText().trimmed();
        serialPort->setPortName(port_name);
        // serialPort->setPortName("/dev/ttyACM0");
        serialPort->setBaudRate(QSerialPort::Baud115200);
        serialPort->setDataBits(QSerialPort::Data8);
        serialPort->setParity(QSerialPort::NoParity);
        serialPort->setStopBits(QSerialPort::OneStop);
        serialPort->setFlowControl(QSerialPort::NoFlowControl);

        if (serialPort->open(QIODevice::ReadWrite))
        {
            QString sentMessage = "Serial port connected to /dev/ttyACM0.";
            ui->label_debug->setText(sentMessage);
            qDebug() << sentMessage;

            // Start the timer when connected
            receiveTimer.start();
            lastReceiveTime = receiveTimer.elapsed(); // Initialize with current elapsed time
            ui->connect_btn->hide();
        } else {
            QString sentMessage = "Failed to open serial port: " + serialPort->errorString();
            ui->label_debug->setText(sentMessage);
            qDebug() << sentMessage;
        }
}

void MainWindow::readSerialData()
{
    // Read all available data
    serialBuffer.append(serialPort->readAll());

    // Process complete messages (terminated by newline)
    while (serialBuffer.contains('\n'))
    {
        int newlineIndex = serialBuffer.indexOf('\n');
        // Extract the complete message (excluding newline) and trim any whitespace
        QByteArray completeMessage = serialBuffer.left(newlineIndex).trimmed();
        // Remove the processed message (including the newline) from the buffer
        serialBuffer.remove(0, newlineIndex + 1);

        //qDebug() << "Received raw: " << completeMessage;
        updateReceivedDataLabels(completeMessage); // Update UI with the message
    }
}


void MainWindow::updateReceivedDataLabels(const QByteArray &data)
{
    // Calculate frequency
    if (receiveTimer.isValid() && lastReceiveTime != 0)
    { // Ensure timer is valid and has a previous timestamp
        qint64 currentTime = receiveTimer.elapsed(); // Get current elapsed time in milliseconds
        qint64 deltaTimeMs = currentTime - lastReceiveTime; // Time elapsed since last message

        if (deltaTimeMs > 0)
        { // Avoid division by zero
            double frequency = 1000.0 / deltaTimeMs;
            ui->label_receive_hz_value->setText(QString::number(frequency, 'f', 2)); // Display with 2 decimal places
        }
        lastReceiveTime = currentTime;
    }
    else if (receiveTimer.isValid() && lastReceiveTime == 0)
    {
        // First message after connection, just set lastReceiveTime
        lastReceiveTime = receiveTimer.elapsed();
    }

    QStringList values = QString(data).split(' ');

    const int EXPECTED_VALUES = 21; // Total number of values in your sprintf format
    QString greenStyle = "background-color: #90EE90; color: black;";

    if (values.size() == EXPECTED_VALUES)
    {
        // Map values to UI labels based on the order in your sprintf:
        // "%hd %hd %ld %ld %.4f %.4f %hd %hd %hd %hd %hd %hd %hd %hd %hd %hd %.4f %.4f %.4f %.4f %hd\r\n"
        // Index correspondence:
        // 0: right_actual_rpm (hd)
        // 1: left_actual_rpm (hd)
        // 2: right_encoder_count (ld)
        // 3: left_encoder_count (ld)
        // 4: volt (f)
        // 5: ampere (f)
        // 6: estop (hd)
        // 7: led1 (hd)
        // 8: led2 (hd)
        // 9: led3 (hd)
        // 10: led4 (hd)
        // 11: led5 (hd)
        // 12: led6 (hd)
        // 13: led7 (hd)
        // 14: led8 (hd)
        // 15: bump1 (hd)
        // 16: cliff (f)
        // 17: ir (f)
        // 18: imu_z_velocity (f)
        // 19: imu_z_heading (f)
        // 20: checksum (hd)

        ui->label_rec_rar_value->setText(values.at(0));             // right_actual_rpm
        ui->label_rec_rar_value->setStyleSheet(greenStyle);

        ui->label_rec_lar_value->setText(values.at(1));             // left_actual_rpm
        ui->label_rec_lar_value->setStyleSheet(greenStyle);

        ui->label_rec_rec_value->setText(values.at(2));             // right_encoder_count
        ui->label_rec_rec_value->setStyleSheet(greenStyle);

        ui->label_rec_lec_value->setText(values.at(3));             // left_encoder_count
        ui->label_rec_lec_value->setStyleSheet(greenStyle);

        ui->label_rec_volt_value->setText(values.at(4));            // volt
        ui->label_rec_volt_value->setStyleSheet(greenStyle);

        ui->label_rec_ampere_value->setText(values.at(5));          // ampere
        ui->label_rec_ampere_value->setStyleSheet(greenStyle);

        ui->label_rec_estop_value->setText(values.at(6));           // estop
        ui->label_rec_estop_value->setStyleSheet(greenStyle);

        ui->label_rec_gpio1_value->setText(values.at(7));           // led1
        ui->label_rec_gpio1_value->setStyleSheet(greenStyle);

        ui->label_rec_gpio2_value->setText(values.at(8));           // led2
        ui->label_rec_gpio2_value->setStyleSheet(greenStyle);

        ui->label_rec_gpio3_value->setText(values.at(9));           // led3
        ui->label_rec_gpio3_value->setStyleSheet(greenStyle);

        ui->label_rec_gpio4_value->setText(values.at(10));          // led4
        ui->label_rec_gpio4_value->setStyleSheet(greenStyle);

        ui->label_rec_gpio5_value->setText(values.at(11));          // led5
        ui->label_rec_gpio5_value->setStyleSheet(greenStyle);

        ui->label_rec_gpio6_value->setText(values.at(12));          // led6
        ui->label_rec_gpio6_value->setStyleSheet(greenStyle);

        ui->label_rec_gpio7_value->setText(values.at(13));          // led7
        ui->label_rec_gpio7_value->setStyleSheet(greenStyle);

        ui->label_rec_gpio8_value->setText(values.at(14));          // led8
        ui->label_rec_gpio8_value->setStyleSheet(greenStyle);

        ui->label_rec_bump_value->setText(values.at(15));           // bump
        ui->label_rec_bump_value->setStyleSheet(greenStyle);

        ui->label_rec_cliff_value->setText(values.at(16));          // cliff
        ui->label_rec_cliff_value->setStyleSheet(greenStyle);

        ui->label_rec_ir_value->setText(values.at(17));             // ir
        ui->label_rec_ir_value->setStyleSheet(greenStyle);

        ui->label_rec_yaw_vel_value->setText(values.at(18));        // imu_z_velocity
        ui->label_rec_yaw_vel_value->setStyleSheet(greenStyle);

        ui->label_rec_yaw_heading_value->setText(values.at(19));    // imu_z_heading
        ui->label_rec_yaw_heading_value->setStyleSheet(greenStyle);

        ui->label_rec_chk_value->setText(values.at(20));            // checksum
        ui->label_rec_chk_value->setStyleSheet(greenStyle);

        int16_t rar_rec = values.at(0).toInt();
        int16_t lar_rec = values.at(1).toInt();
        int32_t rec_rec = values.at(2).toInt();
        int32_t lec_rec = values.at(3).toInt();

        int16_t estop_rec = values.at(6).toInt();
        int16_t gpio1_rec = values.at(7).toInt();
        int16_t gpio2_rec = values.at(8).toInt();
        int16_t gpio3_rec = values.at(9).toInt();
        int16_t gpio4_rec = values.at(10).toInt();
        int16_t gpio5_rec = values.at(11).toInt();
        int16_t gpio6_rec = values.at(12).toInt();
        int16_t gpio7_rec = values.at(13).toInt();
        int16_t gpio8_rec = values.at(14).toInt();
        int16_t bum_rec = values.at(15).toInt();

        int32_t checksum_rec = values.at(20).toInt();

        int32_t checksum_check = rar_rec+lar_rec+rec_rec+lec_rec+estop_rec+gpio1_rec+gpio2_rec+gpio3_rec+gpio4_rec+
                gpio5_rec+gpio6_rec+gpio7_rec+gpio8_rec+bum_rec;
        if ( checksum_rec != checksum_check )
        {
            QString myString = QString("Checksum mismatch: calculated checksum = %1").arg(checksum_check);
            ui->label_checksum->setText(myString);
            //qDebug() << myString;
        }

        on_transmit();

    } else {
        QString sentMessage = "Received data has unexpected number of values:"; // + values.size() + " instead of " + EXPECTED_VALUES;
        ui->label_debug->setText(sentMessage);
        qDebug() << sentMessage;
    }
}


void MainWindow::on_transmit()
{
    if (!serialPort->isOpen()) {
        QString sentMessage = "Serial Port Not Open, Please connect to the serial port first.";
        ui->label_debug->setText(sentMessage);
        qDebug() << sentMessage;
        return;
    }

    // Get values from your LineEdit boxes
    //QString rdr = ui->comboBox_transmit_rdr->currentText();               // right_desired_rpm
    //QString ldr = ui->comboBox_transmit_ldr->currentText();             // left_desired_rpm

    // Get GPIO values (assuming lineEdit_transmit_gpio1 to gpio8)
    // You might want to validate these inputs (e.g., ensure they are 0 or 1)
//    QString estop = ui->comboBox_transmit_estop->currentText();
//    QString gpio1 = ui->comboBox_transmit_gpio1->currentText();
//    QString gpio2 = ui->comboBox_transmit_gpio2->currentText();
//    QString gpio3 = ui->comboBox_transmit_gpio3->currentText();//    QString gpio4 = ui->comboBox_transmit_gpio4->currentText();
//    QString gpio5 = ui->comboBox_transmit_gpio5->currentText();
//    QString gpio6 = ui->comboBox_transmit_gpio6->currentText();
//    QString gpio7 = ui->comboBox_transmit_gpio7->currentText();
//    QString gpio8 = ui->comboBox_transmit_gpio8->currentText();
        int16_t rdr   = ui->comboBox_transmit_rdr->currentText().toInt();
        int16_t ldr   = ui->comboBox_transmit_ldr->currentText().toInt();
        int16_t estop = ui->comboBox_transmit_estop->currentText().toInt();
        int16_t gpio1 = ui->comboBox_transmit_gpio1->currentText().toInt();
        int16_t gpio2 = ui->comboBox_transmit_gpio2->currentText().toInt();
        int16_t gpio3 = ui->comboBox_transmit_gpio3->currentText().toInt();
        int16_t gpio4 = ui->comboBox_transmit_gpio4->currentText().toInt();
        int16_t gpio5 = ui->comboBox_transmit_gpio5->currentText().toInt();
        int16_t gpio6 = ui->comboBox_transmit_gpio6->currentText().toInt();
        int16_t gpio7 = ui->comboBox_transmit_gpio7->currentText().toInt();
        int16_t gpio8 = ui->comboBox_transmit_gpio8->currentText().toInt();

    int32_t checksum = rdr+ldr+estop+gpio1+gpio2+gpio3+gpio4+gpio5+gpio6+gpio7+gpio8;
    ui->label_tran_chk_value->setText(QString::number(checksum));
    // Construct the string to send.
    // IMPORTANT: This format MUST match what your embedded device expects to receive.
    // Example: "RDR:100 LDR:100 G:10101010\n" or "100 100 1 0 1 0 1 0 1 0\n"
    // Let's assume a space-separated format similar to your receive, but with specific fields
    // You will need to define your transmit protocol carefully.
    // Example assuming: "RDR_VALUE LDR_VALUE GPIO1_VAL ... GPIO8_VAL\n"
    QString dataToSend = QString("%1 %2 %3 %4 %5 %6 %7 %8 %9 %10 %11 %12\n")
                            .arg(rdr)
                            .arg(ldr)
                            .arg(estop)
                            .arg(gpio1)
                            .arg(gpio2)
                            .arg(gpio3)
                            .arg(gpio4)
                            .arg(gpio5)
                            .arg(gpio6)
                            .arg(gpio7)
                            .arg(gpio8)
                            .arg(checksum);

    // Convert QString to QByteArray and send
    QByteArray byteArrayData = dataToSend.toUtf8();
    qint64 bytesWritten = serialPort->write(byteArrayData);

    if (bytesWritten == -1) {
        QString sentMessage = "Failed to write to serial port: " + serialPort->errorString();
        ui->label_debug->setText(sentMessage);
        qDebug() << sentMessage;
    } else {
        QString sentMessage = "Sent: " + dataToSend.trimmed();
        ui->label_debug->setText(sentMessage);
        //qDebug() << sentMessage;
        //qDebug() << "Bytes written:" << bytesWritten;
    }
}

