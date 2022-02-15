#ifndef WEILERATHERTONPOLYGONCLIPPING_H
#define WEILERATHERTONPOLYGONCLIPPING_H

#include <iostream>
#include <cmath>
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
    HalfEdge* _pocetnaIvica;
    std::vector<QLineF> _redOdsecenihIvica; //kako se ovaj red menja tako zovemo iscrtavanje

    void ubaciPresekeUPoligone();
    void podebljajPoligonPoIvicama(DCEL& poligon);
    bool tackaPripadaDuzi(const QPointF& tacka, const QPointF& pocetak, const QPointF& kraj);
    HalfEdge* pronadjiPocetnuIvicu();
    bool proveriPripadnostOkviru(QPointF teme);

};

#endif // WEILERATHERTONPOLYGONCLIPPING_H
