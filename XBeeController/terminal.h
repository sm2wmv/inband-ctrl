#ifndef TERMINAL_H
#define TERMINAL_H

#include "qtxb.h"
#include <QDialog>

namespace Ui {
class Terminal;
}

class Terminal : public QDialog
{
    Q_OBJECT

public:
    explicit Terminal(QWidget *parent = nullptr);
    ~Terminal();
    void addTerminalText(QString text);
    void setXBeeController(QTXB *currxb, QByteArray addressInband);

private slots:
    void on_lineEditInput_returnPressed();

    void on_checkBoxEnableOutput_stateChanged(int arg1);

private:
    Ui::Terminal *ui;
    QTXB *xb;
    QByteArray addressInband;
};

#endif // TERMINAL_H
