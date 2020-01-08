#include "v1541imgwidget.h"
#include "v1541commander.h"
#include "cbmdosfsmodel.h"
#include "cbmdosfsoptionsdialog.h"
#include "cbmdosfswidget.h"
#include "cbmdosfilewidget.h"
#include "utils.h"

#include <QFileInfo>
#include <QHBoxLayout>
#include <QListView>
#include <QMessageBox>
#include <QVBoxLayout>

#include <1541img/d64.h>
#include <1541img/d64reader.h>
#include <1541img/d64writer.h>
#include <1541img/cbmdosfile.h>
#include <1541img/cbmdosfs.h>
#include <1541img/cbmdosvfs.h>
#include <1541img/cbmdosvfsreader.h>
#include <1541img/zc45compressor.h>
#include <1541img/zc45extractor.h>
#include <1541img/zcfileset.h>

class V1541ImgWidget::priv
{
    public:
	priv();
	CbmdosFs *fs;
	CbmdosFsModel model;
	QListView dirList;
	CbmdosFsWidget fsprop;
	CbmdosFileWidget file;
};

V1541ImgWidget::priv::priv() :
    fs(0),
    model(),
    dirList(),
    fsprop(),
    file()
{}

V1541ImgWidget::V1541ImgWidget(QWidget *parent) : QWidget(parent)
{
    d = new priv();
    QHBoxLayout *layout = new QHBoxLayout(this);
    QVBoxLayout *propLayout = new QVBoxLayout();
    d->dirList.setMinimumHeight(200);
    d->dirList.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    d->dirList.setModel(&d->model);
    d->dirList.setDragEnabled(true);
    d->dirList.setAcceptDrops(true);
    d->dirList.setDragDropMode(QAbstractItemView::InternalMove);
    d->dirList.addAction(&cmdr.newFileAction());
    d->dirList.addAction(&cmdr.deleteFileAction());
    d->dirList.setContextMenuPolicy(Qt::ActionsContextMenu);
    layout->addWidget(&d->dirList);
    propLayout->addWidget(&d->fsprop);
    propLayout->addWidget(&d->file);
    propLayout->addStretch();
    layout->addLayout(propLayout);
    connect(d->dirList.selectionModel(), &QItemSelectionModel::currentChanged,
	    this, &V1541ImgWidget::selected);
    connect(&d->model,
	    SIGNAL(selectedIndexChanged(const QModelIndex &,
		    QItemSelectionModel::SelectionFlags)),
	    d->dirList.selectionModel(),
	    SLOT(setCurrentIndex(const QModelIndex &,
		    QItemSelectionModel::SelectionFlags)));
    connect(&d->model, &CbmdosFsModel::modified,
	    this, &V1541ImgWidget::modelModified);
}

V1541ImgWidget::~V1541ImgWidget()
{
    CbmdosFs *fs = d->fs;
    delete d;
    CbmdosFs_destroy(fs);
}

void V1541ImgWidget::selected(const QModelIndex &current,
	const QModelIndex &previous)
{
    (void) previous;

    CbmdosFile *file = 0;
    if (current.row() > 0 && d->fs)
    {
	CbmdosVfs *vfs = CbmdosFs_vfs(d->fs);
	if (current.row() <= (int)CbmdosVfs_fileCount(vfs))
	{
	    file = CbmdosVfs_file(vfs, current.row()-1);
	}
    }
    d->file.setFile(file);
    emit selectionChanged();
}

void V1541ImgWidget::modelModified()
{
    emit modified();
}

void V1541ImgWidget::newImage()
{
    CbmdosFs_destroy(d->fs);
    d->fs = 0;
    CbmdosFsOptions opts = CFO_DEFAULT;
    CbmdosFsOptionsDialog optDlg(&opts, parentWidget());
    optDlg.setWindowTitle(QString(tr("Options for new image")));
    if (optDlg.exec() == QDialog::Accepted)
    {
	d->fs = CbmdosFs_create(opts);
	d->model.setFs(d->fs);
	d->fsprop.setFs(d->fs);
	d->dirList.setMinimumWidth(
		d->dirList.sizeHintForColumn(0)
		+ 2 * d->dirList.frameWidth());
	d->dirList.setMinimumHeight(
		d->dirList.sizeHintForRow(0) * 10
		+ 2 * d->dirList.frameWidth());
    }
}

