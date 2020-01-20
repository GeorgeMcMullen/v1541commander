#!/bin/sh

VERSION=${1:-1.0}
NAME=v1541commander

WINTARGET=i686-w64-mingw32.static
LINTARGET=x86_64-unknown-linux-gnu
WINPKGDIR=${NAME}-${VERSION}
WINPKG=${NAME}-win32-${VERSION}.zip
LINPKGDIR=${NAME}-linux-x86_64-${VERSION}
LINPKG=${LINPKGDIR}.tar.xz
DOCS="README.md BUILDING.md LICENSE.txt LICENSE-font.txt"
ICONS=src/bin/${NAME}/icons
DESKTOP=src/bin/${NAME}/${NAME}.desktop
MIMETYPES=src/bin/${NAME}/mime

rm -fr ${WINPKGDIR} ${WINPKG} ${LINPKGDIR} ${LINPKG}

./build-static-mxe-win32.sh clean
./build-static-linux64.sh clean
./build-static-mxe-win32.sh
./build-static-linux64.sh

mkdir ${WINPKGDIR}
cp bin/${WINTARGET}/release/${NAME}.exe ${WINPKGDIR}
cp bin/${WINTARGET}/release/setup.exe ${WINPKGDIR}
cp bin/${WINTARGET}/release/uninstall.exe ${WINPKGDIR}
cp ${DOCS} ${WINPKGDIR}
zip -9r ${WINPKG} ${WINPKGDIR}
rm -fr ${WINPKGDIR}

mkdir ${LINPKGDIR}
cp bin/${LINTARGET}/release/${NAME} ${LINPKGDIR}
cp scripts/setup.sh ${LINPKGDIR}
cp scripts/uninstall.sh ${LINPKGDIR}
chmod 755 ${LINPKGDIR}/setup.sh
chmod 755 ${LINPKGDIR}/uninstall.sh
cp ${DOCS} ${LINPKGDIR}
cp ${DESKTOP} ${LINPKGDIR}
cp -R ${ICONS} ${LINPKGDIR}/
cp -R ${MIMETYPES} ${LINPKGDIR}/
tar cJvf ${LINPKG} ${LINPKGDIR}
rm -fr ${LINPKGDIR}

