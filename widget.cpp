#include <QtGui>
#include <QDebug>

#include "widget.h"
#include "copydialog.h"
#include "deletedialog.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    setupPanels();
    setupButtons();

    mainLayout = new QHBoxLayout;
    mainLayout->addWidget(leftPanel);
    mainLayout->addWidget(buttonBox);
    mainLayout->addWidget(rightPanel);
    setLayout(mainLayout);

    centerWindow();

    setWindowTitle(tr("File manager"));

    loadSettings();
}

void Widget::setupPanels()
{
    dirModel = new QDirModel;
    dirModel->setSorting(QDir::DirsFirst|QDir::IgnoreCase|QDir::LocaleAware);
    dirModel->setReadOnly(false);

    leftPanel = new QTreeView;
    leftPanel->setModel(dirModel);
#ifndef Q_OS_WIN32
    //root is set to all the drives if win 32 (default behaviour)
    //or to dirRoot which might be set manually for convenience
    leftPanel->setRootIndex(dirModel->index(dirRoot(), 0));
    leftPanel->setRootIsDecorated(false);
#endif
    leftPanel->setAnimated(true);
//    leftPanel->setSortingEnabled(true);
    leftPanel->setAllColumnsShowFocus(true);
    leftPanel->setEditTriggers(QAbstractItemView::SelectedClicked);
    leftPanel->setSelectionMode(QAbstractItemView::ExtendedSelection);
    leftPanel->resizeColumnToContents(0);
    leftPanel->setDragEnabled(true);
    leftPanel->setAcceptDrops(true);
    leftPanel->setDropIndicatorShown(true);
    leftPanel->setDragDropOverwriteMode(true);
    leftPanel->installEventFilter(this);
    leftPanel->selectionModel()->select(leftPanel->indexAt(QPoint(0, 0)),
             QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows);
    leftPanel->setCurrentIndex(leftPanel->indexAt(QPoint(0, 0)));

    connect(leftPanel, SIGNAL(clicked(QModelIndex)), this,
            SLOT(onLeftPanelClicked(QModelIndex)));

    leftPanel->setFocus();
    leftPanelHasFocus = true;

    rightPanel = new QTreeView;
    rightPanel->setModel(dirModel);
#ifndef Q_OS_WIN32
    rightPanel->setRootIndex(dirModel->index(dirRoot(), 0));
    rightPanel->setRootIsDecorated(false);
#endif
    rightPanel->setAllColumnsShowFocus(true);
    rightPanel->setEditTriggers(QAbstractItemView::SelectedClicked);
    rightPanel->setSelectionMode(QAbstractItemView::ExtendedSelection);
    rightPanel->resizeColumnToContents(0);
    rightPanel->setDragEnabled(true);
    rightPanel->setAcceptDrops(true);
    rightPanel->setDropIndicatorShown(true);
    rightPanel->setDragDropOverwriteMode(true);
    rightPanel->installEventFilter(this);
    rightPanel->selectionModel()->select(rightPanel->indexAt(QPoint(0, 0)),
             QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows);
    rightPanel->setCurrentIndex(rightPanel->indexAt(QPoint(0, 0)));

    connect(rightPanel, SIGNAL(clicked(QModelIndex)), this,
            SLOT(onRightPanelClicked(QModelIndex)));

}

