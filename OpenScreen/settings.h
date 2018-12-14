#ifndef SETTINGS_H
#define SETTINGS_H
#include <QLabel>
#include <QWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QSettings>
#include <QApplication>
#include <QCheckBox>
class Label:public QLabel{
    Q_OBJECT
public:
    Label(QWidget *parent):QLabel(parent){}
protected:
    void mousePressEvent(QMouseEvent *e){
        emit(pressed());
    }
signals:
    void pressed();
};

class Settings:public QWidget
{
public:
    Settings();

private:
    QString filename;
    bool IsRunning;
    bool AutoStart;
    QPushButton *btnRun;
    QPushButton *btnStop;
    QPushButton *btnKill;
    QTextEdit *FileSelector;
    QSettings *Reg;
    QSettings *RegStart;
    QCheckBox *StartWithSystem;
    QString GetFile();
    bool CheckRunning();
    void Refresh();
    void Stop();
    void Start();

};

#endif // SETTINGS_H
