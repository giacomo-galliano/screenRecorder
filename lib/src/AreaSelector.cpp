#include "../include/AreaSelector.h"

Window::Window(QWidget *parent) :
        QWidget(parent)
{
    QGridLayout *layout = new QGridLayout;
    screenGeometry = QApplication::desktop()->screenGeometry();
    int x = (screenGeometry.width()-this->width()) / 2;
    int y = (screenGeometry.height()-this->height()) / 2;

    // Set size of the window
//    setFixedSize(100, 50);
    this->setWindowTitle("Select screen area to record");
    this->setWindowFlags( (Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowStaysOnTopHint )); //| Qt::WindowCloseButtonHint
    this->setWindowOpacity(0.5);
    this->setLayout(layout);
    this->setGeometry(x, y, 500, 300);

    // Create and position the button
    m_button = new QPushButton("SELECT", this);
    m_button->move(m_button->parentWidget()->geometry().center());
    m_button->setSizePolicy(QSizePolicy ::Fixed , QSizePolicy ::Fixed);
//    m_button->setGeometry(10, 10, 80, 30);
//    m_button->setCheckable(true);

    layout->addWidget(m_button, 0, 0);

    connect(m_button, SIGNAL (clicked()), this, SLOT (slotButtonClicked()));
}

void Window::slotButtonClicked()
{
    //completare ocn gli altri settaggi che sono standard (per ogni if)
//    m_button->setText("Checked");
    if(QWidget::x() < 0){
        this->x = 0;
        this->W = QWidget::width() + QWidget::x();
    }else if(QWidget::x() + QWidget::width() > screenGeometry.width()){
        this->x = QWidget::x();
        this->W = screenGeometry.width() - QWidget::x();
    }else{
        this->x = QWidget::x();
        this->W = QWidget::width();
    }
    if(QWidget::y() < 0){
        this->y = 0;
        this-> H = QWidget::height() + QWidget::y();
    }else if(QWidget::y() + QWidget::height() > screenGeometry.height()){
        this->y = QWidget::y();
        this-> H = screenGeometry.height() - QWidget::y();
    }else{
        this->y = QWidget::y();
        this-> H = QWidget::height();
    }

    if(W%2 != 0)
        W-=1;
    if(H%2 != 0)
        H-=1;

    QApplication::quit();
}