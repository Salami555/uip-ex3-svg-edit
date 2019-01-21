
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QActionGroup>
#include <QtUiTools>
#include <QFileDialog>
#include <QErrorMessage>
#include <QDropEvent>
#include <QMimeDatabase>
#include <QMimeData>
#include <QMessageBox>
#include <QFontDialog>
#include <QDebug>

#include "resource.h"

#include "tab.h"
#include "graphicsview.h"
#include "sourceview.h"


MainWindow::MainWindow(QWidget * parent) :
    QMainWindow(parent),
    m_ui(new Ui::MainWindow)
{
    m_ui->setupUi(this);
    QSettings::setDefaultFormat(QSettings::IniFormat);
    m_settings = new QSettings();

    connect(m_ui->actionSourceViewToLeftSide, &QAction::triggered, this, &MainWindow::swapCurrentTabSplitterContent);
    connect(m_ui->actionSourceViewToRightSide, &QAction::triggered, this, &MainWindow::swapCurrentTabSplitterContent);
    connect(m_ui->actionGraphicViewToLeftSide, &QAction::triggered, this, &MainWindow::swapCurrentTabSplitterContent);
    connect(m_ui->actionGraphicViewToRightSide, &QAction::triggered, this, &MainWindow::swapCurrentTabSplitterContent);

    connect(m_ui->tabView, &QTabWidget::currentChanged, this, &MainWindow::on_tabSelected);
    connect(m_ui->tabView, &QTabWidget::tabCloseRequested, this, &MainWindow::on_tabCloseRequested);
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
        "Unsaved files",
        "Do you want to discard the current changes?",
        QMessageBox::Yes|QMessageBox::No,
        QMessageBox::No
    );
    return reply == QMessageBox::Yes;
}

void MainWindow::updateWindowTitle()
{
    auto widget = m_ui->tabView->currentWidget();
    if(instanceof<Tab>(widget)) {
        this->setWindowTitle(dynamic_cast<Tab*>(widget)->name());
    }
}

const QMimeDatabase db;
bool isSVGFile(const QFileInfo & file)
{
    auto mimeType = db.mimeTypeForFile(file);
    return mimeType.name() == "image/svg+xml";
}
QList<QFileInfo> filterSVGFiles(const QList<QFileInfo> & allFiles)
{
    QList<QFileInfo> svgFiles;
    for(auto file : allFiles) {
        if(isSVGFile(file)) {
            svgFiles.append(file);
        }
    }
    return svgFiles;
}
QList<QFileInfo> filterExistingPaths(const QList<QFileInfo> & allFiles)
{
    QList<QFileInfo> existingFiles;
    for(auto file : allFiles) {
        if(file.exists()) {
            existingFiles.append(file);
        }
    }
    return existingFiles;
}
QList<QFileInfo> names2fileinfos(const QStringList & filenames, const QString & path = "")
{
    QList<QFileInfo> files;
    for(auto filename : filenames) {
        if(!path.isEmpty()) {
            filename = path + QDir::separator() + filename;
        }
        QFileInfo file(filename);
        files.append(file);
    }
    return files;
}
QList<QFileInfo> urls2fileinfos(const QList<QUrl> & fileURLs)
{
    QList<QFileInfo> files;
    for(auto url : fileURLs) {
        QFileInfo file(url.toLocalFile());
        files.append(file);
    }
    return files;
}
QList<QFileInfo> directoryExtractor(const QList<QFileInfo> & filesAndDirs)
{
    QList<QFileInfo> files;
    for(auto fileOrDir : filesAndDirs) {
        if(fileOrDir.isFile()) {
            files.append(fileOrDir);
        } else if(fileOrDir.isDir()) {
            // workaround, because final / is missing with .dir()
            auto path = fileOrDir.filePath();
            if(!path.endsWith(QDir::separator())) {
                path.append(QDir::separator());
            }
            QDir dir(path);
            // end workaround

            auto filenames = dir.entryList(QDir::Files | QDir::NoDotAndDotDot);
            files.append(names2fileinfos(filenames, dir.path()));
        }
    }
    return files;
}
QList<QFileInfo> preprocessFileList(const QList<QFileInfo> & files)
{
    return filterSVGFiles(directoryExtractor(filterExistingPaths(files)));
}


