######################################################################
# Automatically generated by qmake (2.01a) ?? 4? 23 17:10:15 2015
######################################################################

QT       += core  gui   xml  sql  network
TEMPLATE = app
TARGET = VN22.10.3.1
DEPENDPATH += . proto json
INCLUDEPATH += . proto json
CFLAGS += -Wno-psabi



#DYTEST1 电脑运行不去掉光标
#DYTEST2 测试发2个跳过全部功能
#DYTEST3 测试急停用
#DYTEST4 大众MEB测试Label结果显示
#DYTEST5 大众MEB模拟拧紧
DEFINES +=



# Input
#include(./source/websocket.pri)
INCLUDEPATH += eigen3

HEADERS += \
           GlobalVarible.h \
           mainwindow.h \
           worker.h \
    newconfiginfo.h \
    TestInput_Cursor/formhanzitable.h \
    TestInput_Cursor/keyboard.h \
    TestInput_Cursor/dialogtooltip.h \
    TestInput_Cursor/ChineseTranslator.h \
    TestInput_Cursor/formhanziselector.h \
    TestInput_Cursor/formnumpad.h \
    TestInput_Cursor/HanziManager.h \
    TestInput_Cursor/formsymbolpad.h \
    TestInput_Cursor/inputmethod.h \
    TestInput_Cursor/padbase.h \
    optiondialog.h \
    save.h \
    basicset.h \
    fisupdate.h \
    communication.h \
    tightenthread.h \
    xmlcreate.h \
    inputevents.h \
    wificonnect.h \
    taotong.h \
    vinreverse.h \
    sqlthread.h \
    split.h \
    rfidcontroler.h \
    serialComs.h \
    fisupdatequeue.h \
    deletecarsql.h \
    sqlthreadbyd.h \
    tighten_op_net.h \
    tighten_op_net_curve.h \
    tighten_op_atlas_PM.h \
    tighten_op_cleco.h \
    tighten_op_atlas_PFC.h \
    tighten_op_atlas_PF.h \
    ftphandle.h \
    tighten_op_atlas_PMM.h \
    json/FlexLexer.h \
    json/json_parser.hh \
    json/json_scanner.h \
    json/location.hh \
    json/parser_p.h \
    json/parser.h \
    json/parserrunnable.h \
    json/position.hh \
    json/qjson_debug.h \
    json/qjson_export.h \
    json/qobjecthelper.h \
    json/serializer.h \
    json/serializerrunnable.h \
    json/stack.hh \
    ftpjson.h \
    tighten_op_net_ch1.h \
    tighten_op_net_ch2.h \
    rootdialog.h \
    ftpjsonparse.h \
    connectplc.h \
    passwordpanel.h \
    cardread.h \
    usermanagement.h \
    roleadd.h \
    head.h \
    sqlthreadbenz.h \
    paintarea.h \
    quickrequest.h \
    qextserial/qextserialport.h \
    qextserial/qextserialport_global.h \
    gwkquantecmcs.h \
    workshift.h \
    tighten_op_rs232.h \
    mesuploadhaima.h \
    logdelete.h \
    sqlthreadsvw2.h \
    tighten_json_atlas_pm600.h \
    pluscommunication.h \
    gwkPLC.h \
    mesuploadAQCHERY.h \
    mesuploadsvw2.h \
    replacebarcode.h \
    mesuploaddongfeng.h \
    queueinterface.h \
    sqlthreadbaic.h \
    mesuploadbaic.h \
    baicinfointerface.h \
    tighten_op_atlas_IRMM.h \
    ftpdataparse.h \
    mastertcp.h \
    ftpclient.h \
    wirelesslocation.h \
    position.h \
    basestation.h \
    analyzelocationdata.h \
    locationserver.h \
    locationclient.h \
    CurrentStatus.h \
    locationconfig.h \
    locationparsejson.h \
    tagmac.h \
    xmldata.h \
    atlas_OP.h \
    qcustomplot.h \
    location.h \
    leuzerecv.h \
    testalive.h \
    kalman.h \
    serial/serialthread.h \
    queuethread.h \
    proto/matchfeaturecode.h \
    tightenProto/alatspm4000.h \
    sqlthreadgzbaic.h \
    proto/userbind.h \
    proto/fisbaic.h \
    proto/getcarinformation.h \
    proto/sql.h \
    inputcode.h \
    worker_356.h \
    lightoperate.h \
    iobox.h \
    inputcode_aq.h
    
