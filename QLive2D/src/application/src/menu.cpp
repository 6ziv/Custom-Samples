#include "menu.hpp"
#include <QLabel>
#include <QPropertyAnimation>
#include "musicplayer.hpp"
#include "qdebug.h"
MenuItem::MenuItem(QPixmap pm) : QLabel(nullptr)
{
    this->setGeometry(0,0,30,30);
    this->setAttribute(Qt::WA_TranslucentBackground);
    this->setPixmap(pm);
    this->setWindowFlag(Qt::FramelessWindowHint);
    this->setWindowFlag(Qt::Tool);
    this->setWindowFlag(Qt::WindowStaysOnTopHint);
    this->setWindowOpacity(0);
    this->show();

    group1=new QParallelAnimationGroup(this);
    group2=new QParallelAnimationGroup(this);
    ani[0]=new QPropertyAnimation(this,"geometry");
    ani[1]=new QPropertyAnimation(this,"windowOpacity");
    ani[2]=new QPropertyAnimation(this,"geometry");
    ani[3]=new QPropertyAnimation(this,"windowOpacity");

    ani[0]->setDuration(100);
    ani[1]->setDuration(100);
    ani[2]->setDuration(100);
    ani[3]->setDuration(100);
    ani[1]->setStartValue(0);
    ani[1]->setEndValue(qreal(opacity1));
    ani[3]->setStartValue(qreal(opacity1));
    ani[3]->setEndValue(0);
    group1->addAnimation(ani[0]);
    group1->addAnimation(ani[1]);
    group2->addAnimation(ani[2]);
    group2->addAnimation(ani[3]);
    connect(group1,&QParallelAnimationGroup::stateChanged,group1,
            [this](QParallelAnimationGroup::State s1,QParallelAnimationGroup::State){
                if(s1==QParallelAnimationGroup::State::Stopped){
                    this->setDisabled(false);
                }
            }
    );
}
MenuItem::MenuItem(QPixmap p1,QPixmap p2)
    :MenuItem(p1){
    this->pix1=p1;
    this->pix2=p2;
}
void MenuItem::alterimage(bool on){
    if(on)
        this->setPixmap(pix2);
    else
        this->setPixmap(pix1);
}
void MenuItem::enterEvent(QEvent* e){
    if(!this->isEnabled())return;
    this->setWindowOpacity(qreal(opacity2));
}
void MenuItem::leaveEvent(QEvent*e){
    if(!this->isEnabled())return;
    this->setWindowOpacity(qreal(opacity1));
}
void MenuItem::mousePressEvent(QMouseEvent *ev){
    if(!this->isEnabled())return;
    emit this->clicked();
}
void MenuItem::AnimatedShow(){
    if(!this->isEnabled())return;
    group1->start();
}
void MenuItem::AnimatedHide(){
    if(!this->isEnabled())return;
    group2->start();
}
void MenuItem::moveto(QPoint pos){
    ani[0]->setStartValue(QRect(pos,this->size()));
    ani[0]->setEndValue(QRect(pos+QPoint(0,100),this->size()));
    ani[2]->setStartValue(QRect(pos+QPoint(0,100),this->size()));
    ani[2]->setEndValue(QRect(pos,this->size()));
}
Menu::Menu(){
    items[0]=new MenuItem(QPixmap::fromImage(QImage(":/icons/music.png").scaled(30,30)),QPixmap::fromImage(QImage(":/icons/nomusic.png").scaled(30,30)));
    items[1]=new MenuItem(QPixmap::fromImage(QImage(":/icons/setting.png").scaled(30,30)));
    items[2]=new MenuItem(QPixmap::fromImage(QImage(":/icons/power.png").scaled(30,30)));
    items[3]=new MenuItem(QPixmap::fromImage(QImage(":/icons/hide.png").scaled(30,30)));
    for(int i=0;i<4;i++){
        group1.addAnimation(items[3-i]->group1);
        group2.addAnimation(items[i]->group2);
    }
    qDebug()<<group1.totalDuration();
    for(int i=0;i<4;i++){
        connect(items[i],&MenuItem::clicked,this,[i,this](){
            emit selected(i);
            this->AnimatedHide();
        });
    }
}
void Menu::moveto(QPoint pt){
    for(int i=0;i<4;i++)
        items[i]->moveto(pt+QPoint(0,i*50));
}

void Menu::AnimatedShow(){
    items[0]->alterimage(MusicPlayer::GetInstance()->isMuted());
    group2.stop();
    group1.start();
    is_shown=true;
}
void Menu::AnimatedHide(){
    for(int i=0;i<4;i++)items[i]->setDisabled(true);
    group1.stop();
    group2.start();
    is_shown=false;
}