// --- Settings -----------------------------------------------
const QString SETTING_RECENT_FILES = "last_state/used_files";
const QString SETTING_LAST_DIRECTORY_OPEN = "last_state/directories/open";
const QString SETTING_LAST_DIRECTORY_SAVE = "last_state/directories/save";
const QString SETTING_STYLE_DEFAULT_FONT = "style/default_font";
const QString SETTING_STYLE_WORD_WRAP = "style/word_wrap";
const QString SETTING_STYLE_HIGHLIGHTING = "style/highlighting";

void saveDefaultFont(QSettings * settings, const QFont & font, const QString & key = SETTING_STYLE_DEFAULT_FONT)
{
    settings->setValue(key, font.toString());
}

void setFontIfExists(QSettings * settings, SourceView * sourceView, const QString & key = SETTING_STYLE_DEFAULT_FONT)
{
    if(settings->contains(key)) {
        QFont font;
        font.fromString(settings->value(key).toString());
        sourceView->setFont(font);
    }
}


// ----- Events -----------------------------------------------
void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    const QMimeData* mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        auto fileList = preprocessFileList(urls2fileinfos(mimeData->urls()));
        if(!fileList.isEmpty()) {
            event->acceptProposedAction();
        }
    }
}

void MainWindow::dropEvent(QDropEvent * event)
{
    const QMimeData* mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        auto fileList = preprocessFileList(urls2fileinfos(mimeData->urls()));
        this->openFiles(fileList);
    }
}

void openWelcomeTab(QTabWidget * tabView)
{
    QUiLoader loader;
    QFile file(":tab-welcome");
    file.open(QFile::ReadOnly);
    QWidget * tab = loader.load(&file, tabView);
    file.close();
    tabView->insertTab(0, tab, "Welcome");
    tabView->setCurrentIndex(0);
}

void MainWindow::showEvent(QShowEvent * event)
{
    QMainWindow::showEvent(event);
    qApp->setApplicationDisplayName(qApp->applicationName());
    this->setWindowTitle(qApp->applicationName());

    openWelcomeTab(m_ui->tabView);

    QStringList args(qApp->arguments());
    args.removeFirst();
    auto files = preprocessFileList(names2fileinfos(args));
    this->openFiles(files);
}

