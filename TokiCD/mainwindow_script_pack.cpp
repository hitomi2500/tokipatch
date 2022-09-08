#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QFileDialog>
#include <QPicture>
#include <QPainter>

void MainWindow::on_pushButton_update_name_ent_from_clicked()
{
    int i;

     QList<QByteArray> english_chunks;
     QList<int> offsets;
     int iLines;

     QFile _in_file(QString("NAME_ENT.EXN.vanilla"));
     QString fileName_save = QFileDialog::getOpenFileName(this, tr("Open Script for NAME_ENT"), "", tr("All Files (*.*)"));
     QFile _in_file_eng(fileName_save);
     iLines = 43;

     _in_file.open(QIODevice::ReadOnly);
     QByteArray _in_data = _in_file.readAll();
     _in_file.close();

     _in_file_eng.open(QIODevice::ReadOnly);
     for (int i=0;i<iLines;i++)
     {
         english_chunks.append(_in_file_eng.readLine(1024));
         //replace spaces with 0x02
         english_chunks[i].replace(' ',(char)0x02);
         english_chunks[i].chop(2);
         //scan through line, if current 0x02 is too far from previous, make it 0x20.
         int iPrevSpace=0;
         int iPrevDot=0;
         while (english_chunks[i].mid(iPrevDot).contains((char)0x02))
         {
             int in = english_chunks[i].indexOf(0x02,iPrevDot);
             if ( (in -  iPrevSpace )> 20)
             {
                 english_chunks[i][in] = ' ';
                 //if it's even, add another 0x20
                 if (in%2 != 0)
                 {
                     english_chunks[i][in] = 0x02;
                     english_chunks[i].insert(in,(char)' ');
                 }
                 iPrevSpace = in;
             }
             iPrevDot = in+1;
         }
     }
     _in_file_eng.close();

     //patching data
     int iTextIndex = 0x10AD0;
     int iPointersIndex = 0x11258;
     for (int ichunk = 0; ichunk < iLines; ichunk++)
     {
         if (iTextIndex+english_chunks.at(ichunk).size() > iPointersIndex)
         {
             if (iTextIndex < iPointersIndex)
                 _in_data.append((QByteArray(300,0)));//ram to be used by name.ent
             iTextIndex = iPointersIndex+1;
             while(_in_data.size() % 4)
             {
                 _in_data.append(QByteArray(1,0));
             }
             _in_data[iPointersIndex+ichunk*4+2] = (_in_data.size()-0x10000)/0x100;
             _in_data[iPointersIndex+ichunk*4+3] = (_in_data.size()-0x10000)%0x100;
             _in_data.append(english_chunks.at(ichunk));
             _in_data.append(QByteArray(1,0));//at least one zero at the end
             while(_in_data.size() % 4)
             {
                 _in_data.append(QByteArray(1,0));
             }
         }
         else
         {
             _in_data[iPointersIndex+ichunk*4+2] = (iTextIndex-0x10000)/0x100;
             _in_data[iPointersIndex+ichunk*4+3] = (iTextIndex-0x10000)%0x100;
             _in_data.replace(iTextIndex,english_chunks.at(ichunk).size(),english_chunks.at(ichunk));
             iTextIndex+=english_chunks.at(ichunk).size();
             //at least one zero at the end
             _in_data[iTextIndex] = 0;
             iTextIndex++;
             while(iTextIndex % 4)
             {
                 _in_data[iTextIndex] = 0;
                 iTextIndex++;
             }
         }
     }

     //saving result
     //_in_data = _in_data.mid(0x10AD0);

     QFile _out_file(QString("NAME_ENT.EXN"));
     _out_file.open(QIODevice::WriteOnly);
     _out_file.write(_in_data);
     _out_file.close();
}

