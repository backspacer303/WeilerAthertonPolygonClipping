#include "weiler_atherton_polygon_clipping.h"

WeilerAthertonPolygonClipping::WeilerAthertonPolygonClipping(QWidget *pCrtanje,
                                                             int pauzaKoraka,
                                                             const bool &naivni,
                                                             std::string imeDatoteke,
                                                             int brojTacaka,
                                                             std::string imeDatotekePoligon_1,
                                                             std::string imeDatotekePoligon_2)
    : AlgoritamBaza(pCrtanje, pauzaKoraka, naivni),
      _poligon1(imeDatotekePoligon_1, pCrtanje->height(), pCrtanje->width()),
      _poligon2(imeDatotekePoligon_2, pCrtanje->height(), pCrtanje->width()),
      _algoritamPreseci(pCrtanje, pauzaKoraka, naivni, imeDatoteke, brojTacaka)
{

    //TODO mozda ovi QLineF-ovi treba da se brisu jer su dinamicki alocirani sa "new"?
    for(auto i=0ul; i<_poligon1.edges().size(); i++){
        QLineF* l = new QLineF(_poligon1.edge(i)->origin()->coordinates(),
                               _poligon1.edge(i)->next()->origin()->coordinates());
        _zbirniSkupDuzi.emplace_back(*l);
    }
    for(auto i=0ul; i<_poligon2.edges().size(); i++){
        QLineF* l = new QLineF(_poligon2.edge(i)->origin()->coordinates(),
                              _poligon2.edge(i)->next()->origin()->coordinates());
        _zbirniSkupDuzi.emplace_back(*l);
    }

    //TODO
    //Potencijano resenje: proci kroz sva temena i pokupi ivice koej pocinju u svakom od njih
    //ideja: svaka od ivica mora pocetei u tacno jednom od temena, pa cemo ih tako pokupiti sve



    for(QLineF l : _zbirniSkupDuzi)
        std::cout << "p1: "<< l.p1().x() << ", " << l.p1().y() << "     p2: " << l.p2().x() << ", " << l.p2().y() << std::endl;

    //QLineF l1(QPointF(699, 419), QPointF(699, 139));
    //QLineF l2(QPointF(885, 349), QPointF(582, 349));
    //_zbirniSkupDuzi.clear();
    //_zbirniSkupDuzi.emplace_back(l1);
    //_zbirniSkupDuzi.emplace_back(l2);

    //std::cout << "Broj temena 1. poligona: " << _poligon1.vertices().size() << std::endl;
    //std::cout << "Broj temena 2. poligona: " << _poligon2.vertices().size() << std::endl;

    _algoritamPreseci.SetSkupDuzi(_zbirniSkupDuzi);
    _algoritamPreseci.pokreniNaivniAlgoritam();
    _preseci = _algoritamPreseci.GetVektorPreseka();


    /*
    //Ako hocemo stvaran broj preseka obrisemo presecne tacke koje se pojavljuju vise puta
    //svaka presecna tacka se pojavljuje tacno 4 puta, zbog twin ivica
    //za dva osnovna ucitana poligoan (okvir, poligon1) naivni algoritam za nalazenje preseka vraca 52 presecne tacke
    //kada eliminisemo duplikate ostaje 13 razlicitih presecnih tacaka: to su sva temana jednog i drugog poligona [4+5]
    // (jer su ona presecne tacke za duzi stanice poligona koje se u njima spajaju) plus [4] stvarna preseka koja postoje
    std::vector<std::pair<float, float>> tackeKaoParovi;
    for(QPointF t : _preseci){
        tackeKaoParovi.emplace_back(std::pair<float, float>(t.x(), t.y()));
    }
    sort( tackeKaoParovi.begin(), tackeKaoParovi.end() );
    tackeKaoParovi.erase( unique( tackeKaoParovi.begin(), tackeKaoParovi.end() ), tackeKaoParovi.end() );
    std::cout << "WA, _presicei, obrisani duplikati, broj elemenata: " << tackeKaoParovi.size() << std::endl;
    */
}

void WeilerAthertonPolygonClipping::pokreniAlgoritam()
{
    emit animacijaZavrsila();
}

void WeilerAthertonPolygonClipping::crtajAlgoritam(QPainter *painter) const
{
    if (!painter) return;

    QPen pen = painter->pen();
    for(auto i=0ul; i<_poligon1.edges().size(); i++)
    {
        /* Crta se poligon */
        pen.setColor(Qt::red);
        painter->setPen(pen);
        painter->drawLine(_poligon1.edge(i)->origin()->coordinates(),
                          _poligon1.edge(i)->next()->origin()->coordinates());
    }

    int curr_num = 0;
    painter->setBrush(Qt::red);
    painter->setPen(Qt::white);
    /* Crtaju se temena, ali kao elipsa, radi lepote. */
    for(Vertex* v: _poligon1.vertices())
    {
        painter->drawEllipse(v->coordinates(), 10, 10);

        /* Okretanje cetkice kako brojevi ne bi bili obrnuti */
        painter->save();
        painter->scale(1, -1);
        painter->translate(0, -2*v->y());

        painter->drawText(QPointF(v->x() - 4, v->y() + 4),
                         QString::number(curr_num));
        curr_num++;

        /* Ponistavanje transformacija */
        painter->restore();
    }


    for(auto i=0ul; i<_poligon2.edges().size(); i++)
    {
        /* Crta se poligon */
        pen.setColor(Qt::yellow);
        painter->setPen(pen);
        painter->drawLine(_poligon2.edge(i)->origin()->coordinates(),
                         _poligon2.edge(i)->next()->origin()->coordinates());
    }

    curr_num = 0;
    painter->setBrush(Qt::red);
    painter->setPen(Qt::white);
    /* Crtaju se temena, ali kao elipsa, radi lepote. */
    for(Vertex* v: _poligon2.vertices())
    {
        painter->drawEllipse(v->coordinates(), 10, 10);

        /* Okretanje cetkice kako brojevi ne bi bili obrnuti */
        painter->save();
        painter->scale(1, -1);
        painter->translate(0, -2*v->y());

        painter->drawText(QPointF(v->x() - 4, v->y() + 4),
                         QString::number(curr_num));
        curr_num++;

        /* Ponistavanje transformacija */
        painter->restore();
    }


    curr_num = 0;
    painter->setBrush(Qt::red);
    painter->setPen(Qt::white);
    for(auto tacka : _preseci)
    {
        painter->drawEllipse(tacka, 10, 10);

        /* Okretanje cetkice kako brojevi ne bi bili obrnuti */
        painter->save();
        painter->scale(1, -1);
        painter->translate(0, -2*tacka.y());

        painter->drawText(QPointF(tacka.x() - 4, tacka.y() + 4),
                         QString::number(curr_num));
        curr_num++;

        /* Ponistavanje transformacija */
        painter->restore();
    }


}

void WeilerAthertonPolygonClipping::pokreniNaivniAlgoritam()
{
    emit animacijaZavrsila();
}

void WeilerAthertonPolygonClipping::crtajNaivniAlgoritam(QPainter *painter) const
{
    if (!painter) return;
}



