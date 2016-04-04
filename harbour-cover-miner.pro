TEMPLATE = subdirs

SUBDIRS = taglib src

DISTFILES += \
    harbour-cover-miner.desktop \
    harbour-cover-miner.png \
    rpm/harbour-cover-miner.spec \
    LICENSE \
    LEGAL \
    README.md

!exists(icons/86x86/harbour-cover-miner.png) {
    error( "Images not generated - call 'create_icons.sh'" )
}

!exists(icons/108x108/harbour-cover-miner.png) {
    error( "Images not generated - call 'create_icons.sh'" )
}

!exists(icons/128x128/harbour-cover-miner.png) {
    error( "Images not generated - call 'create_icons.sh'" )
}

!exists(icons/256x256/harbour-cover-miner.png) {
    error( "Images not generated - call 'create_icons.sh'" )
}

desktop.files = harbour-cover-miner.desktop
desktop.path = /usr/share/applications/

icon86.files = icons/86x86/harbour-cover-miner.png
icon86.path = /usr/share/icons/hicolor/86x86/apps/

icon108.files = icons/108x108/harbour-cover-miner.png
icon108.path = /usr/share/icons/hicolor/108x108/apps/

icon128.files = icons/128x128/harbour-cover-miner.png
icon128.path = /usr/share/icons/hicolor/128x128/apps/

icon256.files = icons/256x256/harbour-cover-miner.png
icon256.path = /usr/share/icons/hicolor/256x256/apps/

license.files = LICENSE LEGAL
license.path = /usr/share/$${TARGET}

INSTALLS += desktop icon86 icon108 icon128 icon256 license