bool Widget::eventFilter(QObject *target, QEvent *event)
{
    if (target == leftPanel && event->type() == QEvent::ChildRemoved) {
        dirModel->refresh(leftPanel->rootIndex());
        return false;
    } else if (target == rightPanel && event->type() == QEvent::ChildRemoved) {
        dirModel->refresh(rightPanel->rootIndex());
        return false;
    } else if (target == leftPanel && event->type()
        == QEvent::MouseButtonPress) {
        leftPanelHasFocus = true;
        qDebug() << "left panel has focus";
    } else if (target == rightPanel && event->type()
        == QEvent::MouseButtonPress) {
        leftPanelHasFocus = false;
        qDebug() << "right panel has focus";
    } else if (event->type() == QEvent::KeyPress) {
        QKeyEvent *ke = static_cast<QKeyEvent *>(event);
        qDebug("key event %d", ke->key());

        switch (ke->key()) {
            case Qt::Key_Tab:
                qDebug("key_tab");
                if (leftPanel->hasFocus()) {
                    leftPanelHasFocus = false;
                    rightPanel->setFocus();
                } else {
                    leftPanelHasFocus = true;
                    leftPanel->setFocus();
                }
                break;
            case Qt::Key_Return:
                qDebug("key_return");                
                if (leftPanel->hasFocus()) {
                    if (dirModel->hasChildren(leftPanel->currentIndex())) {
                        if (leftPanel->isExpanded(leftPanel->currentIndex())) {
                            leftPanel->collapse(leftPanel->currentIndex());
                        } else {
                            leftPanel->expand(leftPanel->currentIndex());
                        }
                    }
                } else {
                    if (dirModel->hasChildren(rightPanel->currentIndex())) {
                        if (rightPanel->isExpanded(rightPanel->currentIndex())) {
                            rightPanel->collapse(rightPanel->currentIndex());
                        } else {
                            rightPanel->expand(rightPanel->currentIndex());
                        }
                    }
                }
                break;
            case Qt::Key_Insert:
                qDebug("key_insert");
            case Qt::Key_Space:
                qDebug("key_space");
                if (leftPanelHasFocus) {
                    qDebug() << "selecting " <<
                        leftPanel->currentIndex().data().toString();

                    leftPanel->selectionModel()->select(
                            leftPanel->currentIndex(),
                            QItemSelectionModel::SelectCurrent);

                    qDebug() << "moving to " <<
                        leftPanel->indexBelow(leftPanel->currentIndex()).
                                    data().toString();

                    leftPanel->setCurrentIndex(
                            leftPanel->indexBelow(
                                    leftPanel->currentIndex()));
                }
                break;
            case Qt::Key_F5:
                qDebug("key_f5");
                onCopyButtonClicked();
                break;
            case Qt::Key_F6:
                qDebug("key_f6");
                onMoveButtonClicked();
                break;
            case Qt::Key_F7:
                qDebug("key_f7");
                onCreateDirButtonClicked();
                break;
            case Qt::Key_F8:
                qDebug("key_f8");
                onDeleteButtonClicked();
                break;
            case Qt::Key_Escape:
                qDebug("key_escape");
                onQuitButtonClicked();
                break;
            default:
                return QWidget::eventFilter(target, event);
                break;
        } //switch
    }
//        qDebug("Other event happened");
        return QWidget::eventFilter(target, event);
}

void Widget::setupButtons()
{
    middleLayout = new QVBoxLayout;
    buttonBox = new QGroupBox;

    copyButton = new QPushButton(tr("Copy"));
    copyButton->setIcon(QIcon(":/images/copy.png"));
    moveButton = new QPushButton(tr("Move"));
    createDirButton = new QPushButton(tr("Create dir"));
    deleteButton = new QPushButton(tr("Delete"));
    quitButton = new QPushButton(tr("Quit"));

    middleLayout->addStretch(0);

    middleLayout->addWidget(copyButton);
    connect(copyButton, SIGNAL(clicked()), this,
            SLOT(onCopyButtonClicked()));

    middleLayout->addWidget(moveButton);
    connect(moveButton, SIGNAL(clicked()), this,
            SLOT(onMoveButtonClicked()));

    middleLayout->addWidget(createDirButton);
    connect(createDirButton, SIGNAL(clicked()), this,
            SLOT(onCreateDirButtonClicked()));

    middleLayout->addWidget(deleteButton);
    connect(deleteButton, SIGNAL(clicked()), this,
            SLOT(onDeleteButtonClicked()));

    middleLayout->addWidget(quitButton);
    connect(quitButton, SIGNAL(clicked()), this,
            SLOT(onQuitButtonClicked()));

    middleLayout->addStretch(0);

    buttonBox->setLayout(middleLayout);
}

