#include "AreaSelector.h"

Window::Window(QWidget *parent) :
        QWidget(parent)
{
    QGridLayout *layout = new QGridLayout;
    screenGeometry = QApplication::desktop()->screenGeometry();
    int x = (screenGeometry.width()-this->width()) / 2;
    int y = (screenGeometry.height()-this->height()) / 2;

    // Set size of the window
//    setFixedSize(100, 50);
//    this->setWindowTitle("Select screen area to record");
    this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
//    this->setWindowFlags( (Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowStaysOnTopHint )); //| Qt::WindowCloseButtonHint
    this->setWindowOpacity(0.7);
    this->setLayout(layout);
    this->setGeometry(x, y, 500, 300);

    // Create and position the button
    m_button = new QPushButton("SELECT", this);
    m_button->move(m_button->parentWidget()->geometry().center());
    m_button->setSizePolicy(QSizePolicy ::Fixed , QSizePolicy ::Fixed);
    m_button->setIcon(QIcon(":/images/start_hov.png"));
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

//void Window::mousePressEvent(QMouseEvent *event){
//    mpos = event->pos();
//}
//
//void Window::mouseMoveEvent(QMouseEvent *event){
//    if (event->buttons() & Qt::LeftButton) {
//        QPoint diff = event->pos() - mpos;
//        QPoint newpos = this->pos() + diff;
//
//        this->move(newpos);
//    }
//}
void Window::mousePressEvent(QMouseEvent *event){
//From Qt Documentation:
//Reason why pos() wasn't working is because the global
//position at time of event may be very different
//This is why the mpos = event->pos(); line before was
//possibly causing jumping behavior

    if (event->button() == Qt::LeftButton){
        //Coordinates have been mapped such that the mouse position is relative to the
        //upper left of the main window
        mpos = event->globalPos() - frameGeometry().topLeft();

        //At the moment of mouse click, capture global position and
        //lock the size of window for resizing
        global_mpos = event->globalPos();
        storeWidth = this->width();
        storeHeight= this->height();

        event->accept();
    }
}

void Window::mouseMoveEvent(QMouseEvent *event){

    //mapped mouse relative to upper left of window
    const QPoint &rs_mpos = event->globalPos() - frameGeometry().topLeft();//general position tracker for resizing
//    QTextStream out(stdout);

    //How much of the corner is considered a "resizing zone"
    //I was experiencing jumping behavior with rs_size is 10 so
    //I recommend rs_size=50
    int rs_size=50;

    //Big if statement checks if your mouse is in the upper left,
    //upper right, lower left, and lower right
    if ( (abs(rs_mpos.x()) < rs_size && abs(rs_mpos.y()) < rs_size) ||
         (abs(rs_mpos.x()) > this->width()-rs_size && abs(rs_mpos.y()) <rs_size) ||
         (abs(rs_mpos.x()) < rs_size && abs(rs_mpos.y())> this->height()-rs_size) ||
         (abs(rs_mpos.x()) > this->width()-rs_size && abs(rs_mpos.y())> this->height()-rs_size)

            ){

        //Initiate matrix
        int adjXfac=0;
        int adjYfac=0;
        int transXfac=0;
        int transYfac=0;

        //Upper left corner section
        if ( (abs(rs_mpos.x()) < rs_size && abs(rs_mpos.y()) < rs_size)){
            this->setCursor(Qt::SizeFDiagCursor);
            //Upper left. No flipping of axis, no translating window
            adjXfac=1;
            adjYfac=1;

            transXfac=0;
            transYfac=0;
        }
            //Upper right corner section
        else if(abs(rs_mpos.x()) > this->width()-rs_size &&
                abs(rs_mpos.y()) <rs_size){
            this->setCursor(Qt::SizeBDiagCursor);
            //upper right. Flip displacements in mouse movement across x axis
            //and translate window left toward the mouse
            adjXfac=-1;
            adjYfac=1;

            transXfac = 1;
            transYfac =0;
        }
            //Lower left corner section
        else if(abs(rs_mpos.x()) < rs_size &&
                abs(rs_mpos.y())> this->height()-rs_size){
            this->setCursor(Qt::SizeBDiagCursor);

            //lower left. Flip displacements in mouse movement across y axis
            //and translate window up toward mouse
            adjXfac=1;
            adjYfac=-1;

            transXfac=0;
            transYfac=1;
        }
            //Lower right corner section
        else if(abs(rs_mpos.x()) > this->width()-rs_size &&
                abs(rs_mpos.y())> this->height()-rs_size){
            this->setCursor(Qt::SizeFDiagCursor);

            //lower right. Flip mouse displacements on both axis and
            //translate in both x and y direction left and up toward mouse.
            adjXfac=-1;
            adjYfac=-1;

            transXfac=1;
            transYfac=1;
        }
        if (event->buttons()==Qt::LeftButton ){
            //Calculation of displacement. adjXfac=1 means normal displacement
            //adjXfac=-1 means flip over axis
            int adjXdiff = adjXfac*(event->globalPos().x() - global_mpos.x());

            int adjYdiff = adjYfac*(event->globalPos().y() - global_mpos.y());

            //if transfac is 1 then movepoint of mouse is translated
            QPoint movePoint(mpos.x() - transXfac*adjXdiff, mpos.y()-transYfac*adjYdiff);
            move(event->globalPos()-movePoint);
            resize(storeWidth-adjXdiff, storeHeight-adjYdiff);

            event->accept();
        }
    }
        //in any move event if it is not in a resize region use the default cursor
    else{
        this->setCursor(Qt::ArrowCursor);
        //simple move section
        if (event->buttons()==Qt::LeftButton){
            move(event->globalPos() - mpos);
            event->accept();
        }
    }
}
void Window::mouseReleaseEvent(QMouseEvent * event){
//    if (event->buttons()==Qt::LeftButton )
    this->setCursor(Qt::ArrowCursor);
}