#ifndef WEBPAGE_H
#define WEBPAGE_H
#include <QWebEngineView>

class WebPage : public QWebEngineView
{
    Q_OBJECT

public:
    explicit WebPage(QWidget *parent = nullptr);
    void SetCreateNew(bool);
    bool GetCreateNew();
    void Goto(QByteArray address);
    void Goto(QUrl url);
    void SendToChild(QEvent *);
protected:
    bool event(QEvent* evt);
    QWebEngineView *createWindow(QWebEnginePage::WebWindowType);
private:
    QObject *childObj = nullptr;
    bool Create;

signals:

public slots:
};

#endif // WEBPAGE_H
