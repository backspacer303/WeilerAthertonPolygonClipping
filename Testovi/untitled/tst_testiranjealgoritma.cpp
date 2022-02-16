#include <QtTest>
QTEST_MAIN(TestQString)

#include <QtTest/QtTest>
#include <cmath>
#include <QPoint>

// add necessary includes here
#include "./weiler_atherton_polygon_clipping.h"
#include "./algoritambaza.h"
#include "./ga06_dcel.h"
#include "./ga05_preseciduzi.h"
#include "./pomocnefunkcije.h"

class TestiranjeAlgoritma : public QObject
{
    Q_OBJECT

public:
    TestiranjeAlgoritma();
    ~TestiranjeAlgoritma();

private slots:
    void test_case1();
    void test_case2();

    WeilerAthertonPolygonClipping* algoritam;
    DCEL poligon1;
    DCEL poligon2;
    std::vector<Vertex*> odsecenaTemena;


private:
    bool proveriPripadnostOkviru(QPointF teme, DCEL& poligon1);

};


TestiranjeAlgoritma::TestiranjeAlgoritma(){

    algoritam = new WeilerAthertonPolygonClipping(nullptr, 0, false, "", 0, "", "");
    algoritam->pokreniAlgoritam();
    poligon1 = algoritam->get_poligon1();
    poligon2 = algoritam->get_poligon2();
    odsecenaTemena = algoritam->get_temenaOdsecenihDelova();

}

TestiranjeAlgoritma::~TestiranjeAlgoritma(){}

void TestiranjeAlgoritma::test_case1()
{    
    bool zbirnaProvera = true;

    Vertex* pocetnoTemeLanca;
    if(odsecenaTemena.size() > 0)
        Vertex* pocetnoTemeLanca = odsecenaTemena[0];

    for(int i=0; i<odsecenaTemena.size()-1; i++){

        Vertex* v1 = odsecenaTemena[i];
        Vertex* v2 = odsecenaTemena[i+1];

        if(v2 == pocetnoTemeLanca){
            //preskacemo sve do sledeceg lanca, treba da se i uveca za 2
            //ali ga uvecavamo za 1 jer ce na kraju petlje da se uveca i za taj drugi put gde pocinje sledeci lanac temena
            i+=1;
            pocetnoTemeLanca = odsecenaTemena[i+1];
            continue;
        }

        QPointF teme1(v1->x(), v1->y());
        QPointF teme2(v2->x(), v2->y());
        bool prvoPrpadaOkviru = proveriPripadnostOkviru(teme1, poligon1);
        bool drugoPripadaOkviru = proveriPripadnostOkviru(teme2, poligon1);

        if(!prvoPrpadaOkviru || !drugoPripadaOkviru){
            //tada jedno od temena ne pripada okviru
            //sto znaci da ni stranica v1->v2 ne pripada okviru
            //to je dovoljan slucaj da test padne
            //jer smo dobili stranicu koja je u odsecenom delu a ne pripada cela okviru
            zbirnaProvera = false;
            break;
        }
    }
    //ako nie otala na true znaci da je neka ivica u rezultatu vam poligona kojim odsecamo
    QVERIFY(zbirnaProvera == true);

}

void TestiranjeAlgoritma::test_case2()
{
    //nema presecnih tacaka, barem jedna pripada poligonu, tada moraju i ostale da pripadaju poligonmu

    if(algoritam->brojPreseka() == 0){

        Vertex* biloKojeTemePoligona = poligon2.vertex(0);
        QPointF t(biloKojeTemePoligona->x(), biloKojeTemePoligona->y());
        bool temePripadaPoligonu = proveriPripadnostOkviru(t, poligon1);

        if(temePripadaPoligonu){
            //tada i sva ostala temena moraju da pripadaju okviru

            Vertex* tekuceTeme;
            bool sviPripadaju = true;

            for(Vertex* v : poligon2.vertices()){

                QPointF teme(v->x(), v->y());
                bool pripada = proveriPripadnostOkviru(teme, poligon1);
                if(!pripada){
                    sviPripadaju = false;
                    break;
                }
            }

            QVERIFY(sviPripadaju == true);

        }else {
           //tada i sva ostala temena moraju biti van okvira
            Vertex* tekuceTeme;
            bool sviNEPripadaju = true;

            for(Vertex* v : poligon2.vertices()){

                QPointF teme(v->x(), v->y());
                bool pripada = proveriPripadnostOkviru(teme, poligon1);
                if(pripada){
                    sviNEPripadaju = false;
                    break;
                }
            }
            QVERIFY(sviNEPripadaju == true);
        }
    }

}

bool TestiranjeAlgoritma::proveriPripadnostOkviru(QPointF teme, DCEL& poligon1)
{
    QLineF linijaNaDesno(teme.x(), teme.y(), 1565, teme.y());
    int brojPresekaLinije = 0;

    for(HalfEdge* e : poligon1.edges()){

        QLineF ivica(e->origin()->x(), e->origin()->y(), e->next()->origin()->x(), e->next()->origin()->y());
        QPointF* presek = new QPointF();
        bool rezultat = pomocneFunkcije::presekDuzi(linijaNaDesno, ivica, *presek);

        if(rezultat)
            brojPresekaLinije++;
    }

    if(brojPresekaLinije % 2 == 0)
        return false;
    else
        return true;
}

QTEST_APPLESS_MAIN(TestiranjeAlgoritma)

#include "tst_testiranjealgoritma.moc"
