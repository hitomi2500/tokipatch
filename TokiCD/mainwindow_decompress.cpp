#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QFileDialog>
#include <QPicture>
#include <QPainter>


void MainWindow::update_buffers(int index, QByteArray ba)
{
    if (index < 4090)
        backbuffer_big = ba.left(index);
    else
        backbuffer_big = ba.mid(index-4090,4090);
    if (index < 256)
        backbuffer_small = ba.left(index);
    else
        backbuffer_small = ba.mid(index-256,256);
}



#define GET_NEXT_BIT { \
    bits--; \
    if (bits == 0) { \
      current_byte = (uint)src[src_index]; \
      if (src_index > 751) unpack_keys.append(QString("%1,").arg(src_index)); \
      src_index++; \
      bits = 8; \
    } \
    current_bit = current_byte & 1; \
    current_byte = current_byte >> 1; \
    }

void MainWindow::unpack_graphics(char *src,char *dst, int *compressed_size,  int *decompressed_size )
{
  uint backstream;
  uint current_bit;
  uint current_byte;
  int bits;
  int iCopyAmount;
  uint32_t src_index = 0;
  uint32_t dst_index = 0;

  bits = 1;
  do {
      //copy data for all zero in command stream
      while( true ) {
          GET_NEXT_BIT
          if (current_bit == 1) break;
          dst[dst_index] = src[src_index];
          commands_unpack.append(QString("none fill byte %1").arg((uint8_t)dst[dst_index],0,16));
          src_index++;
          dst_index++;
        }
      GET_NEXT_BIT
      if (current_bit == 1) {
          //small backstream repeat opcode (11), link 8-bit in datastream, size 2-bit in command stream, range 2 to 5
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
          //big backstream repeat opcode (10), link 12 bit in datastream, size 8 bit in datastream,
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
      commands_unpack.append(QString("copy %1 bytes from %2").arg(iCopyAmount).arg(backstream));
      while (iCopyAmount>0) {
          dst[dst_index] = dst[dst_index-backstream];
          dst_index++;
          iCopyAmount--;
          }
      if (dst_index > PACK_SIZES_START)
          unpack_sizes.append(QString("%1,").arg(dst_index));
      } while( true );
}

void MainWindow::decompress_from_file(QString filename, int offset, bool tiled, int size_x, int size_y, int bpp, int palette_offset)
{
    int compressed,uncompressed;
    QFile file_in(filename);
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

    commands_unpack.clear();
    unpack_graphics(input+offset,output,&compressed,&uncompressed); //ISO 0x8B494 4bpp, x8, x32, frame and stats

    ui->label_status->setText(QString("Uncompress DONE, in size %1, out size %2").arg(compressed).arg(uncompressed));

    QFile file_out(filename.append(".data"));
    file_out.open(QIODevice::WriteOnly);
    file_out.write(output,uncompressed);
    file_out.close();

    QPicture pic;
    QPainter pai;
    int index;
    int data;
    uint8_t color;

    //loading palette
    QColor palette[256];
    if (palette_offset > 0)
    {
        for (int i=0;i<256;i++)
        {
            uint16_t c16 = ((uint16_t)(uint8_t)input[palette_offset+i*2])<<8 | (uint16_t)(uint8_t)input[palette_offset+i*2+1];
            palette[i].setRed( (c16&0x001F)<<3 );
            palette[i].setGreen( (c16&0x03E0)>>2 );
            palette[i].setBlue( (c16&0x7C00)>>7 );
        }
    }
    else
    {
        if (4 == bpp)
        {
            for (int i=0;i<256;i++)
            {
                palette[i].setRed(0);
                palette[i].setGreen(0);
                palette[i].setBlue(0);
            }
            for (int i=0;i<16;i++)
            {
                palette[i].setRed(i*16);
                palette[i].setGreen(i*16);
                palette[i].setBlue(i*16);
            }
        }
        if (8 == bpp)
        {
            for (int i=0;i<256;i++)
            {
                palette[i].setRed(i);
                palette[i].setGreen(i);
                palette[i].setBlue(i);
            }
        }
    }

    if (true == tiled)
    {
        //draw tiled
        pai.begin(&pic);
        pai.setPen(Qt::black);
        pai.fillRect(0,0,size_x*8,size_y*8,QBrush(Qt::black));
        pai.setPen(Qt::white);
        for (int cell_y=0;cell_y<size_y;cell_y++)
        {
            for (int cell_x=0;cell_x<size_x;cell_x++)
            {
                //mappint cell to map
                for (int y=0;y<8;y++)
                {
                    for (int x=0;x<8;x++)
                    {
                        if (4 == bpp)
                        {
                            //4bpp
                            index = (cell_y*size_x+cell_x)*32+y*4+x/2;
                            if (x%2 == 1)
                                pai.setPen(palette[((uint8_t)output[index])&0xF]);
                            else
                                pai.setPen(palette[(((uint8_t)output[index])&0xF0)>>4]);
                            pai.drawPoint(cell_x*8+x,cell_y*8+y);
                        }
                        if (8 == bpp)
                        {
                            //8bpp
                            int index = (cell_y*size_x+cell_x)*64+y*8+x;
                            pai.setPen(palette[(uint8_t)output[index]]);
                            pai.drawPoint(cell_x*8+x,cell_y*8+y);
                        }
                    }
                }
            }
        }
        pai.end();

        ui->label_graph->setPicture(pic);
    }
    else
    {
        //draw untiled
        pai.begin(&pic);
        pai.setPen(Qt::black);
        pai.fillRect(0,0,size_x,size_y,QBrush(Qt::black));
        pai.setPen(Qt::white);
        for (int y=0;y<size_y;y++)
        {
            for (int x=0;x<size_x;x++)
            {
                if (4 == bpp)
                {
                    //4bpp
                    index = y*size_x/2+x/2;
                    if (x%2 == 1)
                        pai.setPen(palette[((uint8_t)output[index])&0xF]);
                    else
                        pai.setPen(palette[(((uint8_t)output[index])&0xF0)>>4]);
                    pai.drawPoint(x,y);
                }
                if (8 == bpp)
                {
                    //8bpp
                    index = y*size_x+x;
                    pai.setPen(palette[(uint8_t)output[index]]);
                    pai.drawPoint(x,y);
                }
            }
        }
        pai.end();

        ui->label_graph->setPicture(pic);
    }
}

//debug point in HWRAM : 060128B2

void MainWindow::on_pushButton_unpach_graph_clicked()
{
    //from logo.bin
    //decompress_from_file("LOGO.BIN",0x408,true,40,13,8,0); //konami logo part 1
    //decompress_from_file("LOGO.BIN",0x93E,true,40,13,8,0); //konami logo part 2
    //decompress_from_file("LOGO.BIN",0x10E9,true,40,5,8,0); //konami logo part 3
    //decompress_from_file("LOGO.BIN",0x243BC,true,40,13,8,0); //?
    //decompress_from_file("LOGO.BIN",0x25E73,true,40,13,8,0); //?
    //decompress_from_file("LOGO.BIN",0x27BEF,true,40,5,8,0); //?

    //static int shift = 0x1F498;
    //shift+=0x100;

    //from nbn.bin
    //decompress_from_file("NBN.BIN",0x2DA800+0x20,false,128,96,8,0x2DA800+0x20298); //photo x128, mio, ISO 0x1A1F820, pal  0x1A3FB98
    //decompress_from_file("NBN.BIN",0x2DA800+0x1E08,false,128,96,8,0x2DA800+0x1FE98); //photo x128, yuuina
    //decompress_from_file("NBN.BIN",0x2DA800+0x3384,false,128,96,8,0x2DA800+0x20298); //photo x128, ayako
    //decompress_from_file("NBN.BIN",0x2DA800+0x4DF8,false,128,96,8,0x2DA800+0x20698); //photo x128, saki
    //decompress_from_file("NBN.BIN",0x2DA800+0x6894,false,128,96,8,0x2DA800+0x20498); //photo x128, yuuko
    //decompress_from_file("NBN.BIN",0x2DA800+0x7D98,false,128,96,8,0x2DA800+0x20498); //photo x128, nozomi
    //decompress_from_file("NBN.BIN",0x2DA800+0x93E4,false,128,96,8,0x2DA800+0x20098); //photo x128, mira
    //decompress_from_file("NBN.BIN",0x2DA800+0xAB14,false,128,96,8,0x2DA800+0x1FE98); //photo x128, asahina
    //decompress_from_file("NBN.BIN",0x2DA800+0xC600,false,128,96,8,0x2DA800+0x20698); //photo x128, megumi
    //decompress_from_file("NBN.BIN",0x2DA800+0xDBAC,false,128,96,8,0x2DA800+0x20898); //photo x128, yumi
    //decompress_from_file("NBN.BIN",0x2DA800+0xF688,false,128,96,8,0x2DA800+0x20898); //photo x128, miharu
    //decompress_from_file("NBN.BIN",0x2DA800+0x11340,false,128,96,8,0x2DA800+0x20098); //photo x128, rei
    //decompress_from_file("NBN.BIN",0x2DA800+0x125A8,false,128,96,8,0x2DA800+0x20A98); //photo x128, yoshio
    //decompress_from_file("NBN.BIN",0x2DA800+0x14140,true,40,13,8,0);//tiled grass with tree shadow, part 1
    //decompress_from_file("NBN.BIN",0x2DA800+0x18E39,true,40,13,8,0);//tiled grass with tree shadow, part 2
    //decompress_from_file("NBN.BIN",0x2DA800+0x1E058,true,40,5,8,0);//tiled grass with tree shadow, part 3
    //decompress_from_file("NBN.BIN",0x2DA800+0x21E98,false,64,256,8,0);//shiori parts
    //decompress_from_file("NBN.BIN",0x2DA800+0x23164,false,128,128,8,0);//non-tiled game logo parts
    //decompress_from_file("NBN.BIN",0x2DA800+0x241F4,false,64,256,8,0);//shiori parts
    //decompress_from_file("NBN.BIN",0x2DA800+0x252A4,false,128,128,8,0);//non-tiled game logo parts
    //decompress_from_file("NBN.BIN",0x2DA800+0x261E4,false,64,256,8,0);//shiori parts
    //decompress_from_file("NBN.BIN",0x2DA800+0x27378,false,16,512,8,0);//logo screen letters
    //decompress_from_file("NBN.BIN",0x2DA800+0x28400,false,64,256,8,0);//shiori parts
    //decompress_from_file("NBN.BIN",0x2DA800+0x28EFC,false,16,512,8,0);//logo screen letters

    //decompress_from_file("NBN.BIN",0x2C800+0x180,true,2,64,8,0);// name ent some tile symbols for
    decompress_from_file("NBN.BIN",0x2C800+0x180,false,16,512,4,0);// name ent some tile symbols for
    //decompress_from_file("NBN.BIN",0x2C800+0xAF4,true,40,13,8,0);//name ent background shiori, part 1
    //decompress_from_file("NBN.BIN",0x2C800+0x2932,true,40,13,8,0);//name ent background shiori, part 2
    //decompress_from_file("NBN.BIN",0x2C800+0x550E,true,40,5,8,0);//name ent background shiori, part 3



    //from bs.bin
    //decompress_from_file("BS.BIN",0x27DF14+0xAF4,true,40,40,8,0);//zeros
    //decompress_from_file("BS.BIN",0x2C800+0xAF4,true,40,40,8,0);//?
    //decompress_from_file("BS.BIN",0x8C2000+0xE8208,true,40,13,8,0);// name ent backgound part 1,  ISO 0xAA1208
    //decompress_from_file("BS.BIN",0x8C2000+0xEAA3A,true,40,13,8,0);// name ent backgound part 2,  ISO 0xAA3A3A
    //decompress_from_file("BS.BIN",0x8C2000+0xEE7EA,true,40,5,8,0);// name ent backgound part 3,  ISO 0xAA77EA

    //from sys_tbl.bin
    //decompress_from_file("SYS_TBL.BIN",-0x86000+0x97884,false,16,512,4,0);//pictogramms for main screen and affections ISO 0x8A884
    //decompress_from_file("SYS_TBL.BIN",-0x86000+0x98494,false,32,256,4,0); //4bpp, x8, x32, frame and stats ISO 0x8B494
    //decompress_from_file("SYS_TBL.BIN",-0x86000+0x9F64C,true,32,16,8,0); //?
    //decompress_from_file("SYS_TBL.BIN",-0x86000+0xA0DE8,true,32,16,8,0); //?
    //decompress_from_file("SYS_TBL.BIN",-0x86000+0xA2548,true,32,16,8,0); //zeros except 1 tile?
    //decompress_from_file("SYS_TBL.BIN",-0x86000+0xA34E8,true,16,16,4,0); //some symbols


    //from name_ent.bin



}
