#!/bin/sh

MXE=/mnt/storage/git/mxe-xp
TARGET=i686-w64-mingw32.static
MAKE=gmake
MAKEOPTS=-j4
QTL10N=/usr/local/share/qt5/translations

PATH=${MXE}/usr/bin:${PATH} ${MAKE} ${MAKEOPTS} \
	CROSS_COMPILE=${TARGET}- \
	DEFINES="-DQT_STATICPLUGIN -DSTATIC_1541IMG" \
	CC=gcc CXX=g++ \
	PKGCONFIG=${TARGET}-pkg-config \
	RCC=${MXE}/usr/${TARGET}/qt5/bin/rcc \
	MOC=${MXE}/usr/${TARGET}/qt5/bin/moc \
	v1541commander_win32_LDFLAGS="-mwindows -L${MXE}/usr/${TARGET}/qt5/plugins/platforms -L${MXE}/usr/${TARGET}/qt5/plugins/styles" \
	v1541commander_win32_LIBS="qwindows wtsapi32 Qt5PlatformSupport freetype harfbuzz bz2" \
	EMBEDQTL10N=${QTL10N} \
	${1:-strip}

