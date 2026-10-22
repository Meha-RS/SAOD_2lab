TARGET   = 2lab
CC       = gcc
CFLAGS   = -Wall -Wextra -Iinc

SRC_DIR  = src
INC_DIR  = inc
OBJ_DIR  = obj

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(TARGET)
	rm -rf $(OBJ_DIR)
	rm -rf words.txt
	rm -rf plot_data.txt
	rm -rf lookup_plot.png

run: $(TARGET)
	@clear
	./$(TARGET)
	touch plot_data.txt

rebuild: clean run


.PHONY: all clean run rebuild