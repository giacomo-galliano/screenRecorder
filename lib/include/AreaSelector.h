#ifndef AREASELECTOR_H
#define AREASELECTOR_H
#include <QWidget>
#include <QPushButton>
#include <iostream>
#include <QApplication>
#include <QGridLayout>
#include <QDesktopWidget>
#include <QMouseEvent>

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
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent * event);
private:
    QPushButton *m_button;
    QRect screenGeometry;
//    QPoint mpos;

    ///
    QPoint global_mpos;
    int storeWidth;
    int storeHeight;
    QPoint mpos;
};
#endif //AREASELECTOR_H
