#include "WeilerAthertonDialog.h"
#include "ui_WeilerAthertonDialog.h"

WeilerAthertonDialog::WeilerAthertonDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::WeilerAthertonDialog)
{
    ui->setupUi(this);
}

WeilerAthertonDialog::~WeilerAthertonDialog()
{
    delete ui;
}

void WeilerAthertonDialog::on_dugme_odabir_poligona_1_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(
          this, tr("Odaberite CSV Fajl"), "/home", "OFF Files (*.off)");

    ui->prikaz_imena_datoteke_1->setText(fileName);
}

void WeilerAthertonDialog::on_dugme_odabir_poligona_2_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(
          this, tr("Odaberite CSV Fajl"), "/home", "OFF Files (*.off)");

    ui->prikaz_imena_datoteke_2->setText(fileName);
}

void WeilerAthertonDialog::on_dugme_potvrdi_clicked()
{
    QString nazivFajla1 = ui->prikaz_imena_datoteke_1->toPlainText();
    QString nazivFajla2 = ui->prikaz_imena_datoteke_2->toPlainText();

    if(nazivFajla1.isEmpty() || nazivFajla2.isEmpty() || nazivFajla1.trimmed() == "" || nazivFajla2.trimmed() == ""){
        QMessageBox::information(this, "Greska!", "Odabrati obe datoteke!");
        return;
    }

    dynamic_cast<MainWindow*>(this->parent())->setImenaDatotekaPoligona(nazivFajla1.toStdString(), nazivFajla2.toStdString());
    this->close();
}


