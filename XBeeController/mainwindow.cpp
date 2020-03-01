#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QXmlStreamReader>

#include <math.h>

#define PI 3.1415

#define PIXMAP_BLANK QCoreApplication::applicationDirPath()+"/leds/led_blank_15x15.png"
#define PIXMAP_RED_ON QCoreApplication::applicationDirPath()+"/leds/led_red_on_15x15.png"
#define PIXMAP_GREEN_ON QCoreApplication::applicationDirPath()+"/leds/led_green_on_15x15.png"
#define PIXMAP_RED_OFF QCoreApplication::applicationDirPath()+"/leds/led_red_off_15x15.png"
#define PIXMAP_GREEN_OFF QCoreApplication::applicationDirPath()+"/leds/led_green_off_15x15.png"
#define PIXMAP_YELLOW_ON QCoreApplication::applicationDirPath()+"/leds/led_yellow_on_15x15.png"
#define PIXMAP_YELLOW_OFF QCoreApplication::applicationDirPath()+"/leds/led_yellow_off_15x15.png"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),ui(new Ui::MainWindow) {
    ui->setupUi(this);

    addressHiZ = QByteArray::fromHex("0013A20040D50A4D");
    addressInband = QByteArray::fromHex("0013A20040E328ED");

    //Should not be a direct path but easier when using QtCreator
    imagePath = "/home/pi/inband-ctrl/XBeeController/images/map2.png";
    sizeWidth = 600;
    sizeHeight = 600;

    currAzimuthAngle = 180;
    targetAzimuthAngle = 180;
    antBeamWidth = 62;

    serial = new QSerialPort();
    serial->setPortName("/dev/ttyUSB0");

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

    image = QImage(imagePath);

    image = image.convertToFormat(QImage::Format_ARGB32_Premultiplied ,Qt::ColorOnly);
    terminal = new Terminal();
    terminal->setXBeeController(xb,getAddressInband());

    ui->labelLEDHiZNW->setPixmap(QPixmap(PIXMAP_BLANK));
    ui->labelLEDHiZNE->setPixmap(QPixmap(PIXMAP_BLANK));
    ui->labelLEDHiZSE->setPixmap(QPixmap(PIXMAP_BLANK));
    ui->labelLEDHiZSW->setPixmap(QPixmap(PIXMAP_BLANK));
    ui->labelLEDBand10->setPixmap(QPixmap(PIXMAP_BLANK));
    ui->labelLEDBand15->setPixmap(QPixmap(PIXMAP_BLANK));
    ui->labelLEDBand20->setPixmap(QPixmap(PIXMAP_BLANK));
    ui->labelLEDBand40->setPixmap(QPixmap(PIXMAP_BLANK));
    ui->labelLEDBand80->setPixmap(QPixmap(PIXMAP_BLANK));
    ui->labelLEDBand160->setPixmap(QPixmap(PIXMAP_BLANK));

    currTXFreq = 0;

    radioServer = new UDPServer();
    radioServer->initSocket();
    connect(radioServer, SIGNAL(dataAvailable(char *,qint64, QHostAddress *, quint16 *)),SLOT(radioDataAvailable(char *,qint64, QHostAddress *, quint16 *)));
    this->setCursor(QCursor( Qt::BlankCursor ));
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

QByteArray MainWindow::getAddressInband() {
    return(addressInband);
}

QByteArray MainWindow::getAddressHiZ() {
    return(addressHiZ);
}

