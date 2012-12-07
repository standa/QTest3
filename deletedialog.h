#ifndef DELETEDIALOG_H
#define DELETEDIALOG_H

#include <QProgressDialog>

class QStringList;

class DeleteDialog : public QProgressDialog
{
public:
    DeleteDialog();
    bool process();
    static bool deleteDir(const QString & dir);
};

#endif // DELETEDIALOG_H
