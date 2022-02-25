#ifndef AREASELECTOR_H
#define AREASELECTOR_H
#include <QWidget>
#include <QPushButton>
#include <iostream>
#include <QApplication>
#include <QGridLayout>
#include <QDesktopWidget>

class QPushButton;
class Window : public QWidget
{
Q_OBJECT
public:
    explicit Window(QWidget *parent = 0);
    int x;
    int y;
    int W;
    int H;
private slots:
    void slotButtonClicked();
private:
    QPushButton *m_button;
    QRect screenGeometry;
};
#endif //AREASELECTOR_H
