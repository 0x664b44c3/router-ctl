#include <QCoreApplication>
#include <routercore.h>
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    RouterCore core;
    core.testcase();

//    return a.exec();
}
