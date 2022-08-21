#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QFileDialog>
#include <QPicture>
#include <QPainter>

void MainWindow::on_pushButton_clicked()
{
    QFile cd_file("toki1.iso");
    cd_file.open(QIODevice::ReadOnly);
    QByteArray b = cd_file.readAll();
    cd_file.close();

    int index = 0xA000;

    bool bFinished = false;
    iso_directory_record _rec;
    char * p8 = (char *) &_rec;
    unsigned int _u32;
    char * pp8 = (char *) &_u32;
    while (false == bFinished)
    {
        int len = b.at(index);
        _rec.length = b.at(index);
        pp8[0] = b.at(index+2);
        pp8[1] = b.at(index+3);
        pp8[2] = b.at(index+4);
        pp8[3] = b.at(index+5);
        _rec.lba = _u32;
        pp8[0] = b.at(index+10);
        pp8[1] = b.at(index+11);
        pp8[2] = b.at(index+12);
        pp8[3] = b.at(index+13);
        _rec.size = _u32;
        _rec.flags = b.at(index+25);
        _rec.file_unit_size = b.at(index+26);
        pp8[0] = b.at(index+28);
        pp8[1] = b.at(index+29);
        pp8[2] = 0;//b.at(index+30);
        pp8[3] = 0;//b.at(index+31);
        _rec.volume_sequence_number = _u32;
        _rec.name_len = b.at(index+32);
        for (int i=0;i<_rec.name_len;i++)
            _rec.name[i] = b.at(index + 33 + i);
        _rec.name[_rec.name_len] = 0;
        filelist.append(_rec);
        index += len;
        while  ((b.at(index) == 0) && (index <= 0xC000))
           index++;
        if (index >= 0xC000)
             bFinished = true;
    }

    QByteArray clusters;
    QList <QByteArray> filenames;
    clusters.fill(0,500000);
    int iUsed = 0;
    for (int i=0;i<filelist.size();i++)
    {
        _rec = filelist.at(i);
        for (unsigned int j=_rec.lba; j<_rec.lba + (((_rec.size-1)/2048)+1); j++)
        {
            clusters[j] = i;
            iUsed++;
        }
        filenames.append(filelist[i].name);
    }

    while (clusters.at(clusters.size()-1)  == 0)
    {
        clusters.chop(1);
    }
    int iTotal = clusters.size();

    QFile iso_file("toki.clusters");
    iso_file.open(QIODevice::WriteOnly|QIODevice::Truncate);
    iso_file.write(clusters);
    iso_file.close();

    QFile iso_file3("toki.files");
    iso_file3.open(QIODevice::WriteOnly|QIODevice::Truncate);
    for (int i=0;i<filenames.size();i++)
    {
        iso_file3.write(QString("%1: %2 \r\n").arg(i).arg(QString(filenames[i])).toLatin1());
    }
    iso_file3.close();
}
