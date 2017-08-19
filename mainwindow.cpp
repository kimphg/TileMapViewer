#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QPaintEvent>
#include <QPainter>
QString mPath;
CConfig *mConfig= new CConfig;
QList<CCamera*> cameraList;
MainWindow::MainWindow(QWidget *parent) :dxMap(0),dyMap(0),
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    //init UI elements
    ui->setupUi(this);
    ui->frame->setHidden(true);
    this->setGeometry(100,100,1024,768);
    //Load initial setting from config file
    LoadSettings();
    // init Map Object
    map = new CMap(this);
    map->setCenterPos(mLat,mLon);
    map->setPath(mPath);
    isPressed = false;
    CCamera *cam = new CCamera();
    cam->setCamName("Camera 1");!!!
    cam->requestAzi();
    cameraList.push_back(cam);

}
void MainWindow::LoadSettings()
{
    mScale = mConfig->getDouble("mScale",500);
    mPath = mConfig->getString("mPath","C:/mapData/");
    mLat = mConfig->getDouble("mLat",21.117);
    mLon = mConfig->getDouble("mLon",105.325);
}
MainWindow::~MainWindow()
{
    delete ui;
    delete mConfig;
}

void MainWindow::paintEvent(QPaintEvent * e)
{
    e=e;
    QPainter p(this);
    ui->label_scale->setText("OSM scale factor:" + QString::number(map->getScaleRatio()));
    QPixmap pix = map->getImage(mScale);
    p.drawPixmap((width()/2.0-pix.width()/2.0)+dxMap,
                 (height()/2.0-pix.height()/2.0)+dyMap,
                 pix.width(),pix.height(),pix
                 );
    p.setPen(QPen(Qt::white,2));


    // draw the reference 1km line in top left of the map
    if(this->mScale<width()/2)
    {
        p.drawLine(30,10,30+this->mScale,10);
        p.drawText(rect(), Qt::AlignTop|Qt::TextWordWrap,"1 km");
    }
    else if(this->mScale<width())
    {
        p.drawLine(30,10,30+this->mScale/2,10);
        p.drawText(rect(), Qt::AlignTop|Qt::TextWordWrap,"500m");
    }
    else if(this->mScale/5<width()/2)
    {
        p.drawLine(30,10,30+this->mScale/5,10);
        p.drawText(rect(), Qt::AlignTop|Qt::TextWordWrap,"200m");
    }
    else if(this->mScale/10<width()/2)
    {
        p.drawLine(30,10,30+this->mScale/10,10);
        p.drawText(rect(), Qt::AlignTop|Qt::TextWordWrap,"100m");
    }
    else if(this->mScale/20<width()/2)
    {
        p.drawLine(30,10,30+this->mScale/20,10);
        p.drawText(rect(), Qt::AlignTop|Qt::TextWordWrap,"50m");
    }
    p.drawText(rect().adjusted(10,20,0,0), Qt::AlignTop|Qt::TextWordWrap,QString::number(mLat,'f',4));
    p.drawText(rect().adjusted(10,30,0,0), Qt::AlignTop|Qt::TextWordWrap,QString::number(mLon,'f',4));
    // draw the crosshair mark in the center
    int scrCtx = width()/2;
    int scrCty = height()/2;
    drawCrossHairMark(scrCtx,scrCty,&p);
}
void MainWindow::drawCrossHairMark(int x,int y,QPainter* p)
{
    p->drawLine(x-10,y,x-5,y);
    p->drawLine(x+10,y,x+5,y);
    p->drawLine(x,y-10,x,y-5);
    p->drawLine(x,y+10,x,y+5);
}
void MainWindow::resizeEvent(QResizeEvent *)
{
    map->setImgSize(width(), height());

}

void MainWindow::keyPressEvent(QKeyEvent *e)
{
//    if (e->key() != Qt::Key_Z )
//    {
//        return;
//    }
//    else{
//        int zoom = (int) map->getScaleRatio();
//        switch(zoom)
//        {
//        case 14: map->setScaleRatio(15);
//            break;
//        case 15: map->setScaleRatio(16);
//            break;
//        case 16: map->setScaleRatio(14);
//            break;
//        default: map->setScaleRatio(15);
//            break;
//        }

//        map->invalidate();
//        map->UpdateImage();
//    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *e)
{

    if(isPressed&&(e->buttons() & Qt::LeftButton)) {
        dxMap = e->x()-pressPos.x();
        dyMap = e->y()-pressPos.y();
        update();
    }
    else
    {
        //show cursor lat-lon
        short   x = this->mapFromGlobal(QCursor::pos()).x() - width()/2;
        short   y = this->mapFromGlobal(QCursor::pos()).y() - height()/2;
        double lat,lon;
        map->ConvKmToWGS(x/mScale,-y/mScale,&lon,&lat);
        ui->lineEdit_cursor_lat->setText(QString::number(lat));
        ui->lineEdit_cursor_lon->setText(QString::number(lon));
    }
}

void MainWindow::mousePressEvent(QMouseEvent *e)
{

    if(e->button() != Qt::LeftButton)
    {
        return;
    }
    isPressed = true;
    pressPos = e->pos();
}

void MainWindow::mouseReleaseEvent(QMouseEvent *e)
{

    // change center lat-lon of the map

    map->ConvKmToWGS(-(double)dxMap/mScale,(double)dyMap/mScale,&mLon,&mLat);
    map->setCenterPos(mLat,mLon);
    ui->lineEdit_lat->setText(QString::number(mLat,'g',10));
    ui->lineEdit_lon->setText(QString::number(mLon,'g',10));
    dxMap = 0;
    dyMap = 0;
    update();
}


void MainWindow::on_lineEdit_returnPressed()
{
    map->setPath(ui->lineEdit->text());
    update();
}
void MainWindow::wheelEvent(QWheelEvent* event)
{
    if(event->delta()>0)mScale*=1.2;
    if(event->delta()<0)mScale/=1.2;
    if(mScale>8000)mScale = 8000;
    if(mScale<1)mScale = 1;
    update();
    repaint();
}
