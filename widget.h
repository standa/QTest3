#ifndef WIDGET_H
#define WIDGET_H

#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class QTreeWidget;
class QTreeView;
class QHBoxLayout;
class QVBoxLayout;
class QPushButton;
class QGroupBox;
class QDirModel;
class QSettings;
class QModelIndex;

QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = 0);
    ~Widget();

private:
    QString dirRoot();
    void setupPanels();
    void setupButtons();
    void centerWindow();
    QModelIndex currentIndex();
    void loadSettings();
    bool eventFilter(QObject *target, QEvent *event);

    QDirModel   * dirModel;
    QTreeView   * leftPanel;
    QTreeView   * rightPanel;
    QHBoxLayout * mainLayout;

    QVBoxLayout * middleLayout;
    QGroupBox   * buttonBox;
    QPushButton * copyButton;
    QPushButton * moveButton;
    QPushButton * createDirButton;
    QPushButton * deleteButton;
    QPushButton * quitButton;

    QSettings   * settings;

    bool leftPanelHasFocus;
    bool confirmActions;

private slots:
     void onCopyButtonClicked();
     void onMoveButtonClicked();
     void onCreateDirButtonClicked();
     void onDeleteButtonClicked();
     void onQuitButtonClicked();
     void onLeftPanelClicked(const QModelIndex &);
     void onRightPanelClicked(const QModelIndex &);


};

#endif // WIDGET_H