void V1541ImgWidget::open(const QString& filename)
{
    CbmdosFs_destroy(d->fs);
    d->fs = 0;

    D64 *d64 = 0;
    bool zipcode = false;

    QFileInfo fileInfo(filename);
    if (fileInfo.baseName().indexOf('!') == 1 &&
	    !(fileInfo.suffix().toUpper() == "D64"))
    {
	ZcFileSet *fileset = ZcFileSet_fromFile(qfname(filename));
	if (fileset)
	{
	    d64 = extractZc45(fileset);
	    ZcFileSet_destroy(fileset);
	}
	zipcode = true;
    }
    else
    {
	ZcFileSet *fileset = ZcFileSet_fromFile(qfname(filename));
	if (fileset)
	{
	    if (QMessageBox::question(window(),
			tr("Extract Zipcode?"),
			tr("This image seems to contain a set of Zipcode "
			    "files. Do you want to extract them instead?"),
			QMessageBox::Yes|QMessageBox::No)
		    == QMessageBox::Yes)
	    {
		d64 = extractZc45(fileset);
		zipcode = true;
	    }
	    ZcFileSet_destroy(fileset);
	}
	if (!zipcode)
	{
	    FILE *d64file = qfopen(filename, "rb");
	    if (d64file)
	    {
		d64 = readD64(d64file);
		fclose(d64file);
	    }
	}
    }

    if (d64)
    {
	CbmdosFsOptions opts = CFO_DEFAULT;
	int proberc = probeCbmdosFsOptions(&opts, d64);
	if (proberc < 0)
	{
	    opts.flags = (CbmdosFsFlags)
		(opts.flags | CbmdosFsFlags::CFF_RECOVER);
	    proberc = probeCbmdosFsOptions(&opts, d64);
	}
	if (proberc == 0)
	{
	    CbmdosFsOptionsDialog optDlg(&opts, parentWidget(), false);
	    optDlg.setWindowTitle(QString(tr("Options for "))
		    .append(filename));
	    if (D64_type(d64) == D64Type::D64_STANDARD)
	    {
		optDlg.disable42Tracks();
		optDlg.disable40Tracks();
	    }
	    else if (D64_type(d64) == D64Type::D64_40TRACK)
	    {
		optDlg.disable42Tracks();
	    }
	    if (opts.flags & CFF_42TRACK)
	    {
		optDlg.disable35Tracks();
		optDlg.disable40Tracks();
	    }
	    else if (opts.flags & CFF_40TRACK)
	    {
		optDlg.disable35Tracks();
	    }
	    if ((D64_type(d64) != D64Type::D64_STANDARD)
		    && !(opts.flags & (CFF_40TRACK|CFF_42TRACK)))
	    {
		opts.flags = (CbmdosFsFlags)
		    (opts.flags | CFF_40TRACK);
		optDlg.reset();
	    }
	    optDlg.disableZeroFree();
	    optDlg.exec();
	    d->fs = CbmdosFs_fromImage(d64, opts);
	    if (d->fs)
	    {
		d->model.setFs(d->fs);
		d->fsprop.setFs(d->fs);
		d->dirList.setMinimumWidth(
			d->dirList.sizeHintForColumn(0)
			+ 2 * d->dirList.frameWidth());
		int minItems = d->model.rowCount();
		if (minItems < 10) minItems = 10;
		if (minItems > 40) minItems = 40;
		d->dirList.setMinimumHeight(
			d->dirList.sizeHintForRow(0) * minItems
			+ 2 * d->dirList.frameWidth());
		if (opts.flags & CFF_RECOVER)
		{
		    CbmdosFs_rewrite(d->fs);
		    emit modified();
		}
		if (zipcode) emit modified();
	    }
	}
	if (!d->fs)
	{
	    D64_destroy(d64);
	}
    }
}

void V1541ImgWidget::openVfs(CbmdosVfs *vfs)
{
    CbmdosFs_destroy(d->fs);
    d->fs = CbmdosFs_fromVfs(vfs, CFO_DEFAULT);
    if (d->fs)
    {
	d->model.setFs(d->fs);
	d->fsprop.setFs(d->fs);
	d->dirList.setMinimumWidth(
		d->dirList.sizeHintForColumn(0)
		+ 2 * d->dirList.frameWidth());
	int minItems = d->model.rowCount();
	if (minItems < 10) minItems = 10;
	if (minItems > 40) minItems = 40;
	d->dirList.setMinimumHeight(
		d->dirList.sizeHintForRow(0) * minItems
		+ 2 * d->dirList.frameWidth());
    }
    else
    {
	CbmdosVfs_destroy(vfs);
    }
}

