NAME = ircserv
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98

SRC = main.cpp Server.cpp Client.cpp Channel.cpp Utils.cpp handleCommand.cpp handling.cpp remove.cpp 
OBJDIR = Obj
OBJ = $(SRC:.cpp=.o)
OBJ := $(addprefix $(OBJDIR)/, $(OBJ))

all: $(NAME)
	@echo "✅ Build complete: $(NAME)"

$(NAME): $(OBJ)
	@echo "🔧 Linking..."
	@$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJ)

$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(OBJDIR)
	@echo "🛠️  Compiling $<..."
	@$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@echo "🧹 Cleaning object files..."
	@rm -rf $(OBJDIR)

fclean: clean
	@echo "🧹 Cleaning binary..."
	@rm -f $(NAME)

re: fclean all
