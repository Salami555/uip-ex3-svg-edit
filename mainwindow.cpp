
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QActionGroup>
#include <QSplitter>
#include <QFileDialog>
#include <QErrorMessage>
#include <QDropEvent>
#include <QMimeData>
#include <QMessageBox>
#include <QDebug>

// #include "tabmodel.h"
#include "resource.h"

#include "tab.h"
#include "graphicsview.h"
#include "sourceview.h"


MainWindow::MainWindow(QWidget * parent) :
    QMainWindow(parent),
    m_ui(new Ui::MainWindow)
{
    m_ui->setupUi(this);

    auto newTab = new Tab(m_ui->tabView);
    m_ui->tabView->addTab(newTab, "Asdf");
    //    m_ui->graphicsView->setFitView(true);
    //    m_ui->sourceView->setHighlighting(true);
    //    m_ui->sourceView->setWordWrap(true);

    connect(m_ui->tabView, &QTabWidget::tabCloseRequested, this, &MainWindow::on_tabCloseRequested);
    connect(m_ui->actionExit, &QAction::triggered, this, &MainWindow::on_actionExit_triggered);

    // connect(m_ui->actionViewSource, SIGNAL(triggered()), this, SLOT(foobar1()));
    // connect(m_ui->actionViewSource, &QAction::triggered,[]() { qDebug() << "test mit lambda"; });
    // connect(m_ui->actionSelectAll, &QAction::triggered, m_ui->sourceView, &SourceView::selectAll);

    //    connect(m_ui->actionSyntaxHighlighting, &QAction::triggered, m_ui->sourceView, &SourceView::setHighlighting);
    //    connect(m_ui->actionWordWrap, &QAction::triggered, m_ui->sourceView, &SourceView::setWordWrap);
}


// ----- Utils -----------------------------------------------
template<typename Base, typename T>
inline bool instanceof(const T *ptr) {
    return dynamic_cast<const Base*>(ptr) != nullptr;
}

bool confirmDiscardingUnsavedChanges(QWidget *parent)
{
    auto reply = QMessageBox::question(
        parent,
        "File not saved",
        "Do you want to discard the current changes?",
        QMessageBox::Yes|QMessageBox::No
    );
    return reply == QMessageBox::Yes;
}


// ----- Control -----------------------------------------------
void MainWindow::openFiles(const QList<QFileInfo> files)
{
    for(auto c : files) {
        qDebug() << c;
    }
    for(int i = 0; i < m_ui->tabView->count(); ++i) {
        auto tab = m_ui->tabView->widget(i);
        if(instanceof<Tab>(tab)) {
            qDebug() << tab;
        }
    }
}


// ----- Events -----------------------------------------------
void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls())
        event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent * event)
{
    const QMimeData* mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        QList<QFileInfo> files;
        for(QUrl url : mimeData->urls()) {
            files.append(QFileInfo(url.toLocalFile()));
        }
        this->openFiles(files);
    }
}

void MainWindow::showEvent(QShowEvent * event)
{
    QMainWindow::showEvent(event);
    QStringList args = qApp->arguments();
    QList<QFileInfo> files;
    for(short i = 1; i < args.size(); i++) {
        files.append(QFileInfo(args[i]));
    }
    this->openFiles(files);
}

bool isEveryFileSaved(const QTabWidget * tabView)
{
    for(int i = 0; i < tabView->count(); ++i) {
        auto widget = tabView->widget(i);
        if(instanceof<Tab>(widget)) {
            const Tab *tab = dynamic_cast<const Tab*>(widget);
            if(tab->hasPendingChanges()) {
                return false;
            }
        }
    }
    return true;
}

void MainWindow::closeEvent(QCloseEvent * event)
{
    if(!isEveryFileSaved(m_ui->tabView)) {
        if(!confirmDiscardingUnsavedChanges(this)) {
            event->ignore();
            return;
        }
    }
    m_ui->actionExit->trigger();
}


