TEMPLATE = subdirs
SUBDIRS = plugins/displays \
          plugins/file-device \
          plugins/microphone \
          plugins/plots \
          plugins/signal-processing \
          plugins/scripting \
          connector-core
 
# build must be last:
CONFIG  += ordered
SUBDIRS += connector
