#include <QtGui>
#include <QMessageBox>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFileInfoList>
#include <QCoreApplication>

#include "copydialog.h"

/**
  CopyDialog::CopyDialog()

  Go through the directories in the list and copy the whole source directory
  tree to the respective index in the target dir
  */
CopyDialog::CopyDialog(const QStringList & sourceDirs,
                         const QString & targetDir)
{
    this->sourceDirs = sourceDirs;
    this->targetDir  = targetDir;
    QProgressDialog();
    qDebug() << "Copy dialog for" << sourceDirs.count() << "files into"
            << targetDir;
}

/**
  CopyDialog::dirSize()

  determine the size of the whole directory tree
  may take a while!
  */
long CopyDialog::dirSize(const QString & dirPath)
{
    qDebug() << "CopyDialog::dirSize() << " << dirPath;

    QFileInfo fileInfo;

    fileInfo = QFileInfo(dirPath);

    if (fileInfo.exists() && fileInfo.isFile()) {
        qDebug() << fileInfo.fileName() << "=" << fileInfo.size() << "B";
        return (long)fileInfo.size();
    }

    QDir dir(dirPath);
    if (!dir.exists()) {
        return 0;
    }

    long sum = 0;

    foreach (fileInfo, dir.entryInfoList(QDir::NoDotAndDotDot |
            QDir::Dirs | QDir::Files)) {
        if (fileInfo.isDir()) {
            sum += dirSize(fileInfo.absoluteFilePath());
        } else if (fileInfo.isFile()) {
            qDebug() << fileInfo.fileName() << "=" << fileInfo.size() << "B";
            sum += (long)fileInfo.size();
        }
    }
    qDebug() << "dirSize of " << dirPath << "is" << sum;
    return sum;
}

bool CopyDialog::process()
{
    //determine the size of the directory structure to be copied
    //for showing the accurate progress bar
    QString current;
    long treeSize = 0;
    foreach (current, sourceDirs) {
        qDebug() << "CopyDialog::process(): dirSize(" << current << ")";
        treeSize += dirSize(current);
    }

    qDebug() << treeSize/1e6 << " Mb to copy";

    QProgressDialog(tr("File copy"), tr("Abort"),  0, treeSize/1e6);

    foreach (current, sourceDirs) {
        qDebug() << "copying tree" << current << " >> " << targetDir;
        if (!copyDir(current, targetDir) || wasCanceled()) {
            break;
        }
    }
    setValue(maximum());
    return true;
}

/**
  CopyDialog::copyDir()

  copy the whole directory tree from somewhere to somewhere else
  */
bool CopyDialog::copyDir(const QString & sourceDir,
                               const QString & targetDir)
{
    qDebug() << "CopyDialog::copyDir(" << sourceDir << "," << targetDir << ")";

    if (sourceDir == targetDir) {
        qDebug() << "copyDir(): source == target";
        return true;
    }

    QDir srcDir(sourceDir);
    QFileInfo srcInfo(sourceDir);

    QDir destDir(targetDir);
    QString msg;

    if (!destDir.exists()) {
            if (!destDir.mkpath(destDir.absolutePath())) {
                    msg.append(tr("Could not create destination directory %1").
                                arg(destDir.absolutePath()));
                    QMessageBox::critical(this, "Copy Dir", msg);
                    qDebug() << "Could not create the destination directory"
                            << destDir.absolutePath();
                    return false;
            }
    }

    if (!srcInfo.isDir()) {
        //maybe it is file
        qDebug() << "CopyDir()" << sourceDir << "is not a directory";
        if (srcInfo.isFile()) {
            if (destDir.exists(srcInfo.fileName())) {
                if (QMessageBox::warning(0, tr("Copy overwrite"),
                  tr("The file %1 exists in %2. Do you want to overwrite it?").
                  arg(srcInfo.fileName()).arg(destDir.absolutePath()),
                  QMessageBox::Yes|QMessageBox::No,
                  QMessageBox::No) == QMessageBox::Yes) {
                        //file exists, overwrite?
                        destDir.remove(srcInfo.fileName());
                } else {
                    return true;
                }
            }

            if (srcInfo.size() > 1e6) {
                setValue(value()+(int)(srcInfo.size()/1e6));
            }
            qDebug() << "copying " << sourceDir << "to" <<
                    QDir::toNativeSeparators(
                destDir.absolutePath().
                append("/").append(srcInfo.fileName()));
            return QFile::copy(sourceDir, QDir::toNativeSeparators(
                destDir.absolutePath().
                append("/").append(srcInfo.fileName())));

        } else {
            qDebug() << srcInfo.fileName() << "is not a file";
            return false;
        }
    } //isDir() for src

    // get info on all files in source directory
    QFileInfoList srcList = srcDir.entryInfoList(
            QDir::Dirs|QDir::Files|QDir::Readable|QDir::NoDotAndDotDot);
    qDebug() << "processing" << srcList.count() << "entries";

    QFileInfo di;
    foreach (di, srcList) {
        qDebug() << "Copying file" << di.absoluteFilePath();

        msg = QString("Copying file\"%1\"...").
              arg(di.absoluteFilePath());
        setLabelText(msg);

        if (wasCanceled()) {
            qDebug() << "operation cancelled by user";
            break;
        }

        if (di.isDir()) {
            if (copyDir(di.absoluteFilePath(),
                        QString(targetDir).append(QDir::separator()).
                        append(di.fileName()))) {
                continue;
            } else {
                qDebug() << "CopyDialog::CopyDirs() error: " <<
                        di.absoluteFilePath();
                return false;
            }
        } else if (di.isFile()) {
            if (!di.isReadable()) {
                //implicit behaviour is to copy everything possible
                qDebug() << di.absoluteFilePath() << "not readable!";
                continue;
            }

            QFile in(di.absoluteFilePath());
            QFile out(QDir::toNativeSeparators(QString(destDir.absolutePath()).
                      append("/").append(di.fileName())));
            if (out.exists() &&
                QMessageBox::warning(0, tr("Copy overwrite"),
                  tr("The file %1 exists. Do you want to overwrite it?").
                  arg(out.fileName()), QMessageBox::Yes|QMessageBox::No,
                  QMessageBox::No) == QMessageBox::No) {
                //file exists, overwrite?
                continue;
            }

            //all checks performed
            //now we can start the copy procedure
            qDebug() << di.absoluteFilePath() << "to" <<
                    QDir::toNativeSeparators(QString(destDir.absolutePath()).
                               append("/").append(di.fileName()));
            if (!in.copy(QDir::toNativeSeparators(QString(destDir.absolutePath()).
                               append("/").append(di.fileName())))) {
                return false;
            }
            if (in.size() > 1e6) {
                setValue(value()+(int)(in.size()/1e6));
            }
        } //isFile()
    } //foreach
    return true;
}
