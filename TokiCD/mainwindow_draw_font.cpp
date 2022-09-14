#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QFileDialog>
#include <QPicture>
#include <QPainter>

#define KANJI_SIZE_X 12
#define KANJI_SIZE_Y 12
#define KANJI_BYTES (KANJI_SIZE_X*KANJI_SIZE_Y/8)
#define KANJIS_X 64
#define KANJIS_Y 64
#define KANJIS_OFFSET 0
//(64*51*2)

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
    //ba = ba.mid(ui->spinBox->value()*KANJI_BYTES);
    QPicture pic;
    QPainter pai;
    pai.begin(&pic);
    pai.setPen(Qt::black);
    pai.fillRect(0,0,KANJIS_X*KANJI_SIZE_X,KANJIS_Y*KANJI_SIZE_Y,QBrush(Qt::black));
    pai.setPen(Qt::white);
    int cell_map[KANJI_SIZE_X*KANJI_SIZE_Y];
    //for (int cell=0;cell<3421;cell++)
    for (int cell=0;cell<KANJIS_X*KANJIS_Y;cell++)
    {
        int cell_x = cell%KANJIS_X;
        int cell_y = cell/KANJIS_X;
        //mappint cell to map
        for (int i=0;i<KANJI_BYTES;i++)
        {
            char c = ba.at((cell+KANJIS_OFFSET)*KANJI_BYTES+i);
            for (int j=0;j<8;j++)
            {
                if (c & (1<<(7-j)))
                    cell_map[i*8+j] = 1;
                else
                    cell_map[i*8+j] = 0;
            }

        }
        for (int y=0;y<KANJI_SIZE_Y;y++)
        {
            for (int x=0;x<KANJI_SIZE_X;x++)
            {
                if (cell_map[y*KANJI_SIZE_X+x] == 1)
                    pai.drawPoint(cell_x*KANJI_SIZE_X+x,cell_y*KANJI_SIZE_Y+y);
            }
        }
    }
    pai.end();

    ui->label_font->setPicture(pic);
}
