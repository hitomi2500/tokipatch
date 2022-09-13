#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QFileDialog>
#include <QPicture>
#include <QPainter>
#include <QTextCodec>
#include <QMessageBox>

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

void MainWindow::on_pushButton_script_word_bin_clicked()
{

    int i;
    /*QCoreApplication a(argc, argv);

    return a.exec();*/

    QList<QString> decoded;
    QList<QByteArray> undecoded;
    QList<int> markers;
    QList<int> offsets;
    int LinesInBlock;
    bool bPostfix;

    QTextCodec * codec = QTextCodec::codecForName("Shift-JIS");
    QTextDecoder * decoder = codec->makeDecoder();

    /*//loading Pepsiman's script
    QFile _pepsi_file(QString("tokimekiC_20220209_draft10.csv"));
    QList<QByteArray> pepsi_strings;
    _pepsi_file.open(QIODevice::ReadOnly);
    while (false == _pepsi_file.atEnd())
    //while (pepsi_strings.size()<40)
       pepsi_strings.append(_pepsi_file.readLine());
    _pepsi_file.close();
    //appending single lines together
    for (int i=0;i<pepsi_strings.size();i++)
    {
        while ((pepsi_strings[i].endsWith("\n"))
               &&(false == pepsi_strings[i].endsWith("\r\n"))
               &&(i<(pepsi_strings.size()-1)))
        {
            pepsi_strings[i].append(pepsi_strings[i+1]);
            pepsi_strings.removeAt(i+1);
        }
    }
    for (int i=0;i<pepsi_strings.size();i++)
    {
        pepsi_strings[i].replace("\r\n","justsomemagicshit");
        pepsi_strings[i].replace("\n"," ");
        pepsi_strings[i].replace("justsomemagicshit","\r\n");
    }
    printf("Pepsiman's file : %i lines\r\n",pepsi_strings.size());
    //loading sjis
    QList<QByteArray> pepsi_sjis;
    QList<QByteArray> pepsi_englishes;
    QByteArray b;
    int iEnglishesTotal = 0;
    for (int i=0;i<pepsi_strings.size();i++)
    {
        b = pepsi_strings[i].mid(pepsi_strings[i].indexOf(",")+1);
        b = b.mid(b.indexOf(",")+1);
        if (b.indexOf(",") > 0)
            b = b.left(b.indexOf(","));
        if ( (b.startsWith("\"")) && (b.endsWith("\"")) )
            b=b.mid(1,b.length()-2);
        //if ( (b[0] == 0xE3) && (b[1] == 0x80) && (b[2] == 0x8E) )
        //    b=b.mid(3);
        if ( (b[0] == (char)0x81) && (b[1] == (char)0x77))
            b=b.mid(2);
        if (b.endsWith("\r\n"))
            b=b.left(b.length()-2);
        pepsi_sjis.append(b);
        b = pepsi_strings[i].mid(pepsi_strings[i].indexOf(",")+1);
        b = b.mid(b.indexOf(",")+1);
        if (b.indexOf(",") > 0)
        {
            b = b.mid(b.indexOf(",")+1);
            if (b.indexOf(",") > 0)
                b = b.left(b.indexOf(","));
            if ( (b.startsWith("\"")) && (b.endsWith("\"")) )
                b=b.mid(1,b.length()-2);
            if (b.endsWith("\r\n"))
                b=b.left(b.length()-2);
            pepsi_englishes.append(b);
            if (b.length() > 0)
                iEnglishesTotal++;
        }
        else
        {
            pepsi_englishes.append("");
        }
    }*/

    QFile _in_file(QString("WORD.BIN"));

    _in_file.open(QIODevice::ReadOnly);
    QByteArray _in_data = _in_file.readAll();
    _in_file.close();

    //getting string list
    int iPos = 0;
    int iPos2 = 0;
    int iCnt = 0;
    //for (i=0;i<1000;i++)
    LinesInBlock = 0;
    int iChapter = 0;
    int iChapterOffset = 0;
    while (iPos < _in_data.size())
    {
        if (iPos > 0x2A3640)
        {
            volatile int brkpt=100500;
        }

        //WORD.BIN
        //skipping region from 0x1FF800 to 0x228000
        if (iPos == 0x1FF800) iPos = 0x228000;

        //checking stream
        if ( ( ((unsigned char)(_in_data.at(iPos)) >= 0x81) && ((unsigned char)(_in_data.at(iPos)) <= 0x9F) ) ||
             ( ((unsigned char)(_in_data.at(iPos)) >= 0xE0) && ((unsigned char)(_in_data.at(iPos)) <= 0xEF) ) ||
             ( (unsigned char)(_in_data.at(iPos)) == 'A' ) ||
             ( (unsigned char)(_in_data.at(iPos)) == 'F' ) ||
             ( (unsigned char)(_in_data.at(iPos)) == 'P' ) ||
             ( (unsigned char)(_in_data.at(iPos)) == 'G' ) ||
             ( (unsigned char)(_in_data.at(iPos)) == 'M' ) ||
             ( (unsigned char)(_in_data.at(iPos)) == 'm' ) ||
             ( (unsigned char)(_in_data.at(iPos)) == 'p' ) ||
             ( (unsigned char)(_in_data.at(iPos)) == 'q' ) ||
             ( (unsigned char)(_in_data.at(iPos)) == '>' ) ||
             ( (unsigned char)(_in_data.at(iPos)) == '<' ) ||
             ( (unsigned char)(_in_data.at(iPos)) == '-' ) ||
             ( (unsigned char)(_in_data.at(iPos)) == '+' ) ||
             ( (unsigned char)(_in_data.at(iPos)) == ' ' )
             )
        {
            //presumably jis
            //looking up for a next block that is not started with zeros
            iPos2 = _in_data.indexOf((char)0,iPos);
            QString str = decoder->toUnicode(_in_data.mid(iPos),iPos2-iPos);
            QByteArray ba = _in_data.mid(iPos,iPos2-iPos);
            if (ba.endsWith("\r\n"))
                ba = ba.left(ba.length()-2);
            if (ba.size() > 0)
            {
                undecoded.append(ba);
                offsets.append(iPos);
            }
            if (str.length() > 0)
            {
                decoded.append(str);
            }
            iPos = iPos2;
            iPos = (((iPos-1)/4)+1)*4;
        }
        else if ( ((unsigned char)(_in_data.at(iPos)) == 0x00) && ((unsigned char)(_in_data.at(iPos+1)) >= 0x20) && ((unsigned char)(_in_data.at(iPos+1)) <= 0x28)  )
        {
            //memory address
            if (decoded.size() > markers.size())
                markers.append((unsigned char)_in_data.at(iPos)*0x1000000 + (unsigned char)_in_data.at(iPos+1)*0x10000 + (unsigned char)_in_data.at(iPos+2)*0x100 + (unsigned char)_in_data.at(iPos+3));
            iPos+=4;
        }
        else if ( (iPos == 0x75804)||(iPos == 0x8AC28)||(iPos == 0xAF8A0)||(iPos == 0xAFF44)||(iPos == 0xB0570))
        {
            //special zero string
            QByteArray ba;
            undecoded.append(ba);
            offsets.append(iPos);
            decoded.append(ba);
            iPos+=4;
        }
        else
        {
            //some non-jis data in the stream, moving on
            iPos++;
            //rounding to 4
            iPos = (((iPos-1)/4)+1)*4;
        }
    }

    while(markers.size()<decoded.size())
        markers.append(0);

    QFile _out_file(QString("WORD.TXT"));
    _out_file.open(QIODevice::WriteOnly);
    int diff;
    for (i=0;i<decoded.length();i++)
    {
        diff = offsets.at(i)-markers.at(i)+0x205000;
        if (diff != iChapterOffset)
        {
            iChapter++;
            iChapterOffset = diff;
        }
        //try translating a few first lines
        if (i<5)
        {
            //get translated line
        }
        _out_file.write(QString("%1:%2:%3:%4:%5:").arg(i).arg(iChapter).arg(offsets.at(i),8,16).arg(markers.at(i),8,16).arg(diff,8,16).toUtf8());
        _out_file.write(undecoded.at(i));
        _out_file.write("\r\n");
    }
    _out_file.close();

/*
    //cleanup equals in pepsiman's file
    for (int i=0;i<pepsi_sjis.size();i++)
    {
        for (int j=i+1;j<pepsi_sjis.size();j++)
        {
            if (pepsi_sjis[i].compare(pepsi_sjis[j])==0)
            {
                pepsi_sjis.removeAt(j);
                pepsi_strings.removeAt(j);
                pepsi_englishes.removeAt(j);
                //i=0;j=1;
                j--;
            }
        }
    }

    //simplity for matching and dump
    QFile _out_pepsi(QString("pepsi.TXT"));
    _out_pepsi.open(QIODevice::WriteOnly);
    for (int i=0;i<pepsi_sjis.size();i++)
    {
        pepsi_sjis[i] = pepsi_sjis[i].replace(" ","");
        _out_pepsi.write(pepsi_sjis[i]);
        //if (false == pepsi_sjis[i].endsWith("\r\n"))
            _out_pepsi.write("\r\n");
    }
    _out_pepsi.close();


    QFile _out_pepsi2(QString("pepsi2.TXT"));
    _out_pepsi2.open(QIODevice::WriteOnly);
    for (int i=0;i<undecoded.size();i++)
    {
        undecoded[i] = undecoded[i].replace(" ","");
        _out_pepsi2.write(undecoded[i]);
        //if (false == undecoded[i].endsWith("\r\n"))
            _out_pepsi2.write("\r\n");
    }
    _out_pepsi2.close();

    //searching matches
    QByteArray m1,m2;
    int index;
    int iMatches = 0;
    int iMatchesEnlish = 0;
    for (int i=0;i<undecoded.size();i++)
    {
        m1 = undecoded[i];
        for (int j=0;j<pepsi_sjis.size();j++)
        {
            m2 = pepsi_sjis[j];
            if (m1.compare(m2)==0)
            {
                iMatches++;
                if (pepsi_englishes[j].length() > 0)
                    iMatchesEnlish++;
            }
        }
    }

    ui->label_pepsiman_stats->setText(QString("Pepsiman's file : eng %1 , japmatch %2, japmatch with end %3").arg(iEnglishesTotal).arg(iMatches).arg(iMatchesEnlish));
    printf("Pepsiman's file : %i englishes\r\n",iEnglishesTotal);

    printf("Pepsiman's file : %i matches, %i englishes\r\n",iMatches,iMatchesEnlish);*/

}

