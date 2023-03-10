TEMPLATE = app
CONFIG += console
CONFIG += c++17
CONFIG += strict_c++
#CONFIG += exceptions_off
#CONFIG += rtti_off

CONFIG -= app_bundle
CONFIG -= qt

#clang:CONFIG += win32

#QMAKE_CXXFLAGS_DEBUG += -DDEBUG_BUILD
QMAKE_CXXFLAGS_DEBUG += -O0
QMAKE_CXXFLAGS_RELEASE += -Os
#QMAKE_CXXFLAGS += -Os
#QMAKE_CXXFLAGS += -ffreestanding
QMAKE_CXXFLAGS += -fno-threadsafe-statics
#linux:QMAKE_LFLAGS += -L/usr/lib/x86_64-linux-musl
#linux:QMAKE_LFLAGS += -lc

#QMAKE_CXXFLAGS += -stdlib=libc++
#QMAKE_LFLAGS += -stdlib=libc++

CONFIG += link_pkgconfig
PKGCONFIG += libcurl
PKGCONFIG += sqlite3


SOURCES += \
        data_types.cpp \
        endian_types.cpp \
        main.cpp \
        parsers.cpp \
        simplified/simple_sqlite.cpp

HEADERS += \
  data_types.h \
  endian_types.h \
  parsers.h \
  scrapers/utilities.h \
  scrapers/scraper_base.h \
  scrapers/chargehub_scraper.h \
  shortjson/shortjson.h \
  simplified/simple_curl.h \
  simplified/simple_sqlite.h
