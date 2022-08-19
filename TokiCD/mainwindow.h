#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

  /* file flags: */
  #define CD_VISABLE  0x01 /* file name is hidden or visable to user */
  #define CD_DIRECTORY  0x02 /* file is a directory and contains entries */
  #define CD_ASSOCIATED  0x04/* file is opaque to filesystem, visable
                                to system implementation */
  #define CD_EAHSFRECORD 0x04 /* file has HSF extended attribute record
                                 fmt */
  #define CD_PROTECTION  0x04 /* used extended attributes for protection */
  #define CD_ANOTHEREXTNT 0x80 /* file has at least one more extent */

 #define ISODCL(from, to) (to - from + 1)

  struct iso_directory_record {
       char length;
       unsigned int lba;
       unsigned int size;
       char flags;
       char file_unit_size;
       unsigned int volume_sequence_number;
       char name_len ;
       char name           [20];

  };

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_unpach_graph_clicked();

    void on_pushButton_pack_clicked();

    void on_pushButton_4_clicked();

private:
    Ui::MainWindow *ui;
    QList <iso_directory_record> filelist;
};
#endif // MAINWINDOW_H
