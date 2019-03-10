
#include <QtCore/QCoreApplication>
#include "decoder.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    
    decoder app(&a);
    
    return a.exec();

    getchar();
}