void Widget::centerWindow()
{
    resize(800, 600);

    QDesktopWidget *desktop = QApplication::desktop();

    int screenWidth, width;
    int screenHeight, height;
    int x, y;
    QSize windowSize;

    screenWidth = desktop->width(); // get width of screen
    screenHeight = desktop->height(); // get height of screen

    windowSize = size(); // size of our application window
    width = windowSize.width();
    height = windowSize.height();

    x = (screenWidth - width) / 2;
    y = (screenHeight - height) / 2;
//    x += 20; //just fine tuning
//    y -= 50;

    // move window to desired coordinates
    move ( x, y );
}

void Widget::loadSettings()
{
    confirmActions = true;
}

Widget::~Widget()
{
    //@todo save main app info
    //settings->setValue("leftPanel/header", leftPanel->header());
}

/**
  get the root of the disk... on windows it should be c:, on linux /
  */
QString Widget::dirRoot()
{
    return QString("/");
}

void Widget::onCopyButtonClicked()
{
    //determine current dir and selected files
    QDir * targetDir;
    QModelIndexList indexes;

    if (leftPanelHasFocus) {

        indexes = leftPanel->selectionModel()->selectedRows(0);

        QFileInfo dirInfo(dirModel->filePath(rightPanel->currentIndex()));
        if (!dirInfo.isDir()) {
            targetDir = new QDir(dirModel->filePath(
                    rightPanel->currentIndex().parent()));
        } else {
            targetDir = new QDir(dirModel->filePath(rightPanel->currentIndex()));
        }
         qDebug() << "left panel has focus" << indexes.count()
                 << " items selected" << "targetDir is "
                 << targetDir->absolutePath();
    } else {
        indexes = rightPanel->selectionModel()->selectedRows(0);

        QFileInfo dirInfo(dirModel->filePath(leftPanel->currentIndex()));
        if (!dirInfo.isDir()) {
            targetDir = new QDir(dirModel->filePath(
                    leftPanel->currentIndex().parent()));
        } else {
            targetDir = new QDir(dirModel->filePath(leftPanel->currentIndex()));
        }
         qDebug() << "right panel has focus" << indexes.count()
                 << " items selected" << "targetDir is "
                 << targetDir->absolutePath();
    }

    //get the count of the files to copy
    QString count;
    if (indexes.count() == 0) {
        return;
    } else if (indexes.count() == 1) {
        count = "1 file";
    } else {
        count = QString("%1 files").arg(indexes.count());
    }

    QModelIndex index;

    if (!confirmActions ||
        (confirmActions && QMessageBox::question(this, tr("Copy"),
                              tr("Are you sure you want to copy the %1 to %2?")
                              .arg(count).arg(targetDir->absolutePath()),
                              QMessageBox::Ok|QMessageBox::Cancel,
                              QMessageBox::Ok) == QMessageBox::Ok)) {
        //copy the individual files and dirs
        QStringList dirs;
        foreach (index, indexes) {
            dirs << dirModel->filePath(index);
        }

        CopyDialog copyDialog(dirs, targetDir->absolutePath());

        if (!copyDialog.process()) {
            QMessageBox::critical(this, tr("Copy error"),
                                  tr("Error occured when copying."),
                                  QMessageBox::Ok, QMessageBox::Ok);
        }
    }

    dirModel->refresh(leftPanel->rootIndex());
    dirModel->refresh(rightPanel->rootIndex());

    if (leftPanelHasFocus) {
        rightPanel->expand(rightPanel->currentIndex());
    } else {
        leftPanel->expand(leftPanel->currentIndex());
    }

    delete targetDir;
}

/**
  Widget::onMoveButtonClicked()

  Basically the same as the copy action as far as the code is concerned.
  Only after the copying has finished, the files are also deteleted.
  */
void Widget::onMoveButtonClicked()
{
    onCopyButtonClicked();
    onDeleteButtonClicked();
}

/**
  Widget::onCreateDirButtonClicked()

  Create the directory in the selected branch of the file system.
  If any error occured (no write permission, directory already exists),
  the error message is shown.
  */
