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

    const int EXPECTED_VALUES = 25; // Total number of values in your sprintf format
    QString greenStyle = "background-color: #90EE90; color: black;";
    QString solidBlue = "color: blue;";
    QString solidRed = "color: red;";

    if (values.size() == EXPECTED_VALUES)
    {
        // Map values to UI labels based on the order in your sprintf:
        // "%hd %hd %ld %ld %.4f %.4f %hd %hd %hd %hd %hd %hd %hd %hd %hd %hd %.4f %.4f %.4f %.4f %hd\r\n"
        // Index correspondence:
        // 0: right_actual_rpm (hd)
        // 1: left_actual_rpm (hd)
        // 2: right_encoder_count (ld)
        // 3: left_encoder_count (ld)
        // 4: led1 (hd)
        // 5: led2 (hd)
        // 6: led3 (hd)
        // 7: led4 (hd)
        // 8: gpio1 (hd)
        // 9: gpio2 (hd)
        // 10: gpio3 (hd)
        // 11: gpio4 (hd)
        // 12: adc1 (hd)
        // 13: adc2 (hd)
        // 14: estop_status (hd)
        // 15: emergency_shutdown (hd)
        // 16: bump1 (hd)
        // 17: temperature (f)
        // 18: volt (f)
        // 19: ampere (f)
        // 20: cliff (f)
        // 21: ir (f)
        // 22: imu_z_velocity (f)
        // 23: imu_z_heading (f)
        // 24: checksum (hd)

        ui->label_rec_rar_value->setText(values.at(0));             // right_actual_rpm
        ui->label_rec_rar_value->setStyleSheet(greenStyle);

        ui->label_rec_lar_value->setText(values.at(1));             // left_actual_rpm
        ui->label_rec_lar_value->setStyleSheet(greenStyle);

        ui->label_rec_rec_value->setText(values.at(2));             // right_encoder_count
        ui->label_rec_rec_value->setStyleSheet(greenStyle);

        ui->label_rec_lec_value->setText(values.at(3));             // left_encoder_count
        ui->label_rec_lec_value->setStyleSheet(greenStyle);

        ui->label_rec_led1_value->setText(values.at(4));           // led1
        ui->label_rec_led1_value->setStyleSheet(greenStyle);

        ui->label_rec_led2_value->setText(values.at(5));           // led2
        ui->label_rec_led2_value->setStyleSheet(greenStyle);

        ui->label_rec_led3_value->setText(values.at(6));           // led3
        ui->label_rec_led3_value->setStyleSheet(greenStyle);

        ui->label_rec_led4_value->setText(values.at(7));           // led4
        ui->label_rec_led4_value->setStyleSheet(greenStyle);

        ui->label_rec_gpio1_value->setText(values.at(8));          // gpio1
        ui->label_rec_gpio1_value->setStyleSheet(greenStyle);

        ui->label_rec_gpio2_value->setText(values.at(9));          // gpio2
        ui->label_rec_gpio2_value->setStyleSheet(greenStyle);

        ui->label_rec_gpio3_value->setText(values.at(10));          // gpio3
        ui->label_rec_gpio3_value->setStyleSheet(greenStyle);

        ui->label_rec_gpio4_value->setText(values.at(11));          // gpio4
        ui->label_rec_gpio4_value->setStyleSheet(greenStyle);

        ui->label_rec_adc1_value->setText(values.at(12));           // adc1
        ui->label_rec_adc1_value->setStyleSheet(greenStyle);

        ui->label_rec_adc2_value->setText(values.at(13));           // adc2
        ui->label_rec_adc2_value->setStyleSheet(greenStyle);

        ui->label_rec_estop_status_value->setText(values.at(14));   // estop
        ui->label_rec_estop_status_value->setStyleSheet(greenStyle);

        ui->label_rec_shutdown_value->setText(values.at(15));       // emergency shutdown
        ui->label_rec_shutdown_value->setStyleSheet(greenStyle);

        ui->label_rec_bump_value->setText(values.at(16));           // bump
        ui->label_rec_bump_value->setStyleSheet(greenStyle);

        ui->label_rec_temperature_value->setText(values.at(17));    // temperature
        ui->label_rec_temperature_value->setStyleSheet(greenStyle);

        ui->label_rec_volt_value->setText(values.at(18));            // volt
        ui->label_rec_volt_value->setStyleSheet(greenStyle);

        ui->label_rec_ampere_value->setText(values.at(19));          // ampere
        ui->label_rec_ampere_value->setStyleSheet(greenStyle);

        ui->label_rec_cliff_value->setText(values.at(20));          // cliff
        ui->label_rec_cliff_value->setStyleSheet(greenStyle);

        ui->label_rec_ir_value->setText(values.at(21));             // ir
        ui->label_rec_ir_value->setStyleSheet(greenStyle);

        ui->label_rec_yaw_vel_value->setText(values.at(22));        // imu_z_velocity
        ui->label_rec_yaw_vel_value->setStyleSheet(greenStyle);

        ui->label_rec_yaw_heading_value->setText(values.at(23));    // imu_z_heading
        ui->label_rec_yaw_heading_value->setStyleSheet(greenStyle);

        ui->label_rec_chk_value->setText(values.at(24));            // checksum
        ui->label_rec_chk_value->setStyleSheet(greenStyle);

        int16_t rar_rec = values.at(0).toInt();
        int16_t lar_rec = values.at(1).toInt();
        int32_t rec_rec = values.at(2).toInt();
        int32_t lec_rec = values.at(3).toInt();

        int16_t led1_rec  = values.at(4).toInt();
        int16_t led2_rec  = values.at(5).toInt();
        int16_t led3_rec  = values.at(6).toInt();
        int16_t led4_rec  = values.at(7).toInt();
        int16_t gpio1_rec = values.at(8).toInt();
        int16_t gpio2_rec = values.at(9).toInt();
        int16_t gpio3_rec = values.at(10).toInt();
        int16_t gpio4_rec = values.at(11).toInt();
        int16_t adc1_rec  = values.at(12).toInt();
        int16_t adc2_rec  = values.at(13).toInt();
        int16_t e_status  = values.at(14).toInt();
        int16_t emergency = values.at(15).toInt();
        int16_t bump1_rec = values.at(16).toInt();

        int32_t checksum_rec = values.at(24).toInt();

        int32_t checksum_check = rar_rec+
        lar_rec+
        rec_rec+
        lec_rec+
        led1_rec+
        led2_rec+
        led3_rec+
        led4_rec+
        gpio1_rec+
        gpio2_rec+
        gpio3_rec+
        gpio4_rec+
        adc1_rec+
        adc2_rec+
        e_status+
        emergency+
        bump1_rec;

        if ( checksum_rec != checksum_check )
        {
            QString myString = QString("Checksum mismatch: calculated checksum = %1").arg(checksum_check);
            ui->label_checksum->setStyleSheet(solidRed);
            ui->label_checksum->setText(myString);
            //qDebug() << myString;
        }
        else
        {
            QString my_str = "Checksum OK!";
            ui->label_checksum->setStyleSheet(solidBlue);
            ui->label_checksum->setText(my_str);
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

    // Get GPIO values (assuming lineEdit_transmit_led1 to gpio4)
    // You might want to validate these inputs (e.g., ensure they are 0 or 1)
//    QString estop = ui->comboBox_transmit_estop->currentText();
//    QString led1 = ui->comboBox_transmit_led1->currentText();
//    QString led2 = ui->comboBox_transmit_led2->currentText();
//    QString led3 = ui->comboBox_transmit_led3->currentText();//    QString led4 = ui->comboBox_transmit_led4->currentText();
//    QString gpio1 = ui->comboBox_transmit_gpio1->currentText();
//    QString gpio2 = ui->comboBox_transmit_gpio2->currentText();
//    QString gpio3 = ui->comboBox_transmit_gpio3->currentText();
//    QString gpio4 = ui->comboBox_transmit_gpio4->currentText();
        int16_t rdr   = ui->comboBox_transmit_rdr->currentText().toInt();
        int16_t ldr   = ui->comboBox_transmit_ldr->currentText().toInt();

        int16_t led1 = ui->comboBox_transmit_led1->currentText().toInt();
        int16_t led2 = ui->comboBox_transmit_led2->currentText().toInt();
        int16_t led3 = ui->comboBox_transmit_led3->currentText().toInt();
        int16_t led4 = ui->comboBox_transmit_led4->currentText().toInt();

        int16_t gpio1 = ui->comboBox_transmit_gpio1->currentText().toInt();
        int16_t gpio2 = ui->comboBox_transmit_gpio2->currentText().toInt();
        int16_t gpio3 = ui->comboBox_transmit_gpio3->currentText().toInt();
        int16_t gpio4 = ui->comboBox_transmit_gpio4->currentText().toInt();

        int16_t adc1 = ui->comboBox_transmit_adc1->currentText().toInt();
        int16_t adc2 = ui->comboBox_transmit_adc2->currentText().toInt();


        int16_t estop = ui->comboBox_transmit_estop->currentText().toInt();
        int16_t emergency_shutdown = ui->comboBox_transmit_shutdown->currentText().toInt();

    int32_t checksum = rdr+ldr+
            led1+led2+led3+led4+
            gpio1+gpio2+gpio3+gpio4+
            adc1+adc2+
            estop+emergency_shutdown;
    ui->label_tran_chk_value->setText(QString::number(checksum));
    // Construct the string to send.
    // IMPORTANT: This format MUST match what your embedded device expects to receive.
    // Example: "RDR:100 LDR:100 G:10101010\n" or "100 100 1 0 1 0 1 0 1 0\n"
    // Let's assume a space-separated format similar to your receive, but with specific fields
    // You will need to define your transmit protocol carefully.
    // Example assuming: "RDR_VALUE LDR_VALUE led1_VAL ... gpio4_VAL\n"
    QString dataToSend = QString("%1 %2 %3 %4 %5 %6 %7 %8 %9 %10 %11 %12 %13 %14 %15\n")
                            .arg(rdr)
                            .arg(ldr)
                            .arg(led1)
                            .arg(led2)
                            .arg(led3)
                            .arg(led4)
                            .arg(gpio1)
                            .arg(gpio2)
                            .arg(gpio3)
                            .arg(gpio4)
                            .arg(adc1)
                            .arg(adc2)
                            .arg(estop)
                            .arg(emergency_shutdown)
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