/*
( (unsigned char)(_in_data.at(iPos)) == 'A' ) ||Alouette
( (unsigned char)(_in_data.at(iPos)) == 'F' ) ||Frederique
( (unsigned char)(_in_data.at(iPos)) == 'P' ) ||Pantagruelle
( (unsigned char)(_in_data.at(iPos)) == 'G' ) ||Gargantua
( (unsigned char)(_in_data.at(iPos)) == 'M' ) ||Mezzanine
( (unsigned char)(_in_data.at(iPos)) == 'm' ) ||Marseille
( (unsigned char)(_in_data.at(iPos)) == 'p' ) ||Patricia
( (unsigned char)(_in_data.at(iPos)) == 'q' ) ||Quincy
( (unsigned char)(_in_data.at(iPos)) == '>' ) ||
( (unsigned char)(_in_data.at(iPos)) == '<' ) ||
( (unsigned char)(_in_data.at(iPos)) == '-' ) ||
( (unsigned char)(_in_data.at(iPos)) == '+' ) ||
( (unsigned char)(_in_data.at(iPos)) == ' ' )
*/


void MainWindow::on_pushButton_script_word_bin_update_clicked()
{
    int i;

    QList<QString> decoded;
    QList<QByteArray> undecoded;
    QList<int> line_pointers;
    QList<int> line_pointers_pointers;
    QList<int> offsets;
    QList<int> subchapter_pointers;
    QList<int> subchapter_pointers_pointers;
    QList<int> chapters;
    QList<int> subchapters;
    QList<int> chapter_sizes;
    QList<int> chapter_offsets;
    QList<int> chapter_english_sizes;
    int LinesInBlock;
    int iChaptersNumber;
    int iSubchaptersNumber;
    bool bPostfix;
    QFile _out_file;

    QTextCodec * codec = QTextCodec::codecForName("Shift-JIS");
    QTextDecoder * decoder = codec->makeDecoder();

    QFile _in_file(QString("WORD.BIN"));

    _in_file.open(QIODevice::ReadOnly);
    QByteArray _in_data = _in_file.readAll();
    _in_file.close();

    //getting string list
    int iPos = 0;
    int iPos2 = 0;
    int iCnt = 0;
    //for (i=0;i<1000;i++)
    LinesInBlock = 0;
    int iChapter = 0;
    int iChapterOffset = 0;
    while (iPos < _in_data.size())
    {
        if (iPos > 0x2A3640)
        {
            volatile int brkpt=100500;
        }

        //WORD.BIN
        //skipping region from 0x1FF800 to 0x228000
        if (iPos == 0x1FF800) iPos = 0x228000;

        //checking stream
        if ( ( ((unsigned char)(_in_data.at(iPos)) >= 0x81) && ((unsigned char)(_in_data.at(iPos)) <= 0x9F) ) ||
             ( ((unsigned char)(_in_data.at(iPos)) >= 0xE0) && ((unsigned char)(_in_data.at(iPos)) <= 0xEF) ) ||
             ( (unsigned char)(_in_data.at(iPos)) == 'A' ) ||
             ( (unsigned char)(_in_data.at(iPos)) == 'F' ) ||
             ( (unsigned char)(_in_data.at(iPos)) == 'P' ) ||
             ( (unsigned char)(_in_data.at(iPos)) == 'G' ) ||
             ( (unsigned char)(_in_data.at(iPos)) == 'M' ) ||
             ( (unsigned char)(_in_data.at(iPos)) == 'm' ) ||
             ( (unsigned char)(_in_data.at(iPos)) == 'p' ) ||
             ( (unsigned char)(_in_data.at(iPos)) == 'q' ) ||
             ( (unsigned char)(_in_data.at(iPos)) == '>' ) ||
             ( (unsigned char)(_in_data.at(iPos)) == '<' ) ||
             ( (unsigned char)(_in_data.at(iPos)) == '-' ) ||
             ( (unsigned char)(_in_data.at(iPos)) == '+' ) ||
             ( (unsigned char)(_in_data.at(iPos)) == ' ' )
             )
        {
            //presumably jis
            //looking up for a next block that is not started with zeros
            iPos2 = _in_data.indexOf((char)0,iPos);
            QString str = decoder->toUnicode(_in_data.mid(iPos),iPos2-iPos);
            QByteArray ba = _in_data.mid(iPos,iPos2-iPos);
            if (ba.endsWith("\r\n"))
                ba = ba.left(ba.length()-2);
            if (ba.size() > 0)
            {
                undecoded.append(ba);
                offsets.append(iPos);
            }
            if (str.length() > 0)
            {
                decoded.append(str);
            }
            iPos = iPos2;
            iPos = (((iPos-1)/4)+1)*4;
        }
        else if ( ((unsigned char)(_in_data.at(iPos)) == 0x00) && ((unsigned char)(_in_data.at(iPos+1)) >= 0x20) && ((unsigned char)(_in_data.at(iPos+1)) <= 0x28)  )
        {
            //memory address pointers
            if (decoded.size() > line_pointers.size())
            {
                line_pointers.append((unsigned char)_in_data.at(iPos)*0x1000000 + (unsigned char)_in_data.at(iPos+1)*0x10000 + (unsigned char)_in_data.at(iPos+2)*0x100 + (unsigned char)_in_data.at(iPos+3));
                line_pointers_pointers.append(iPos);
            }
            else
            {
                //some pointer, but not line pointer
                //maybe a subchapter pointer?
                subchapter_pointers.append((unsigned char)_in_data.at(iPos)*0x1000000 + (unsigned char)_in_data.at(iPos+1)*0x10000 + (unsigned char)_in_data.at(iPos+2)*0x100 + (unsigned char)_in_data.at(iPos+3));
                subchapter_pointers_pointers.append(iPos);
            }
            iPos+=4;
        }
        else if ( (iPos == 0x75804)||(iPos == 0x8AC28)||(iPos == 0xAF8A0)||(iPos == 0xAFF44)||(iPos == 0xB0570))
        {
            //special zero string
            QByteArray ba;
            undecoded.append(ba);
            offsets.append(iPos);
            decoded.append(ba);
            iPos+=4;
        }
        else
        {
            //some non-jis data in the stream, moving on
            iPos++;
            //rounding to 4
            iPos = (((iPos-1)/4)+1)*4;
        }
    }

    if (line_pointers.size()!=decoded.size())
    {
        QMessageBox msgBox;
        msgBox.setText("line_pointers and decoded size mismatch.");
        msgBox.exec();
    }

    //detect chapters by offset change and subchapters by hole between data
    iChapter = 0;
    iChapterOffset = 0;
    iChaptersNumber = 0;
    iSubchaptersNumber = 0;
    for (i=0;i<undecoded.length();i++)
    {
        int diff = offsets.at(i)-line_pointers.at(i)+0x205000;
        if (diff != iChapterOffset)
        {
            iChapter++;
            iChapterOffset = diff;
            iSubchaptersNumber++;
        }
        else if (i> 0)
        {
            int hole = offsets.at(i)-undecoded[i-1].length()-offsets.at(i-1);
            if (hole > 4)
            {
                    iSubchaptersNumber++;
            }
        }
        chapter_offsets.append(diff);
        chapters.append(iChapter);
        subchapters.append(iSubchaptersNumber);
    }
    iChaptersNumber = iChapter+1;

    _out_file.setFileName(QString("WORD_chapters.TXT"));
    _out_file.open(QIODevice::WriteOnly);
    for (i=0;i<chapters.size();i++)
    {
        _out_file.write(QString("%1 : %2 : %3 : %4 : %5").arg(i).arg(chapters[i]).arg(subchapters[i]).arg(offsets[i],0,16).arg(line_pointers.at(i),0,16).toLatin1());
        _out_file.write("\r\n");
    }
    _out_file.close();

    _out_file.setFileName(QString("WORD_subchapters_pointers.TXT"));
    _out_file.open(QIODevice::WriteOnly);
    for (i=0;i<subchapter_pointers.size();i++)
    {
        _out_file.write(QString("%1 : %2 : %3").arg(i).arg(subchapter_pointers[i],0,16).arg(subchapter_pointers_pointers[i],0,16).toLatin1());
        _out_file.write("\r\n");
    }
    _out_file.close();

    return;

    //calculate chapter sizes
    chapter_sizes.clear();
    for (int ch=0;ch<iChaptersNumber;ch++)
    {
        chapter_sizes.append(0);
        for (i=0;i<decoded.length();i++)
        {
            if (chapters.at(i) == ch)
                chapter_sizes[ch] += undecoded[i].length();
        }
    }

    //open translated file
    QFile _in_file_e(QString("WORD_english.txt"));
    QList<QByteArray> Englishes;
    _in_file_e.open(QIODevice::ReadOnly);
    while(false == _in_file_e.atEnd())
        Englishes.append(_in_file_e.readLine());

    //replace tokens in translated file
    for (int i=0; i< Englishes.size(); i++)
    {
        Englishes[i].replace("Pantagruelle",QByteArray(1,5));
        Englishes[i].replace("Gargantua",QByteArray(1,6));
        Englishes[i].replace("Alouette",QByteArray(1,7));
        Englishes[i].replace("Frederique",QByteArray(1,8));
        Englishes[i].replace("Mezzanine",QByteArray(1,9));
        Englishes[i].replace("Marseille",QByteArray(1,10));
        Englishes[i].replace("Patricia",QByteArray(1,11));
        Englishes[i].replace("Quincy",QByteArray(1,12));
    }

    QList<QByteArray> tokens3;
    tokens3.append("you");
    tokens3.append("the");
    tokens3.append("hat");
    tokens3.append("ing");
    tokens3.append("thi");
    tokens3.append("ome");
    tokens3.append("tha");
    tokens3.append("was");
    tokens3.append("all");
    tokens3.append("hin");
    tokens3.append("her");
    tokens3.append("ell");
    tokens3.append("ood");
    tokens3.append("for");
    tokens3.append("hen");
    tokens3.append("ere");

    QList<QByteArray> tokens2;
    tokens2.append("th");
    tokens2.append("ou");
    tokens2.append("in");
    tokens2.append("ha");
    tokens2.append("he");
    tokens2.append("it");
    tokens2.append("re");
    tokens2.append("at");
    tokens2.append("er");
    tokens2.append("me");
    tokens2.append("yo");
    tokens2.append("to");
    tokens2.append("on");
    tokens2.append("ll");
    tokens2.append("an");
    tokens2.append("ng");

    tokens2.append("en");
    tokens2.append("ea");
    tokens2.append("is");
    tokens2.append("go");
    tokens2.append("hi");
    tokens2.append("as");
    tokens2.append("or");
    tokens2.append("oo");
    tokens2.append("ar");
    tokens2.append("es");
    tokens2.append("et");
    tokens2.append("wa");
    tokens2.append("al");
    tokens2.append("ti");
    tokens2.append("te");
    tokens2.append("st");

    //token-comress translated file
    for (int i=0; i< Englishes.size(); i++)
    {
        for (int j=0;j<16;j++)
            Englishes[i].replace(tokens3[j],QByteArray(1,16+j));
        for (int j=0;j<32;j++)
            Englishes[i].replace(tokens2[j],QByteArray(1,224+j));
    }

    //replace endings in english lines with zeros
    for (int i=0; i< Englishes.size(); i++)
    {
        Englishes[i].replace("\r\n",QByteArray(1,0));
    }

    int diff;
    int iFits=0;
    iChapter=0;
    for (i=0;i<decoded.length();i++)
    {
        diff = offsets.at(i)-line_pointers.at(i)+0x205000;
        if (diff != iChapterOffset)
        {
            iChapter++;
            iChapterOffset = diff;
        }
        if (Englishes[i].length() <= undecoded[i].length())
            iFits++;
    }

    int iChapterFits=0;
    //calculate english chapter sizes
    chapter_english_sizes.clear();
    for (int ch=0;ch<iChaptersNumber;ch++)
    {
        chapter_english_sizes.append(0);
        for (i=0;i<decoded.length();i++)
        {
            if (chapters.at(i) == ch)
                chapter_english_sizes[ch] += Englishes[i].length();
        }
       if (chapter_english_sizes[ch] <= chapter_sizes[ch]) iChapterFits++;
    }

    ui->label_wordbin_insert_stats->setText(QString("Size fits : %1, chapters fit: %2 out of %3").arg(iFits).arg(iChapterFits).arg(iChaptersNumber));

    //inserting english lines, chapter 1 only for now
    //for (int ch=0;ch<iChaptersNumber;ch++)
    for (int ch=0;ch<1;ch++)
    {
        //get the chapter offset in sjis
        int iChapterNode = 0;
        while (chapters[iChapterNode]!=ch)
            iChapterNode++;
        int iDataPtr = offsets[iChapterNode];
        //now do insert for each line in chapter
        while (chapters[iChapterNode] == ch)
        {
            _in_data.replace(iDataPtr,Englishes[iChapterNode].length(),Englishes[iChapterNode]);//update data
            int iOffset = iDataPtr + chapter_offsets[iChapterNode];
            char * p8 = (char*)&iOffset;
            _in_data.replace(line_pointers_pointers[iChapterNode],1,QByteArray(1,p8[3]));//update pointer
            _in_data.replace(line_pointers_pointers[iChapterNode]+1,1,QByteArray(1,p8[2]));//update pointer
            _in_data.replace(line_pointers_pointers[iChapterNode]+2,1,QByteArray(1,p8[1]));//update pointer
            _in_data.replace(line_pointers_pointers[iChapterNode]+3,1,QByteArray(1,p8[0]));//update pointer
            iDataPtr += Englishes[iChapterNode].length();
            iChapterNode++;
        }
    }

    //save results
    _out_file.setFileName(QString("WORD_dump.TXT"));
    _out_file.open(QIODevice::WriteOnly);
    for (i=0;i<decoded.length();i++)
    {
        diff = offsets.at(i)-line_pointers.at(i)+0x205000;
        _out_file.write(QString("%1:%2:%3:%4:%5:").arg(i).arg(chapters[i]).arg(offsets.at(i),8,16).arg(line_pointers.at(i),8,16).arg(diff,8,16).toUtf8());
        _out_file.write(undecoded.at(i));
        _out_file.write("\r\n");
    }
    _out_file.close();

    _out_file.setFileName(QString("WORD_chapters_match.TXT"));
    _out_file.open(QIODevice::WriteOnly);
    for (i=0;i<iChaptersNumber;i++)
    {
        if (chapter_english_sizes[i]<=chapter_sizes[i])
            _out_file.write(QString("%1 : FIT :%2:%3").arg(i).arg(chapter_english_sizes[i]).arg(chapter_sizes[i]).toLatin1());
        else
            _out_file.write(QString("%1 :ERROR:%2:%3").arg(i).arg(chapter_english_sizes[i]).arg(chapter_sizes[i]).toLatin1());
        _out_file.write("\r\n");
    }
    _out_file.close();

    _out_file.setFileName(QString("WORD_patched.BIN"));
    _out_file.open(QIODevice::WriteOnly);
    _out_file.write(_in_data);
    _out_file.close();
}


