#include "fraglets.h"


// std::string alphabet = {"abcdefghijklmnopqrstuvwxyz"};

std::string alphabet = {"abcdtuvxz"};


int main(int argc, char *argv[]) {


    // testset1.insert(&testset2);
    // // std:cout << testset1.count(&testset2);
    // // printUset(testset1);
    // cout << testset2.count("tes23123t") << "";
    QApplication a(argc, argv);

    

    fraglets frag;

    // for (int i = 0; i< 10; i++){
    //     // molecule mol = {"fork nop x match z split match x fork fork fork nop x * split match x fork fork fork nop x * copy z"};
    //     molecule mol = {"fork nop z match x split match z fork fork fork nop z * split match z fork fork fork nop z * x"};
    //     frag.inject(mol);

    //     std::string::iterator alphaIt;
    //     for (alphaIt = alphabet.begin();alphaIt!=alphabet.end();alphaIt++){
    //         molecule alphaMol = "fork nop ";
    //         alphaMol += *alphaIt;
    //         alphaMol += " match ";
    //         alphaMol += *alphaIt;
    //         alphaMol += " split match ";
    //         alphaMol +=  *alphaIt; 
    //         alphaMol +=  " fork fork fork nop ";
    //         alphaMol +=  *alphaIt;
    //         alphaMol +=  " * split match ";
    //         alphaMol +=  *alphaIt; 
    //         alphaMol += " fork fork fork nop "; 
    //         alphaMol +=  *alphaIt;
    //         alphaMol +=  " * "; 
    //         alphaMol +=  *alphaIt;
    //         frag.inject(alphaMol);
    //         frag.inject({*alphaIt});
    //     }
    // }

    molecule mol = {"fork nop z match z split match z fork fork fork nop z * split match z fork fork fork nop z * copy z"};
    frag.inject(mol);
    // molecule molll = {"fork nop a match b split match a fork fork fork nop a * split match a fork fork fork nop a * copy b"};
    for (int i = 0; i< 50; i++){
        frag.inject(mol);
    //     frag.inject(moll);
    //     // frag.inject(molll);
    //     frag.inject({"z"});
        frag.inject({"z"});
    //     // frag.inject({"b"});
    }

    // std::unordered_set<std::string>::iterator it;

    // for (it = unimolTags.begin();it!=unimolTags.end();it++){
    //     molecule mol1 = {"matchp z "};
    //     molecule mol2 = {"matchp z z "};
    //     molecule mol4 = {"matchp z z z "};
    //     molecule mol3 = {"z"};
    //     mol1.append(*it);
    //     mol2.append(*it);
    //     mol4.append(*it);
    //     frag.inject(mol1);
    //     frag.inject(mol2);
    //     frag.inject(mol3);
    //     frag.inject(mol4);
    // }
    
    molecule mol2 = {"matchp z "};
    std::string::iterator alphaIt2;
    std::unordered_set<std::string>::iterator uIt;
    for (alphaIt2 = alphabet.begin();alphaIt2!=alphabet.end();alphaIt2++){
        molecule newMol = mol2 + *alphaIt2;
        frag.inject(newMol);
        newMol = mol2 + "z " + *alphaIt2;
        frag.inject(newMol);

    }
    for (uIt = unimolTags.begin();uIt!=unimolTags.end();uIt++){

        molecule newMolTag = mol2  + *uIt;
        frag.inject(newMolTag);
        newMolTag = mol2 + "z " +*uIt;
        frag.inject(newMolTag);
    }
    for (alphaIt2 = alphabet.begin();alphaIt2!=alphabet.end();alphaIt2++){
        molecule newMol2 = mol2 + "match ";// + *alphaIt2;
        frag.inject(newMol2);
        newMol2 = mol2 + "z " + "match"; //+ *alphaIt2;
        frag.inject(newMol2);
        // molecule molalphap = {"matchp "+*alphaIt2};
        // frag.inject(molalphap);
    }
    // for (alphaIt = alphabet.begin();alphaIt!=alphabet.end();alphaIt++){
    //     molecule newMol2 = mol2 + "z " + "match " + *alphaIt;
    //     frag.inject(newMol2);


    // }




    // frag.interpret("sort.fra");

    frag.run(3000,200);


    QtCharts::QLineSeries *series = new QtCharts::QLineSeries();
    QtCharts::QLineSeries *series2 = new QtCharts::QLineSeries();
    
    for (int j = 0;j< frag.activeMultisetSize.size();j++){
        // std::cout <<j << " " << frag.activeMultisetSize[j]<<'\n';
        series->append(j,frag.activeMultisetSize[j]);

    }

    for (int j = 0;j< frag.passiveMultisetSize.size();j++){
        // std::cout <<j << " " << frag.passiveMultisetSize[j]<<'\n';
        series2->append(j,frag.passiveMultisetSize[j]);

    }

    QtCharts::QChart *chart = new QtCharts::QChart();
    chart->legend()->hide();
    for (std::vector<int> vec : frag.StackplotVector){
        QtCharts::QLineSeries *series3 = new QtCharts::QLineSeries();
        for (int r = 0;r< vec.size();r++){
            series3->append(r,vec[r]);
        }
        chart->addSeries(series3);
    }


//![2]

//![3]

    // chart->addSeries(series);
    // chart->addSeries(series2);
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