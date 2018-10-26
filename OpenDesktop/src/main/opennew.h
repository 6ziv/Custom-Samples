
#ifndef OPENNEW_H
#define OPENNEW_H
#include <QWebEngineView>

//This class is used to do the "open in new window" action.
//I know it is wasting, and I wish to see a better solution
class OpenNew : public QWebEngineView
{
    Q_OBJECT
public:
    explicit OpenNew();
};

#endif // OPENNEW_H