void V1541ImgWidget::save(const QString& filename)
{
    if (!hasValidImage()) return;
    FILE *d64file = qfopen(filename, "wb");
    if (d64file)
    {
	if (writeD64(d64file, CbmdosFs_image(d->fs)) < 0)
	{
	    QMessageBox::critical(this, tr("Error writing file"),
		    tr("The selected file couldn't be written."));
	}
	else
	{
	    emit saved();
	}
	fclose(d64file);
    }
    else
    {
	QMessageBox::critical(this, tr("Error opening file"),
		tr("The selected file couldn't be opened for writing."));
    }
}

void V1541ImgWidget::exportZipcode(const QString& filename)
{
    if (!hasValidImage()) return;
    ZcFileSet *fileset = compressZc45(CbmdosFs_image(d->fs));
    if (fileset)
    {
	if (ZcFileSet_save(fileset, qfname(filename)) < 0)
	{
	    QMessageBox::critical(this, tr("Error saving zipcode files"),
		    tr("The compressed files couldn't be written."));
	}
	ZcFileSet_destroy(fileset);
    }
    else
    {
	QMessageBox::critical(this, tr("Error compressing to Zipcode"),
		tr("compressing the image as Zipcode failed."));
    }
}

CbmdosVfs *V1541ImgWidget::exportZipcodeVfs()
{
    if (!hasValidImage()) return 0;
    ZcFileSet *fileset = compressZc45(CbmdosFs_image(d->fs));
    if (fileset)
    {
	CbmdosVfs *vfs = CbmdosVfs_create();
	if (ZcFileSet_saveVfs(fileset, vfs) < 0)
	{
	    QMessageBox::critical(this, tr("Error writing zipcode files"),
		    tr("The compressed files couldn't be written."));
	    ZcFileSet_destroy(fileset);
	    CbmdosVfs_destroy(vfs);
	    return 0;
	}
	ZcFileSet_destroy(fileset);
	return vfs;
    }
    else
    {
	QMessageBox::critical(this, tr("Error compressing to Zipcode"),
		tr("compressing the image as Zipcode failed."));
	return 0;
    }
}

void V1541ImgWidget::fsOptions()
{
    if (!hasValidImage()) return;
    CbmdosFsOptions opts = CbmdosFs_options(d->fs);
    CbmdosFsOptionsDialog optDlg(&opts, parentWidget());
    if (optDlg.exec() == QDialog::Accepted)
    {
	CbmdosFs_setOptions(d->fs, opts);
    }
}

void V1541ImgWidget::rewriteImage()
{
    if (!hasValidImage()) return;
    QMessageBox::StandardButton reply = QMessageBox::question(this,
	    tr("Rewrite the image?"), tr("When rewriting an image from "
		"scratch, you might lose data that isn't stored in standard "
		"CBM DOS format on the disk, like for example raw sectors in "
		"demos or games. Are you sure you want to proceed?"),
	    QMessageBox::Ok|QMessageBox::Cancel);
    if (reply == QMessageBox::Ok)
    {
	CbmdosFs_rewrite(d->fs);
	emit modified();
    }
}

void V1541ImgWidget::newFile()
{
    if (!hasValidImage()) return;
    CbmdosFile *newFile = CbmdosFile_create();
    const QModelIndex &index = d->dirList.selectionModel()->currentIndex();
    d->model.addFile(index, newFile);
}

void V1541ImgWidget::deleteFile()
{
    if (!hasValidImage() || !hasValidSelection()) return;
    QMessageBox::StandardButton reply = QMessageBox::question(this,
	    tr("Delete this file?"), tr("A deleted file cannot be restored. "
	    "Are you sure you want to delete this file now?"),
	    QMessageBox::Ok|QMessageBox::Cancel);
    if (reply == QMessageBox::Ok)
    {
	const QModelIndex &index = d->dirList.selectionModel()->currentIndex();
	d->model.deleteAt(index);
    }
}

bool V1541ImgWidget::hasValidImage() const
{
    return d->fs;
}

bool V1541ImgWidget::hasValidSelection() const
{
    const QModelIndex &index = d->dirList.selectionModel()->currentIndex();
    if (!index.isValid()) return false;
    return (index.row() > 0 && index.row() < d->model.rowCount() - 1);
}
