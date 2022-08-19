#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QFileDialog>
#include <QPicture>
#include <QPainter>

QList<QString> commands_rcv;
QString pack_keys;
QString unpack_keys;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    /*QFile cd_file1("toki_f.bin");
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

    return;*/

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


void unpack_graphics(char *src,char *dst, int *compressed_size = NULL,  int *decompressed_size = NULL  )
{
  uint backstream;
  uint current_bit;
  uint current_byte;
  int bits;
  int iCopyAmount;
  uint src_index = 0;
  uint dst_index = 0;

#define GET_NEXT_BIT { \
    bits--; \
    if (bits == 0) { \
      current_byte = (uint)src[src_index]; \
      if (src_index > 101) unpack_keys.append(QString("%1,").arg(src_index)); \
      src_index++; \
      bits = 8; \
    } \
    current_bit = current_byte & 1; \
    current_byte = current_byte >> 1; \
    }

  bits = 1;
  do {
      //copy data for all zero in command stream
      while( true ) {
          GET_NEXT_BIT
          if (current_bit == 1) break;
          dst[dst_index] = src[src_index];
          commands_rcv.append(QString("none fill byte %1").arg((uint8_t)dst[dst_index],0,16));
          src_index++;
          dst_index++;
        }
      GET_NEXT_BIT
      if (current_bit == 1) {
          //small backstream multiple repeat opcode (11), link 8-bit in datastream, size 2-bit in command stream, range 2 to 5
          GET_NEXT_BIT
          iCopyAmount = 2 + current_bit*2;
          GET_NEXT_BIT
          iCopyAmount += current_bit; //2 + 2-bit value, range from 2 to 5
          backstream = (int)src[src_index] & 0xff; //8-bit backstream link
          src_index++;
          if (backstream == 0) {
              backstream = 0x100;
          }
      }
      else {
          //big backstream multiple repeat opcode (10), link 12 bit in datastream, size 8 bit in datastream,
          backstream = ((int)src[src_index] & 0xffU) << 8 | ((int)src[src_index+1] & 0xffU); //12-bit backstream link ( 4 LSB ignored)
          if (backstream == 0) {
              if (compressed_size != NULL)
                compressed_size[0] = src_index;
              if (decompressed_size != NULL)
                decompressed_size[0] = dst_index+2;
              return;
          }
          backstream = backstream >> 4;
          if ((src[src_index+1] & 0xf) == 0) {
              iCopyAmount = (unsigned char)src[src_index+2] + 1;
              src_index++;
          }
          else {
              iCopyAmount = (src[src_index+1] & 0xf) + 2;
          }
          src_index += 2;
      }
      //duplicating single bit from previous stream data
      commands_rcv.append(QString("copy %1 bytes from %2").arg(iCopyAmount).arg(backstream));
      while (iCopyAmount>0) {
          dst[dst_index] = dst[dst_index-backstream];
          dst_index++;
          iCopyAmount--;
          }
      } while( true );
}

//debug point in HWRAM : 060128B2

