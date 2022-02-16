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
    for(std::pair<float, float> p : preseciSaDuplikatima_parovi){
        _preseci.emplace_back(QPointF(p.first, p.second));
    }

    //izracunate preseke ubacujemo u DCEL strukture poligona
    //i formiramo nove ivice postujuci pravila DCEL strukture
    ubaciPresekeUPoligone();

}

void WeilerAthertonPolygonClipping::pokreniAlgoritam()
{

    std::cout << "-----------------------------------------------------------" << std::endl;    

    //u ovom trenutku imamo ispravno formirane DCEL strukture za svaki od poligona
    //one ce nam pomoci da sortiramo temena u smeru suprotnom kazaljki na satu
    //i to jednostavnim obilaskom ivica

    //formiramo niz temena okvira o koji odsecamo poligon
    std::vector<Vertex*> temenaOkvira;
    int indeks = 0;
    for(int i=0; i<_poligon1.fields().size(); i++){

        HalfEdge* pocetna = _poligon1.field(i)->outerComponent();

        temenaOkvira.emplace_back(pocetna->origin());
        temenaOkvira[indeks]->_indeksUPoligonu = indeks;
        indeks++;

        HalfEdge* trenutna = pocetna;
        while(trenutna->next() != pocetna){
            trenutna = trenutna->next();
            temenaOkvira.emplace_back(trenutna->origin());
            temenaOkvira[indeks]->_indeksUPoligonu = indeks;
            indeks++;
        }
    }

    //ZA SVAKO LICE POLIGONA IZVRSAVAMO ALGORITAM
    std::vector<Vertex*> temenaPoligona;
    for(int i=0; i<_poligon2.fields().size(); i++){


        Field* f = _poligon2.field(i);
        _pocetnaIvica = pronadjiPocetnuIvicu(f->outerComponent());
        temenaPoligona.clear();

        //formiramo niz temena za trenutno lice i pamtimo njihove indekse u nizu
        int indeks = 0;
        temenaPoligona.emplace_back(_pocetnaIvica->origin());
        temenaPoligona[i]->_indeksUPoligonu = indeks;
        indeks++;
        HalfEdge* trenutna = _pocetnaIvica;
        while(trenutna->next() != _pocetnaIvica){
            trenutna = trenutna->next();
            temenaPoligona.emplace_back(trenutna->origin());
            temenaPoligona[i]->_indeksUPoligonu = indeks;
            indeks++;
        }


        //************************************************
        //Weiler-Atherton Algoritam za odsecanje poligona
        //************************************************

        updateCanvasAndBlock();

        //oznaka da li smo usli u okvir
        int unutarOkvira = false;
        //pamticemo tacku ulaska i tacku izlaska iz okvira
        Vertex* temeUlaska;
        Vertex* temeIzlaska;

        //prolazimo kroz temena poligona
        for(int i =0; i<temenaPoligona.size(); i++){

            //naisli smo na teme koje je presek
            if(temenaPoligona[i]->getTemeJePresecno()){

                //ako prethodno nismo bili u okviru znaci da je presek na koji smo naisli ulazno teme
                //pamtimo ga i postavljamo zastavicu da smo usli u okvir
                if(unutarOkvira == false){
                    unutarOkvira = true;
                    temeUlaska = temenaPoligona[i];
                    _temenaOdsecenihDelova.emplace_back(temenaPoligona[i]);
                    updateCanvasAndBlock();
                }

                //ako smo prethodno bili u okviru onda smo naisli na presek koji je izlazno teme
                else if(unutarOkvira == true){
                    unutarOkvira = false;
                    temeIzlaska = temenaPoligona[i];

                    _temenaOdsecenihDelova.emplace_back(temenaPoligona[i]);
                    updateCanvasAndBlock();

                    //obilazimo okvir koriscenjem veze koju smo zapamtili kako bismo
                    //duz okvira zatvorili deo koji odsecamo
                    for(int j = temeIzlaska->getVezaZaDrugiPoligon()->_indeksUPoligonu+1;
                        j<temenaOkvira.size(); j++)
                    {
                            //zatvaramo ivicama duz okvira sve dok ne dostignemo tacku ulaska
                            _temenaOdsecenihDelova.emplace_back(temenaOkvira[j]);
                            updateCanvasAndBlock();
                            if(temenaOkvira[j]->getVezaZaDrugiPoligon() == temeUlaska)
                                break;
                    }
                }

            }else{
                //naisli smo na teme koje nije presek i ako smo prethodno usli u poligon
                //dodajemo ga u niz za odsecanje
                if(unutarOkvira == true){
                    _temenaOdsecenihDelova.emplace_back(temenaPoligona[i]);
                    updateCanvasAndBlock();
                }
            }
        }
    }

    updateCanvasAndBlock();

    emit animacijaZavrsila();
}

