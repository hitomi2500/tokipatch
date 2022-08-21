#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QFileDialog>
#include <QPicture>
#include <QPainter>

void MainWindow::on_pushButton_3_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,tr("Open Font file"), "", tr("Font file (*.bin)"));
    QFile file_in(fileName);
    QByteArray ba;
    if (file_in.exists())
    {
        file_in.open(QIODevice::ReadOnly);
        ba = file_in.readAll();
    }
    //ba = ba.mid(ui->spinBox->value()*18);
    QPicture pic;
    QPainter pai;
    pai.begin(&pic);
    pai.setPen(Qt::black);
    pai.fillRect(0,0,768,768,QBrush(Qt::black));
    pai.setPen(Qt::white);
    int cell_map[144];
    //for (int cell=0;cell<3421;cell++)
    for (int cell=0;cell<4096;cell++)
    {
        int cell_x = cell%64;
        int cell_y = cell/64;
        //mappint cell to map
        for (int i=0;i<18;i++)
        {
            char c = ba.at(cell*18+i);
            for (int j=0;j<8;j++)
            {
                if (c & (1<<(7-j)))
                    cell_map[i*8+j] = 1;
                else
                    cell_map[i*8+j] = 0;
            }

        }
        for (int y=0;y<12;y++)
        {
            for (int x=0;x<12;x++)
            {
                if (cell_map[y*12+x] == 1)
                    pai.drawPoint(cell_x*12+x,cell_y*12+y);
            }
        }
    }
    pai.end();

    ui->label_font->setPicture(pic);
}