FORMS += \
         mainwindow.ui \
    newconfiginfo.ui \
    TestInput_Cursor/formsymbolpad.ui \
    TestInput_Cursor/formnumpad.ui \
    TestInput_Cursor/formhanziselector.ui \
    TestInput_Cursor/keyboard.ui \
    TestInput_Cursor/dialogtooltip.ui \
    TestInput_Cursor/formhanzitable.ui \
    optiondialog.ui \
    save.ui \
    basicset.ui \
    rootdialog.ui \
    passwordpanel.ui \
    usermanagement.ui \
    roleadd.ui \
    head.ui \
    workshift.ui \
    basestation.ui \
    locationconfig.ui \
    tagmac.ui \
    location.ui \
    inputcode.ui \
    inputcode_aq.ui
SOURCES += \
           main.cpp \
           mainwindow.cpp \
           worker.cpp \
    newconfiginfo.cpp \
    TestInput_Cursor/formhanzitable.cpp \
    TestInput_Cursor/formsymbolpad.cpp \
    TestInput_Cursor/HanziManager.cpp \
    TestInput_Cursor/formhanziselector.cpp \
    TestInput_Cursor/formnumpad.cpp \
    TestInput_Cursor/padbase.cpp \
    TestInput_Cursor/keyboard.cpp \
    TestInput_Cursor/dialogtooltip.cpp \
    TestInput_Cursor/inputmethod.cpp \
    optiondialog.cpp \
    save.cpp \
    basicset.cpp \
    fisupdate.cpp \
    communication.cpp \
    tightenthread.cpp \
    xmlcreate.cpp \
    inputevents.cpp \
    wificonnect.cpp \
    taotong.cpp \
    vinreverse.cpp \
    sqlthread.cpp \
    split.cpp \
    rfidcontroler.cpp \
    serialComs.cpp \
    fisupdatequeue.cpp \
    deletecarsql.cpp \
    sqlthreadbyd.cpp \
    tighten_op_net.cpp \
    tighten_op_net_curve.cpp \
    tighten_op_atlas_PM.cpp \
    tighten_op_cleco.cpp \
    tighten_op_atlas_PFC.cpp \
    tighten_op_atlas_PF.cpp \
    ftphandle.cpp \
    tighten_op_atlas_PMM.cpp \
    json/json_parser.cc \
    json/json_scanner.cc \
    json/json_scanner.cpp \
    json/parser.cpp \
    json/parserrunnable.cpp \
    json/qobjecthelper.cpp \
    json/serializer.cpp \
    json/serializerrunnable.cpp \
    ftpjson.cpp \
    tighten_op_net_ch2.cpp \
    tighten_op_net_ch1.cpp \
    rootdialog.cpp \
    ftpjsonparse.cpp \
    connectplc.cpp \
    passwordpanel.cpp \
    cardread.cpp \
    usermanagement.cpp \
    roleadd.cpp \
    head.cpp \
    sqlthreadbenz.cpp \
    paintarea.cpp \
    quickrequest.cpp \
    qextserial/qextserialport_unix.cpp \
    qextserial/qextserialport.cpp \
    gwkquantecmcs.cpp \
    workshift.cpp \
    tighten_op_rs232.cpp \
    mesuploadhaima.cpp \
    logdelete.cpp \
    sqlthreadsvw2.cpp \
    tighten_json_atlas_pm600.cpp \
    pluscommunication.cpp \
    gwkPLC.cpp \
    mesuploadAQCHERY.cpp \
    mesuploadsvw2.cpp \
    replacebarcode.cpp \
    mesuploaddongfeng.cpp \
    queueinterface.cpp \
    sqlthreadbaic.cpp \
    mesuploadbaic.cpp \
    baicinfointerface.cpp \
    tighten_op_atlas_IRMM.cpp \
    ftpdataparse.cpp \
    mastertcp.cpp \
    ftpclient.cpp \
    wirelesslocation.cpp \
    position.cpp \
    basestation.cpp \
    analyzelocationdata.cpp \
    locationserver.cpp \
    locationclient.cpp \
    locationconfig.cpp \
    locationparsejson.cpp \
    tagmac.cpp \
    xmldata.cpp \
    atlas_OP.cpp \
    qcustomplot.cpp \
    location.cpp \
    leuzerecv.cpp \
    testalive.cpp \
    kalman.cpp \
    serial/serialthread.cpp \
    queuethread.cpp \
    proto/matchfeaturecode.cpp \
    tightenProto/alatspm4000.cpp \
    sqlthreadgzbaic.cpp \
    proto/userbind.cpp \
    proto/fisbaic.cpp \
    proto/getcarinformation.cpp \
    proto/sql.cpp \
    inputcode.cpp \
    worker_356.cpp \
    lightoperate.cpp \
    iobox.cpp \
    inputcode_aq.cpp
    
RESOURCES += file.qrc \
    TestInput_Cursor/keypad.qrc

OTHER_FILES +=

UI_DIR=./UI

DISTFILES += \
    explain.py
