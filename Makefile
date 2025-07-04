NAME = ircserv
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98

SRC = main.cpp Server.cpp Client.cpp Channel.cpp Utils.cpp
OBJ = $(SRC:.cpp=.o)

all: $(NAME)
	@echo "✅ Build complete: $(NAME)"

$(NAME): $(OBJ)
	@echo "🔧 Linking..."
	@$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJ)

%.o: %.cpp
	@echo "🛠️  Compiling $<..."
	@$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@echo "🧹 Cleaning object files..."
	@rm -f $(OBJ)

fclean: clean
	@echo "🧹 Cleaning binary..."
	@rm -f $(NAME)

re: fclean all
