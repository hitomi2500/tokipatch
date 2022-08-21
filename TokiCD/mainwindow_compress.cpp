#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QFileDialog>
#include <QPicture>
#include <QPainter>

void MainWindow::on_pushButton_pack_clicked()
{
    int compressed,uncompressed;
    //trying to pack in compatible format
    //String fileName("SYS_TBL_stats_unpacked.data");
    QString fileName("8bpp.data");
    QFile file_in(fileName);
    QByteArray ba;
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
    pack_sizes.clear();
    unpack_sizes.clear();
    commands_pack.clear();
    uncompressed = ba.size();
    char * input = (char*)malloc(ba.size());
    for (int i=0;i<ba.size();i++)
        input[i] = ba[i];
    char * output = (char*)malloc(1005000);
    memset(output,0,1005000);
    compressed=0;
    QByteArray compr;
    QByteArray command_stream_buf;
    int index = 0;
    //int searchsize = 1;
    //int repeats = 0;
    int command_stream_value = 0;
    int command_stream_index = 0;
    int command_stream_reserve_index = 0;
    //int use_repeats;
    uint8_t buf8[32];
    uint16_t * buf16 = (uint16_t *)buf8;
    compr.append(QByteArray(1,0)); //first byte of command index
    int repeats_max;
    int repeats_max_link;
    while (index < ba.size())
    {
        //detecting how many repeats of the previous byte we have
        repeats_max = 0;
        repeats_max_link=0;
        for (int repeats_try_link = 1; repeats_try_link < 4000; repeats_try_link++)
        {
            //trying to repeat with the current link
            int repeats_try_size = 0;
                while ( (ba.mid(index+repeats_try_size,1).size() == 1) &&
                        (ba.mid(index+repeats_try_size,1) == ba.mid(index+repeats_try_size-repeats_try_link,1)) )
                    repeats_try_size++;
                if (repeats_try_size > repeats_max)
                {
                    repeats_max = repeats_try_size;
                    repeats_max_link = repeats_try_link;
                }
        }

        if (repeats_max>256)
            repeats_max = 256;

        //repeats--;

        /*if (commands_pack.size()==486)
                repeats_max_link++;*/

        //detecting gains for big and small buffer
        int big_size = 0;
        int big_link = 0;
        while ( ( backbuffer_big.contains(ba.mid(index,big_size))) && (ba.mid(index,big_size).size()==big_size) && big_size < 257 )
        {
            big_size++;
        }
        big_size--;
        big_link = (backbuffer_big.size()-backbuffer_big.lastIndexOf(ba.mid(index,big_size)));
        if (big_size < 2)
            big_size = 0; //is this 2 or less? don't use big
        if ( (big_size < 6) && (big_link < 257) )
            big_size = 0; //is this 5 or less not too far away? small can hadle this, don't use big

        int small_size = 0;
        int small_link = 0;
        while ( (backbuffer_small.contains(ba.mid(index,small_size))) && (ba.mid(index,small_size).size()==small_size)
                && (small_size < 6) )
        {
            small_size++;
        }
        small_size--;
        small_link = (backbuffer_small.size()-backbuffer_small.indexOf(ba.mid(index,small_size)));

        //repeats go first
        if ( (repeats_max > small_size) && (repeats_max > big_size) && (repeats_max > 1) )
        {
            if ( ( repeats_max <= 5) && (repeats_max_link < 257))
            {
                //use small buffer
                commands_pack.append(QString("small repeat %1 bytes from %2").arg(repeats_max).arg(repeats_max_link));
                commands_type.append(1);
                commands_value.append(repeats_max);
                commands_link.append(repeats_max_link);
                index += repeats_max;
                if (index > PACK_SIZES_START)
                    pack_sizes.append(QString("%1,").arg(index));
                update_buffers(index,ba);
            }
            else
            {
                //use big buffer
                //now writing to data stream, link first, amount second
                commands_pack.append(QString("big repeat %1 bytes from %2").arg(repeats_max).arg(repeats_max_link));
                commands_type.append(2);
                commands_value.append(repeats_max);
                commands_link.append(repeats_max_link*16);
                index += repeats_max;
                if (index > PACK_SIZES_START)
                    pack_sizes.append(QString("%1,").arg(index));
                update_buffers(index,ba);
            }
        }
        else if ( (small_size > big_size) && (small_size > 1) )
        {
            //now writing to data stream, link only
            compr.append(QByteArray(1,buf8[0]));
            commands_pack.append(QString("small copy %1 bytes from %2").arg(small_size).arg(small_link));
            commands_type.append(1);
            commands_value.append(small_size);
            commands_link.append(small_link);
            //updating things
            index += small_size;
            if (index > PACK_SIZES_START)
                pack_sizes.append(QString("%1,").arg(index));
            update_buffers(index,ba);
        }
        else if ( (big_size > 2) )
        {
            //now writing to data stream, link first, amount second
            compr.append(QByteArray(1,buf8[0]));
            compr.append(QByteArray(1,buf8[1]));
            commands_pack.append(QString("big copy %1 bytes from %2").arg(big_size).arg(big_link));
            commands_type.append(2);
            commands_value.append(big_size);
            commands_link.append(big_link*16);
            //updating things
            index += big_size;
            if (index > PACK_SIZES_START)
                pack_sizes.append(QString("%1,").arg(index));
            update_buffers(index,ba);
        }
        else
        {
            //everything failed, writing byte as-is
            //writing to data stream
            commands_pack.append(QString("none fill byte %1").arg((uint8_t)ba[index],0,16));
            commands_type.append(0);
            commands_value.append(ba[index]);
            commands_link.append(0);
            index++;
            update_buffers(index,ba);
        }
    }
    //end sequence
    commands_pack.append(QString("end"));
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
        if (compr.size() > 751)
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

    commands_unpack.clear();
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