void MainWindow::setTargetDir(int newDir) {
    if ((newDir >= 0) && (newDir < 360)) {
        QString cmd;
        cmd.append("SRH ");
        cmd.append(QString::number(newDir));

        terminal->addTerminalText(">> "+cmd);
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
    currADCValue = list->at(10).toInt();

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

    terminal->addTerminalText("<< "+QString(digiMeshPacket->getData()));

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
    terminal->addTerminalText(">> GCS");
    xb->unicast(addressInband, "GCS");
    terminal->addTerminalText(">> GCS");

    //radioServer.readPendingDatagrams();
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
    terminal->addTerminalText(">> SRS");
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
    QTimer::singleShot(500, this, SLOT(sendXbeeHiZDirNW2()));

    ui->labelLEDHiZNW->setPixmap(QPixmap(PIXMAP_GREEN_ON));
    ui->labelLEDHiZNE->setPixmap(QPixmap(PIXMAP_BLANK));
    ui->labelLEDHiZSE->setPixmap(QPixmap(PIXMAP_BLANK));
    ui->labelLEDHiZSW->setPixmap(QPixmap(PIXMAP_BLANK));
}

void MainWindow::on_pushButtonHiZNE_clicked() {
    QTimer::singleShot(0, this, SLOT(sendXbeeHiZDirNE1()));
    QTimer::singleShot(500, this, SLOT(sendXbeeHiZDirNE2()));

    ui->labelLEDHiZNW->setPixmap(QPixmap(PIXMAP_BLANK));
    ui->labelLEDHiZNE->setPixmap(QPixmap(PIXMAP_GREEN_ON));
    ui->labelLEDHiZSE->setPixmap(QPixmap(PIXMAP_BLANK));
    ui->labelLEDHiZSW->setPixmap(QPixmap(PIXMAP_BLANK));
}

void MainWindow::on_pushButtonHiZSE_clicked() {
    QTimer::singleShot(0, this, SLOT(sendXbeeHiZDirSE1()));
    QTimer::singleShot(500, this, SLOT(sendXbeeHiZDirSE2()));

    ui->labelLEDHiZNW->setPixmap(QPixmap(PIXMAP_BLANK));
    ui->labelLEDHiZNE->setPixmap(QPixmap(PIXMAP_BLANK));
    ui->labelLEDHiZSE->setPixmap(QPixmap(PIXMAP_GREEN_ON));
    ui->labelLEDHiZSW->setPixmap(QPixmap(PIXMAP_BLANK));
}

void MainWindow::on_pushButtonHiZSW_clicked() {
    QTimer::singleShot(0, this, SLOT(sendXbeeHiZDirSW1()));
    QTimer::singleShot(500, this, SLOT(sendXbeeHiZDirSW2()));

    ui->labelLEDHiZNW->setPixmap(QPixmap(PIXMAP_BLANK));
    ui->labelLEDHiZNE->setPixmap(QPixmap(PIXMAP_BLANK));
    ui->labelLEDHiZSE->setPixmap(QPixmap(PIXMAP_BLANK));
    ui->labelLEDHiZSW->setPixmap(QPixmap(PIXMAP_GREEN_ON));
}

void MainWindow::activateBand(enum band bandIndex) {
    qDebug() << "Activate Band: " << bandIndex;

    switch(bandIndex) {
        case BAND_160:
	    terminal->addTerminalText(">> SDO HI 12");
            xb->unicast(addressInband, "SDO HI 12");
            ui->pushButton160m->setChecked(true);
            ui->labelCurrentBandValue->setText("160m");
            ui->labelLEDBand160->setPixmap(QPixmap(PIXMAP_GREEN_ON));
            break;
        case BAND_80:
	    terminal->addTerminalText(">> SDO HI 11");
            xb->unicast(addressInband, "SDO HI 11");
            ui->pushButton80m->setChecked(true);
            ui->labelCurrentBandValue->setText("80m");
            ui->labelLEDBand80->setPixmap(QPixmap(PIXMAP_GREEN_ON));
            break;
        case BAND_40:
	    terminal->addTerminalText(">> SDO HI 10");
            xb->unicast(addressInband, "SDO HI 10");
            ui->pushButton40m->setChecked(true);
            ui->labelCurrentBandValue->setText("40m");
            ui->labelLEDBand40->setPixmap(QPixmap(PIXMAP_GREEN_ON));
            break;
        case BAND_20:
	    terminal->addTerminalText(">> SDO HI 9");
            xb->unicast(addressInband, "SDO HI 9");
            ui->pushButton20m->setChecked(true);
            ui->labelCurrentBandValue->setText("20m");
            ui->labelLEDBand20->setPixmap(QPixmap(PIXMAP_GREEN_ON));
            break;
        case BAND_15:
	    terminal->addTerminalText(">> SDO HI 9");
            xb->unicast(addressInband, "SDO HI 9");
            ui->pushButton15m->setChecked(true);
            ui->labelCurrentBandValue->setText("15m");
            ui->labelLEDBand15->setPixmap(QPixmap(PIXMAP_GREEN_ON));
            break;
        case BAND_10:
	    terminal->addTerminalText(">> SDO HI 9");
            xb->unicast(addressInband, "SDO HI 9");
            ui->pushButton10m->setChecked(true);
            ui->labelCurrentBandValue->setText("10m");
            ui->labelLEDBand10->setPixmap(QPixmap(PIXMAP_GREEN_ON));
            break;
        default:
            break;
    }

    currentBand = bandIndex;
}

void MainWindow::deactivateBand(enum band bandIndex) {
    qDebug() << "Deactivate Band: " << bandIndex;
    switch(bandIndex) {
        case BAND_160:
	    terminal->addTerminalText(">> SDO LO 12");
            xb->unicast(addressInband, "SDO LO 12");
            ui->pushButton160m->setChecked(false);
            ui->labelLEDBand160->setPixmap(QPixmap(PIXMAP_BLANK));
            break;
        case BAND_80:
	    terminal->addTerminalText(">> SDO LO 11");
            xb->unicast(addressInband, "SDO LO 11");
            ui->pushButton80m->setChecked(false);
            ui->labelLEDBand80->setPixmap(QPixmap(PIXMAP_BLANK));
            break;
        case BAND_40:
	    terminal->addTerminalText(">> SDO LO 10");
            xb->unicast(addressInband, "SDO LO 10");
            ui->pushButton40m->setChecked(false);
            ui->labelLEDBand40->setPixmap(QPixmap(PIXMAP_BLANK));
            break;
        case BAND_20:
            terminal->addTerminalText(">> SDO LO 9");
            xb->unicast(addressInband, "SDO LO 9");
            ui->pushButton20m->setChecked(false);
            ui->labelLEDBand20->setPixmap(QPixmap(PIXMAP_BLANK));
            break;
        case BAND_15:
	    terminal->addTerminalText(">> SDO LO 9");
            xb->unicast(addressInband, "SDO LO 9");
            ui->pushButton15m->setChecked(false);
            ui->labelLEDBand15->setPixmap(QPixmap(PIXMAP_BLANK));
            break;
        case BAND_10:
	    terminal->addTerminalText(">> SDO LO 9");
            xb->unicast(addressInband, "SDO LO 9");
            ui->pushButton10m->setChecked(false);
            ui->labelLEDBand10->setPixmap(QPixmap(PIXMAP_BLANK));
            break;
        default:
            break;
    }

    currentBand = BAND_NONE;

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
}

void MainWindow::on_pushButtonClearErrors_clicked() {
    terminal->addTerminalText(">> SEC");
    xb->unicast(addressInband, "SEC");
}

void MainWindow::on_pushButtonTerminal_clicked() {
    terminal->hide();
    terminal->show();
}

void MainWindow::radioDataAvailable(char *data, qint64 size, QHostAddress *fromAddr, quint16 *port) {
    QXmlStreamReader xml(data);

    int txFreq = 0;

    while(!xml.atEnd()) {
        xml.readNext();
        if (xml.isStartElement()) {
            QString name = xml.name().toString();

            if (name == "TXFreq") {
                txFreq = xml.readElementText().toInt();
            }
        }
    }

    if (ui->checkBoxBandControlAuto) {
        if (txFreq != currTXFreq) {
            if ((txFreq > 10000) && txFreq < 250000) {
                if (currentBand != BAND_160)
			on_pushButton160m_clicked(true);
            }
            else if ((txFreq > 250000) && txFreq < 400000) {
		 if (currentBand != BAND_80)
                	on_pushButton80m_clicked(true);
            }
            else if ((txFreq > 650000) && txFreq < 800000) {
		 if (currentBand != BAND_40)
                	on_pushButton40m_clicked(true);
            }
            else if ((txFreq > 1300000) && txFreq < 1500000) {
		 if (currentBand != BAND_20)
                	on_pushButton20m_clicked(true);
            }
            else if ((txFreq > 2000000) && txFreq < 2200000) {
		 if (currentBand != BAND_15)
                	on_pushButton15m_clicked(true);
            }
            else if ((txFreq > 2700000) && txFreq < 3000000) {
		 if (currentBand != BAND_10)
                	on_pushButton10m_clicked(true);
            }
            else
                on_pushButtonNone_clicked();

            currTXFreq = txFreq;
        }
    }
}

void MainWindow::on_pushButtonQuit_clicked()
{
    deactivateBand(BAND_160);
    deactivateBand(BAND_80);
    deactivateBand(BAND_40);
    deactivateBand(BAND_20);
    deactivateBand(BAND_15);
    deactivateBand(BAND_10);

    QTimer::singleShot(2000, this, SLOT(quitApplication()));
}
