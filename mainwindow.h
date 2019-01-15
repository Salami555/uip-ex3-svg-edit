#pragma once

#include <QMainWindow>
#include <QFileInfo>

class Controller;
class Model;

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
    ~MainWindow();

    void setModel(const Model * model);
    const Model * model() const;

    void setController(const Controller * controller);
    const Controller * controller() const;

protected:
    void initializeGraphicsView();
    void initializeSourceView();

    void fileOpenedWith_handling(QFileInfo file);

    virtual void showEvent(QShowEvent * event);
    virtual void closeEvent(QCloseEvent * event);

private slots:
    void onResourceOpenend();
    void onResourceClosed();
    void onResourceModified();

    void onResourceOperationFailed(ResourceOperationResult result);

    void on_actionOpenFile_triggered();

    void on_actionSwapViews_triggered();

    void on_actionFitView_toggled(bool enabled) const;
    void on_graphicsView_fitViewChanged(bool enabled) const;

    void on_sourceView_highlightChanged(bool enabled) const;
    void on_sourceView_wordWrapChanged(bool enabled) const;
    void on_sourceView_sourceChanged() const;

    void on_actionZoomIn_triggered();
    void on_actionZoomOut_triggered();

protected:
    const Model * m_model = nullptr;            // Intent: receive signals from.
    const Controller * m_controller = nullptr;  // Intent: emit signals to.

private:
    Ui::MainWindow * m_ui;
};
