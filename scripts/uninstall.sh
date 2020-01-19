#!/bin/sh

echo
echo V1541Commander uninstall
echo
echo This script will uninstall icons, menu entry and filetype associations
echo installed by \`setup.sh\'.
echo

if test $(id -u) = 0 ; then
	echo This script must not be run as root, exiting.
	echo
	exit 1;
fi

missingtool=n

if ! tool="$(type xdg-icon-resource 2>/dev/null)" || test -z "$tool"; then
	echo ERROR: required tool \`xdg-icon-resource\' not found in path.
	missingtool=y
fi

if ! tool="$(type xdg-desktop-menu 2>/dev/null)" || test -z "$tool"; then
	echo ERROR: required tool \`xdg-desktop-menu\' not found in path.
	missingtool=y
fi

if ! tool="$(type xdg-mime 2>/dev/null)" || test -z "$tool"; then
	echo ERROR: required tool \`xdg-mime\' not found in path.
	missingtool=y
fi

if test "$missingtool" = "y"; then
	exit 1;
fi

read -p "Do you want to continue (y/n)? " yn
case $yn in
	[Yy]* ) break;;
	* ) exit;;
esac

echo Uninstalling file type associations ...
xdg-mime uninstall --mode user mime/v1541commander.xml
echo Uninstalling desktop icon ...
xdg-desktop-menu uninstall --mode user v1541commander-local.desktop
echo Uninstalling icons ...
xdg-icon-resource uninstall --mode user \
	--size 16 icons/16x16/v1541commander.png
xdg-icon-resource uninstall --mode user \
	--size 32 icons/32x32/v1541commander.png
xdg-icon-resource uninstall --mode user \
	--size 48 icons/48x48/v1541commander.png
xdg-icon-resource uninstall --mode user \
	--size 256 icons/256x256/v1541commander.png
echo All done.

