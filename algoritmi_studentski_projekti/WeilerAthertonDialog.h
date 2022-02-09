#ifndef WEILERATHERTONDIALOG_H
#define WEILERATHERTONDIALOG_H

#include <QDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QString>
#include "mainwindow.h"

namespace Ui {
class WeilerAthertonDialog;
}

class WeilerAthertonDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WeilerAthertonDialog(QWidget *parent = nullptr);
    ~WeilerAthertonDialog();

private slots:
    void on_dugme_odabir_poligona_1_clicked();
    void on_dugme_odabir_poligona_2_clicked();
    void on_dugme_potvrdi_clicked();

private:
    Ui::WeilerAthertonDialog *ui;
    std::string _daoteka1;
    std::string _daoteka2;
};

#endif // WEILERATHERTONDIALOG_H
