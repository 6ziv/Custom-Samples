
#ifndef DESKTOPITEM_H
#define DESKTOPITEM_H
#include<QWidget>
#include"webpage.h"

class DesktopItem : public QWidget
{
    Q_OBJECT
public:
    explicit DesktopItem(QScreen *parent);

    void Dispatch(QMouseEvent *e);
    QUrl GetUrl();
    void Goto(const QUrl url);
    void Kill();
    bool GetCreate();
    void SetCreate(bool on);
protected:
    WebPage *webpage;
    bool nativeEvent(const QByteArray &eventType, void *message, long *result);
    void closeEvent(QCloseEvent *);
    bool dying=false;
};

#endif // DESKTOPITEM_H
