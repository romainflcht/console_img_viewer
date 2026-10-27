TARGET = tv

# DIRECTORIES OPTIONS
SRCS_DIR  = srcs
INCS_DIR  = includes
OBJS_DIR  = obj
BIN_DIR   = bin

# INSTALL OPTIONS
PREFIX    = /usr/local

# OPTIONAL DECODERS (make WITH_JPEG=0 ...), BMP and netpbm are always built
WITH_JPEG ?= 1
WITH_PNG  ?= 1
WITH_GIF  ?= 1
WITH_WEBP ?= 1

# COMPILER OPTIONS (CC, CFLAGS, CPPFLAGS, LDFLAGS and LDLIBS can be given on 
# the command line, e.g. for cross compilation)
CC          = gcc
CFLAGS      = -Wall -Wextra -Werror -g
CPPFLAGS    =
LDFLAGS     =
LDLIBS      =

DECODERS_DIR = $(SRCS_DIR)/decoders
FEATURES     =
FEATURE_LIBS =
EXCLUDED     =

ifeq ($(WITH_JPEG),1)
    FEATURES     += -DWITH_JPEG
    FEATURE_LIBS += -ljpeg
else
    EXCLUDED     += $(DECODERS_DIR)/jpeg_handler.c
endif

ifeq ($(WITH_PNG),1)
    FEATURES     += -DWITH_PNG
    FEATURE_LIBS += -lpng
else
    EXCLUDED     += $(DECODERS_DIR)/png_handler.c
endif

ifeq ($(WITH_GIF),1)
    FEATURES     += -DWITH_GIF
    FEATURE_LIBS += -lgif
else
    EXCLUDED     += $(DECODERS_DIR)/gif_handler.c
endif

ifeq ($(WITH_WEBP),1)
    FEATURES     += -DWITH_WEBP
    FEATURE_LIBS += -lwebpdemux -lwebp
else
    EXCLUDED     += $(DECODERS_DIR)/webp_handler.c
endif

ALL_CPPFLAGS = -I$(INCS_DIR) $(FEATURES) $(CPPFLAGS)
ALL_LDLIBS   = $(FEATURE_LIBS) $(LDLIBS)

# COMPILATION
INCLUDES = $(shell find $(INCS_DIR) -name '*.h')
SOURCES  = $(filter-out $(EXCLUDED), $(shell find $(SRCS_DIR) -name '*.c'))
OBJS     = $(SOURCES:$(SRCS_DIR)/%.c=$(OBJS_DIR)/%.o)

# Objects are rebuilt when the build options change. 
BUILD_FLAGS = $(OBJS_DIR)/build.flags

# RUN
ARGS = "img/img(15x15).jpeg"

# FONT
MAGENTA  = \e[35m
CYAN     = \e[36m
WHITE    = \e[37m
YELLOW   = \e[33m
RED      = \e[31m
GREEN    = \e[32m

BOLD     = \e[1m
RST      = \e[0m
CLEAR    = \e[2J
CUR_HOME = \e[H


all: $(BIN_DIR)/$(TARGET)


$(BIN_DIR)/$(TARGET): $(OBJS) $(BUILD_FLAGS)
	@printf "\n$(RED)--SOURCES FILE FOUND : $(RST)$(BOLD)$(SOURCES)$(RST)\n"
	@printf "$(YELLOW)--OBJECTS FILE FOUND : $(RST)$(BOLD)$(OBJS)$(RST)\n"
	@printf "\n$(CYAN)~LINKING $(RST)$(BOLD)$(OBJS_DIR)$(RST)$(CYAN) TO EXECUTABLE TARGET $(RST)$(BOLD)$@$(RST)\n"
	@mkdir -p $(BIN_DIR)
	@$(CC) $(OBJS) -o $@ $(CFLAGS) $(LDFLAGS) $(ALL_LDLIBS)
	@printf "$(GREEN)-> FINISHED!$(RST)\n"


$(OBJS_DIR)/%.o: $(SRCS_DIR)/%.c $(INCLUDES) $(BUILD_FLAGS)
	@printf "$(MAGENTA)~COMPILING $(RST)$(BOLD)$<$(RST)$(MAGENTA) TO $(RST)$(BOLD)$@$(RST)\n"
	@mkdir -p $(dir $@)
	@$(CC) -c $< -o $@ $(CFLAGS) $(ALL_CPPFLAGS)


$(BUILD_FLAGS): FORCE
	@mkdir -p $(OBJS_DIR)
	@echo '$(CC) $(CFLAGS) $(ALL_CPPFLAGS) $(LDFLAGS) $(ALL_LDLIBS)' | cmp -s - $@ \
		|| echo '$(CC) $(CFLAGS) $(ALL_CPPFLAGS) $(LDFLAGS) $(ALL_LDLIBS)' > $@


run: all
	@printf "$(CLEAR)$(CUR_HOME)\r"
	@./$(BIN_DIR)/$(TARGET) $(ARGS)


install: all
	@install -Dm755 $(BIN_DIR)/$(TARGET) $(DESTDIR)$(PREFIX)/bin/$(TARGET)
	@printf "$(GREEN)-> INSTALLED TO $(RST)$(BOLD)$(DESTDIR)$(PREFIX)/bin/$(TARGET)$(RST)\n"


uninstall:
	@rm -f $(DESTDIR)$(PREFIX)/bin/$(TARGET)


clean:
	@printf "$(CLEAR)$(CUR_HOME)$(RED)$(BOLD)~CLEANING DIRECTORY... $(RST)\n"
	@rm -rf $(OBJS_DIR)
	@rm -rf $(BIN_DIR)
	@printf "$(GREEN)-> FINISHED!$(RST)\n"


.PHONY: all clean run install uninstall FORCE
