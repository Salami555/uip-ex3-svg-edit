
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


// ----- File/IO -----------------------------------------------
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


// --- State / Settings -----------------------------------------------
const short SETTING_MAX_RECENT_FILES_COUNT = 10;
const QString SETTING_RECENT_FILES = "last_state/used_files";
const QString SETTING_LAST_OPEN_PATH = "last_state/directories/open";
const QString SETTING_LAST_SAVE_PATH = "last_state/directories/save";
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

void MainWindow::updateRecentFileMenuList(const QList<QFileInfo> & files)
{
    if(files.isEmpty()) {
        m_ui->menu_RecentlyUsedFiles->setDisabled(true);
        return;
    }
    m_ui->menu_RecentlyUsedFiles->setEnabled(true);
    m_ui->menu_RecentlyUsedFiles->clear();
    for(auto file : files) {
        auto recentFileAction = new QAction(file.fileName(), m_ui->menu_RecentlyUsedFiles);
        recentFileAction->setEnabled(file.exists());

        recentFileAction->connect(recentFileAction, &QAction::triggered, this, [this, file]() {
            this->openFiles({file});
        });
        m_ui->menu_RecentlyUsedFiles->addAction(recentFileAction);
    }
}

void MainWindow::addRecentFile(const QFileInfo & file)
{
    if(!file.exists()) return;
    auto filename = file.absoluteFilePath();
    auto filenames = m_settings->value(SETTING_RECENT_FILES).toStringList();
    int index = filenames.indexOf(filename);
    if(index >= 0) {
        filenames.removeAt(index);
    } else if(filenames.size() >= SETTING_MAX_RECENT_FILES_COUNT) {
        filenames.removeLast();
    }
    filenames.push_front(filename);
    m_settings->setValue(SETTING_RECENT_FILES, filenames);

    this->updateRecentFileMenuList(names2fileinfos(filenames));
}

void MainWindow::restoreRecentFileList()
{
    auto filenames = m_settings->value(SETTING_RECENT_FILES).toStringList();
    this->updateRecentFileMenuList(names2fileinfos(filenames));
}

QString lastDirectoryOpenPath(const QSettings * settings)
{
    return settings->value(
        SETTING_LAST_OPEN_PATH,
        QFileInfo("./").absoluteFilePath()
    ).toString();
}

QString lastDirectorySavePath(const QSettings * settings)
{
    return settings->value(
        SETTING_LAST_SAVE_PATH,
        lastDirectoryOpenPath(settings)
    ).toString();
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
        this->setWindowTitle(dynamic_cast<Tab*>(widget)->name(true));
    }
}

void updateTabName(QTabWidget * tabView, const Tab * tab)
{
    // too lazy for good implementation
    for(int i = 0; i < tabView->count(); ++i) {
        if(tab == tabView->widget(i)) {
            tabView->setTabText(i, tab->name());
        }
    }
}

int indexOfTabForFile(const QTabWidget * tabs, QFileInfo &file)
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
                return i;
            }
        }
    }
    return -1;
}

void MainWindow::addTab(Tab * tab)
{
    setFontIfExists(m_settings, tab->sourceView());
    m_ui->tabView->addTab(tab, tab->name());
    m_ui->tabView->setCurrentIndex(m_ui->tabView->count() - 1);
    m_ui->actionSaveCurrentFile->setDisabled(true);
    connect(tab->resource(), &Resource::changed, this, &MainWindow::on_modifiedStatusChange);
    connect(tab->sourceView(), &SourceView::undoAvailable, m_ui->actionUndo, &QAction::setEnabled);
    connect(tab->sourceView(), &SourceView::redoAvailable, m_ui->actionRedo, &QAction::setEnabled);
    this->addRecentFile(tab->resource()->file());
}