void MainWindow::on_pushButton_get_2char_stats_clicked()
{
    //open translated file
    QFile _in_file_e(QString("WORD_english.txt"));
    QList<QByteArray> Englishes;
    _in_file_e.open(QIODevice::ReadOnly);
    while(false == _in_file_e.atEnd())
        Englishes.append(_in_file_e.readLine());

    //replace tokens in translated file
    for (int i=0; i< Englishes.size(); i++)
    {
        Englishes[i].replace("Pantagruelle",QByteArray(1,5));
        Englishes[i].replace("Gargantua",QByteArray(1,6));
        Englishes[i].replace("Alouette",QByteArray(1,7));
        Englishes[i].replace("Frederique",QByteArray(1,8));
        Englishes[i].replace("Mezzanine",QByteArray(1,9));
        Englishes[i].replace("Marseille",QByteArray(1,10));
        Englishes[i].replace("Patricia",QByteArray(1,11));
        Englishes[i].replace("Quincy",QByteArray(1,12));
    }

    //now get the 2-chars frequencies
    QList<int> _2_char_sizes;
    QList<QByteArray> _2_chars;
    QByteArray test;
    int score=0;
    for (uint8_t c = 65; c<123; c++)
    {
        for (uint8_t c2 = 65;c2<123;c2++)
        {
            if ((c == 0x0a) || (c == 0x0d)  || (c2 == 0x0a)  || (c2 == 0x0d) )
                continue;
            score=0;
            test.clear();
            test.append(QByteArray(1,c));
            test.append(QByteArray(1,c2));
            for (int en=0;en<Englishes.size(); en++)
            {
                QByteArray copy = Englishes[en];
                while (copy.indexOf(test) > 0)
                {
                    score++;
                    copy = copy.mid(copy.indexOf(test)+2);
                }
            }
            if (score>0)
            {
                _2_char_sizes.append(score);
                _2_chars.append(test);
            }
        }
    }
    //sorting
    for (int i=0;i<_2_char_sizes.size();i++)
    {
        for (int j=i+1;j<_2_char_sizes.size();j++)
        {
            if (_2_char_sizes.at(j)>_2_char_sizes.at(i))
            {
                _2_char_sizes.swap(i,j);
                _2_chars.swap(i,j);
            }
        }
    }
    //saving
    QFile _out_file;
    _out_file.setFileName(QString("WORD_2chars_stat.TXT"));
    _out_file.open(QIODevice::WriteOnly);
    for (int i=0;i<_2_char_sizes.size();i++)
    {
        _out_file.write(QString("%1 : %2").arg(_2_char_sizes[i]).arg(QString::fromLatin1(_2_chars[i])).toLatin1());
        _out_file.write("\r\n");
    }
    _out_file.close();

}


