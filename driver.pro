######################################################################
# Automatically generated by qmake (3.0) Tue May 3 22:08:49 2016
######################################################################

TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

ARCH=x86
SRC_PROJECT_PATH = /home/justin/driver
LINUX_HEADERS_PATH = /usr/src/linux-headers-$$system(uname -r)

SOURCES += $$system(find -L $$SRC_PROJECT_PATH -type f -name "*.c" -o -name "*.S" ) \
    testdrv.c
HEADERS += $$system(find -L $$SRC_PROJECT_PATH -type f -name "*.h" ) \
    testdrv.h
OTHER_FILES += $$system(find -L $$SRC_PROJECT_PATH -type f -not -name "*.h" -not -name "*.c" -not -name "*.S" )

INCLUDEPATH += $$system(find -L $$SRC_PROJECT_PATH -type d)
INCLUDEPATH += $$system(find -L $$LINUX_HEADERS_PATH/include -type d)
INCLUDEPATH += $$system(find -L $$LINUX_HEADERS_PATH/arch/$$ARCH/include -type d)
