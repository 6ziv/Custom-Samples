#ifndef MENU_H
#define MENU_H

#include <QLabel>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>
class MenuItem : public QLabel
{
    Q_OBJECT
public:
    explicit MenuItem(QPixmap pm);
    explicit MenuItem(QPixmap pm1,QPixmap pm2);
    void AnimatedShow();
    void AnimatedHide();
    void moveto(QPoint pos);
    void alterimage(bool on);
protected:
    QPixmap pix1;
    QPixmap pix2;
    void enterEvent(QEvent*)override;
    void leaveEvent(QEvent*)override;
    void mousePressEvent(QMouseEvent *ev) override;
private:
    const long double opacity1=0.5;
    const long double opacity2=0.8;
    QParallelAnimationGroup *group1;
    QParallelAnimationGroup *group2;
    QPropertyAnimation *ani[4];
    QPoint point;
    friend class Menu;
signals:
    void clicked();
};
class Menu:public QObject
{
    Q_OBJECT
public:
    Menu();
    void moveto(QPoint pt);
    void AnimatedShow();
    void AnimatedHide();
    inline bool IsShown(){
        return is_shown;
    }
private:
    bool is_shown=false;
    QSequentialAnimationGroup group1;
    QSequentialAnimationGroup group2;
    MenuItem *items[4];
signals:
    void selected(int);
};
#endif // MENU_H
