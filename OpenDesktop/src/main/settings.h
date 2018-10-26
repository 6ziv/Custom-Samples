#ifndef SETTINGS_H
#define SETTINGS_H

#include <QWidget>
#include<QCheckBox>
#include<QComboBox>
#include<QLineEdit>
#include<QPushButton>
#include<QUrl>
class Settings;
class URLLineEdit :public QLineEdit{
    Q_OBJECT
public:
    explicit URLLineEdit(QWidget *parent);
protected:
    void mouseReleaseEvent(QMouseEvent *);
    Settings *par;
};

class Settings : public QWidget
{
    Q_OBJECT
friend class URLLineEdit;
public:
    explicit Settings(QWidget *parent = nullptr);
    ~Settings();
    static QUrl GetUrl(QString key);
    static bool GetCreate(QString key);
    static Settings* ptr_Settings;
private:
    QPushButton *OK;
    QPushButton *Apply;
    QPushButton *Cancel;
    static void Set(QString key,QUrl url,bool create);
    int oldIndex;
    bool Changed=false;
    QComboBox *SelectAScreen;
    URLLineEdit *url;
    QUrl qurl;
    QCheckBox *CreateNew;
    QCheckBox *StartWithWindows;
    static void SetStartUp(bool);
    static bool GetStartUp();
signals:

public slots:
    void FuncChanged(int);
};

#endif // SETTINGS_H
