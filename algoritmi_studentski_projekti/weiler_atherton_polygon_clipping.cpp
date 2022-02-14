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
    //Pravi se zbirni skup duzi
    //TODO mozda ovi QLineF-ovi treba da se brisu jer su dinamicki alocirani sa "new"?
    for(auto i=0ul; i<_poligon1.getStraniceBezBlizanaca().size(); i++){
        QLineF* l = new QLineF(_poligon1.getStranica(i)->origin()->coordinates(),
                               _poligon1.getStranica(i)->next()->origin()->coordinates());
        _zbirniSkupDuzi.emplace_back(*l);
    }
    for(auto i=0ul; i<_poligon2.getStraniceBezBlizanaca().size(); i++){
        QLineF* l = new QLineF(_poligon2.getStranica(i)->origin()->coordinates(),
                              _poligon2.getStranica(i)->next()->origin()->coordinates());
        _zbirniSkupDuzi.emplace_back(*l);
    }


    std::cout << "WA, broj stranica u _zbirniSkupDuzi: " << _zbirniSkupDuzi.size() << std::endl;
    for(QLineF l : _zbirniSkupDuzi)
        std::cout << "p1: "<< l.p1().x() << ", " << l.p1().y() << "     p2: " << l.p2().x() << ", " << l.p2().y() << std::endl;


    //Pozivanje algoritma za preseke duzi
    std::vector<QPointF> preseciSaTemenimaIDuplikatima;
    _algoritamPreseci.SetSkupDuzi(_zbirniSkupDuzi);
    _algoritamPreseci.pokreniNaivniAlgoritam();
    preseciSaTemenimaIDuplikatima = _algoritamPreseci.GetVektorPreseka();


    std::cout << "Broj preseciSaTemenimaIDuplikatima nakon algoritma za preseke: " << preseciSaTemenimaIDuplikatima.size() << std::endl;

    //Izbacivanje svih preseka koji su temena
    std::vector<QPointF> temena1;
    std::vector<QPointF> temena2;
    for(Vertex* v : _poligon1.vertices()){
        temena1.emplace_back(v->coordinates());
    }
    for(Vertex* v : _poligon2.vertices()){
        temena2.emplace_back(v->coordinates());
    }

    std::vector<QPointF> preseciSaDuplikatima;

    std::copy_if(preseciSaTemenimaIDuplikatima.begin(), preseciSaTemenimaIDuplikatima.end(), std::back_inserter(preseciSaDuplikatima),
    [temena1, temena2](QPointF t){

        if ( (std::find(temena1.begin(), temena1.end(), t) == temena1.end()) &&
             (std::find(temena2.begin(), temena2.end(), t) == temena2.end()) )
        {
            return true;
        }else
            return false;
    });


    std::cout << "Broj preseciSaDuplikatima nakon izbacivanja temena: " << preseciSaDuplikatima.size() << std::endl;


    //Izbacivanje svih duplikata preseka (oni koji su prijavljeni vise puta)
    //(npr. preseci koji se nalaze na susednim ivicama dva lica ce biti prijavljeni dva puta)
    std::vector<std::pair<float, float>> preseciSaDuplikatima_parovi;
    for(QPointF p : preseciSaDuplikatima){
        preseciSaDuplikatima_parovi.emplace_back(std::pair<float, float>(p.x(), p.y()));
    }
    std::sort(preseciSaDuplikatima_parovi.begin(), preseciSaDuplikatima_parovi.end());
    preseciSaDuplikatima_parovi.erase( unique( preseciSaDuplikatima_parovi.begin(), preseciSaDuplikatima_parovi.end() ),
                                       preseciSaDuplikatima_parovi.end() );

    std::cout << "Broj preseka nakon izbacivanja temeta i duplikata: " << preseciSaDuplikatima_parovi.size() << std::endl;


    //Konvertovanje parova koordinata nazad u QPointF i upisivanje u lokalni vektor preseka
    for(std::pair<float, float> p : preseciSaDuplikatima_parovi)
        _preseci.emplace_back(QPointF(p.first, p.second));

}

