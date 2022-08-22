#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QFileDialog>
#include <QPicture>
#include <QPainter>

void MainWindow::on_pushButton_2_clicked()
{
    QFile iso_file("8bpp.bin");
    iso_file.open(QIODevice::ReadOnly);
    QByteArray b = iso_file.readAll();
    iso_file.close();

    QFile iso_file2("1bpp.bin");
    iso_file2.open(QIODevice::WriteOnly|QIODevice::Truncate);
    for (int i=0; i<b.length()/8;i++)
    {
        char c = 0;
        for (int j=0;j<8;j++)
        {
            if (b.at(i*8+j) == 1)
                c |= 1<<(7-j);
        }
        iso_file2.write(QByteArray(1,c));
    }
    iso_file2.close();
}


void MainWindow::on_pushButton_4_clicked()
{
    QFile iso_file("2x4bpp.data");
    iso_file.open(QIODevice::ReadOnly);
    QByteArray b = iso_file.readAll();
    iso_file.close();

    QFile iso_file2("4bpp.data");
    iso_file2.open(QIODevice::WriteOnly|QIODevice::Truncate);
    char c1,c2;
    for (int i=0; i<b.length()/2;i++)
    {
        c1 = b.at(i*2);
        //if (c1 > 3) c1++;
        c2 = (c1&0xF)<<4;
        c1 = b.at(i*2+1);
        //if (c1 > 3) c1++;
        c2 |= c1&0xF;
        iso_file2.write(QByteArray(1,c2));
    }
    iso_file2.close();
}

void MainWindow::on_pushButton_5_clicked()
{
    QFile cd_file1("toki_f.bin");
    cd_file1.open(QIODevice::ReadOnly);
    QByteArray b1 = cd_file1.readAll();
    cd_file1.close();

    QFile iso_file1("toki_f.iso");
    iso_file1.open(QIODevice::WriteOnly|QIODevice::Truncate);
    for (int i=0;i<b1.size()/2352;i++)
    {
        iso_file1.write(b1.mid(i*2352+16,2048));
    }
    iso_file1.close();
}

void MainWindow::on_pushButton_6_clicked()
{
    QFile cd_file1("8bpp.data.pal");
    cd_file1.open(QIODevice::ReadOnly);
    QByteArray b1 = cd_file1.readAll();
    cd_file1.close();

    QFile iso_file1("8bpp.data.pal15");
    iso_file1.open(QIODevice::WriteOnly|QIODevice::Truncate);
    uint8_t color;
    for (int i=0;i<b1.size()/3;i++)
    {
        color = (((b1[i*3+1]>>3)&0x18)>>3) | ((b1[i*3+2]&0xF8)>>1);
        iso_file1.write(QByteArray(1,color));
        color = (b1[i*3+0]>>3) | (((b1[i*3+1]>>3)&0x7)<<5);
        iso_file1.write(QByteArray(1,color));
    }
    iso_file1.close();
}
