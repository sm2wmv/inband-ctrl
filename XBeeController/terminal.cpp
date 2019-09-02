#include "terminal.h"
#include "ui_terminal.h"
#include "mainwindow.h"

Terminal::Terminal(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Terminal)
{
    ui->setupUi(this);
}

Terminal::~Terminal()
{
    delete ui;
}

void Terminal::addTerminalText(QString text) {
    if (ui->checkBoxEnableOutput->isChecked())
        ui->textEditOutput->append(text);
}

void Terminal::setXBeeController(QTXB *currxb, QByteArray addressInband) {
    xb = currxb;
    this->addressInband = addressInband;
}

void Terminal::on_lineEditInput_returnPressed() {
    xb->unicast(addressInband, ui->lineEditInput->text());
    ui->lineEditInput->clear();
}

void Terminal::on_checkBoxEnableOutput_stateChanged(int arg1) {

}
