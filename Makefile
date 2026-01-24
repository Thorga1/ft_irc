CC = c++
NAME = ircserv
CFLAGS = -Wall -Wextra -Werror -std=c++98 -fsanitize=address,undefined -g
OBJ_DIR = obj
SRC = main.cpp \
	server.cpp \
	client.cpp \
	client_handler.cpp \
	channel.cpp
OBJ = $(addprefix $(OBJ_DIR)/, $(SRC:.cpp=.o))
DIRS = $(sort $(dir $(OBJ)))

all: $(NAME)

$(NAME): $(OBJ)
	@echo "$(BLUE)Compiling $(NAME)...$(RESET)"
	@$(CC) $(CFLAGS) $(OBJ) -o $(NAME)
	@echo "$(GREEN)Compilation successful!$(RESET)"

$(OBJ_DIR)/%.o: %.cpp | $(DIRS)
	@echo "$(YELLOW)Compiling $<...$(RESET)"
	@$(CC) $(CFLAGS) -c $< -o $@

$(DIRS):
	@mkdir -p $@

clean:
	@echo "$(RED)Cleaning object files...$(RESET)"
	@rm -rf $(OBJ_DIR)

fclean: clean
	@echo "$(RED)Cleaning executable $(NAME)...$(RESET)"
	@rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re $(DIRS)

RESET  = \033[0m
RED    = \033[31m
GREEN  = \033[32m
YELLOW = \033[33m
BLUE   = \033[34m
BOLD   = \033[1m
