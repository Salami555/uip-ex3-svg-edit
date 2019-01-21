#pragma once

#include <QMainWindow>
#include <QFileInfo>

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
    void initializeGraphicsView();
    void initializeSourceView();

    virtual void dragEnterEvent(QDragEnterEvent * event);
    virtual void dropEvent(QDropEvent * event);

    virtual void showEvent(QShowEvent * event);
    virtual void closeEvent(QCloseEvent * event);

private slots:
    void on_actionExit_triggered();
//    void onResourceOpenend();
//    void onResourceClosed();
//    void onResourceModified();

//    void onResourceOperationFailed(ResourceOperationResult result);

    void on_actionOpenFiles_triggered();

    void on_actionCloseCurrentFile_triggered();

    void on_tabSelected();
    void on_tabCloseRequested(int index);

//    void on_actionSwapViews_triggered();

//    void on_actionFitView_toggled(bool enabled) const;
//    void on_graphicsView_fitViewChanged(bool enabled) const;

//    void on_sourceView_highlightChanged(bool enabled) const;
//    void on_sourceView_wordWrapChanged(bool enabled) const;
//    void on_sourceView_sourceChanged() const;

//    void on_actionZoomIn_triggered();
//    void on_actionZoomOut_triggered();

private:
    Ui::MainWindow * m_ui;

    void openFiles(const QList<QFileInfo> files);
};