// ----- SLOTS -----------------------------------------------
//void MainWindow::onResourceOpenend()
//{
//    auto alert = new QMessageBox(this);
//    alert->setText("opened resource: " + m_model->resource()->fileInfo().fileName());
//    alert->showNormal();
    //    m_ui->sourceView->setResource(m_model->resource());
    //    m_ui->graphicsView->setResource(m_model->resource());
//}

//void MainWindow::onResourceClosed()
//{
    //    m_ui->sourceView->setResource(nullptr);
    //    m_ui->graphicsView->setResource(nullptr);
//}

//void MainWindow::onResourceModified()
//{
    //    m_ui->graphicsView->reloadFromResource();
    //m_ui->graphicsView->setResource(m_model->resource());
//}

//void MainWindow::onResourceOperationFailed(const ResourceOperationResult result)
//{
//    auto errorDialog = new QErrorMessage(this);
//    errorDialog->showMessage(QString("A Problem occurred: %1").arg(Resource::operationResultString(result)));
//}


//void MainWindow::on_actionSwapViews_triggered()
//{
    //    const auto widget0 = m_ui->splitter->widget(0);
    //    const auto widget1 = m_ui->splitter->widget(1);

    //    const auto sizes = m_ui->splitter->sizes();

    //    m_ui->splitter->insertWidget(0, widget1);
    //    m_ui->splitter->insertWidget(1, widget0);

    //    m_ui->splitter->setSizes(sizes);
//}

void MainWindow::on_actionExit_triggered()
{
    qApp->exit();
}

void MainWindow::on_actionOpenFiles_triggered()
{
    auto dialog = new QFileDialog(this);
    dialog->setFileMode(QFileDialog::ExistingFiles);
    dialog->setViewMode(QFileDialog::Detail);
    dialog->setAcceptMode(QFileDialog::AcceptOpen);

    dialog->setMimeTypeFilters({ "image/svg+xml" });
    dialog->setNameFilter("Scalable Vector Graphic Files (*.svg);; All Files (*.*)");

    dialog->open(const_cast<MainWindow *>(this), SLOT(openResources(const QStringList &)));
}

void MainWindow::on_tabCloseRequested(int index)
{
    auto widget = m_ui->tabView->widget(index);
    if(instanceof<Tab>(widget)) {
        const Tab *tab = dynamic_cast<const Tab*>(widget);
        if(tab->hasPendingChanges()) {
            if(!confirmDiscardingUnsavedChanges((QWidget *)tab)) {
                return;
            }
        }
    }

    m_ui->tabView->removeTab(index);

    qDebug() << m_ui->tabView->count();
    if(m_ui->tabView->count() == 0) {
        m_ui->actionExit->trigger();
    }
}

//void MainWindow::on_actionFitView_toggled(bool enabled) const
//{
    //    m_ui->graphicsView->setFitView(enabled);
//}

//void MainWindow::on_graphicsView_fitViewChanged(bool enabled) const
//{
//    m_ui->actionFitView->blockSignals(true);
//    m_ui->actionFitView->setChecked(enabled);
//    m_ui->actionFitView->blockSignals(false);
//}

//void MainWindow::on_sourceView_highlightChanged(bool enabled) const
//{
//    m_ui->actionSyntaxHighlighting->blockSignals(true);
//    m_ui->actionSyntaxHighlighting->setChecked(enabled);
//    m_ui->actionSyntaxHighlighting->blockSignals(false);
//}

//void MainWindow::on_sourceView_wordWrapChanged(bool enabled) const
//{
//    m_ui->actionWordWrap->blockSignals(true);
//    m_ui->actionWordWrap->setChecked(enabled);
//    m_ui->actionWordWrap->blockSignals(false);
//}

//void MainWindow::on_sourceView_sourceChanged() const
//{
    //    const auto source = m_ui->sourceView->source();
    //    m_controller->modifyResource(source);
//}

//void MainWindow::on_actionZoomIn_triggered()
//{
    //    m_ui->graphicsView->setZoom(1.25);
//}

//void MainWindow::on_actionZoomOut_triggered()
//{
    //    m_ui->graphicsView->setZoom(1.0 / 1.25);
//}
