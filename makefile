TARGET = main

# DIRECTORIES OPTIONS
SRCS_DIR  = src
INCS_DIR  = include
OBJS_DIR  = obj
BUILD_DIR = build

# COMPILER OPTIONS 
CC          = gcc
FLAGS       = -Wall -Wextra -Werror -I$(INCS_DIR)
LINK        = 

# COMPILATION
INCLUDES = $(wildcard $(INCS_DIR)/*.h)
SOURCES  = main.c pixel.c image.c utils.c
OBJS     = $(addprefix obj/,$(SOURCES:.c=.o))

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

all: $(BUILD_DIR)/$(TARGET)

$(BUILD_DIR)/$(TARGET): $(OBJS)
	@echo "$(RED) SOURCES FILE FOUND : $(RST)$(BOLD)$(SOURCES)$(RST)"
	@echo "$(YELLOW) OBJECTS FILE FOUND : $(RST)$(BOLD)$(OBJS)$(RST)"
	@echo "$(CYAN)~LINKING $(RST)$(BOLD)$<$(RST)$(CYAN) TO EXECUTABLE TARGET $(RST)$(BOLD)$@$(RST)"
	@$(CC) -o $@ $^ $(FLAGS) $(LINK)
	@echo "$(GREEN)-> FINISHED!$(RST)"


$(OBJS_DIR)/%.o: $(SRCS_DIR)/%.c $(INCLUDES) | mkdir_obj
	@echo "$(MAGENTA)~COMPILING $(RST)$(BOLD)$<$(RST)$(MAGENTA) TO $(RST)$(BOLD)$@$(RST)"
	@$(CC) -c $< -o $@ $(FLAGS)

$(OBJS_DIR)/$(TARGET).o: $(TARGET).c $(INCLUDES) | mkdir_obj
	@echo "$(MAGENTA)~COMPILING $(WHITE)$(BOLD)$<$(RST)$(MAGENTA) TO $(RST)$(BOLD)$@$(RST)"
	@$(CC) -c $< -o $@ $(FLAGS)

run: all
	@echo "$(CLEAR)$(CUR_HOME)\r"
	@./$(BUILD_DIR)/$(TARGET)

mkdir_obj: 
	@mkdir -p $(OBJS_DIR)

clean:
	@echo "$(RED)$(BOLD)~CLEANING DIRECTORY... $(RST)"
	@rm -rf $(OBJS_DIR)
	@rm -rf $(BUILD_DIR)/$(TARGET)
	@echo "$(GREEN)-> FINISHED!$(RST)"


.PHONY: all clean run mkdir_obj