bool isEveryFileSaved(const QTabWidget * tabView)
{
    for(int i = 0; i < tabView->count(); ++i) {
        auto widget = tabView->widget(i);
        if(instanceof<Tab>(widget)) {
            const Tab *tab = dynamic_cast<const Tab*>(widget);
            if(tab->resource()->isUnsaved()) {
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
    qApp->exit();
}


// ----- SLOTS -----------------------------------------------
void MainWindow::on_actionOpenFiles_triggered()
{
    auto dialog = new QFileDialog(this);
    dialog->setWindowModality(Qt::WindowModality::ApplicationModal);
    dialog->setFileMode(QFileDialog::ExistingFiles);
    dialog->setViewMode(QFileDialog::Detail);
    dialog->setAcceptMode(QFileDialog::AcceptOpen);

    dialog->setMimeTypeFilters({ "image/svg+xml" });
    dialog->setNameFilter("Scalable Vector Graphic Files (*.svg);; All Files (*.*)");
    dialog->setDirectory(m_settings->value(
        SETTING_LAST_DIRECTORY_OPEN,
        QFileInfo("./").absoluteFilePath()
    ).toString());

    if(dialog->exec()) {
        m_settings->setValue(SETTING_LAST_DIRECTORY_OPEN, dialog->directory().path());
        auto files = preprocessFileList(names2fileinfos(dialog->selectedFiles()));
        this->openFiles(files);
    }
}

bool isFileOpened(const QTabWidget * tabs, QFileInfo &file)
{
    for(int i = 0; i < tabs->count(); ++i) {
        auto widget = tabs->widget(i);
        if(instanceof<Tab>(widget)) {
            const Tab *tab = dynamic_cast<const Tab*>(widget);
            const Resource *res = tab->resource();
            if(!res->hasFile()) {
                continue;
            }
            if(res->file() == file) {
                return true;
            }
        }
    }
    return false;
}

void MainWindow::addTab(Tab * tab)
{
    setFontIfExists(m_settings, tab->sourceView());
    m_ui->tabView->addTab(tab, tab->name());
    connect(tab->resource(), &Resource::changed, this, &MainWindow::on_modifiedStatusChange);
}

void MainWindow::openFiles(const QList<QFileInfo> files)
{
    int newTabIndex = m_ui->tabView->count();
    bool tabAdded = false;
    for(auto file : files) {
        if(isFileOpened(m_ui->tabView, file)) {
            continue;
        }

        auto newTab = new Tab(m_ui->tabView);
        if(newTab->loadFile(file)) {
            this->addTab(newTab);
            tabAdded = true;
        }
    }
    if(tabAdded) {
        m_ui->tabView->setCurrentIndex(newTabIndex);
    }
}

void MainWindow::on_actionSaveCurrentFile_triggered()
{
    auto widget = m_ui->tabView->currentWidget();
    if(instanceof<Tab>(widget)) {
        Tab *tab = dynamic_cast<Tab*>(widget);
        if(tab->saveFile()) {
            this->on_modifiedStatusChange();
        }
    }
}

void MainWindow::on_actionSaveCurrentFileAs_triggered()
{

}

void MainWindow::on_actionSaveAllFiles_triggered()
{
    for(int i = 0; i < m_ui->tabView->count(); ++i) {
        auto widget = m_ui->tabView->widget(i);
        if(instanceof<Tab>(widget)) {
            const Tab *tab = dynamic_cast<const Tab*>(widget);
            if(tab->resource()->isUnsaved()) {

            }
        }
    }
    this->setWindowModified(false);
    this->on_modifiedStatusChange();
}

void MainWindow::on_actionCloseCurrentFile_triggered()
{
    this->on_tabCloseRequested(m_ui->tabView->currentIndex());
}

void MainWindow::on_actionCloseAllFiles_triggered()
{
    bool closeConfirmed = false;
    if(!isEveryFileSaved(m_ui->tabView)) {
        closeConfirmed = confirmDiscardingUnsavedChanges(this);
    }

    for(int i = 0; i < m_ui->tabView->count(); ++i) {
        auto widget = m_ui->tabView->widget(i);
        if(instanceof<Tab>(widget)) {
            const Tab *tab = dynamic_cast<const Tab*>(widget);
            if(!tab->resource()->isUnsaved() || closeConfirmed) {
                m_ui->tabView->removeTab(i--);
            }
        } else {
            m_ui->tabView->removeTab(i--);
        }
    }
}

void MainWindow::on_actionExit_triggered()
{
    if(!isEveryFileSaved(m_ui->tabView)) {
        if(!confirmDiscardingUnsavedChanges(this)) {
            return;
        }
    }
    qApp->exit();
}

void MainWindow::on_actionChangeFont_triggered()
{
    const Tab *tab = dynamic_cast<const Tab*>(m_ui->tabView->currentWidget());
    QFontDialog fontChooser(tab->sourceView()->font(), this);
    fontChooser.setOption(QFontDialog::MonospacedFonts);
    if(fontChooser.exec()) {
        saveDefaultFont(m_settings, fontChooser.selectedFont());
        tab->sourceView()->setFont(fontChooser.selectedFont());
    }
}

void MainWindow::toggleAllSplitterPositionModifiers()
{
    m_ui->actionSourceViewToLeftSide->setDisabled(m_ui->actionSourceViewToLeftSide->isEnabled());
    m_ui->actionSourceViewToRightSide->setDisabled(m_ui->actionSourceViewToRightSide->isEnabled());
    m_ui->actionGraphicViewToLeftSide->setDisabled(m_ui->actionGraphicViewToLeftSide->isEnabled());
    m_ui->actionGraphicViewToRightSide->setDisabled(m_ui->actionGraphicViewToRightSide->isEnabled());
}

void MainWindow::swapCurrentTabSplitterContent()
{
    auto widget = m_ui->tabView->currentWidget();
    if(instanceof<Tab>(widget)) {
        dynamic_cast<Tab*>(widget)->swapContentPositions();
        this->toggleAllSplitterPositionModifiers();
    }
}

void MainWindow::on_actionWelcomeTab_triggered()
{
    openWelcomeTab(m_ui->tabView);
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(
        this, "About SVG-Edit",
        "This application was designed and written for the User Interface Programming course at the Hasso Plattner Institute Potsdam.\nThis is the contribution for exercise 3 by Maximilian Stiede.\nIt is horrible simple, thus please don't look at it and especially do _not_ use it in real life matters *!\nGood luck trying to use it ;)\n(c) 2019 Maximilian Stiede\n\n(*Beware of memory leaks)"
    );
}

void MainWindow::on_tabSelected()
{
    auto widget = m_ui->tabView->currentWidget();
    if(instanceof<Tab>(widget)) {
        m_ui->menu_Edit->setEnabled(true);
        m_ui->menu_View->setEnabled(true);
        const Tab *tab = dynamic_cast<const Tab*>(widget);
        this->updateWindowTitle();

        m_ui->actionSetLineWrap->disconnect(SIGNAL(triggered()));
        m_ui->actionSetLineWrap->setChecked(tab->sourceView()->hasWordWrap());
        connect(m_ui->actionSetLineWrap, &QAction::triggered, tab->sourceView(), &SourceView::setWordWrap);

        m_ui->actionSetSyntaxHighlighting->disconnect(SIGNAL(triggered()));
        m_ui->actionSetSyntaxHighlighting->setChecked(tab->sourceView()->hasHighlighting());
        connect(m_ui->actionSetSyntaxHighlighting, &QAction::triggered, tab->sourceView(), &SourceView::setHighlighting);

        if(m_ui->actionGraphicViewToLeftSide->isEnabled() != tab->isDefaultPositioned()) {
            this->toggleAllSplitterPositionModifiers();
        }
    } else {
        m_ui->menu_Edit->setDisabled(true);
        m_ui->menu_View->setDisabled(true);
        this->setWindowTitle(qApp->applicationName());
    }
}

void MainWindow::on_tabCloseRequested(int index)
{
    auto widget = m_ui->tabView->widget(index);
    if(instanceof<Tab>(widget)) {
        Tab *tab = dynamic_cast<Tab*>(widget);
        if(tab->resource()->isUnsaved()) {
            if(!confirmDiscardingUnsavedChanges(dynamic_cast<QWidget *>(tab))) {
                return;
            }
        }
    }

    m_ui->tabView->removeTab(index);
}

void MainWindow::on_modifiedStatusChange()
{
    auto widget = m_ui->tabView->currentWidget();
    if(instanceof<Tab>(widget)) {
        QString name = dynamic_cast<Tab*>(widget)->name();
        m_ui->tabView->setTabText(
            m_ui->tabView->currentIndex(),
            name
        );
        this->updateWindowTitle();
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

//void MainWindow::on_actionZoomIn_triggered()
//{
    //    m_ui->graphicsView->setZoom(1.25);
//}

//void MainWindow::on_actionZoomOut_triggered()
//{
    //    m_ui->graphicsView->setZoom(1.0 / 1.25);
//}
