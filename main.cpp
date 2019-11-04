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
    // molecule mol = {"fork nop a match y split match a fork fork fork nop a * split match a fork fork fork nop a * y"};
    // frag.inject({"y"});
    // frag.inject(mol);
    frag.interpret("sort.fra");

    frag.run(10);

    
    for (int j = 0;j< frag.activeMultisetSize.size();j++){
        std::cout <<j << " " << frag.activeMultisetSize[j]<<'\n';
        series->append(j,frag.activeMultisetSize[j]);

    }

    for (int j = 0;j< frag.passiveMultisetSize.size();j++){
        std::cout <<j << " " << frag.passiveMultisetSize[j]<<'\n';
        series->append(j,frag.passiveMultisetSize[j]);

    }



//![2]

//![3]
    QtCharts::QChart *chart = new QtCharts::QChart();
    chart->legend()->hide();
    chart->addSeries(series);
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