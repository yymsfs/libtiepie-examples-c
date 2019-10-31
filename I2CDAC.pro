TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

LIBS += -ltiepie

win32 {
  msvc*:error("The Microsoft Visual C++ Compiler is not supported, please use MinGW.")

  QMAKE_CFLAGS += -std=c99
  LIBS += -L$$PWD
  COPY_FILE_TO_BUILD_DIRECTORY += $$PWD\libtiepie.dll
}

unix {
  QMAKE_CFLAGS += -std=gnu99
  LIBS += -lm
}

HEADERS += CheckStatus.h \
           PrintInfo.h \
           Utils.h


SOURCES += I2CDAC.c \
           CheckStatus.c \
           PrintInfo.c \
           Utils.c

# Copy files to build directory:
for(FILE,COPY_FILE_TO_BUILD_DIRECTORY) {
  QMAKE_POST_LINK += $$quote($(COPY_FILE) \"$${FILE}\" \"$$OUT_PWD/\"$(DESTDIR) $$escape_expand(\n\t))
}