void MainWindow::on_pushButton_unpach_graph_clicked()
{
    int compressed,uncompressed;

    //QString fileName("NBN.BIN");// = QFileDialog::getOpenFileName(this,tr("Open graph file"), "", tr("Graph file (*.*)"));
    //QString fileName("SYS_TBL.BIN");// = QFileDialog::getOpenFileName(this,tr("Open graph file"), "", tr("Graph file (*.*)"));
    QString fileName("SYS_TBL_stats_packed.data");// = QFileDialog::getOpenFileName(this,tr("Open graph file"), "", tr("Graph file (*.*)"));
    //QString fileName("SYS_TBL_stats_unpacked.data.pack");// = QFileDialog::getOpenFileName(this,tr("Open graph file"), "", tr("Graph file (*.*)"));

    QFile file_in(fileName);
    QByteArray ba;
    if (file_in.exists())
    {
        file_in.open(QIODevice::ReadOnly);
        ba = file_in.readAll();
    }
    char * input = (char*)malloc(ba.size());
    for (int i=0;i<ba.size();i++)
        input[i] = ba[i];
    char * output = (char*)malloc(1005000);
    memset(output,0,1005000);

    //from logo.bin
    //unpack_graphics(input+0x408,output,0);
    //unpack_graphics(input+0x93E,output,0);
    //unpack_graphics(input+0x10E9,output,0);
    //unpack_graphics(input+0x243BC,output,0);
    //unpack_graphics(input+0x25E73,output,0);
    //unpack_graphics(input+0x27BEF,output,0);


    //from nbn.bin
    //unpack_graphics(input+0x2DA800+0x20,output); //photo x128, mio
    //unpack_graphics(input+0x2DA800+0x1E08,output); //photo x128, yuuina
    //unpack_graphics(input+0x2DA800+0x3384,output); //photo x128, ayako
    //unpack_graphics(input+0x2DA800+0x4DF8,output); //photo x128, asahina?
    //unpack_graphics(input+0x2DA800+0x6894,output); //photo x128, yuuko
    //unpack_graphics(input+0x2DA800+0x7D98,output); //photo x128, nozomi
    //unpack_graphics(input+0x2DA800+0x93E4,output); //photo x128, mira
    //unpack_graphics(input+0x2DA800+0xAB14,output); //photo x128, ?
    //unpack_graphics(input+0x2DA800+0xC600,output); //photo x128, megumi
    //unpack_graphics(input+0x2DA800+0xDBAC,output); //photo x128, yumi?
    //unpack_graphics(input+0x2DA800+0xF688,output); //photo x128, miharu
    //unpack_graphics(input+0x2DA800+0x11340,output); //photo x128, ?
    //unpack_graphics(input+0x2DA800+0x125A8,output); //photo x128, ?
    //unpack_graphics(input+0x2DA800+0x14140,output);  //tiled grass with tree shadow 8bpp x320
    //unpack_graphics(input+0x2DA800+0x18E39,output); //tiled grass with tree shadow 8bpp x320, part 2
    //unpack_graphics(input+0x2DA800+0x1E058,output); //tiled grass with tree shadow 8bpp x320, part 3
    //unpack_graphics(input+0x2DA800+0x21E98,output); //shiori parts, x64,...
    //unpack_graphics(input+0x2DA800+0x23164,output); //non-tiled game logo parts, x128, x32
    //unpack_graphics(input+0x2DA800+0x241F4,output); //shiori parts, x64,...
    //unpack_graphics(input+0x2DA800+0x252A4,output); //non-tiled game logo parts, x128, x16
    //unpack_graphics(input+0x2DA800+0x261E4,output); //shiori parts, x64,...
    //unpack_graphics(input+0x2DA800+0x27378,output); //game logo parts, x16
    //unpack_graphics(input+0x2DA800+0x28400,output); //shiori parts, x64,...
    //unpack_graphics(input+0x2DA800+0x28EFC,output); //logo screen letters, x16


    //unpack_graphics(input+0x2C800+0x2932,output,0); //tiled uniform, x320
    //unpack_graphics(input+0x2C800+0x550E,output,0); //tiled uniform, x320
    //unpack_graphics(input+0x2C800+0x180,output,0); //some tile symbols

    //from bs.bin
    //unpack_graphics(input+0x27DF14+0xAF4,output,0); //??

    //from sys_tbl.bin
    //unpack_graphics(input-0x86000+0x97884,output,0); //x8, x16, pictogramms for main screen and affections
    //unpack_graphics(input-0x86000+0x98494,output); //4bpp, x8, x32, frame and stats
    //unpack_graphics(input-0x86000+0x9F64C,output,0); //tiled 8bpp, ??
    //unpack_graphics(input-0x86000+0xA0DE8,output,0); //tiled 8bpp, ??
    //unpack_graphics(input-0x86000+0xA2548,output,0); //single tile ??
    //unpack_graphics(input-0x86000+0xA34E8,output,0); //tiled 4bpp, 16x16, some symbols

    unpack_graphics(input,output,&compressed,&uncompressed); //

    ui->label_status->setText(QString("Uncompress OK, in size %1, out size %2").arg(compressed).arg(uncompressed));

    QFile file_out(fileName.append(".data"));
    file_out.open(QIODevice::WriteOnly);
    file_out.write(output,uncompressed);
    file_out.close();

    //draw tiled
    int CELL_Y=40;
    int CELL_X=6;
    QPicture pic;
    QPainter pai;
    pai.begin(&pic);
    pai.setPen(Qt::black);
    pai.fillRect(0,0,CELL_X*8,CELL_Y*8,QBrush(Qt::black));
    pai.setPen(Qt::white);
    for (int cell_y=0;cell_y<CELL_Y;cell_y++)
    {
        for (int cell_x=0;cell_x<CELL_X;cell_x++)
        {
            //mappint cell to map
            for (int y=0;y<8;y++)
            {
                for (int x=0;x<8;x++)
                {
                    //4bpp
                    int index = (cell_y*CELL_X+cell_x)*32+y*4+x/2;
                    int data;
                    if (x%2 == 1)
                        data = (((uint8_t)output[index])&0xF)*0x10;
                    else
                        data = (((uint8_t)output[index])&0xF0);
                    pai.setPen(QColor(data,data,data,255));
                    pai.drawPoint(cell_x*8+x,cell_y*8+y);
                    //8bpp
                    /*int index = (cell_y*CELL_X+cell_x)*64+y*8+x;
                    pai.setPen(QColor(output[index],output[index],output[index],255));
                    pai.drawPoint(cell_x*8+x,cell_y*8+y);*/
                }
            }
        }
    }
    pai.end();

    ui->label_graph->setPicture(pic);

    //draw untiled
    int SIZE_Y=256;
    int SIZE_X=32;
    QPicture pic2;
    QPainter pai2;
    pai2.begin(&pic2);
    pai2.setPen(Qt::black);
    pai2.fillRect(0,0,SIZE_X,SIZE_Y,QBrush(Qt::black));
    pai2.setPen(Qt::white);
    for (int y=0;y<SIZE_Y;y++)
    {
        for (int x=0;x<SIZE_X;x++)
        {
            //4bpp
            int index = y*SIZE_X/2+x/2;
            int data;
            if (x%2 == 1)
                data = (((uint8_t)output[index])&0xF)*0x10;
            else
                data = (((uint8_t)output[index])&0xF0);
            pai2.setPen(QColor(data,data,data,255));
            pai2.drawPoint(x,y);
            //8bpp
            /*int index = y*SIZE_X+x;
            uint8_t color = (uint8_t)output[index];
            pai2.setPen(QColor(color,color,color,255));
            pai2.drawPoint(x,y);*/
        }
    }
    pai2.end();


    ui->label_graph2->setPicture(pic2);


}

