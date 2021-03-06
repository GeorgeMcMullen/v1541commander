#include "petsciiedit.h"
#include "petsciiconvert.h"
#include "petsciistr.h"
#include "settings.h"
#include "v1541commander.h"

#include <QKeyEvent>

static const ushort cbmLetterChars[] =
{
    0xe0b0,
    0xe0bf,
    0xe0bc,
    0xe0ac,
    0xe0b1,
    0xe0bb,
    0xe0a5,
    0xe0b4,
    0xe0a2,
    0xe0b5,
    0xe0a1,
    0xe0b6,
    0xe0a7,
    0xe0aa,
    0xe0b9,
    0xe0af,
    0xe0ab,
    0xe0b2,
    0xe0ae,
    0xe0a3,
    0xe0b8,
    0xe0be,
    0xe0b3,
    0xe0bd,
    0xe0b7,
    0xe0ad
};

PetsciiEdit::PetsciiEdit(QWidget *parent) :
    QLineEdit(QString(), parent)
{
    setFont(cmdr.c64font());
    connect(this, &QLineEdit::textEdited, this, &PetsciiEdit::editText);
}

void PetsciiEdit::editText(const QString &text)
{
    int pos = cursorPosition();
    QString translated;
    for (QString::const_iterator i = text.cbegin(); i != text.cend(); ++i)
    {
	QChar tc = PetsciiConvert::unicodeToFont(*i,
		cmdr.settings().lowercase());
	if (tc.isNull())
	{
	    if (pos > 0) --pos;
	}
	else
	{
            if (tc == 0xe0a0 || tc == 0xe1a0) tc = 0xefa3;
	    translated.append(tc);
	}
    }
    setCursorPosition(pos);
    PetsciiStr petscii(translated);
    emit petsciiEdited(petscii);
}

void PetsciiEdit::setPetscii(const PetsciiStr &petscii, bool keepCursorPos)
{
    int pos = cursorPosition();
    bool lc = cmdr.settings().lowercase();
    setText(petscii.toString(lc).replace(lc ? 0xe1a0 : 0xe0a0, 0xefa3));
    if (keepCursorPos) setCursorPosition(pos);
}

void PetsciiEdit::updateCase(bool lowerCase)
{
    int pos = cursorPosition();
    QString translated;
    for (QString::const_iterator i = text().cbegin(); i != text().cend(); ++i)
    {
	QChar tc = PetsciiConvert::unicodeToFont(*i, lowerCase);
        if (tc == 0xe0a0 || tc == 0xe1a0) tc = 0xefa3;
	translated.append(tc);
    }
    setText(translated);
    setCursorPosition(pos);
}

void PetsciiEdit::petsciiInput(ushort val)
{
    insert(QString(val));
}

void PetsciiEdit::setMaxLength(int length)
{
    QFontMetricsF fm(font());
    setMinimumWidth(minimumSizeHint().width() - fm.maxWidth()
	    + length * fm.boundingRect(0xe220).width());
    QLineEdit::setMaxLength(length);
}

void PetsciiEdit::keyPressEvent(QKeyEvent *event)
{
    Qt::KeyboardModifiers mods = QApplication::keyboardModifiers();
    int key = event->key();
    if (mods == Qt::AltModifier)
    {
	if (key >= Qt::Key_A && key <= Qt::Key_Z)
	{
	    petsciiInput(cbmLetterChars[key - Qt::Key_A]);
	}
    }
    else if (mods == Qt::ShiftModifier && key == Qt::Key_Space)
    {
        petsciiInput(0xefa3);
    }
    else QLineEdit::keyPressEvent(event);
}

bool PetsciiEdit::event(QEvent *event)
{
    if (event->type() == QEvent::ShortcutOverride)
    {
	Qt::KeyboardModifiers mods = QApplication::keyboardModifiers();
	if (mods == Qt::AltModifier)
	{
	    int key = static_cast<QKeyEvent *>(event)->key();
	    if (key >= Qt::Key_A && key <= Qt::Key_Z)
	    {
		event->accept();
	    }
	}
    }
    return QLineEdit::event(event);
}
