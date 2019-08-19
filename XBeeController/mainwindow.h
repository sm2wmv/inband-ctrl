#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "qtxb.h"

#include <QMainWindow>
#include <QDebug>

#include <QPainter>
#include <QGraphicsView>
#include <QHBoxLayout>
#include <QLabel>

#include <QMouseEvent>
#include <QColor>
#include <QBrush>
#include <QSettings>
#include <QTimer>

#define TARGET_DIR_BEAMWIDTH	1
#define TARGET_DIR_BEAMWIDTH_COLOR	black
#define CURRENT_DIR_BEAMWIDTH_COLOR	black

#define FLAG_ROTATOR_ROTATION_STOPPED   0
#define FLAG_ROTATOR_ROTATION_CCW   1
#define FLAG_ROTATOR_ROTATION_CW    2
#define FLAG_ROTATOR_ROTATION_MANUAL_CCW   3
#define FLAG_ROTATOR_ROTATION_MANUAL_CW    4



namespace Ui {
class MainWindow;
}

enum band {
    BAND_NONE,
    BAND_160,
    BAND_80,
    BAND_40,
    BAND_20,
    BAND_15,
    BAND_10
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    QPainter painter;
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void updateGUI(QString command);
    void setTargetDir(int newDir);
private:
    Ui::MainWindow *ui;
    QString imagePath;
    QImage image;
    int sizeWidth;
    int sizeHeight;
    int currAzimuthAngle;
    int targetAzimuthAngle;
    int antBeamWidth;
    int errorStatus;
    int rotatorStatus;
    int chargeStatus;
    float temperatureBox;
    float temperatureOutside;
    float batteryVoltage;
    float batteryCurrent;
    QTimer *timerPollXbee;
    QTXB *xb;
    QSerialPort *serial;
    QByteArray addressHiZ;
    QByteArray addressInband;
    void parseGCSData(QList<QByteArray> *list);
    enum band currentBand;
    void deactivateBand(enum band bandIndex);
    void activateBand(enum band bandIndex);

protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent ( QMouseEvent * event );
private slots:
    void parseXBeeATCommandResponse(ATCommandResponse *digiMeshPacket);
    void parseXBeeModemStatus(ModemStatus *digiMeshPacket);
    void parseXBeeTransmitStatus(TransmitStatus *digiMeshPacket);
    void parseXBeeRXIndicator(RXIndicator *digiMeshPacket);
    void parseXBeeRXIndicatorExplicit(RXIndicatorExplicit *digiMeshPacket);
    void parseXBeeNodeIdentificationIndicator(NodeIdentificationIndicator *digiMeshPacket);
    void parseXBeeRemoteCommandResponse(RemoteCommandResponse *digiMeshPacket);
    void on_pushButtonPreset1_clicked();
    void timerPollXbeeTimeout();
    void on_pushButtonPreset2_clicked();
    void on_pushButtonPreset3_clicked();
    void on_pushButtonPreset4_clicked();
    void on_pushButtonPreset5_clicked();
    void on_pushButtonSTOP_clicked();
    void on_pushButtonHiZNW_clicked();
    void on_pushButtonHiZNE_clicked();
    void on_pushButtonHiZSE_clicked();
    void on_pushButtonHiZSW_clicked();
    void sendXbeeHiZDirNW1();
    void sendXbeeHiZDirNE1();
    void sendXbeeHiZDirSE1();
    void sendXbeeHiZDirSW1();
    void sendXbeeHiZDirNW2();
    void sendXbeeHiZDirNE2();
    void sendXbeeHiZDirSE2();
    void sendXbeeHiZDirSW2();
    void on_pushButton160m_clicked(bool checked);
    void on_pushButton80m_clicked(bool checked);
    void on_pushButton40m_clicked(bool checked);
    void on_pushButton20m_clicked(bool checked);
    void on_pushButton15m_clicked(bool checked);
    void on_pushButton10m_clicked(bool checked);
    void on_pushButtonNone_clicked();
    void on_pushButtonClearErrors_clicked();
    void closeEvent(QCloseEvent *event);
    void quitApplication();
};

#endif // MAINWINDOW_H