void WeilerAthertonPolygonClipping::pokreniAlgoritam()
{

    std::vector<Vertex*> lista1;
    std::vector<Vertex*> lista2;

    for(Vertex* v : _poligon1.vertices())
        lista1.emplace_back(v);
    for(Vertex* v : _poligon2.vertices())
        lista2.emplace_back(v);

    for(QPointF p : _preseci){
        Vertex* v = new Vertex(p);
        lista1.emplace_back(v);
        lista2.emplace_back(v);
    }

    //Pronalazenje tacke sa najvecom x-koordinatom ili, ako je vise takvih, sa najmanjom y-koodrinatom
    Vertex* maxTackaPoli1 = lista1[0];
    Vertex* maxTackaPoli2 = lista2[0];

    for(int i = 1; i<lista1.size(); i++){
        if(lista1[i]->x() > maxTackaPoli1->x() ||
          (lista1[i]->x() == maxTackaPoli1->x() && lista1[i]->y() < maxTackaPoli1->y()) )
            maxTackaPoli1 = lista1[i];
    }

    for(int i = 1; i<lista2.size(); i++){
        if(lista2[i]->x() > maxTackaPoli2->x() ||
          (lista2[i]->x() == maxTackaPoli2->x() && lista2[i]->y() < maxTackaPoli2->y()) )
            maxTackaPoli2 = lista2[i];
    }


    //provera nadjenih max tacaka
    std::cout << "poligon 1 - max tacka: " << maxTackaPoli1->x() << " " << maxTackaPoli1->y() << std::endl;
    std::cout << "poligon 2 - max tacka: " << maxTackaPoli2->x() << " " << maxTackaPoli2->y() << std::endl;


    /*
    //testiranje fje povrsinaTrougla
    Vertex v1(100, 10);
    Vertex v2(100,60);
    Vertex v3(150,34);
    auto rez = povrsinaTrougla(&v1,&v2,&v3);
    std::cout << "Rezultat povrsine trougla: " << rez << std::endl;
    */


    //Sortiranje prve liste temena prema uglu koji zaklapaju sa x-osom

    std::sort(lista1.begin(), lista1.end(),
    [&](Vertex* lhs, Vertex* rhs){

        if(lhs->x() == maxTackaPoli1->x() && lhs->y() == maxTackaPoli1->y())
            return true;
        if(rhs->x() == maxTackaPoli1->x() && rhs->y() == maxTackaPoli1->y())
            return false;

        qreal P = povrsinaTrougla(maxTackaPoli1, lhs, rhs);

        if(P>0)
            return true;
        else if(P<0)
            return false;
        else if(P==0){
            if(lhs->x() == maxTackaPoli1->x() && rhs->x() == maxTackaPoli1->x()){
                return distanceKvadratF(maxTackaPoli1, lhs) < distanceKvadratF(maxTackaPoli1, rhs);
            }else
                return distanceKvadratF(maxTackaPoli1, lhs) > distanceKvadratF(maxTackaPoli1, rhs);
        }
    });


    std::cout << "lista 1:" << std::endl;
    for(auto v : lista1)
        std::cout << v->x() << "  " << v->y() << std::endl;


    //Sortiranje druge liste temena prema uglu koji zaklapaju sa x-osom

    std::sort(lista2.begin(), lista2.end(),
    [&](Vertex* lhs, Vertex* rhs){

        if(lhs->x() == maxTackaPoli2->x() && lhs->y() == maxTackaPoli2->y())
            return true;
        if(rhs->x() == maxTackaPoli2->x() && rhs->y() == maxTackaPoli2->y())
            return false;

        qreal P = povrsinaTrougla(maxTackaPoli2, lhs, rhs);

        if(P>0)
            return true;
        else if(P<0)
            return false;
        else if(P==0){
            if(lhs->x() == maxTackaPoli2->x() && rhs->x() == maxTackaPoli2->x()){
                return distanceKvadratF(maxTackaPoli2, lhs) < distanceKvadratF(maxTackaPoli2, rhs);
            }else
                return distanceKvadratF(maxTackaPoli2, lhs) > distanceKvadratF(maxTackaPoli2, rhs);
        }
    });


    std::cout << "lista 2:" << std::endl;
    for(auto v : lista2)
        std::cout << v->x() << "  " << v->y() << std::endl;



    for(int i =0; i<lista2.size()-1; i++){
        _redOdsecenihIvica.emplace_back(QLineF(lista2[i]->x(), lista2[i]->y(),
                                               lista2[i+1]->x(), lista2[i+1]->y()));
    }



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


    for(QLineF l : _redOdsecenihIvica)
    {
        /* Crta se poligon */
        pen.setColor(Qt::green);
        painter->setPen(pen);
        painter->drawLine(l);
    }




    curr_num = 0;
    painter->setBrush(Qt::blue);
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

qreal WeilerAthertonPolygonClipping::povrsinaTrougla(Vertex* A, Vertex* B, Vertex* C)
{
    return (B->x() - A->x())*(C->y() - A->y()) - (C->x() - A->x())*(B->y() - A->y());
}

qreal WeilerAthertonPolygonClipping::distanceKvadratF(Vertex* A, Vertex* B)
{
    return (A->x() - B->x())*(A->x() - B->x()) + (A->y() - B->y())*(A->y() - B->y());
}



