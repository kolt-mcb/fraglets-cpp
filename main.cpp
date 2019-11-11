#include "fraglets.h"



// typedef unordered_multiset<unordered_multiset<string>*>  nested_unordered_multiset;
// typedef unordered_multiset<string>   string_unordered_multiset;

// nested_unordered_multiset testset1;
// string_unordered_multiset testset2 = {"test","test","test2"};

// typedef unordered_multiset<unordered_multiset<string>*>::iterator numit;
// typedef unordered_multiset<string>::iterator umit;

// void printUset(nested_unordered_multiset ums)
// {
//     //  begin() returns iterator to first element of set
//     numit it = ums.begin();
//     for (; it != ums.end(); it++){
//         umit nested_iterator = (*it)->begin();
//         for (; nested_iterator != (*it)->end(); nested_iterator++ ){
//             cout << *nested_iterator << "";
//         }
//     }
//     cout << endl;
// }





int main(int argc, char *argv[]) {


    // testset1.insert(&testset2);
    // // std:cout << testset1.count(&testset2);
    // // printUset(testset1);
    // cout << testset2.count("tes23123t") << "";
    QApplication a(argc, argv);

    
    QtCharts::QLineSeries *series = new QtCharts::QLineSeries();
    QtCharts::QLineSeries *series2 = new QtCharts::QLineSeries();
//![1]

//![2]

    // series->append(0, 6);
    // series->append(2, 4);
    // series->append(3, 8);
    // series->append(7, 4);
    // series->append(10, 5);
    // *series << QPointF(11, 1) << QPointF(13, 3) << QPointF(17, 6) << QPointF(18, 3) << QPointF(20, 2);


    fraglets frag;
    molecule mol = {"fork nop x match z split match x fork fork fork nop x * split match x fork fork fork nop x * copy z"};
    molecule moll = {"fork nop z match x split match z fork fork fork nop z * split match z fork fork fork nop z * copy x"};
    for (int i = 0; i< 10; i++){
        frag.inject(mol);
        // frag.inject(moll);
        frag.inject({"y"});
    }

    std::unordered_set<std::string>::iterator it;

    for (it = unimolTags.begin();it!=unimolTags.end();it++){
        molecule mol1 = {"matchp z "};
        molecule mol2 = {"matchp z z "};
        molecule mol4 = {"matchp z z z "};
        molecule mol3 = {"z"};
        mol1.append(*it);
        mol2.append(*it);
        mol4.append(*it);
        frag.inject(mol1);
        frag.inject(mol2);
        frag.inject(mol3);
        frag.inject(mol4);
    }
    frag.inject({"matchp x match"});
    frag.inject({"matchp x c"});
    frag.inject({"matchp x v"});
    frag.inject({"matchp x b"});
    frag.inject({"matchp x n"});
    frag.inject({"matchp x m"});
    frag.inject({"matchp x x match"});
    frag.inject({"matchp x x x"});
    frag.inject({"matchp z match"});
    frag.inject({"matchp z z match"});
    frag.inject({"matchp z z z"});
    frag.inject({"matchp z c"});
    frag.inject({"matchp z v"});
    frag.inject({"matchp z b"});
    frag.inject({"matchp z n"});
    frag.inject({"matchp z m"});
    // frag.inject({"matchp x y z"});
    // frag.inject({"matchp x y x"});
    // frag.inject({"matchp x y c"});
    // frag.inject({"matchp x y v"});
    // frag.inject({"matchp x y b"});
    // frag.inject({"matchp x y n"});
    // frag.inject({"matchp x y m"});

    // frag.interpret("sort.fra");

    frag.run(10000000,250);

    
    for (int j = 0;j< frag.activeMultisetSize.size();j++){
        // std::cout <<j << " " << frag.activeMultisetSize[j]<<'\n';
        series->append(j,frag.activeMultisetSize[j]);

    }

    for (int j = 0;j< frag.passiveMultisetSize.size();j++){
        // std::cout <<j << " " << frag.passiveMultisetSize[j]<<'\n';
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