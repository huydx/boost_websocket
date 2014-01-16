#
# 2007.05.02 sudoh
#
#### for outputting core dump:
### required gcc option is gcc "-g"
### [scott@foobar ~]$ ulimit -c 1000000
# counter SJIS string
#

#---------------------------------------
# Base definitions
#---------------------------------------
TOPDIR = ../../
COMMONDIR=$(TOPDIR)server/src/common/
GALDIR=$(TOPDIR)server/gal64/
OUENGINEDIR=$(TOPDIR)ouengine/
TOOLDIR=$(TOPDIR)tool/
BOOSTDIR=/usr/

PHP=/usr/bin/php
RUBY=/usr/bin/ruby

#---------------------------------------
# Language setting
#---------------------------------------
LANGMODE=LANG_JAPANESE

#---------------------------------------
# Build target
#---------------------------------------
TARGET = ./chat_tool

#---------------------------------------
# Compilers
#---------------------------------------
CC = $(strip $(CCPROG) gcc)
CXX = $(strip $(CCPROG) g++)

#---------------------------------------
# Source list
#---------------------------------------

# common/ に関しては結合したくないファイルが多いので個別に記述
COMMONSRC = $(COMMONDIR)CSvUtil.cpp

# アプリケーション階層のコード
# ディレクトリ階層が増えたら追加して下さい。
#
# ヘッダーファイルに関しての注意！
# .cpp が一つも存在しないディレクトリは -I オプションの対象から漏れるので
# インクルードパスが通りません
# 下の方にある -I のオプションを手動で付与して下さい。
BASESRC = $(wildcard */*.cpp) \
		$(wildcard */*/*.cpp) \
		$(wildcard */*/*/*.cpp)

# 意図的にコンパイルから除外したいファイルがあればココに記述
IGNORESRC = $(wildcard test/*.cpp) \
		$(wildcard MRSGmProto/*.cpp)

#---------------------------------------
# GALGEN
#---------------------------------------
RPCGEN=$(OUENGINEDIR)source/tools/protocoder/protocoder.rb
RPCGENOUT=./


#-- client -> gmsv
MRSGMRPCDEFXML=$(TOPDIR)common/MRSGmProto.xml
MRSGMRPCDEFIGNORE=
MRSGMRPCDEFSRC=\
	$(filter-out $(MRSGMRPCDEFIGNORE),$(wildcard $(RPCGENOUT)MRSGmProto/*.cpp))

#---------------------------------------
# All sources
#---------------------------------------
ALLSRC = $(filter-out $(IGNORESRC),$(BASESRC)) $(filter-out $(IGNORESRC),$(COMMONSRC))
ALLRPCSRC = $(MRSGMRPCDEFSRC)

#---------------------------------------
# Compile options
#---------------------------------------

#
# -Wno-deprecated は std::hash_map が将来的に場所が変わる旨が出力されるのを抑制するオプション
#
CFLAGS = -m64 -includesystem/stdafx.h -pipe -pthread -fpack-struct=8 -fpermissive \
	-Wall -Wno-invalid-offsetof \
	-Wno-deprecated  -Wno-unused-parameter -Wno-unused-variable -Wno-switch -Wno-uninitialized \
	-D_FILE_OFFSET_BITS=64 -D_THREAD_SAFE -D$(USER) -DHAVE_HASH_MAP -D$(LANGMODE)
FLAGS += -DAPP_GMSV

#	-Wno-deprecated -Werror -Wno-error=unused-parameter -Wno-error=unused-variable -Wno-error=switch -Wno-error=uninitialized \

CXXFLAGS = -std=gnu++0x

ifdef RELEASE
#
# boost ライブラリの一部でエラーが出るので -fno-strict-aliasing オプション付けてますが
# boost 以外の場所でこのエラーが出るのは好ましくない
#
CFLAGS += -O2 -fno-strict-aliasing
CXXFLAGS += -O2 -Wall -Wclobbered -Wempty-body -Wignored-qualifiers -Wmissing-field-initializers -Wsign-compare -Wtype-limits -Wuninitialized -Wunused-parameter -Wunused-but-set-parameter -Werror $(CFLAGS)
else
CFLAGS += -g -ggdb -pg -D_FORTIFY_SOURCE=2 -D_DEBUG ####-D_PLACEMENT_NEW ####-fstack-protector-all
CXXFLAGS += -Wall -Wclobbered -Wempty-body -Wignored-qualifiers -Wmissing-field-initializers -Wsign-compare -Wtype-limits -Wuninitialized -Wunused-parameter -Wunused-but-set-parameter -Werror $(CFLAGS)
endif

CPPFLAGS = -DLANG_JAPANESE \
	-I$(BOOSTDIR)/include/ \
	-I$(OUENGINEDIR)source/engine/cpp/ \
	-I$(GALDIR)include/ \
	-I$(GALDIR)lib/ \
	-I$(GALDIR)lib/gal_script/include/ \
	-I$(GALDIR)lib/gal_variable/src/ \
	-I$(GALDIR)lib/gal_thread/src/ \
	-I$(GALDIR)lib/gal_timer/src/ \
	-I$(GALDIR)lib/gal_hashmap/src/ \
	-I$(GALDIR)lib/gal_core/src/ \
	-I$(GALDIR)lib/gal_tbr/src/ \
	-I$(TOPDIR)sasv/bin/src/ \
	-I/usr/include/libxml2/ \
	-I$(TOPDIR)common/ \
	-I$(COMMONDIR) \
	-I./ \
	$(addprefix -I./,$(sort $(dir $(ALLSRC))))

CPPFLAGS += `python-config --includes`

#---------------------------------------
# Linker options
#---------------------------------------
LDFLAGS = -L$(GALDIR)lib/linux/dist/
LDFLAGS +=  $(COMMONDIR)jsoncpp/jsoncpp-src-0.5.0/libs/linux-gcc-4.1.2/libjson_linux-gcc-4.1.2_libmt.a
LDFLAGS +=  -L/usr/lib64/mysql/ -lssl -lcrypto -ldl -lboost_system -lmysqlclient_r -lboost_thread -lboost_python -lboost_chrono -lboost_regex `pkg-config icu --libs` `python-config --ldflags`
LDFLAGS +=  /usr/lib64/libxml2.so
LDFLAGS +=  -L/usr/lib64 -lcurl 

ifdef RELEASE
LDFLAGS +=  $(OUENGINEDIR)library/gcc/Release/libnetwork.a
LDFLAGS +=  $(OUENGINEDIR)library/gcc/Release/libfoundation.a
#LDFLAGS +=  $(OUENGINEDIR)library/gcc/Debug/libnetwork.a
#LDFLAGS +=  $(OUENGINEDIR)library/gcc/Debug/libfoundation.a
LDFLAGS += -lgal_timer -lgal_core -lgal_script -lgal_vm -lgal_thread
else
LDFLAGS +=  $(OUENGINEDIR)library/gcc/Release/libnetwork.a
LDFLAGS +=  $(OUENGINEDIR)library/gcc/Release/libfoundation.a
#LDFLAGS +=  $(OUENGINEDIR)library/gcc/Debug/libnetwork.a
#LDFLAGS +=  $(OUENGINEDIR)library/gcc/Debug/libfoundation.a
LDFLAGS +=  -lgal_timer_d -lgal_core_d -lgal_script.d -lgal_vm.d -lgal_thread_d #-lprofiler -ltcmalloc
endif
LDFLAGS += `python-config --ldflags`

#---------------------------------------
# Temporary files
#---------------------------------------
ifdef RELEASE
	OBJDIR = object_release
else
	OBJDIR = object_debug
endif

ifdef JSON_PROTO
	CXXFLAGS += -D_JSON_PROTO
endif

ALLDIR = $(dir $(ALLSRC)) $(dir $(ALLRPCSRC))
vpath %.c $(ALLDIR)
vpath %.cpp $(ALLDIR)
vpath %.o $(OBJDIR)
vpath %.d $(OBJDIR)

ALLOBJ = $(notdir $(ALLSRC:%.cpp=%.o))
ALLRPCOBJ = $(subst $(RPCGENOUT),$(OBJDIR)/,$(ALLRPCSRC:%.cpp=%.o))
OBJ = $(ALLOBJ:%.o=$(OBJDIR)/%.o)
ALLCOMPILESRC = $(ALLSRC) $(ALLRPCSRC)

# depend用定義
ALLDEP = $(ALLOBJ:%.o=%.d)
DEP = $(OBJ:%.o=%.d)

#---------------------------------------
# Build all dependencies
#---------------------------------------
all : $(TARGET)
rpc	: $(ALLRPCOBJ)

$(MRSGMRPCDEFSRC) : $(RPCGEN) $(MRSGMRPCDEFXML)
	@if [ $@ = $(word 1,$(MRSGMRPCDEFSRC)) ]; then \
		$(RUBY) $(RPCGEN) $(MRSGMRPCDEFXML) --tool -t cpp -p Client -o $(RPCGENOUT) | (grep 'generated' || exit 0); \
	fi

rpcgen:
	@$(RUBY) $(RPCGEN) $(MRSGMRPCDEFXML) --tool -t cpp -p Client -o $(RPCGENOUT) | (grep 'generated' || exit 0);

distclean: clean
	$(RM) $(wildcard $(RPCGENOUT)MRSGmProto/*.h) $(wildcard $(RPCGENOUT)MRSGmProto/*.cpp)
	$(RM) $(wildcard $(RPCGENOUT)MRSGmProto/*.h) $(wildcard $(RPCGENOUT)MRSGmProto/*.cpp)

test:
	@echo ALLRPCSRC
	@echo $(ALLRPCSRC)
	@echo ''
	@echo OBJ
	@echo $(ALLOBJ)
	@echo ''
	@echo DEP
	@echo $(DEP)
	@echo ''
	@echo $(CC) / $(CXX)


.SUFFIXES: .c .cpp
.PHONY: all rpcgen clean distclean test rpc lib cleanall

.cpp.o:
	@if [ -e $(OBJDIR)/$(notdir $@) ] ; then rm $(OBJDIR)/$(notdir $(@:%.o=%.*)) ; fi
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $(OBJDIR)/$(notdir $@)
	@$(CXX) -MM -MP $(CXXFLAGS) $(CPPFLAGS) -c $< > $(OBJDIR)/$(notdir $(@:%.o=%.d))

.c.o:
	@if [ -e $(OBJDIR)/$(notdir $@) ] ; then rm $(OBJDIR)/$(notdir $(@:%.o=%.*)) ; fi
	$(CC) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $(OBJDIR)/$(notdir $@)
	@$(CC) -MM -MP $(CXXFLAGS) $(CPPFLAGS) -c $< > $(OBJDIR)/$(notdir $(@:%.o=%.d))

clean:
	$(RM) -rf $(OBJDIR)/*
	$(RM) $(TARGET)

lib:
	$(MAKE)	-C ../ lib

cleanall:
	$(MAKE) CLEAN=1 -C ../

#---------------------------------------
# Build all dependencies
#---------------------------------------
$(TARGET) :  $(ALLRPCOBJ) $(ALLOBJ)
	$(CXX) $(CFLAGS) $(CPPFLAGS) -o $@ $(OBJ) $(ALLRPCOBJ) $(LDFLAGS)
	@echo -e "\e[1;32m+++ $@ ... OK\e[0;39m"

$(ALLRPCOBJ):$(ALLRPCSRC)
	@if [ ! -d $(dir $@) ]; then \
		mkdir -p $(dir $@); \
	fi ;
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $(subst $(OBJDIR)/,$(RPCGENOUT),$(@:%.o=%.cpp)) -o $@
-include $(OBJDIR)/*.d
# DO NOT DELETE

