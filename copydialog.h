#ifndef COPYDIALOG_H
#define COPYDIALOG_H

#include <QProgressDialog>

class QStringList;

class CopyDialog : public QProgressDialog
{
public:
    CopyDialog(const QStringList & sourceDirs,
                         const QString & targetDir);
    bool process();

private:
    long dirSize(const QString & dir);
    bool copyDir(const QString & sourceDir, const QString & targetDir);

    QStringList sourceDirs;
    QString     targetDir;

};

#endif // COPYDIALOG_H