void MainWindow::on_pushButton_pack_clicked()
{
    int compressed,uncompressed;
    //trying to pack in compatible format
    //String fileName("SYS_TBL_stats_unpacked.data");
    QString fileName("4bpp.data");
    QFile file_in(fileName);
    QByteArray ba;
    QList<QString> commands;
    QList<int> commands_type;
    QList<int> commands_value;
    QList<int> commands_link;
    if (file_in.exists())
    {
        file_in.open(QIODevice::ReadOnly);
        ba = file_in.readAll();
    }

    //ba = ba.left(1490);

    pack_keys.clear();
    unpack_keys.clear();
    uncompressed = ba.size();
    char * input = (char*)malloc(ba.size());
    for (int i=0;i<ba.size();i++)
        input[i] = ba[i];
    char * output = (char*)malloc(1005000);
    memset(output,0,1005000);
    compressed=0;
    QByteArray backbuffer_small;
    QByteArray backbuffer_big;
    QByteArray compr;
    QByteArray command_stream_buf;
    int index = 0;
    int searchsize = 1;
    int repeats = 0;
    int command_stream_value = 0;
    int command_stream_index = 0;
    int command_stream_reserve_index = 0;
    bool bMovingOn;
    int use_repeats;
    uint8_t buf8[32];
    uint16_t * buf16 = (uint16_t *)buf8;
    compr.append(QByteArray(1,0)); //first byte of command index
    while (index < ba.size())
    {
        //detecting how many repeats of the previous byte we have
        repeats = 1;
        bool end_of_repeat = false;
        while (false == end_of_repeat)
        {
            if ( ( (index+repeats) < ba.size()) && (index > 0) )
            {
                if (ba[index+repeats-1] == ba[index-1])
                    repeats++;
                else
                    end_of_repeat=true;
            }
            else
                end_of_repeat=true;
        }

        //choosing the buffer to use
        use_repeats = 0;
        if (repeats > 5)
        {
            if (backbuffer_big.contains(ba[index]))
            {
                use_repeats = 2;//use big
                if (repeats > 250) repeats = 250; //big is limited with 1-byte size
            }
        }
        else if (repeats > 2)
        {
            if (backbuffer_small.contains(ba[index]))
                use_repeats = 1;//use small
        }

        //checking if current search sample exists in backbuffer
        bMovingOn = false;

        //repeats go first
        if ( use_repeats == 1)
        {
            //use small buffer
            repeats--;
            commands.append(QString("small copy %1 bytes from %2").arg(repeats).arg(1));
            commands_type.append(1);
            commands_value.append(repeats);
            commands_link.append(1);
            index += repeats;
            searchsize = 1;
            bMovingOn = true;
            if (index < 4000)
                backbuffer_big = ba.left(index);
            else
                backbuffer_big = ba.mid(index-4000,4000);
            if (index < 256)
                backbuffer_small = ba.left(index);
            else
                backbuffer_small = ba.mid(index-256,256);
        }

        if ( (false==bMovingOn) && (use_repeats == 2) )
        {
            //use big buffer
            //now writing to data stream, link first, amount second
            repeats --;
            commands.append(QString("big copy %1 bytes from %2").arg(repeats).arg(1));
            commands_type.append(2);
            commands_value.append(repeats);
            commands_link.append(1*16);
            index += repeats;
            searchsize = 1;
            bMovingOn = true;
            if (index < 4000)
                backbuffer_big = ba.left(index);
            else
                backbuffer_big = ba.mid(index-4000,4000);
            if (index < 256)
                backbuffer_small = ba.left(index);
            else
                backbuffer_small = ba.mid(index-256,256);
        }

        //big buffer check
        if ( (false==bMovingOn) && backbuffer_big.contains(ba.mid(index,searchsize)))
        {
            if (index+6 >= ba.length())
            {
                //not enough bytes at the end, don't use big
            }
            else if (false == backbuffer_big.contains(ba.mid(index,6)) )
            {
                //is this 5 or less? too small, don't use big
            }
            else
            {
               //using big all right, but right now?
               //is this the max?
               if (backbuffer_big.contains(ba.mid(index,searchsize+1)) && (index+searchsize < ba.size()) )
               {
                    //not max for big buffer yet, moving on
                    searchsize++;
                    bMovingOn = true;
               }
               else
               {
                    //max big buffer
                    //bigbuffer entries are 24 bit wide, so our sample must be at least 4 to add
                    //if (searchsize >= 4)
                    {
                        //now writing to data stream, link first, amount second
                        buf16[0] = (backbuffer_big.size()-backbuffer_big.indexOf(ba.mid(index,searchsize)))*16;
                        compr.append(QByteArray(1,buf8[0]));
                        compr.append(QByteArray(1,buf8[1]));
                        commands.append(QString("big copy %1 bytes from %2").arg(searchsize).arg(buf16[0]/16));
                        commands_type.append(2);
                        commands_value.append(searchsize);
                        commands_link.append(buf16[0]);
                        //updating things
                        index += searchsize;
                        searchsize = 1;
                        if (index < 4000)
                            backbuffer_big = ba.left(index);
                        else
                            backbuffer_big = ba.mid(index-4000,4000);
                        if (index < 256)
                            backbuffer_small = ba.left(index);
                        else
                            backbuffer_small = ba.mid(index-256,256);
                        bMovingOn = true;
                    }
                }
            }
        }

        //small buffer check
        if ( (false==bMovingOn) && (backbuffer_small.contains(ba.mid(index,searchsize))) )
        {
            //is this the max?
            if ((backbuffer_small.contains(ba.mid(index,searchsize+1))) && (searchsize < 5) && (index+searchsize < ba.length()))
            {
                //not max for small buffer, moving on
                searchsize++;
                bMovingOn = true;
            }
            else
            {
                //max small buffer
                //smallbuffer entries are 10 bit wide, so our sample must be at least 2 to add
                if ( (searchsize >= 2) && (searchsize <= 5) )
                {
                    //now writing to data stream, link only
                    buf8[0] = (backbuffer_small.size()-backbuffer_small.indexOf(ba.mid(index,searchsize)));
                    compr.append(QByteArray(1,buf8[0]));
                    commands.append(QString("small copy %1 bytes from %2").arg(searchsize).arg(buf8[0]));
                    commands_type.append(1);
                    commands_value.append(searchsize);
                    commands_link.append(buf8[0]);
                    //updating things
                    index += searchsize;
                    searchsize = 1;
                    if (index < 4000)
                        backbuffer_big = ba.left(index);
                    else
                        backbuffer_big = ba.mid(index-4000,4000);
                    if (index < 256)
                        backbuffer_small = ba.left(index);
                    else
                        backbuffer_small = ba.mid(index-256,256);
                    bMovingOn = true;
                }
                else
                {
                    //failed invoking small buffer, will try to invoke others
                    bMovingOn = false;
                }
            }
        }

        if (false==bMovingOn)
        {
            //everything failed, writing data as-is
            for (int i=0;i<searchsize;i++)
            {
                //writing to data stream
                commands.append(QString("none fill byte %1").arg((uint8_t)ba[index],0,16));
                commands_type.append(0);
                commands_value.append(ba[index]);
                commands_link.append(0);
                index++;
            }
            searchsize = 1;
            if (index < 4000)
                backbuffer_big = ba.left(index);
            else
                backbuffer_big = ba.mid(index-4000,4000);
            if (index < 256)
                backbuffer_small = ba.left(index);
            else
                backbuffer_small = ba.mid(index-256,256);
        }
    }
    //end sequence
    commands.append(QString("end"));
    commands_type.append(3);
    commands_value.append(0);
    commands_link.append(0);

    //trying to assemble commands from 2 streams
    compr.clear();
    command_stream_index = 0;
    command_stream_value = 0;
    command_stream_reserve_index = 0;
    command_stream_buf.clear();
    //compr.append(QByteArray(1,0)); //first command stream byte
    int curr_command=0;
    int cmd_buf = 0;
    int cmd_buf_size = 0;
    int cmds_to_pack = 0;
    while (curr_command < (commands_type.size()-1))
    {
        //packing command bits into current byte
        while ( (cmd_buf_size<8) && ((curr_command+cmds_to_pack) < commands_type.size()) )
        {
            switch (commands_type[curr_command+cmds_to_pack])
            {
            case 0:
                cmd_buf_size++;
                break;
            case 1:
                //short
                cmd_buf = cmd_buf | (0x3 << cmd_buf_size);
                cmd_buf_size+=2;
                if (commands_value[curr_command+cmds_to_pack] > 3)
                    cmd_buf |= 1 << cmd_buf_size;
                cmd_buf_size++;
                if (commands_value[curr_command+cmds_to_pack] % 2)
                    cmd_buf |= 1 << cmd_buf_size;
                cmd_buf_size++;
                break;
            case 2:
                //long
                cmd_buf = cmd_buf | (0x1 << cmd_buf_size);
                cmd_buf_size+=2;
                break;
            case 3:
                //stop = long
                cmd_buf = cmd_buf | (0x1 << cmd_buf_size);
                cmd_buf_size+=2;
                break;
            }
            cmds_to_pack++;
        }

        //if last command didn't fit, kkep it removed
        if (cmd_buf_size > 8)
            cmds_to_pack--;

        //writing the command byte
        if (compr.size() > 101)
            pack_keys.append(QString("%1,").arg(compr.size()));
        compr.append(QByteArray(1,(cmd_buf&0xFF)));
        cmd_buf = cmd_buf >> 8;
        cmd_buf_size -= 8;

        //writing databytes
        for (int i=curr_command; (i<curr_command+cmds_to_pack) && (i<commands_type.size());i++)
        {
            switch (commands_type[i])
            {
            case 0:
                compr.append(QByteArray(1,commands_value[i]));
                break;
            case 1:
                //short
                compr.append(QByteArray(1,commands_link[i]));
                break;
            case 2:
                //long
                if (commands_value[i] < 18)
                {
                    compr.append(QByteArray(1,commands_link[i]/0x100));
                    compr.append(QByteArray(1,commands_link[i]%0x100 | (commands_value[i]-2)));
                }
                else
                {
                    compr.append(QByteArray(1,commands_link[i]/0x100));
                    compr.append(QByteArray(1,commands_link[i]%0x100));
                    compr.append(QByteArray(1,commands_value[i]-1));
                }
                break;
            case 3:
                //stop
                compr.append(QByteArray(2,0));
                break;
            }
        }

        curr_command+= cmds_to_pack;
        if (cmd_buf_size > 0)
            cmds_to_pack = 1;
        else
            cmds_to_pack = 0;
    }

    compressed = compr.size();

    //uncompress test
    uint8_t test_in[20000];
    uint8_t test_out[20000];
    memset(test_in,0,20000);
    memset(test_out,0,20000);
    for (int i=0;i<compr.size();i++)
        test_in[i] = compr[i];

    commands_rcv.clear();
    unpack_graphics((char*)test_in,(char*)test_out);
    //verify
    QByteArray test_out_b;
    int errors=0;
    for (int i=0;i<ba.size();i++)
    {
        test_out_b.append(test_out[i]);
        if (test_out_b[i] != ba[i])
            errors++;
    }

    if (errors > 0)
        ui->label_status->setText(QString("Compress FAIL, errors %3 in size %1, out size %2").arg(ba.size()).arg(compr.size()).arg(errors));
    else
        ui->label_status->setText(QString("Compress OK, in size %1, out size %2").arg(ba.size()).arg(compr.size()));

    QFile file_out(fileName.append(".pack"));
    file_out.open(QIODevice::WriteOnly);
    file_out.write(compr);
    file_out.close();
}
