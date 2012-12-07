#include <QtGui>
#include <QMessageBox>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFileInfoList>
#include <QCoreApplication>

#include "deletedialog.h"

DeleteDialog::DeleteDialog()
{

}

/**
  DeleteDialog::deleteDir()

  Delete the whole directory tree.
  */
bool DeleteDialog::deleteDir(const QString & dir)
{
    QDir srcDir(dir);

    if (!srcDir.exists()) {
        QFileInfo srcInfo(dir);
        if (srcInfo.isFile()) {
            return QFile::remove(dir);
        } else {
            return false;
            qDebug() << "not a dir and not a file" << dir;
        }
    }

    QFileInfoList srcList = srcDir.entryInfoList(
            QDir::Dirs|QDir::Files|QDir::Readable|QDir::NoDotAndDotDot);
    qDebug() << "processing" << srcList.count() << "entries";

    QFileInfo di;
    foreach (di, srcList) {
        qDebug() << "Deleting file" << di.absoluteFilePath();

        if (di.isDir()) {
            if (deleteDir(di.absoluteFilePath())) {
                continue;
            } else {
                qDebug() << "DeleteDialog::deleteDir() error: " <<
                        di.absoluteFilePath();
                return false;
            }
        } else if (di.isFile()) {
            if (!di.isWritable()) {
                //implicit behaviour is to copy everything possible
                qDebug() << di.absoluteFilePath() << "not readable!";
                continue;
            }
            qDebug() << "removing" << di.absoluteFilePath();
            QFile::remove(di.absoluteFilePath());
        } //isFile()
    } //foreach

    return srcDir.rmdir(".");
}