void MainWindow::on_pushButton_get_3char_stats_clicked()
{
    //open translated file
    QFile _in_file_e(QString("WORD_english.txt"));
    QList<QByteArray> Englishes;
    _in_file_e.open(QIODevice::ReadOnly);
    while(false == _in_file_e.atEnd())
        Englishes.append(_in_file_e.readLine());

    //replace tokens in translated file
    for (int i=0; i< Englishes.size(); i++)
    {
        Englishes[i].replace("Pantagruelle",QByteArray(1,5));
        Englishes[i].replace("Gargantua",QByteArray(1,6));
        Englishes[i].replace("Alouette",QByteArray(1,7));
        Englishes[i].replace("Frederique",QByteArray(1,8));
        Englishes[i].replace("Mezzanine",QByteArray(1,9));
        Englishes[i].replace("Marseille",QByteArray(1,10));
        Englishes[i].replace("Patricia",QByteArray(1,11));
        Englishes[i].replace("Quincy",QByteArray(1,12));
    }

    //now get the 2-chars frequencies
    QList<int> _3_char_sizes;
    QList<QByteArray> _3_chars;
    QByteArray test;
    int score=0;
    for (uint8_t c = 65; c<123; c++)
    {
        for (uint8_t c2 = 65;c2<123;c2++)
        {
            for (uint8_t c3 = 65;c3<123;c3++)
            {
                /*if ((c == 0x0a) || (c == 0x0d)  || (c2 == 0x0a)  || (c2 == 0x0d) || (c3 == 0x0a)  || (c3 == 0x0d)  )
                    continue;*/
                score=0;
                test.clear();
                test.append(QByteArray(1,c));
                test.append(QByteArray(1,c2));
                test.append(QByteArray(1,c3));
                for (int en=0;en<Englishes.size(); en++)
                {
                    QByteArray copy = Englishes[en];
                    while (copy.indexOf(test) > 0)
                    {
                        score++;
                        copy = copy.mid(copy.indexOf(test)+3);
                    }
                }
                if (score>0)
                {
                    _3_char_sizes.append(score);
                    _3_chars.append(test);
                }
            }
        }
    }
    //sorting
    for (int i=0;i<_3_char_sizes.size();i++)
    {
        for (int j=i+1;j<_3_char_sizes.size();j++)
        {
            if (_3_char_sizes.at(j)>_3_char_sizes.at(i))
            {
                _3_char_sizes.swap(i,j);
                _3_chars.swap(i,j);
            }
        }
    }
    //saving
    QFile _out_file;
    _out_file.setFileName(QString("WORD_3chars_stat.TXT"));
    _out_file.open(QIODevice::WriteOnly);
    for (int i=0;i<_3_char_sizes.size();i++)
    {
        _out_file.write(QString("%1 : %2").arg(_3_char_sizes[i]).arg(QString::fromLatin1(_3_chars[i])).toLatin1());
        _out_file.write("\r\n");
    }
    _out_file.close();

}
