#ifndef GLOBALDEF_HPP
#define GLOBALDEF_HPP
#include <QString>
#include <QDebug>
#include <QFile>

#ifndef LOADQSS
#define LOADQSS(qssFile)                        \
{                                               \
    QFile file(qssFile);                        \
    file.open(QFile::ReadOnly);                 \
    if(file.isOpen())                           \
{                                           \
    qApp->setStyleSheet(file.readAll());    \
    file.close();                           \
    }                                           \
    }
#endif

#ifndef SAFEDELETE
#define SAFEDELETE(pointer) \
{                           \
    if(pointer)             \
{                       \
    delete pointer;     \
    }                       \
    pointer = nullptr;          \
    }
#endif

enum WidgetTabType
{
    TAB_BANNA,
    TAB_CYLINDER,
    TAB_PROGRESS,
    TAB_FRAME,
    TAB_LIST,

    TAB_TABLE_WIDGET,
    TAB_MAX

};
namespace GlobalSpace
{
const QString STYLE_QSS_FILE_PATH = ":/res/res/stylesheet/stylesheet.qss";
const QString LOGO_PATH = ":/res/res/image/image.png";

const int INT_DOUBLE_TIMES = 2;
const float FLOAT_DOUBLE_TIMES = 2.0f;


}

class TEMP: public QObject{
    Q_OBJECT

public:
    TEMP() {}
signals:
    void sender();
};

#endif // GLOBALDEF_HPP
