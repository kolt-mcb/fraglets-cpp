#include "fraglets.h"
#include <QtWidgets/QApplication>
#include <QtQml/QQmlContext>
#include <QtQuick/QQuickView>
#include <QtQml/QQmlEngine>
#include <QtCore/QDir>


// std::string alphabet = {"abcdefghijklmnopqrstuvwxyz!@#$%^&*():,.{}-=_+`~<>?/"};

std::string alphabet = {"abcdefghijklmnzx*"};


int main(int argc, char *argv[]) {





        // Qt Charts uses Qt Graphics View Framework for drawing, therefore QApplication must be used.
    QApplication app(argc, argv);

    QQuickView viewer;

    // The following are needed to make examples run without having to install the module
    // in desktop environments.
#ifdef Q_OS_WIN
    QString extraImportPath(QStringLiteral("%1/../../../../%2"));
#else
    QString extraImportPath(QStringLiteral("%1/../../../%2"));
#endif
    viewer.engine()->addImportPath(extraImportPath.arg(QGuiApplication::applicationDirPath(),
                                      QString::fromLatin1("qml")));
    QObject::connect(viewer.engine(), &QQmlEngine::quit, &viewer, &QWindow::close);

    viewer.setTitle(QStringLiteral("QML Oscilloscope"));

    DataSource dataSource(&viewer);
    viewer.rootContext()->setContextProperty("dataSource", &dataSource);

    viewer.setSource(QUrl("qrc:/qml/qmloscilloscope/main.qml"));
    viewer.setResizeMode(QQuickView::SizeRootObjectToView);
    viewer.setColor(QColor("#404040"));
    viewer.show();



    QApplication a(argc, argv);

    
    QtCharts::QLineSeries *series = new QtCharts::QLineSeries();
    QtCharts::QLineSeries *series2 = new QtCharts::QLineSeries();



    fraglets frag;
    molecule mol = {"fork nop x match z split match x fork fork fork nop x * split match x fork fork fork nop x * copy z"};
    molecule moll = {"fork nop z match x split match z fork fork fork nop z * split match z fork fork fork nop z * copy x"};
    // molecule molll = {"fork nop a match b split match a fork fork fork nop a * split match a fork fork fork nop a * copy b"};
    for (int i = 0; i< 100; i++){
        frag.inject(mol);
        frag.inject(moll);
        // frag.inject(molll);
        frag.inject({"z"});
        frag.inject({"x"});
        // frag.inject({"b"});
    }


    
    molecule mol2 = {"matchp z "};
    std::string::iterator alphaIt;
    std::unordered_set<std::string>::iterator uIt;
    for (alphaIt = alphabet.begin();alphaIt!=alphabet.end();alphaIt++){
        molecule newMol = mol2 + *alphaIt;
        frag.inject(newMol);
        newMol = mol2 + "z " + *alphaIt;
        frag.inject(newMol);
        for (uIt = unimolTags.begin();uIt!=unimolTags.end();uIt++){

            molecule newMolTag = mol2  + *uIt;
            frag.inject(newMolTag);
            newMolTag = mol2 + "z " +*uIt;
            frag.inject(newMolTag);
        }
    }
    for (alphaIt = alphabet.begin();alphaIt!=alphabet.end();alphaIt++){
        molecule newMol2 = mol2 + "match " + *alphaIt;
        frag.inject(newMol2);
        newMol2 = mol2 + "z " + "match " + *alphaIt;
        frag.inject(newMol2);


    }
    // for (alphaIt = alphabet.begin();alphaIt!=alphabet.end();alphaIt++){
    //     molecule newMol2 = mol2 + "z " + "match " + *alphaIt;
    //     frag.inject(newMol2);


    frag.run(5000,1100);

    
    for (int j = 0;j< frag.activeMultisetSize.size();j++){
        series->append(j,frag.activeMultisetSize[j]);

    }

    for (int j = 0;j< frag.passiveMultisetSize.size();j++){
        series2->append(j,frag.passiveMultisetSize[j]);
    }



//![2]

//![3]
    QtCharts::QChart *chart = new QtCharts::QChart();
    chart->legend()->hide();
    chart->addSeries(series);
    chart->addSeries(series2);
    chart->createDefaultAxes();
    chart->setTitle("Simple line chart example");
//![3]

//![4]
    QtCharts::QChartView *chartView = new QtCharts::QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
//![4]


//![5]
    QMainWindow window;
    window.setCentralWidget(chartView);
    window.resize(400, 300);
    window.show();
// //![5]




  

    // return 0;
    return a.exec();
}