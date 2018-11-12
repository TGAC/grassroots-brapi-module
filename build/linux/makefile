#
# Create a file called user.prefs by copying the example-user.prefs file
# to user.prefs and editing its values.
#
DIR_BUILD :=  $(realpath $(dir $(lastword $(MAKEFILE_LIST))))
DIR_SRC := $(realpath $(DIR_BUILD)/../../source)
DIR_INCLUDE := $(realpath $(DIR_BUILD)/../../include)

ifeq ($(DIR_BUILD_CONFIG),)
export DIR_BUILD_CONFIG := $(realpath $(DIR_BUILD)/../../../../build-config/linux)
endif


-include user.prefs

MODNAME      ?= grassroots_brapi
SHARED_FNAME := mod_$(MODNAME).so
CC	:= gcc
OUTPUT_DIR	 := out
SHARED       := $(OUTPUT_DIR)/$(SHARED_FNAME)

APXS := $(APACHE_DIR)/bin/apxs

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

CFILES := brapi_module.c brapi_location.c

VPATH := $(DIR_SRC)
HFILES := $(CFILES:%.c=%.h)
SRCFILES := $(CFILES) $(HFILES)
OBJFILES := $(CFILES:%.c=$(OUTPUT_DIR)/%.o)

INCLUDE_PATHS += \
	$(DIR_INCLUDE) \
	$(APACHE_INCLUDES_DIR) \
	$(DIR_GRASSROOTS_GEOCODER_INC) \
	$(DIR_GRASSROOTS_UTIL_INC) \
	$(DIR_GRASSROOTS_UTIL_INC)/containers \
	$(DIR_GRASSROOTS_UTIL_INC)/io \
	$(DIR_GRASSROOTS_NETWORK_INC) \
	$(DIR_GRASSROOTS_PARAMS_INC) \
	$(DIR_GRASSROOTS_MONGODB_INC) \
	$(DIR_JANSSON_INC) \
	$(DIR_UUID_INC) \
	$(DIR_MONGODB_INC) \
	$(DIR_BSON_INC) \


LIB_PATHS += $(DIR_GRASSROOTS_GEOCODER_LIB) \
	$(DIR_GRASSROOTS_NETWORK_LIB) \
	$(DIR_GRASSROOTS_MONGODB_LIB) \
	$(DIR_GRASSROOTS_UTIL_LIB) \
	$(DIR_JANSSON_LIB) \
	$(DIR_UUID_LIB)


LIBS := $(GRASSROOTS_GEOCODER_LIB_NAME) \
	$(GRASSROOTS_UTIL_LIB_NAME) \
	$(GRASSROOTS_NETWORK_LIB_NAME) \
	$(GRASSROOTS_MONGODB_LIB_NAME) \
	uuid \
	jansson
	
MACROS := \
	UNIX
	
WARNINGS :=                           \
	all                           \
	extra                         \
	no-unused-parameter           \
	no-missing-field-initializers \
	no-format                     \
	fatal-errors \
	shadow

CFLAGS +=                              \
	-g3                            \
	-ggdb                          \
	-O0							\
	-pg \
	-pedantic                      \
	$(addprefix -W, $(WARNINGS))   \
	$(addprefix -D, $(MACROS)) \
	$(addprefix -I, $(INCLUDE_PATHS))

LDFLAGS +=                           \
	$(addprefix -l, $(LIBS))     \
	$(addprefix -L, $(LIB_PATHS)) \
	-shared 

comma := ,

.PHONY: all shared install test clean init

all: init shared




init:
	mkdir -p $(OUTPUT_DIR)

install: $(SHARED)
	cp $(SHARED) $(INSTALL_DIR)/

shared: $(SHARED)

$(SHARED): init $(OBJFILES)
	@echo "LIB_PATHS: $(LIB_PATHS)"
	$(LD) -o $(SHARED) $(OBJFILES) $(STATIC_LIBS) $(LDFLAGS)

clean:
	rm -rvf $(OUTPUT_DIR)/*

	
#$(SHARED): apxs $(SRCFILES) 
#	$(APXS) -c   \
#	$(addprefix -Wc$(comma), $(CFLAGS))  \
#	$(addprefix -Wl$(comma), $(LDFLAGS)) \
#	-o $(SHARED_FNAME) $(SRCFILES)  

$(OUTPUT_DIR)/%.o: %.c 
	$(CC) -c $(CFLAGS) $(CPPFLAGS) -fPIC -DLINUX -D_REENTRANT -D_GNU_SOURCE $< -o $@


