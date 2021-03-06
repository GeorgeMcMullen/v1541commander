#include "petsciibutton.h"

PetsciiButton::PetsciiButton(
	ushort petscii, const QFont &c64font, QWidget *parent) :
    QLabel(parent)
{
    setFont(c64font);
    setText(QString(petscii));
    setStyleSheet("QLabel { padding: 4px 2px; background-color: white; }");
}

void PetsciiButton::enterEvent(QEvent *ev)
{
    (void) ev;
    setStyleSheet("QLabel { padding: 4px 2px; "
            "background-color : lightskyblue; color: blue; }");
}

void PetsciiButton::leaveEvent(QEvent *ev)
{
    (void) ev;
    setStyleSheet("QLabel { padding: 4px 2px; background-color: white; }");
}

void PetsciiButton::mousePressEvent(QMouseEvent *ev)
{
    (void) ev;
    emit clicked(text().begin()->unicode());
}
