
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QActionGroup>
#include <QSplitter>
#include <QFileDialog>
#include <QErrorMessage>
#include <QDropEvent>
#include <QMimeData>
#include <QMessageBox>

#include "model.h"
#include "resource.h"
#include "controller.h"

#include "graphicsview.h"
#include "sourceview.h"

#include <QDebug>


MainWindow::MainWindow(QWidget * parent) :
    QMainWindow(parent),
    m_ui(new Ui::MainWindow)
{
    m_ui->setupUi(this);

    //    m_ui->graphicsView->setFitView(true);
    //    m_ui->sourceView->setHighlighting(true);
    //    m_ui->sourceView->setWordWrap(true);

    // connect(m_ui->actionViewSource, SIGNAL(triggered()), this, SLOT(foobar1()));
    // connect(m_ui->actionViewSource, &QAction::triggered,[]() { qDebug() << "test mit lambda"; });
    // connect(m_ui->actionSelectAll, &QAction::triggered, m_ui->sourceView, &SourceView::selectAll);

    //    connect(m_ui->actionSyntaxHighlighting, &QAction::triggered, m_ui->sourceView, &SourceView::setHighlighting);
    //    connect(m_ui->actionWordWrap, &QAction::triggered, m_ui->sourceView, &SourceView::setWordWrap);
}

MainWindow::~MainWindow()
{
    delete m_ui;
}


void MainWindow::setModel(const Model * model)
{
    if(m_model == model) {
        return;
    }

    if(m_model != nullptr) {
        // disconnect();
    }
    m_model = model;

    if(m_model!= nullptr) {
        connect(m_model, &Model::resourceOpened, this, &MainWindow::onResourceOpenend);
        connect(m_model, &Model::resourceClosed, this, &MainWindow::onResourceClosed);
        connect(m_model, &Model::resourceModified, this, &MainWindow::onResourceModified);
        connect(m_model, &Model::operationFailed, this, &MainWindow::onResourceOperationFailed);
    }
}

const Model * MainWindow::model() const
{
    return m_model;
}


void MainWindow::setController(const Controller * controller)
{
    if(m_controller == controller) {
        return;
    }

    if(m_controller != nullptr) {
        m_controller->disconnect();
    }
    m_controller = controller;

    if(m_controller != nullptr) {
        connect(m_ui->actionNewFile, &QAction::triggered, m_controller, &Controller::newResource);
        connect(m_ui->actionSaveFile, &QAction::triggered, m_controller, &Controller::saveResource);
        connect(m_ui->actionCloseFile, &QAction::triggered, m_controller, &Controller::closeResource);

        connect(m_ui->actionExit, &QAction::triggered, m_controller, &Controller::exit);       
    }
}

const Controller * MainWindow::controller() const
{
    return m_controller;
}


void MainWindow::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasUrls())
        event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent * event) {
    const QMimeData* mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        QList<QFileInfo> files;
        for(QUrl url : mimeData->urls()) {
            files.append(QFileInfo(url.toLocalFile()));
        }
        this->controller()->openResources(files);
    }
}

void MainWindow::showEvent(QShowEvent * event) {
    QMainWindow::showEvent(event);
    QStringList args = qApp->arguments();
    QList<QFileInfo> files;
    for(short i = 1; i < args.size(); i++) {
        files.append(QFileInfo(args[i]));
    }
    this->controller()->openResources(files);
}

void MainWindow::closeEvent(QCloseEvent * event)
{
    m_ui->actionExit->trigger();
    Q_UNUSED(event);
}

void MainWindow::onResourceOpenend()
{
    auto alert = new QMessageBox(this);
    alert->setText("opened resource: " + m_model->resource()->fileInfo().fileName());
    alert->showNormal();
    //    m_ui->sourceView->setResource(m_model->resource());
    //    m_ui->graphicsView->setResource(m_model->resource());
}

void MainWindow::onResourceClosed()
{
    //    m_ui->sourceView->setResource(nullptr);
    //    m_ui->graphicsView->setResource(nullptr);
}

void MainWindow::onResourceModified()
{
    //    m_ui->graphicsView->reloadFromResource();
    //m_ui->graphicsView->setResource(m_model->resource());
}

void MainWindow::onResourceOperationFailed(const ResourceOperationResult result)
{
    auto errorDialog = new QErrorMessage(this);
    errorDialog->showMessage(QString("A Problem occurred: %1").arg(Resource::operationResultString(result)));
}


void MainWindow::on_actionSwapViews_triggered()
{
    //    const auto widget0 = m_ui->splitter->widget(0);
    //    const auto widget1 = m_ui->splitter->widget(1);

    //    const auto sizes = m_ui->splitter->sizes();

    //    m_ui->splitter->insertWidget(0, widget1);
    //    m_ui->splitter->insertWidget(1, widget0);

    //    m_ui->splitter->setSizes(sizes);
}

void MainWindow::on_actionOpenFile_triggered()
{
    auto dialog = new QFileDialog(this);
    dialog->setFileMode(QFileDialog::ExistingFile);
    dialog->setViewMode(QFileDialog::Detail);
    dialog->setAcceptMode(QFileDialog::AcceptOpen);

    dialog->setMimeTypeFilters({ "image/svg+xml" });
    dialog->setNameFilter("Scalable Vector Graphic Files (*.svg);; All Files (*.*)");

    dialog->open(const_cast<Controller *>(m_controller), SLOT(openResource(const QString &)));
}

void MainWindow::on_actionFitView_toggled(bool enabled) const
{
    //    m_ui->graphicsView->setFitView(enabled);
}

void MainWindow::on_graphicsView_fitViewChanged(bool enabled) const
{
//    m_ui->actionFitView->blockSignals(true);
//    m_ui->actionFitView->setChecked(enabled);
//    m_ui->actionFitView->blockSignals(false);
}

void MainWindow::on_sourceView_highlightChanged(bool enabled) const
{
//    m_ui->actionSyntaxHighlighting->blockSignals(true);
//    m_ui->actionSyntaxHighlighting->setChecked(enabled);
//    m_ui->actionSyntaxHighlighting->blockSignals(false);
}

void MainWindow::on_sourceView_wordWrapChanged(bool enabled) const
{
//    m_ui->actionWordWrap->blockSignals(true);
//    m_ui->actionWordWrap->setChecked(enabled);
//    m_ui->actionWordWrap->blockSignals(false);
}

void MainWindow::on_sourceView_sourceChanged() const
{
    //    const auto source = m_ui->sourceView->source();
    //    m_controller->modifyResource(source);
}

void MainWindow::on_actionZoomIn_triggered()
{
    //    m_ui->graphicsView->setZoom(1.25);
}

void MainWindow::on_actionZoomOut_triggered()
{
    //    m_ui->graphicsView->setZoom(1.0 / 1.25);
}
