#pragma once

#include <QMainWindow>
#include <QFileInfo>
#include <QSettings>

class Tab;

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
    void addRecentFile(const QFileInfo & file);
    void restoreRecentFileList();
    void updateRecentFileMenuList(const QList<QFileInfo> & files);
    void updateWindowTitle();
    void addTab(Tab * tab);
    void openFiles(const QList<QFileInfo> files);

    void toggleAllSplitterPositionModifiers();
    void swapCurrentTabSplitterContent();

    virtual void dragEnterEvent(QDragEnterEvent * event);
    virtual void dropEvent(QDropEvent * event);

    virtual void showEvent(QShowEvent * event);
    virtual void closeEvent(QCloseEvent * event);

private slots:
    // menu trigger
    void on_actionNewFile_triggered();
    void on_actionOpenFiles_triggered();
    void on_actionSaveCurrentFile_triggered();
    void on_actionSaveCurrentFileAs_triggered();
    void on_actionSaveAllFiles_triggered();
    void on_actionCloseCurrentFile_triggered();
    void on_actionCloseAllFiles_triggered();
    void on_actionExit_triggered();

    void on_actionChangeFont_triggered();

    void on_actionWelcomeTab_triggered();
    void on_actionAbout_triggered();

    // generic
    void on_tabSelected();
    void on_tabCloseRequested(int index);
    void on_modifiedStatusChange();

private:
    Ui::MainWindow * m_ui;

    QSettings * m_settings;
};
