#include "mainwindow.h"
#include "v1541commander.h"
#include "v1541imgwidget.h"

#include <QCloseEvent>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>

class MainWindow::priv
{
    public:
        priv();
        Content content;
	QString filename;
};

MainWindow::priv::priv() :
    content(None),
    filename()
{}

MainWindow::MainWindow()
{
    d = new priv();
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(&cmdr.newAction());
    fileMenu->addAction(&cmdr.openAction());
    fileMenu->addAction(&cmdr.saveAction());
    fileMenu->addAction(&cmdr.saveAsAction());
    fileMenu->addAction(&cmdr.closeAction());
    fileMenu->addSeparator();
    fileMenu->addAction(&cmdr.aboutAction());
    fileMenu->addAction(&cmdr.exitAction());
    QMenu *cbmdosMenu = menuBar()->addMenu(tr("CBM &DOS"));
    cbmdosMenu->addAction(&cmdr.newFileAction());
    cbmdosMenu->addAction(&cmdr.deleteFileAction());
    QMenu *windowsMenu = menuBar()->addMenu(tr("&Windows"));
    windowsMenu->addAction(&cmdr.petsciiWindowAction());
    windowsMenu->addAction(&cmdr.logWindowAction());
    (void) statusBar();

    setWindowTitle(tr("V1541Commander: virtual 1541 disk commander"));
}

MainWindow::~MainWindow()
{
    delete d;
}

void MainWindow::contentSelectionChanged()
{
    emit selectionChanged();
}

void MainWindow::contentModified()
{
    setWindowModified(true);
}

void MainWindow::contentSaved()
{
    setWindowModified(false);
}

MainWindow::Content MainWindow::content() const 
{
    return d->content;
}

const QString &MainWindow::filename() const
{
    return d->filename;
}

bool MainWindow::hasValidContent() const
{
    switch (d->content)
    {
	V1541ImgWidget *imgWidget;

	case Content::Image:
	    imgWidget = static_cast<V1541ImgWidget *>(centralWidget());
	    return imgWidget->hasValidImage();

	default:
	    return false;
    }
}

bool MainWindow::hasValidSelection() const
{
    switch (d->content)
    {
	V1541ImgWidget *imgWidget;

	case Content::Image:
	    imgWidget = static_cast<V1541ImgWidget *>(centralWidget());
	    return imgWidget->hasValidSelection();

	default:
	    return false;
    }
}

bool MainWindow::event(QEvent *e)
{
    if (e->type() == QEvent::WindowActivate)
    {
        emit activated();
    }
    return QMainWindow::event(e);
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    closeDocument();
    if (isWindowModified()) e->ignore();
    else emit closed();
}

QSize MainWindow::sizeHint() const
{
    if (d->content == Content::None)
    {
        return QSize(480,200);
    }
    else
    {
        return QWidget::sizeHint();
    }
}

void MainWindow::newImage()
{
    V1541ImgWidget *imgWidget = new V1541ImgWidget(this);
    imgWidget->newImage();
    if (imgWidget->hasValidImage())
    {
	QWidget *current = centralWidget();
	setCentralWidget(imgWidget);
	delete current;
	setWindowTitle(tr("<new disk image>[*]"));
	d->content = Content::Image;
	connect(imgWidget, &V1541ImgWidget::selectionChanged,
		this, &MainWindow::contentSelectionChanged);
	connect(imgWidget, &V1541ImgWidget::modified,
		this, &MainWindow::contentModified);
	connect(imgWidget, &V1541ImgWidget::saved,
		this, &MainWindow::contentSaved);
	emit contentChanged();
	setWindowModified(false);
	adjustSize();
    }
    else
    {
	delete imgWidget;
    }
}

void MainWindow::openImage(const QString &imgFile)
{
    if (!imgFile.isEmpty())
    {
	V1541ImgWidget *imgWidget = new V1541ImgWidget(this);
	imgWidget->open(imgFile);
	if (imgWidget->hasValidImage())
	{
            QWidget *current = centralWidget();
            setCentralWidget(imgWidget);
            delete current;
            d->content = Content::Image;
	    d->filename = imgFile;
            setWindowTitle(QString(imgFile).append("[*]"));
	    connect(imgWidget, &V1541ImgWidget::selectionChanged,
		    this, &MainWindow::contentSelectionChanged);
	    connect(imgWidget, &V1541ImgWidget::modified,
		    this, &MainWindow::contentModified);
	    connect(imgWidget, &V1541ImgWidget::saved,
		    this, &MainWindow::contentSaved);
            emit contentChanged();
	    setWindowModified(false);
            adjustSize();
	}
	else
	{
            delete imgWidget;
	}
    }
}

void MainWindow::save(const QString &imgFile)
{
    switch (d->content)
    {
	V1541ImgWidget *imgWidget;

	case Content::Image:
	    imgWidget = static_cast<V1541ImgWidget *>(centralWidget());
	    imgWidget->save(imgFile.isEmpty() ? d->filename : imgFile);
	    if (d->filename.isEmpty())
	    {
		d->filename = imgFile;
		setWindowTitle(QString(imgFile).append("[*]"));
	    }
	    break;

	default:
	    break;
    }
}

void MainWindow::closeDocument()
{
    if (isWindowModified())
    {
	QMessageBox::StandardButton btn = QMessageBox::question(this,
		tr("Discard unsaved changes?"), QString(tr("%1 has unsaved "
			"changes. \nDo you want to save now?"))
		.arg(d->filename.isEmpty() ? "<new disk image>" : d->filename),
		QMessageBox::Save|QMessageBox::Cancel|QMessageBox::Discard,
		QMessageBox::Save);
	if (btn == QMessageBox::Cancel) return;
	if (btn == QMessageBox::Save)
	{
	    if (d->filename.isEmpty())
	    {
		emit activated();
		cmdr.saveAs();
	    }
	    else
	    {
		save(QString());
	    }
	}
    }
    QWidget *current = centralWidget();
    setCentralWidget(0);
    delete current;
    d->content = Content::None;
    emit contentChanged();
    setWindowModified(false);
    adjustSize();
    setWindowTitle(tr("V1541Commander: virtual 1541 disk commander"));
}

void MainWindow::newFile()
{
    if (d->content != Content::Image) return;
    static_cast<V1541ImgWidget *>(centralWidget())->newFile();
}

void MainWindow::deleteFile()
{
    if (d->content != Content::Image) return;
    static_cast<V1541ImgWidget *>(centralWidget())->deleteFile();
}

void MainWindow::showStatusLine(const QString &line)
{
    statusBar()->showMessage(line);
}
