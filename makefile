TARGET = tv

# DIRECTORIES OPTIONS
SRCS_DIR  = srcs
INCS_DIR  = includes
OBJS_DIR  = obj
BIN_DIR   = bin

# INSTALL OPTIONS
PREFIX    = /usr/local

# COMPILER OPTIONS
CC          = gcc
CFLAGS      = -Wall -Wextra -Werror -g
CPPFLAGS    = -I$(INCS_DIR)
LDFLAGS     =
LDLIBS      = -ljpeg -lpng -lgif -lwebpdemux -lwebp

# COMPILATION
INCLUDES = $(shell find $(INCS_DIR) -name '*.h')
SOURCES  = $(shell find $(SRCS_DIR) -name '*.c')
OBJS     = $(SOURCES:$(SRCS_DIR)/%.c=$(OBJS_DIR)/%.o)

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


$(BIN_DIR)/$(TARGET): $(OBJS)
	@printf "\n$(RED)--SOURCES FILE FOUND : $(RST)$(BOLD)$(SOURCES)$(RST)\n"
	@printf "$(YELLOW)--OBJECTS FILE FOUND : $(RST)$(BOLD)$(OBJS)$(RST)\n"
	@printf "\n$(CYAN)~LINKING $(RST)$(BOLD)$(OBJS_DIR)$(RST)$(CYAN) TO EXECUTABLE TARGET $(RST)$(BOLD)$@$(RST)\n"
	@mkdir -p $(BIN_DIR)
	@$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS) $(LDLIBS)
	@printf "$(GREEN)-> FINISHED!$(RST)\n"


$(OBJS_DIR)/%.o: $(SRCS_DIR)/%.c $(INCLUDES)
	@printf "$(MAGENTA)~COMPILING $(RST)$(BOLD)$<$(RST)$(MAGENTA) TO $(RST)$(BOLD)$@$(RST)\n"
	@mkdir -p $(dir $@)
	@$(CC) -c $< -o $@ $(CFLAGS) $(CPPFLAGS)


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


.PHONY: all clean run install uninstall
