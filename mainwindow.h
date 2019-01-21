#pragma once

#include <QMainWindow>
#include <QFileInfo>
#include <QSettings>

class GraphicsView;
class SourceView;

enum class ResourceOperationResult;


namespace Ui {
    class MainWindow;
}


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget * parent = nullptr);

protected:
    void openFiles(const QList<QFileInfo> files);

    virtual void dragEnterEvent(QDragEnterEvent * event);
    virtual void dropEvent(QDropEvent * event);

    virtual void showEvent(QShowEvent * event);
    virtual void closeEvent(QCloseEvent * event);

private slots:
//    void onResourceOpenend();
//    void onResourceClosed();
//    void onResourceModified();

//    void onResourceOperationFailed(ResourceOperationResult result);

    // menu trigger
    void on_actionOpenFiles_triggered();

    void on_actionCloseCurrentFile_triggered();
    void on_actionCloseAllFiles_triggered();
    void on_actionExit_triggered();

    void on_actionChangeFont_triggered();

    void on_actionAbout_triggered();

    // generic
    void on_tabSelected();
    void on_tabCloseRequested(int index);

private:
    Ui::MainWindow * m_ui;

    QSettings * m_settings;
};
