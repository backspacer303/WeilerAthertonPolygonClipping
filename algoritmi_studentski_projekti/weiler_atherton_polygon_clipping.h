#ifndef WEILERATHERTONPOLYGONCLIPPING_H
#define WEILERATHERTONPOLYGONCLIPPING_H

#include "algoritambaza.h"
#include "../algoritmi_sa_vezbi/ga06_dcel.h"
#include "../algoritmi_sa_vezbi/ga05_preseciduzi.h"

class WeilerAthertonPolygonClipping : public AlgoritamBaza
{
public:
    WeilerAthertonPolygonClipping(QWidget *pCrtanje,
                                  int pauzaKoraka,
                                  const bool &naivni = false,
                                  std::string imeDatoteke = "",
                                  int brojTacaka = BROJ_SLUCAJNIH_OBJEKATA);

    void pokreniAlgoritam() final;
    void crtajAlgoritam(QPainter *painter) const final;
    void pokreniNaivniAlgoritam() final;
    void crtajNaivniAlgoritam(QPainter *painter) const final;

private:
    DCEL poligon1;
    DCEL poligon2;
    DCEL okvir;
};

#endif // WEILERATHERTONPOLYGONCLIPPING_H
