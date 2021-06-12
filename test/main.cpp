#include <DeerShortcut.h>
#include <QCoreApplication>
#include <QDebug>
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    /* code */
    DeerShortcut test;
    QKeySequence keyseq("F6");
    if(!test.setShortcut(keyseq)){
        qDebug() << keyseq.toString() << " setting fail!";
    }
    test.connect(&test, &DeerShortcut::activated, [&]{
        qDebug() << test.shortcut().toString() << " press";
    });
    return a.exec();
}
