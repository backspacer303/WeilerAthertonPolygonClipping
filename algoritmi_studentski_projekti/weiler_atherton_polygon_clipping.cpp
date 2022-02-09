#include "weiler_atherton_polygon_clipping.h"

WeilerAthertonPolygonClipping::WeilerAthertonPolygonClipping(QWidget *pCrtanje,
                                                             int pauzaKoraka,
                                                             const bool &naivni,
                                                             std::string imeDatoteke,
                                                             int brojTacaka,
                                                             std::string imeDatotekePoligon_1,
                                                             std::string imeDatotekePoligon_2)
    : AlgoritamBaza(pCrtanje, pauzaKoraka, naivni)
{
    std::cout << imeDatotekePoligon_1 << "*****" <<imeDatotekePoligon_2 << std::endl;
}

void WeilerAthertonPolygonClipping::pokreniAlgoritam()
{
    return;
}

void WeilerAthertonPolygonClipping::crtajAlgoritam(QPainter *painter) const
{
    return;
}

void WeilerAthertonPolygonClipping::pokreniNaivniAlgoritam()
{
    return;
}

void WeilerAthertonPolygonClipping::crtajNaivniAlgoritam(QPainter *painter) const
{
    return;
}

