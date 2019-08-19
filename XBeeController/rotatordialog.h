#ifndef ROTATORDIALOG_H
#define ROTATORDIALOG_H
//
#include <QDialog>
#include "ui_rotatorwindow.h"

#include <QGraphicsView>
#include <QHBoxLayout>
#include <QLabel>

#include <QMouseEvent>
#include <QColor>
#include <QBrush>
#include <QSettings>

#define TARGET_DIR_BEAMWIDTH	1
#define TARGET_DIR_BEAMWIDTH_COLOR	black
#define CURRENT_DIR_BEAMWIDTH_COLOR	black

class RotatorDialog : public QDialog, public Ui::Dialog
{
Q_OBJECT
public:
    RotatorDialog( QWidget * parent = 0, Qt::WindowFlags f = 0 );
	QPainter painter;
	void setTargetDir(int antIndex, int targetAngle);
	void setRotatorAngle(int antIndex, unsigned int angle);
	int getTargetDir(unsigned char antIndex);
    void keyPressEvent(QKeyEvent *e);
    void setupLayout();
protected:
	void paintEvent(QPaintEvent *event);
	void mousePressEvent ( QMouseEvent * event );
	QSettings settings;
private:
	void loadMap(QString path);
	QString imagePath;
    int currAzimuthAngle;
    int targetAzimuthAngle;
	int currAntIndex;
	int sizeWidth;
	int sizeHeight;
	QImage image;
	unsigned char rotationEventStatus;
	QString bandName;
	QString antName[4];
	bool antExist[4];
	bool antHasRotor[4];
	bool antBiDirectional[4];
	bool antFixed[4];
	int antBeamWidth[4];
	int antFixedAngle[4];
	bool antVerticalArray[4];
	int presetButtonValue[5];
	char rotatorStatus[4];
	int verticalArrayNrDirs[4];
	int verticalArrayDirAngle[4][4];
	QString verticalArrayDirName[4][4];
	void setStatusPresetButtons();
private slots:

public slots:
	void pushButtonAnt1Clicked();
	void pushButtonAnt2Clicked();
	void pushButtonAnt3Clicked();
	void pushButtonAnt4Clicked();
	void pushButtonPreset1Clicked();
	void pushButtonPreset2Clicked();
	void pushButtonPreset3Clicked();
	void pushButtonPreset4Clicked();
	void pushButtonPreset5Clicked();
	void pushButtonSTOPClicked();
	void pushButtonRotateCWPressed();
	void pushButtonRotateCCWPressed();
	void pushButtonRotateCWReleased();
	void pushButtonRotateCCWReleased();
};
#endif





