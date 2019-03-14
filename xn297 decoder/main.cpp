#include "xn297decoder.h"
#include <QtWidgets/QApplication>
#include <QHBoxLayout>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    xn297decoder w;

   

    w.show();
    return a.exec();
}
