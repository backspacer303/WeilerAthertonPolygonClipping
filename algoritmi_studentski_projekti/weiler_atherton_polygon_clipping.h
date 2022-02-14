#ifndef WEILERATHERTONPOLYGONCLIPPING_H
#define WEILERATHERTONPOLYGONCLIPPING_H

#include <iostream>
#include <QPoint>
#include "algoritambaza.h"
#include "../algoritmi_sa_vezbi/ga06_dcel.h"
#include "../algoritmi_sa_vezbi/ga05_preseciduzi.h"
#include "pomocnefunkcije.h"

class WeilerAthertonPolygonClipping : public AlgoritamBaza
{
public:
    WeilerAthertonPolygonClipping(QWidget *pCrtanje,
                                  int pauzaKoraka,
                                  const bool &naivni = false,
                                  std::string imeDatoteke = "",
                                  int brojTacaka = BROJ_SLUCAJNIH_OBJEKATA,
                                  std::string imeDatotekePoligon_1 = "", std::string imeDatotekePoligon_2 = "");

    void pokreniAlgoritam() final;
    void crtajAlgoritam(QPainter *painter) const final;
    void pokreniNaivniAlgoritam() final;
    void crtajNaivniAlgoritam(QPainter *painter) const final;

private:
    DCEL _poligon1;
    DCEL _poligon2;
    PreseciDuzi _algoritamPreseci;
    std::vector<QLineF> _zbirniSkupDuzi;
    std::vector<QPointF> _preseci;
    std::vector<QLineF> _redOdsecenihIvica; //kako se ovaj red menja tako zovemo iscrtavanje

    qreal povrsinaTrougla(Vertex* A, Vertex* B, Vertex* C);
    qreal distanceKvadratF(Vertex* A, Vertex* B);
    bool tackaPripadaPravoj(const QPointF& tacka, const QPointF& pocetak, const QPointF& kraj);
};

#endif // WEILERATHERTONPOLYGONCLIPPING_H