void MainWindow::openFiles(const QList<QFileInfo> files)
{
    int newTabIndex = m_ui->tabView->count();
    bool tabAdded = false;
    for(auto file : files) {
        int index = indexOfTabForFile(m_ui->tabView, file);
        if(index > -1) {
            m_ui->tabView->setCurrentIndex(index);
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

    this->restoreRecentFileList();
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
void MainWindow::on_actionNewFile_triggered()
{
    auto newTab = new Tab(m_ui->tabView);
    newTab->loadEmptyTemplate();
    this->addTab(newTab);
    m_ui->actionSaveCurrentFile->setEnabled(true);
    m_ui->actionSaveAllFiles->setEnabled(true);
}

void MainWindow::on_actionOpenFiles_triggered()
{
    auto dialog = new QFileDialog(this);
    dialog->setWindowModality(Qt::WindowModality::ApplicationModal);
    dialog->setFileMode(QFileDialog::ExistingFiles);
    dialog->setViewMode(QFileDialog::Detail);
    dialog->setAcceptMode(QFileDialog::AcceptOpen);

    dialog->setMimeTypeFilters({ "image/svg+xml" });
    dialog->setNameFilter("Scalable Vector Graphic Files (*.svg);; All Files (*.*)");
    dialog->setDirectory(lastDirectoryOpenPath(m_settings));

    if(dialog->exec()) {
        m_settings->setValue(SETTING_LAST_OPEN_PATH, dialog->directory().path());
        auto files = preprocessFileList(names2fileinfos(dialog->selectedFiles()));
        this->openFiles(files);
    }
}

void MainWindow::on_actionSaveCurrentFile_triggered()
{
    auto widget = m_ui->tabView->currentWidget();
    if(instanceof<Tab>(widget)) {
        Tab *tab = dynamic_cast<Tab*>(widget);
        if(tab->resource()->hasFile()) {
            if(tab->saveFile()) {
                this->on_modifiedStatusChange();
                this->addRecentFile(tab->resource()->file());
            }
        } else {
            this->on_actionSaveCurrentFileAs_triggered();
        }
    }
}

void MainWindow::on_actionSaveCurrentFileAs_triggered()
{
    auto widget = m_ui->tabView->currentWidget();
    if(instanceof<Tab>(widget)) {
        Tab *tab = dynamic_cast<Tab*>(widget);

        auto dialog = new QFileDialog(this);
        dialog->setWindowModality(Qt::WindowModality::ApplicationModal);
        dialog->setFileMode(QFileDialog::AnyFile);
        dialog->setViewMode(QFileDialog::Detail);
        dialog->setAcceptMode(QFileDialog::AcceptSave);

        dialog->setMimeTypeFilters({ "image/svg+xml" });
        dialog->setNameFilter("Scalable Vector Graphic Files (*.svg);; All Files (*.*)");
        dialog->setDirectory(lastDirectorySavePath(m_settings));
        dialog->selectFile(tab->resource()->file().fileName());

        if(dialog->exec()) {
            m_settings->setValue(SETTING_LAST_SAVE_PATH, dialog->directory().path());
            QFileInfo file(dialog->selectedFiles().first());
            if(tab->saveFileAs(file)) {
                this->on_modifiedStatusChange();
                this->addRecentFile(tab->resource()->file());
            }
        }
    }
}

void MainWindow::on_actionSaveAllFiles_triggered()
{
    for(int i = 0; i < m_ui->tabView->count(); ++i) {
        auto widget = m_ui->tabView->widget(i);
        if(instanceof<Tab>(widget)) {
            Tab *tab = dynamic_cast<Tab*>(widget);
            if(tab->resource()->isUnsaved()) {
                if(tab->resource()->hasFile()) {
                    if(tab->saveFile()) {
                        updateTabName(m_ui->tabView, tab);
                    }
                } else {
                    m_ui->tabView->setCurrentIndex(i);
                    this->on_actionSaveCurrentFileAs_triggered();
                }
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
        const Tab *tab = dynamic_cast<const Tab*>(widget);

        m_ui->actionSaveCurrentFile->setEnabled(tab->resource()->isUnsaved());
        m_ui->actionSaveCurrentFileAs->setEnabled(true);
        m_ui->menu_Edit->setEnabled(true);
        m_ui->menu_View->setEnabled(true);
        this->updateWindowTitle();

        m_ui->actionSetLineWrap->disconnect(SIGNAL(triggered()));
        m_ui->actionSetLineWrap->setChecked(tab->sourceView()->hasWordWrap());
        connect(m_ui->actionSetLineWrap, &QAction::triggered, tab->sourceView(), &SourceView::setWordWrap);

        m_ui->actionSetSyntaxHighlighting->disconnect(SIGNAL(triggered()));
        m_ui->actionSetSyntaxHighlighting->setChecked(tab->sourceView()->hasHighlighting());
        connect(m_ui->actionSetSyntaxHighlighting, &QAction::triggered, tab->sourceView(), &SourceView::setHighlighting);

        m_ui->actionUndo->disconnect(SIGNAL(triggered()));
        m_ui->actionRedo->disconnect(SIGNAL(triggered()));
        m_ui->actionCut->disconnect(SIGNAL(triggered()));
        m_ui->actionCopy->disconnect(SIGNAL(triggered()));
        m_ui->actionPaste->disconnect(SIGNAL(triggered()));
        m_ui->actionSelectAll->disconnect(SIGNAL(triggered()));
        m_ui->actionFind->disconnect(SIGNAL(triggered()));
        m_ui->actionReplace->disconnect(SIGNAL(triggered()));
        m_ui->actionGoto->disconnect(SIGNAL(triggered()));
        connect(m_ui->actionUndo, &QAction::triggered, tab->sourceView(), &SourceView::undo);
        connect(m_ui->actionRedo, &QAction::triggered, tab->sourceView(), &SourceView::redo);
        connect(m_ui->actionCut, &QAction::triggered, tab->sourceView(), &SourceView::cut);
        connect(m_ui->actionCopy, &QAction::triggered, tab->sourceView(), &SourceView::copy);
        connect(m_ui->actionPaste, &QAction::triggered, tab->sourceView(), &SourceView::paste);
        connect(m_ui->actionSelectAll, &QAction::triggered, tab->sourceView(), &SourceView::selectAll);
        connect(m_ui->actionFind, &QAction::triggered, tab->sourceView(), &SourceView::find);
        connect(m_ui->actionReplace, &QAction::triggered, tab->sourceView(), &SourceView::replace);
        connect(m_ui->actionGoto, &QAction::triggered, tab->sourceView(), &SourceView::gotoLine);
        m_ui->actionUndo->setEnabled(tab->sourceView()->isUndoAvailable());
        m_ui->actionRedo->setEnabled(tab->sourceView()->isRedoAvailable());

        if(m_ui->actionGraphicViewToLeftSide->isEnabled() != tab->isDefaultPositioned()) {
            this->toggleAllSplitterPositionModifiers();
        }
    } else {
        m_ui->actionSaveCurrentFile->setDisabled(true);
        m_ui->actionSaveCurrentFileAs->setDisabled(true);
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
        this->addRecentFile(tab->resource()->file());
    }

    m_ui->tabView->removeTab(index);
    this->on_modifiedStatusChange();
}

void MainWindow::on_modifiedStatusChange()
{
    auto widget = m_ui->tabView->currentWidget();
    if(instanceof<Tab>(widget)) {
        const Tab * tab = dynamic_cast<Tab*>(widget);
        m_ui->actionSaveCurrentFile->setEnabled(tab->resource()->isUnsaved());
        updateTabName(m_ui->tabView, tab);
        this->updateWindowTitle();
    }
    m_ui->actionSaveAllFiles->setDisabled(isEveryFileSaved(m_ui->tabView));
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
