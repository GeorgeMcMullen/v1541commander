#include "aboutbox.h"

#include <QCoreApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

class AboutBox::priv
{
    public:
	priv();
	QVBoxLayout mainLayout;
	QHBoxLayout contentLayout;
	QVBoxLayout textLayout;
	QHBoxLayout buttonsLayout;
	QLabel appLogo;
	QLabel excessLogo;
	QLabel aboutText;
	QPushButton okButton;
};

AboutBox::priv::priv() :
    mainLayout(),
    contentLayout(),
    textLayout(),
    buttonsLayout(),
    appLogo(),
    excessLogo("\n"
	    "\n"
	    "\n"
	    "\n"
	    ""),
    aboutText(QString("%1 v%2\n"
	    "virtual 1541 disk commander\n\n"
	    "by Zirias/Excess")
	    .arg(QCoreApplication::applicationName())
	    .arg(QCoreApplication::applicationVersion())),
    okButton(tr("Ok"))
{
    appLogo.setPixmap(QPixmap(":/gfx/icon_256.png"));
}

AboutBox::AboutBox(const QFont &c64font) :
    QDialog(0, Qt::WindowSystemMenuHint
            | Qt::WindowTitleHint | Qt::WindowCloseButtonHint)
{
    d = new priv();
    d->excessLogo.setFont(c64font);
    d->textLayout.addWidget(&d->excessLogo);
    d->textLayout.addWidget(&d->aboutText);
    d->contentLayout.addWidget(&d->appLogo);
    d->contentLayout.addLayout(&d->textLayout);
    d->mainLayout.addLayout(&d->contentLayout);
    d->buttonsLayout.addStretch();
    d->buttonsLayout.addWidget(&d->okButton);
    d->mainLayout.addLayout(&d->buttonsLayout);
    d->mainLayout.setSizeConstraint(QLayout::SetFixedSize);
    setLayout(&d->mainLayout);
    connect(&d->okButton, &QPushButton::clicked, this, &AboutBox::hide);
}

AboutBox::~AboutBox()
{
    delete d;
}

void AboutBox::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    resize(0, 0);
}

