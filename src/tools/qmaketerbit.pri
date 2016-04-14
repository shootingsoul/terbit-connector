######################################
#defines and compiler flags
######################################
CONFIG(debug, debug|release) {
   DEFINES += DEBUG
}

win32{
   DEFINES += _WINDOWS _AFXDLL _VARIADIC_MAX=10
}

linux-g++*{
   QMAKE_CXXFLAGS += -std=c++11
}

message($$QMAKESPEC)

######################################
#BUILD_DIR
######################################
CONFIG(debug, debug|release) {
   BUILD_DIR = $${REPO_DIR}/build-debug
} else {
   BUILD_DIR = $${REPO_DIR}/build-release
}
BUILD_LIB_DIR    = $${BUILD_DIR}/lib
BUILD_PLUGIN_DIR = $${BUILD_DIR}/buildall/connector/plugins

#explicitly set DESTDIR so windows doesn't create debug and release subfolders
win32-msvc*{
   DESTDIR = $${BUILD_DIR}/$${TARGET}
}


######################################
#library dirs, include dirs and libs
#OS and arch specific variables . . .

#TEMP_LIB_DIRS
#TEMP_LIB_DIR_BUILD
#TEMP_LIB_DIR_BOOST

#TEMP_INC_BOOST

#TEMP_LIBS
#TEMP_LIB_BOOST

######################################

# Use QMAKE_TARGET.arch to determine the target architecture (Windows only)
# You'll see the message "Building for xx bit" in the _QMAKE_ output.
win32-msvc*{
   TEMP_LIB_DIR_BUILD = -L$$quote($${BUILD_LIB_DIR})


   TEMP_INC_BOOST    += $$quote($$(BOOST_ROOT))


   contains(QMAKE_TARGET.arch, x86_64):{
     message("Building for 64 bit")


     TEMP_LIB_DIRS += -L$$(QTDIR)/mkspecs/win32-msvc2012
     TEMP_LIB_DIR_BOOST    += -L$$quote($$(BOOST_ROOT)/lib/x64)

   }
   else{
     message("Building for 32 bit")
     #TEMP_LIB_DIRS += -L"C:/Qt/Qt5.2.1-32/5.2.1/msvc2012/mkspecs/win32-msvc2012"
     TEMP_LIB_DIRS += -L$$(QTDIR)/mkspecs/win32-msvc2012
     TEMP_LIB_DIR_BOOST    += -L$$quote($$(BOOST_ROOT)/lib/win32)
   }

}

# QMAKE_TARGET.arch doesn't work the same way on Linux as it does Windows
# The web suggests something like the following:
#   linux-g++:QMAKE_TARGET.arch = $$QMAKE_HOST.arch
#   linux-g++-32:QMAKE_TARGET.arch = x86
#   linux-g++-64:QMAKE_TARGET.arch = x86_64

linux-g++*{

   TEMP_LIB_DIRS += -L/usr/local/lib
   TEMP_LIB_DIR_BUILD = -L$${BUILD_LIB_DIR}
   #TEMP_LIB_DIR_BOOST = standard is fine
 
   #TEMP_INC_BOOST = standard is fine
 
   TEMP_LIBS = -ldl -lrt
   TEMP_LIB_BOOST = -lboost_system -lboost_date_time -lboost_thread -lboost_filesystem -lboost_chrono -lboost_timer -lpthread
 
}

#always have all inc dirs available for all projects
TEMP_INC_DIRS += $$REPO_DIR/src \
     $$TEMP_INC_BOOST
 
INCLUDEPATH += $$TEMP_INC_DIRS

######################################
#post link command to copy libs and dynamic libs/plugins to lib sub dirs
#under to appropriate build dir.  Store in temp variable to allow pro file
#to use these commands if needed.
######################################
linux-g++*{
   TEMP_POST_LINK_LIB_COPY += $$quote(mkdir -p $${BUILD_LIB_DIR})
   TEMP_POST_LINK_LIB_COPY += && $$quote(cp -f *.a $${BUILD_LIB_DIR})

   TEMP_POST_LINK_DLIB_COPY += $$quote(mkdir -p $${BUILD_PLUGIN_DIR})
   TEMP_POST_LINK_DLIB_COPY += && $$quote(cp -f *.so $${BUILD_PLUGIN_DIR})
}

win32 {
   TEMP_POST_LINK_LIB_COPY += $$quote(cmd /c mkdir $$replace(BUILD_LIB_DIR,"/","\\"))
   TEMP_POST_LINK_LIB_COPY += & $$quote(copy /y $$replace(DESTDIR,"/", "\\")\*.lib $$replace(BUILD_LIB_DIR, "/", "\\"))

   TEMP_POST_LINK_DLIB_COPY += $$quote(cmd /c mkdir $$replace(BUILD_PLUGIN_DIR,"/","\\"))
   TEMP_POST_LINK_DLIB_COPY += & $$quote(copy /y $$replace(DESTDIR,"/", "\\")\*.dll $$replace(BUILD_PLUGIN_DIR, "/", "\\"))
}

HEADERS +=




