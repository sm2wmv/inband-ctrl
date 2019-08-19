#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <math.h>

#define PI 3.1415

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),ui(new Ui::MainWindow) {
    ui->setupUi(this);

    addressHiZ = QByteArray::fromHex("0013A20040D50A4D");
    addressInband = QByteArray::fromHex("0013A20040F7B346");

    imagePath = "images/map.png";
    sizeWidth = 600;
    sizeHeight = 600;

    currAzimuthAngle = 180;
    targetAzimuthAngle = 180;
    antBeamWidth = 62;

    serial = new QSerialPort();
    serial->setPortName("com15");

    xb = new QTXB(serial);

    connect(xb, SIGNAL(receivedATCommandResponse(ATCommandResponse*)), this, SLOT(parseXBeeATCommandResponse(ATCommandResponse*)));
    connect(xb, SIGNAL(receivedModemStatus(ModemStatus*)), this, SLOT(parseXBeeModemStatus(ModemStatus*)));
    connect(xb, SIGNAL(receivedTransmitStatus(TransmitStatus*)), this, SLOT(parseXBeeTransmitStatus(TransmitStatus*)));
    connect(xb, SIGNAL(receivedRXIndicator(RXIndicator*)), this, SLOT(parseXBeeRXIndicator(RXIndicator*)));
    connect(xb, SIGNAL(receivedRXIndicatorExplicit(RXIndicatorExplicit*)), this, SLOT(parseXBeeRXIndicatorExplicit(RXIndicatorExplicit*)));
    connect(xb, SIGNAL(receivedNodeIdentificationIndicator(NodeIdentificationIndicator*)), this, SLOT(parseXBeeNodeIdentificationIndicator(NodeIdentificationIndicator*)));
    connect(xb, SIGNAL(receivedRemoteCommandResponse(RemoteCommandResponse*)), this, SLOT(parseXBeeRemoteCommandResponse(RemoteCommandResponse*)));

    timerPollXbee = new QTimer(this);
    connect(timerPollXbee, SIGNAL(timeout()), this, SLOT(timerPollXbeeTimeout()));
    timerPollXbee->start(1000);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::quitApplication() {
    qDebug("Bye Bye");
    QApplication::quit();
}

void MainWindow::closeEvent(QCloseEvent *event) {
    event->ignore();

    deactivateBand(BAND_160);
    deactivateBand(BAND_80);
    deactivateBand(BAND_40);
    deactivateBand(BAND_20);
    deactivateBand(BAND_15);
    deactivateBand(BAND_10);

    QTimer::singleShot(2000, this, SLOT(quitApplication()));
}

void MainWindow::paintEvent(QPaintEvent *event) {
    QPainter painter(this);

    image = QImage(imagePath);
    image.convertToFormat(QImage::Format_ARGB32_Premultiplied ,Qt::ColorOnly);

    painter.drawImage(-2 , -2 , image);

    float rectWidth = sizeWidth - (sizeWidth * 0.06);
    float rectHeight = sizeHeight - (sizeHeight * 0.06);
    QRectF rectangle(sizeWidth/2-rectWidth/2,sizeWidth/2-rectHeight/2, rectWidth-2, rectHeight-2);

    painter.setPen(Qt::CURRENT_DIR_BEAMWIDTH_COLOR);
    painter.setBrush(QBrush(QColor(Qt::CURRENT_DIR_BEAMWIDTH_COLOR),Qt::FDiagPattern));

    painter.drawPie(rectangle,(360-(-90+currAzimuthAngle + antBeamWidth/2))*16,antBeamWidth*16);

    if ((rotatorStatus == FLAG_ROTATOR_ROTATION_CW) || (rotatorStatus == FLAG_ROTATOR_ROTATION_CCW)) {
        painter.setBrush(QBrush(QColor(Qt::TARGET_DIR_BEAMWIDTH_COLOR),Qt::SolidPattern));
        painter.drawPie(rectangle,(360-(-90+targetAzimuthAngle + 0.5))*16,0.5*16);
    }
}

void MainWindow::setTargetDir(int newDir) {
    if ((newDir >= 0) && (newDir < 360)) {
        QString cmd;
        cmd.append("SRH ");
        cmd.append(QString::number(newDir));

        xb->unicast(addressInband, cmd);
        qDebug() << "NEW DIR: " << newDir;
    }
}

void MainWindow::mousePressEvent ( QMouseEvent * event ) {
  if ((event->x() >= 8) && (event->y() >= 8) && (event->x() <= (sizeWidth)) && (event->y() <= sizeHeight)) {
      double mapX = abs(sizeWidth/2 - event->x());
      double mapY = sizeHeight/2 - event->y();

      if ((sizeWidth/2 - event->x()) < 0)
                    setTargetDir(90-atan(mapY/mapX)*(180/PI));
            else
                    setTargetDir(270+atan(mapY/mapX)*(180/PI));
    }
}

void MainWindow::updateGUI(QString command) {
    if (command == "GCS") {
        ui->labelTempBoxValue->setText(QString::number(temperatureBox) + "°C");
        ui->labelTempOutsideValue->setText(QString::number(temperatureOutside) + "°C");
        ui->labelVoltageValue->setText(QString::number(batteryVoltage) + "V");
        ui->labelCurrentValue->setText(QString::number(batteryCurrent) + "A");
        ui->labelRotatorHeadingValue->setText(QString::number(currAzimuthAngle) + "°");
        ui->labelRotatorTargetValue->setText(QString::number(targetAzimuthAngle) + "°");

        if (rotatorStatus == 0) {
            ui->labelRotatorStateValue->setText("Stopped");
            ui->labelRotatorTargetValue->setText("");
        }
        else if (rotatorStatus == 1)
            ui->labelRotatorStateValue->setText("CCW");
        else if (rotatorStatus == 2)
            ui->labelRotatorStateValue->setText("CW");
        else if (rotatorStatus == 3) {
            ui->labelRotatorStateValue->setText("CCW (M)");
            ui->labelRotatorTargetValue->setText("");
        }
        else if (rotatorStatus == 4) {
            ui->labelRotatorStateValue->setText("CW (M)");
            ui->labelRotatorTargetValue->setText("");
        }
        else {
            ui->labelRotatorStateValue->setText("Unknown");
            qDebug() << "UKNOWN ROTATOR STATE: " << rotatorStatus;
        }

        if (errorStatus != 0)
            ui->labelERROR->setText("ERROR");
        else {
            ui->labelERROR->setText("");
            ui->labelERRORMessage->setText("");
        }

        if (errorStatus & (1<<1))
            ui->labelERRORMessage->setText("END LIMIT CW");
        else if (errorStatus & (1<<2))
            ui->labelERRORMessage->setText("END LIMIT CCW");
        else if (errorStatus & (1<<0))
            ui->labelERRORMessage->setText("ROTATOR STUCK");

        repaint();
    }
}

void MainWindow::parseGCSData(QList<QByteArray> *list) {
    errorStatus = list->at(1).toInt();
    rotatorStatus = list->at(2).toInt();
    chargeStatus = list->at(3).toInt();
    currAzimuthAngle = list->at(4).toInt();
    targetAzimuthAngle = list->at(5).toInt();
    temperatureBox = list->at(6).toFloat();
    temperatureOutside = list->at(7).toFloat();
    batteryVoltage = list->at(8).toFloat();
    batteryCurrent = list->at(9).toFloat();

/*    qDebug() << "RAW DATA\n--------";
    for (int i=0;i<list->length(); i++) {

        qDebug() << "[" << i << "]: " << list->at(i);
    }

    qDebug() << "errorStatus:\t" << errorStatus;
    qDebug() << "rotatorStatus:\t " << rotatorStatus;
    qDebug() << "chargeStatus:\t" << chargeStatus;
    qDebug() << "currAzimuthAngle:\t" << currAzimuthAngle;
    qDebug() << "targetAzimuthAngle:\t" << targetAzimuthAngle;
    qDebug() << "temperatureBox:\t" << temperatureBox;
    qDebug() << "temperatureOutside:\t" << temperatureOutside;
    qDebug() << "batteryVoltage:\t" << batteryVoltage;
    qDebug() << "batteryCurrent:\t" << batteryCurrent;*/

    updateGUI("GCS");
}

void MainWindow::parseXBeeATCommandResponse(ATCommandResponse *digiMeshPacket) {
    //qDebug("parseXBeeATCommandResponse");
}

void MainWindow::parseXBeeModemStatus(ModemStatus *digiMeshPacket) {
    //qDebug("parseXBeeModemStatus");
}

void MainWindow::parseXBeeTransmitStatus(TransmitStatus *digiMeshPacket) {
//    qDebug("parseXBeeTransmitStatus");
}

void MainWindow::parseXBeeRXIndicator(RXIndicator *digiMeshPacket) {
    QList<QByteArray> fields = digiMeshPacket->getData().split(' ');

    if (fields.at(0) == "GCS") {
        parseGCSData(&fields);
    }
}

void MainWindow::parseXBeeRXIndicatorExplicit(RXIndicatorExplicit *digiMeshPacket) {
    //qDebug("parseXBeeRXIndicatorExplicit");
}

void MainWindow::parseXBeeNodeIdentificationIndicator(NodeIdentificationIndicator *digiMeshPacket) {
    //qDebug("parseXBeeNodeIdentificationIndicator");
}

void MainWindow::parseXBeeRemoteCommandResponse(RemoteCommandResponse *digiMeshPacket) {
    //  qDebug("parseXBeeRemoteCommandResponse");
}

void MainWindow::timerPollXbeeTimeout() {
    xb->unicast(addressInband, "GCS");
}

void MainWindow::on_pushButtonPreset1_clicked() {
    setTargetDir(50);
}

void MainWindow::on_pushButtonPreset2_clicked() {
    setTargetDir(80);
}

void MainWindow::on_pushButtonPreset3_clicked() {
    setTargetDir(180);
}

void MainWindow::on_pushButtonPreset4_clicked() {
    setTargetDir(270);
}

void MainWindow::on_pushButtonPreset5_clicked() {
    setTargetDir(300);
}

void MainWindow::on_pushButtonSTOP_clicked() {
    xb->unicast(addressInband, "SRS");
}

void MainWindow::sendXbeeHiZDirNW1() {
    xb->unicastRemoteCommandRequest(addressHiZ, "D4", QByteArray::fromHex("04"));
}

void MainWindow::sendXbeeHiZDirNW2() {
    xb->unicastRemoteCommandRequest(addressHiZ, "P2", QByteArray::fromHex("04"));
}

void MainWindow::sendXbeeHiZDirNE1() {
    xb->unicastRemoteCommandRequest(addressHiZ, "D4", QByteArray::fromHex("04"));
}

void MainWindow::sendXbeeHiZDirNE2() {
    xb->unicastRemoteCommandRequest(addressHiZ, "P2", QByteArray::fromHex("05"));
}

void MainWindow::sendXbeeHiZDirSE1() {
    xb->unicastRemoteCommandRequest(addressHiZ, "D4", QByteArray::fromHex("05"));
}

void MainWindow::sendXbeeHiZDirSE2() {
    xb->unicastRemoteCommandRequest(addressHiZ, "P2", QByteArray::fromHex("04"));
}

void MainWindow::sendXbeeHiZDirSW1() {
    xb->unicastRemoteCommandRequest(addressHiZ, "D4", QByteArray::fromHex("05"));
}

void MainWindow::sendXbeeHiZDirSW2() {
    xb->unicastRemoteCommandRequest(addressHiZ, "P2", QByteArray::fromHex("05"));
}

void MainWindow::on_pushButtonHiZNW_clicked() {
    QTimer::singleShot(0, this, SLOT(sendXbeeHiZDirNW1()));
    QTimer::singleShot(250, this, SLOT(sendXbeeHiZDirNW2()));
}

void MainWindow::on_pushButtonHiZNE_clicked() {
    QTimer::singleShot(0, this, SLOT(sendXbeeHiZDirNE1()));
    QTimer::singleShot(250, this, SLOT(sendXbeeHiZDirNE2()));}

void MainWindow::on_pushButtonHiZSE_clicked() {
    QTimer::singleShot(0, this, SLOT(sendXbeeHiZDirSE1()));
    QTimer::singleShot(250, this, SLOT(sendXbeeHiZDirSE2()));}

void MainWindow::on_pushButtonHiZSW_clicked() {
    QTimer::singleShot(0, this, SLOT(sendXbeeHiZDirSW1()));
    QTimer::singleShot(250, this, SLOT(sendXbeeHiZDirSW2()));
}

void MainWindow::activateBand(enum band bandIndex) {
    qDebug() << "Activate Band: " << bandIndex;

    switch(bandIndex) {
        case BAND_160:
            xb->unicast(addressInband, "SDO HI 12");
            ui->pushButton160m->setChecked(true);
            ui->labelCurrentBandValue->setText("160m");
            break;
        case BAND_80:
            xb->unicast(addressInband, "SDO HI 11");
            ui->pushButton80m->setChecked(true);
            ui->labelCurrentBandValue->setText("80m");
            break;
        case BAND_40:
            xb->unicast(addressInband, "SDO HI 10");
            ui->pushButton40m->setChecked(true);
            ui->labelCurrentBandValue->setText("40m");
            break;
        case BAND_20:
            xb->unicast(addressInband, "SDO HI 9");
            ui->pushButton20m->setChecked(true);
            ui->labelCurrentBandValue->setText("20m");
            break;
        case BAND_15:
            xb->unicast(addressInband, "SDO HI 9");
            ui->pushButton15m->setChecked(true);
            ui->labelCurrentBandValue->setText("15m");
            break;
        case BAND_10:
            xb->unicast(addressInband, "SDO HI 9");
            ui->pushButton10m->setChecked(true);
            ui->labelCurrentBandValue->setText("10m");
            break;
        default:
            break;
    }

    currentBand = bandIndex;
    ui->pushButtonNone->setChecked(false);
}

void MainWindow::deactivateBand(enum band bandIndex) {
    qDebug() << "Deactivate Band: " << bandIndex;
    switch(bandIndex) {
        case BAND_160:
            xb->unicast(addressInband, "SDO LO 12");
            ui->pushButton160m->setChecked(false);
            break;
        case BAND_80:
            xb->unicast(addressInband, "SDO LO 11");
            ui->pushButton80m->setChecked(false);
            break;
        case BAND_40:
            xb->unicast(addressInband, "SDO LO 10");
            ui->pushButton40m->setChecked(false);
            break;
        case BAND_20:
            xb->unicast(addressInband, "SDO LO 9");
            ui->pushButton20m->setChecked(false);
            break;
        case BAND_15:
            xb->unicast(addressInband, "SDO LO 9");
            ui->pushButton15m->setChecked(false);
            break;
        case BAND_10:
            xb->unicast(addressInband, "SDO LO 9");
            ui->pushButton10m->setChecked(false);
            break;
        default:
            break;
    }

    currentBand = BAND_NONE;

    ui->pushButtonNone->setChecked(true);
    ui->labelCurrentBandValue->setText("None");
}

void MainWindow::on_pushButton160m_clicked(bool checked) {
    if (checked) {
        deactivateBand(currentBand);
        activateBand(BAND_160);
    }
    else {
        deactivateBand(BAND_160);
    }
}

void MainWindow::on_pushButton80m_clicked(bool checked) {
    if (checked) {
        deactivateBand(currentBand);
        activateBand(BAND_80);
    }
    else {
        deactivateBand(BAND_80);
    }
}

void MainWindow::on_pushButton40m_clicked(bool checked) {
    if (checked) {
        deactivateBand(currentBand);
        activateBand(BAND_40);
    }
    else {
        deactivateBand(BAND_40);
    }
}

void MainWindow::on_pushButton20m_clicked(bool checked) {
    if (checked) {
        deactivateBand(currentBand);
        activateBand(BAND_20);
    }
    else {
        deactivateBand(BAND_20);
    }
}

void MainWindow::on_pushButton15m_clicked(bool checked) {
    if (checked) {
        deactivateBand(currentBand);
        activateBand(BAND_15);
    }
    else {
        deactivateBand(BAND_15);
    }
}

void MainWindow::on_pushButton10m_clicked(bool checked) {
    if (checked) {
        deactivateBand(currentBand);
        activateBand(BAND_10);
    }
    else {
        deactivateBand(BAND_10);
    }
}

void MainWindow::on_pushButtonNone_clicked() {
    /*deactivateBand(BAND_160);
    deactivateBand(BAND_80);
    deactivateBand(BAND_40);
    deactivateBand(BAND_20);
    deactivateBand(BAND_15);
    deactivateBand(BAND_10);*/
    deactivateBand(currentBand);

    ui->pushButtonNone->setChecked(true);
}

void MainWindow::on_pushButtonClearErrors_clicked() {
    xb->unicast(addressInband, "SEC");
}