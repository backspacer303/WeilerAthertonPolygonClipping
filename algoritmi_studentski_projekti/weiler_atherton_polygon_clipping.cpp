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

    std::cout << "-----------------------------------------------------------" << std::endl;

    /*
    for(QPointF p : _preseci){

        bool pripada = false;

        for(Field* polje : _poligon2.fields()){


            HalfEdge* pocetnaIvica = polje->outerComponent();

            if(pocetnaIvica == nullptr)
                continue;

            pripada = tackaPripadaPravoj(p, pocetnaIvica->origin()->coordinates(),
                                                 pocetnaIvica->next()->origin()->coordinates());
            if(pripada){

                std::cout << "Presecna tacka p(" << p.x() << ", " << p.y()
                          << ") pripada ivici pocetak("
                          << pocetnaIvica->origin()->coordinates().x() << ", " << pocetnaIvica->origin()->coordinates().y()
                          << ")   kraj("
                          << pocetnaIvica->next()->origin()->coordinates().x() << ", "
                          << pocetnaIvica->next()->origin()->coordinates().y()
                          << ")" << std::endl;

                break;
            }


            HalfEdge* trenutnaIvica = pocetnaIvica->next();

            while(trenutnaIvica != pocetnaIvica){

                pripada = tackaPripadaPravoj(p, trenutnaIvica->origin()->coordinates(),
                                                trenutnaIvica->next()->origin()->coordinates());

                if(pripada){

                    std::cout << "Presecna tacka p(" << p.x() << ", " << p.y()
                              << ") pripada ivici pocetak("
                              << trenutnaIvica->origin()->coordinates().x() << ", " << trenutnaIvica->origin()->coordinates().y()
                              << ")   kraj("
                              << trenutnaIvica->next()->origin()->coordinates().x() << ", "
                              << trenutnaIvica->next()->origin()->coordinates().y()
                              << ")" << std::endl;
                    break;
                }

                trenutnaIvica = trenutnaIvica->next();
            }

        }

        if(!pripada)
            std::cout << "Tacka p(" << p.x() << ", " << p.y() << ") ne pripada ni jednoj ivici" << std::endl;
    }
    */



    for(QPointF p : _preseci){

        for(int i=0; i<_poligon2.fields().size(); i++){

            HalfEdge* pocetnaIvica = _poligon2.field(i)->outerComponent();

            if(pocetnaIvica == nullptr)
                continue;



            //da li je presecna tacka bas na prvoj ivici?
            Vertex* pocetniOrigin = pocetnaIvica->origin();
            Vertex* zavrsetakPocetneIvice = pocetnaIvica->next()->origin();


            if(tackaPripadaPravoj(p, pocetniOrigin->coordinates(), zavrsetakPocetneIvice->coordinates())){
                //nasli smo ivicu kojoj pripada presek za tekuce lice

                //cela logika sa papira

                Vertex* v = new Vertex(p.x(), p.y());
                _poligon2.ubaciTeme(v);

                //1)
                HalfEdge* sledecaStareIvice = pocetnaIvica->next();
                pocetnaIvica->twin()->setOrigin(v);

                //2)
                HalfEdge* novaIvica = new HalfEdge(v);
                HalfEdge* novaIvicaTwin = new HalfEdge(sledecaStareIvice->origin());
                _poligon2.insertEdge(novaIvica);
                _poligon2.insertEdge(novaIvicaTwin);
                novaIvica->setIncidentFace(_poligon2.field(i));
                novaIvica->setTwin(novaIvicaTwin);
                novaIvicaTwin->setTwin(novaIvica);

                //3)
                pocetnaIvica->setNext(novaIvica);
                pocetnaIvica->twin()->setPrev(novaIvicaTwin);

                //4)
                novaIvica->setNext(sledecaStareIvice);
                novaIvica->setPrev(pocetnaIvica);
                novaIvicaTwin->setNext(pocetnaIvica->twin());
                novaIvicaTwin->setPrev(sledecaStareIvice);

                //5)
                sledecaStareIvice->setPrev(novaIvica);
                sledecaStareIvice->twin()->setNext(novaIvicaTwin);

                continue;
            }



            HalfEdge* trenutnaIvica = pocetnaIvica->next();
            Vertex* trenutniOrigin = trenutnaIvica->origin();

            while(  trenutniOrigin->x() != pocetniOrigin->x() ||
                   (trenutniOrigin->x() == pocetniOrigin->x() && trenutniOrigin->y() != pocetniOrigin->y())
                 )
            {

                //da li je presecna tacka na trenutnoj ivici??

                if(tackaPripadaPravoj(p, trenutniOrigin->coordinates(), trenutnaIvica->next()->origin()->coordinates())){

                    //cela logika sa papira za

                    Vertex* v = new Vertex(p.x(), p.y());
                    _poligon2.ubaciTeme(v);

                    //1)
                    HalfEdge* sledecaStareIvice = trenutnaIvica->next();
                    trenutnaIvica->twin()->setOrigin(v);

                    //2)
                    HalfEdge* novaIvica = new HalfEdge(v);
                    HalfEdge* novaIvicaTwin = new HalfEdge(sledecaStareIvice->origin());
                    _poligon2.insertEdge(novaIvica);
                    _poligon2.insertEdge(novaIvicaTwin);
                    novaIvica->setIncidentFace(_poligon2.field(i));
                    novaIvica->setTwin(novaIvicaTwin);
                    novaIvicaTwin->setTwin(novaIvica);

                    //3)
                    trenutnaIvica->setNext(novaIvica);
                    trenutnaIvica->twin()->setPrev(novaIvicaTwin);

                    //4)
                    novaIvica->setNext(sledecaStareIvice);
                    novaIvica->setPrev(trenutnaIvica);
                    novaIvicaTwin->setNext(trenutnaIvica->twin());
                    novaIvicaTwin->setPrev(sledecaStareIvice);

                    //5)
                    sledecaStareIvice->setPrev(novaIvica);
                    sledecaStareIvice->twin()->setNext(novaIvicaTwin);

                    break;
                }


                //ako nije idemo dalje
                trenutnaIvica = trenutnaIvica->next();
                trenutniOrigin = trenutnaIvica->origin();
            }

        }
    }







    updateCanvasAndBlock();

    for(int i=0; i<_poligon2.fields().size(); i++){

        HalfEdge* pocetnaIvica = _poligon2.field(i)->outerComponent();

        //Spoljasnje neograniceno lice preskacemo
        if(pocetnaIvica == nullptr)
            continue;

        Vertex* pocetniOrigin = pocetnaIvica->origin();

        _redOdsecenihIvica.emplace_back(QLineF(pocetniOrigin->x(), pocetniOrigin->y(),
                                               pocetnaIvica->next()->origin()->x(), pocetnaIvica->next()->origin()->y()
                                              ));
        updateCanvasAndBlock();

        HalfEdge* trenutnaIvica = pocetnaIvica->next();
        Vertex* trenutniOrigin = trenutnaIvica->origin();

        //Obilazimo ivice koje ogranicavaju lice dokle god ponovo ne dostignemo pocetnu
        while(  trenutniOrigin->x() != pocetniOrigin->x() ||
               (trenutniOrigin->x() == pocetniOrigin->x() && trenutniOrigin->y() != pocetniOrigin->y())
             )
        {
            _redOdsecenihIvica.emplace_back(QLineF(trenutniOrigin->x(), trenutniOrigin->y(),
                                                   trenutnaIvica->next()->origin()->x(), trenutnaIvica->next()->origin()->y()
                                                   ));
            updateCanvasAndBlock();
            trenutnaIvica = trenutnaIvica->next();
            trenutniOrigin = trenutnaIvica->origin();
        }
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

bool WeilerAthertonPolygonClipping::tackaPripadaPravoj(const QPointF &tacka, const QPointF &pocetak, const QPointF &kraj)
{

    qreal k = (kraj.x() - pocetak.x());

    QLineF prava(pocetak.x(), pocetak.y(), kraj.x(), kraj.y());

    auto diff = (tacka.y()-pocetak.y()) -
                (kraj.y() - pocetak.y())/(kraj.x() - pocetak.x())*(tacka.x() - pocetak.x())
                 ;
    /*
    std::cout << "Diff: " << diff << "k: "<< k << "  "
              << "-> pocetka(" << pocetak.x() << ", " << pocetak.y() << ") kraj("
              << kraj.x() << ", " << kraj.y() << ")" << std::endl;
    */


    return fabsf(diff) < EPSf;


}