void Widget::onCreateDirButtonClicked()
{
    //determine current dir
    QFileInfo dirInfo(dirModel->filePath(currentIndex()));
    QDir * dir;
    if (!dirInfo.isDir()) {
        dir = new QDir(dirModel->filePath(currentIndex().parent()));
    } else {
        dir = new QDir(dirModel->filePath(currentIndex()));
    }

    //create the directory or show the error message
    bool ok;
    QString newDir =
         QInputDialog::getText(this,
               tr("Create Directory"),
               tr("Current dir: ").
                append(dir->absolutePath()),
               QLineEdit::Normal,
               tr("new subdir"), &ok);


    if (ok && !newDir.isEmpty()) {

        qDebug() << "creating dir " << dir->absolutePath() <<
        "/" << newDir;
        if (!dir->mkdir(newDir)) {
            QMessageBox::critical(this, tr("Make dir error"),
                           tr("The dir %1 could not be created.").
                           arg(newDir), QMessageBox::Ok,
                           QMessageBox::Ok);
        }

    }

    delete dir;

    if (leftPanelHasFocus) {
        dirModel->refresh(leftPanel->rootIndex());
        leftPanel->setFocus();
        leftPanel->expand(currentIndex());
    } else {
        dirModel->refresh(rightPanel->rootIndex());
        rightPanel->setFocus();
        rightPanel->expand(currentIndex());
    }


}

/**
  Widget::onDeleteButtonClicked()

  Delete currently selected directory tree.
  The function deletes either a single file or the whole directory tree
  including the files contained inside the directory. The user is asked for
  confirmation but once only. An error message is shown if any error occured.
  */
void Widget::onDeleteButtonClicked()
{
    //determine what to delete actually
    QModelIndexList indexes;
    if (leftPanelHasFocus) {
        indexes = leftPanel->selectionModel()->selectedRows();
    } else {
        indexes = rightPanel->selectionModel()->selectedRows();
    }

    //get the count of the files to delete
    QString count;
    if (indexes.count() == 0) {
        return;
    } else if (indexes.count() == 1) {
        count = "1 item";
    } else {
        count = QString("%1 items").arg(indexes.count());
    }

    if (!confirmActions ||
        (confirmActions && QMessageBox::Yes == QMessageBox::question(this,
                              tr("Delete confirmation"),
                              tr("Are you sure you want to delete %1?").
                              arg(count),
                              QMessageBox::Yes | QMessageBox::No,
                              QMessageBox::No))) {
        QModelIndex index;
        foreach (index, indexes) {
            QString filePath = dirModel->filePath(index);
            qDebug() << "Deleting item " << filePath;
            QFileInfo fileInfo(filePath);

            if (fileInfo.isFile()) {
                //user confirmed the deletion
                if (!QFile::remove(filePath) && fileInfo.isWritable()) {
                    QMessageBox::critical(this, tr("Remove error"),
                                          tr("Unable to delete the file %1.").
                                          arg(filePath),
                                          QMessageBox::Ok,
                                          QMessageBox::Ok);
                    return;
                }
            } else if (fileInfo.isDir() && fileInfo.isWritable()) {
                if (!DeleteDialog::deleteDir(QString(fileInfo.absolutePath()))) {
                    QMessageBox::critical(this, tr("Remove error"),
                               tr("Unable to delete the dir %1.").
                               arg(fileInfo.absolutePath()),
                                          QMessageBox::Ok,
                                          QMessageBox::Ok);
                    return;
                } //if
            } //else if
        } //foreach
    } //if

    if (leftPanelHasFocus) {
        dirModel->refresh(leftPanel->rootIndex());
        leftPanel->setFocus();
    } else {
        dirModel->refresh(rightPanel->rootIndex());
        rightPanel->setFocus();
    }
}

void Widget::onQuitButtonClicked()
{
    QApplication::quit();
}

void Widget::onLeftPanelClicked(const QModelIndex & modelIndex)
{
    leftPanelHasFocus = true;
    leftPanel->resizeColumnToContents(0);
    qDebug() << "left panel has focus on index " << modelIndex.data();
}

void Widget::onRightPanelClicked(const QModelIndex & modelIndex)
{
    leftPanelHasFocus = false;
    rightPanel->resizeColumnToContents(0);
    qDebug() << "right panel has focus on index " << modelIndex.data();
}

QModelIndex Widget::currentIndex()
{
    return leftPanelHasFocus?
            leftPanel->currentIndex():rightPanel->currentIndex();
}
