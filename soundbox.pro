QT += widgets
requires(qtConfig(combobox))

HEADERS     = window.h
SOURCES     = main.cpp \
              window.cpp

# install
target.path = install
INSTALLS += target
