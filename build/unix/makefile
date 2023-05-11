#
# Create a file called user.prefs by copying the example-user.prefs file
# to user.prefs and editing its values.
#
DIR_BUILD :=  $(realpath $(dir $(lastword $(MAKEFILE_LIST)))/$(PLATFORM))
DIR_SRC := $(realpath $(DIR_BUILD)/../../../source)
DIR_INCLUDE := $(realpath $(DIR_BUILD)/../../../include)

ifeq ($(DIR_BUILD_CONFIG),)
export DIR_BUILD_CONFIG = $(realpath $(DIR_BUILD)/../../../../../build-config/unix/)
#export DIR_BUILD_CONFIG = /Users/tyrrells/Projects/grassroots/build-config/unix/
endif

include $(DIR_BUILD_CONFIG)/project.properties
-include $(DIR_BUILD)/user.prefs

NAME := grassroots_brapi
CC	:= gcc
OUTPUT_DIR	 := out

APXS := $(DIR_APACHE)/bin/apxs

APACHE_INCLUDES_DIR := $(APACHE_DIR)/include

include $(DIR_BUILD_CONFIG)/project.properties

ifeq ($(strip $(APXS)),)
APXS	:= $(shell which apxs)
endif


# XXX: These are currently unused as we rely on the apxs utility for module
#      installation.
INSTALL_DIR  ?= $(APACHE_DIR)/modules
INSTALLED    := $(INSTALL_DIR)/mod_$(MODNAME).so
BUILD_DIR := build

CFILES := \
	$(DIR_SRC)/brapi_method.c \
	$(DIR_SRC)/brapi_module.c \
	$(DIR_SRC)/brapi_location.c \
	$(DIR_SRC)/brapi_program.c \
	$(DIR_SRC)/brapi_study.c \
	$(DIR_SRC)/brapi_trait.c \
	$(DIR_SRC)/brapi_trial.c

VPATH := $(DIR_SRC)
HFILES := $(CFILES:%.c=%.h)
SRCFILES := $(CFILES) $(HFILES)

INCLUDE_PATHS += \
	$(DIR_INCLUDE) \
	$(APACHE_INCLUDES_DIR) \
	$(DIR_GRASSROOTS_GEOCODER_INC) \
	$(DIR_GRASSROOTS_UUID_INC) \
	$(DIR_GRASSROOTS_UTIL_INC) \
	$(DIR_GRASSROOTS_UTIL_INC)/containers \
	$(DIR_GRASSROOTS_UTIL_INC)/io \
	$(DIR_GRASSROOTS_NETWORK_INC) \
	$(DIR_GRASSROOTS_SERVICES_INC)/parameters \
	$(DIR_GRASSROOTS_PLUGIN_INC) \
	$(DIR_GRASSROOTS_MONGODB_INC) \
	$(DIR_GRASSROOTS_SQLITE_INC) \
	$(DIR_JANSSON_INC) \
	$(DIR_UUID_INC) \
	$(DIR_MONGODB_INC) \
	$(DIR_BSON_INC) \
	$(DIR_SQLITE_INC) \
	$(DIR_GRASSROOTS_SERVER_INC) \
	$(DIR_GRASSROOTS_SERVICES_INC) \
	$(DIR_GRASSROOTS_HANDLER_INC) \
	$(DIR_GRASSROOTS_TASK_INC) \
	$(FIELD_TRIAL_SERVICE_PATH)/include


LIB_PATHS += \
	$(DIR_GRASSROOTS_INSTALL)/lib \
	$(DIR_GRASSROOTS_INSTALL)/services \
	$(DIR_JANSSON_LIB) \
	$(DIR_MONGODB_LIB) \
	$(DIR_GRASSROOTS_UUID_LIB) \
	$(DIR_UUID_LIB) \
	$(DIR_APACHE)/lib


LIBS += \
	$(GRASSROOTS_UUID_LIB_NAME) \
	$(GRASSROOTS_GEOCODER_LIB_NAME) \
	$(GRASSROOTS_UTIL_LIB_NAME) \
	$(GRASSROOTS_NETWORK_LIB_NAME) \
	$(GRASSROOTS_MONGODB_LIB_NAME) \
	$(GRASSROOTS_UUID_LIB_NAME) \
	$(GRASSROOTS_SERVER_LIB_NAME) \
	$(GRASSROOTS_SERVICES_LIB_NAME) \
	uuid \
	jansson \
	aprutil-1.0 \
	apr-1.0 \
	mongoc-1.0
	
MACROS := \
	$(PLATFORM) \

WARNINGS :=                           \
	all                           \
	extra                         \
	no-unused-parameter           \
	no-missing-field-initializers \
	no-format                     \
	fatal-errors \
	shadow


CFLAGS 	+=  -Wc,-Wall  -Wc,-fpic

ifeq ($(BUILD),release)
	CFLAGS 	+= -Wc,-O3 -Wc,-s
 	LDFLAGS += Wl,-s
else
	CFLAGS 	+= -Wc,-g -Wc,-Wall -Wc,-O0
	LDFLAGS += -Wl,pg
	CPPFLAGS += -Wc,-D_DEBUG
endif


CFLAGS +=                              \
	$(addprefix -D, $(MACROS)) \
	$(addprefix -I, $(INCLUDE_PATHS)) 

LDFLAGS +=                           \
	$(addprefix -l, $(LIBS))     \
	$(addprefix -L, $(LIB_PATHS)) \


comma := ,

.PHONY: all shared install test clean init

all: init shared




init:
	mkdir -p $(OUTPUT_DIR)

install: shared
	cp $(SHARED) $(INSTALL_DIR)/

shared: init $(OBJFILES)
	$(APXS) -n $(NAME) $(INCLUDES) -D UNIX=1 -D BUILD=$(BUILD) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -c $(CFILES)

clean:
	rm -rvf $(OUTPUT_DIR)/*

	
#$(SHARED): apxs $(SRCFILES) 
#	$(APXS) -c   \
#	$(addprefix -Wc$(comma), $(CFLAGS))  \
#	$(addprefix -Wl$(comma), $(LDFLAGS)) \
#	-o $(SHARED_FNAME) $(SRCFILES)  

# Compile and generate dependency info
# 1. Compile the .c file
# 2. Generate dependency information, explicitly specifying the target name
$(DIR_OBJS)/%.o : %.c
	@echo ">>> c build for $@"
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $<
	$(MAKEDEPEND) $(basename $@).d -MT $(basename $@).o $(CPPFLAGS) $(CFLAGS) $<  
	@mv -f $(basename $@).d $(basename $@).d.tmp
	@sed -e 's|.*:|$*.o:|' < $(basename $@).d.tmp > $(basename $@).d
	@sed -e 's/.*://' -e 's/\\$$//' < $(basename $@).d.tmp | fmt -1 | \
		sed -e 's/^ *//' -e 's/$$/:/' >> $(basename $@).d
	@rm -f $(basename $@).d.tmp   	