void WeilerAthertonPolygonClipping::crtajAlgoritam(QPainter *painter) const
{
    if (!painter) return;

    QPen pen = painter->pen();

    //Crtanje poligona s kojim se odseca
    for(auto i=0ul; i<_poligon1.getStraniceBezBlizanaca().size(); i++)
    {
        pen.setColor(Qt::red);
        painter->setPen(pen);
        painter->drawLine(_poligon1.getStranica(i)->origin()->coordinates(),
                          _poligon1.getStranica(i)->next()->origin()->coordinates());
    }

    //Crtanje temena poligona sa kojim se odseca
    int curr_num = 0;
    painter->setBrush(Qt::red);
    painter->setPen(Qt::white);
    for(Vertex* v: _poligon1.vertices())
    {
        painter->drawEllipse(v->coordinates(), 10, 10);
        painter->save();
        painter->scale(1, -1);
        painter->translate(0, -2*v->y());

        painter->drawText(QPointF(v->x() - 4, v->y() + 4),
                         QString::number(curr_num));
        curr_num++;
        painter->restore();
    }

    //Crtanje poligona koji se odseca
    for(auto i=0ul; i<_poligon2.getStraniceBezBlizanaca().size(); i++)
    {
        pen.setColor(Qt::yellow);
        painter->setPen(pen);
        painter->drawLine(_poligon2.getStranica(i)->origin()->coordinates(),
                         _poligon2.getStranica(i)->next()->origin()->coordinates());
    }


    //Crtanje temena poligona koji se odseca
    curr_num = 0;
    painter->setBrush(Qt::red);
    painter->setPen(Qt::white);
    for(Vertex* v: _poligon2.vertices())
    {
        painter->drawEllipse(v->coordinates(), 10, 10);
        painter->save();
        painter->scale(1, -1);
        painter->translate(0, -2*v->y());
        painter->drawText(QPointF(v->x() - 4, v->y() + 4),
                         QString::number(curr_num));
        curr_num++;
        painter->restore();
    }

    //Crtanje presecnih tacaka
    curr_num = 0;
    painter->setBrush(Qt::blue);
    painter->setPen(Qt::white);
    for(auto tacka : _preseci)
    {
        painter->drawEllipse(tacka, 10, 10);
        painter->save();
        painter->scale(1, -1);
        painter->translate(0, -2*tacka.y());
        painter->drawText(QPointF(tacka.x() - 4, tacka.y() + 4),
                         QString::number(curr_num));
        curr_num++;
        painter->restore();
    }


    //GLAVNO ISCRTAVANJE REZULTATA ALGORITMA
    //Cratenje do sada odsecenih ivica poligona
    Vertex* pocetakLanca;
    bool preskociLiniju = false;

    if(_temenaOdsecenihDelova.size() > 0)
        pocetakLanca = _temenaOdsecenihDelova[0];

    for(int i=0; i<_temenaOdsecenihDelova.size()-1; i++){

        if(_temenaOdsecenihDelova.size() < 2)
            break;

        Vertex* v1 = _temenaOdsecenihDelova[i];
        Vertex* v2 = _temenaOdsecenihDelova[i+1];

        if(preskociLiniju){
            preskociLiniju = false;
            pocetakLanca = v2;
            continue;
        }

        if(v2->getVezaZaDrugiPoligon() == pocetakLanca){
            preskociLiniju = true;
        }

        QLineF l(_temenaOdsecenihDelova[i]->x(), _temenaOdsecenihDelova[i]->y(),
                 _temenaOdsecenihDelova[i+1]->x(), _temenaOdsecenihDelova[i+1]->y());

        pen.setColor(Qt::green);
        painter->setPen(pen);
        painter->drawLine(l);
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

void WeilerAthertonPolygonClipping::ubaciPresekeUPoligone()
{
    for(int i=0; i<_preseci.size(); i++){

        QPointF p = _preseci[i];

        Vertex* v2 = new Vertex(p.x(), p.y());
        Vertex* v1 = new Vertex(p.x(), p.y());
        //postavljamo medjusobne veze izmedju presecnih tacaka u poligonima
        //kao i oznake da se radi o presecnom temenu
        //ove onformacije su nam potrebne u realizaciji samog algoritma
        v2->setTemeJePresecno(true);
        v2->setVezaZaDrugiPoligon(v1);
        v1->setTemeJePresecno(true);
        v1->setVezaZaDrugiPoligon(v2);

        for(int j=0; j<_poligon2.fields().size(); j++){

            HalfEdge* pocetnaIvica = _poligon2.field(j)->outerComponent();

            if(pocetnaIvica == nullptr)
               continue;


            Vertex* pocetniOrigin = pocetnaIvica->origin();
            Vertex* zavrsetakPocetneIvice = pocetnaIvica->next()->origin();

            //da li je presecna tacka bas na prvoj ivici?
            if(tackaPripadaDuzi(p, pocetniOrigin->coordinates(), zavrsetakPocetneIvice->coordinates())){

                //nasli smo ivicu kojoj pripada presek za tekuce lice pa je potrebno uvesti novu ivicu
                //i povezati je sa vec postojecim ivicama na ispravan nacin

                _poligon2.ubaciTeme(v2);

                //1) pamtimo sarog suseda
                HalfEdge* sledecaStareIvice = pocetnaIvica->next();

                //2) formiramo novu ivicu
                HalfEdge* novaIvica = new HalfEdge(v2);
                v2->setIncidentEdge(novaIvica);
                _poligon2.ubaciIvicu(novaIvica);
                novaIvica->setIncidentFace(_poligon2.field(j));

                //3) staroj ivici podesavamo novog suseda
                pocetnaIvica->setNext(novaIvica);

                //4) novoj ivici podesavamo susede
                novaIvica->setNext(sledecaStareIvice);
                novaIvica->setPrev(pocetnaIvica);

                //5) starom susedu podesavamo novog suseda
                sledecaStareIvice->setPrev(novaIvica);

                continue;
            }

            //ako presecna tacka nije na pocetnoj ivici proveravamo sve ostales
            HalfEdge* trenutnaIvica = pocetnaIvica->next();
            Vertex* trenutniOrigin = trenutnaIvica->origin();

            while(trenutnaIvica != pocetnaIvica)
            {
                if(tackaPripadaDuzi(p, trenutniOrigin->coordinates(), trenutnaIvica->next()->origin()->coordinates())){

                    //presecna tacka pripada ivici koju trenutno obilazimo

                    _poligon2.ubaciTeme(v2);

                    //1) pamtimo sarog suseda
                    HalfEdge* sledecaStareIvice = trenutnaIvica->next();

                    //2) formiramo novu ivicu
                    HalfEdge* novaIvica = new HalfEdge(v2);
                    v2->setIncidentEdge(novaIvica);
                    _poligon2.ubaciIvicu(novaIvica);
                    novaIvica->setIncidentFace(_poligon2.field(j));

                    //3) staroj ivici podesavamo novog suseda
                    trenutnaIvica->setNext(novaIvica);

                    //4) novoj ivici podesavamo susede
                    novaIvica->setNext(sledecaStareIvice);
                    novaIvica->setPrev(trenutnaIvica);

                    //5)starom susedu podesavamo novog suseda
                    sledecaStareIvice->setPrev(novaIvica);

                    break;
                }

                //ako ne pripada takucoj idemo dalje
                trenutnaIvica = trenutnaIvica->next();
                trenutniOrigin = trenutnaIvica->origin();
            }
        }

        //------------ISPOD SLEDI POTPUNO ISTA LOGIKA ZA UMETANJE PRESEKA I U DRUGI POLIGON-----
        for(int j=0; j<_poligon1.fields().size(); j++){

            HalfEdge* pocetnaIvica = _poligon1.field(j)->outerComponent();

            if(pocetnaIvica == nullptr)
               continue;

            Vertex* pocetniOrigin = pocetnaIvica->origin();
            Vertex* zavrsetakPocetneIvice = pocetnaIvica->next()->origin();


            if(tackaPripadaDuzi(p, pocetniOrigin->coordinates(), zavrsetakPocetneIvice->coordinates())){

                _poligon1.ubaciTeme(v1);

                HalfEdge* sledecaStareIvice = pocetnaIvica->next();

                HalfEdge* novaIvica = new HalfEdge(v1);
                v1->setIncidentEdge(novaIvica);
                _poligon1.ubaciIvicu(novaIvica);
                novaIvica->setIncidentFace(_poligon1.field(j));

                pocetnaIvica->setNext(novaIvica);

                novaIvica->setNext(sledecaStareIvice);
                novaIvica->setPrev(pocetnaIvica);

                sledecaStareIvice->setPrev(novaIvica);

                continue;
            }

            HalfEdge* trenutnaIvica = pocetnaIvica->next();
            Vertex* trenutniOrigin = trenutnaIvica->origin();

            while(trenutnaIvica != pocetnaIvica)
            {
                if(tackaPripadaDuzi(p, trenutniOrigin->coordinates(), trenutnaIvica->next()->origin()->coordinates())){

                    _poligon1.ubaciTeme(v1);

                    HalfEdge* sledecaStareIvice = trenutnaIvica->next();

                    HalfEdge* novaIvica = new HalfEdge(v1);
                    v1->setIncidentEdge(novaIvica);
                    _poligon1.ubaciIvicu(novaIvica);
                    novaIvica->setIncidentFace(_poligon1.field(j));

                    trenutnaIvica->setNext(novaIvica);

                    novaIvica->setNext(sledecaStareIvice);
                    novaIvica->setPrev(trenutnaIvica);

                    sledecaStareIvice->setPrev(novaIvica);

                    break;
                }
                trenutnaIvica = trenutnaIvica->next();
                trenutniOrigin = trenutnaIvica->origin();
            }
        }
    }
}

bool WeilerAthertonPolygonClipping::tackaPripadaDuzi(const QPointF &tacka, const QPointF &pocetak, const QPointF &kraj)
{
    qreal crossProduct = (tacka.y() - pocetak.y()) * (kraj.x() - pocetak.x()) - (tacka.x() - pocetak.x()) * (kraj.y() - pocetak.y());

    if(fabsf(crossProduct) > EPSf)
        return false;

    qreal dotProduct = (tacka.x() - pocetak.x()) * (kraj.x() - pocetak.x()) + (tacka.y() - pocetak.y())*(kraj.y() - pocetak.y());

    if(dotProduct < 0)
        return false;

    qreal squaredLength = (kraj.x() - pocetak.x())*(kraj.x() - pocetak.x()) + (kraj.y() - pocetak.y())*(kraj.y() - pocetak.y());

    if(dotProduct > squaredLength)
        return false;

    return true;
}

HalfEdge *WeilerAthertonPolygonClipping::pronadjiPocetnuIvicu(HalfEdge* prvaIvicaLica)
{
    if(!prvaIvicaLica->origin()->getTemeJePresecno()){

        QPointF teme1(prvaIvicaLica->origin()->x(), prvaIvicaLica->origin()->y());
        bool pripadaOkviru1 = proveriPripadnostOkviru(teme1);
        if(!pripadaOkviru1){
            return prvaIvicaLica;
        }
    }

    HalfEdge* trenutnaIvica = prvaIvicaLica->next();
    while(trenutnaIvica != prvaIvicaLica){

        if(!trenutnaIvica->origin()->getTemeJePresecno()){
            QPointF teme(trenutnaIvica->origin()->x(), trenutnaIvica->origin()->y());
            bool pripadaOkviru = proveriPripadnostOkviru(teme);
            if(!pripadaOkviru){
                return trenutnaIvica;
            }
        }
        trenutnaIvica = trenutnaIvica->next();
    }
}

bool WeilerAthertonPolygonClipping::proveriPripadnostOkviru(QPointF teme)
{
    QLineF linijaNaDesno(teme.x(), teme.y(), 1565, teme.y());

    int brojPresekaLinije = 0;

    for(HalfEdge* e : _poligon1.edges()){

        QLineF ivica(e->origin()->x(), e->origin()->y(), e->next()->origin()->x(), e->next()->origin()->y());
        QPointF* presek = new QPointF();
        bool rezultat = pomocneFunkcije::presekDuzi(linijaNaDesno, ivica, *presek);

        if(rezultat)
            brojPresekaLinije++;
    }

    std::cout << "Broj preseka: " << brojPresekaLinije << std::endl;

    if(brojPresekaLinije % 2 == 0)
        return false;
    else
        return true;
}









