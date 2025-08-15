NAME =  WebServ
OBJDIR = obj
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98  -fsanitize=address
RM = rm -rf
#################
VPATH = inc:src
SRC = main.cpp server.cpp Parser.cpp #Request.cpp Parser.cpp Server.cpp Client.cpp 
# SRC += AutoIndex.cpp StatusCode.cpp Response.cpp Library.cpp main.cpp

OBJS =  $(addprefix $(OBJDIR)/,  $(notdir $(SRC:.cpp=.o)))

$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: all clean fclean re

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)
clean:
	rm -rf $(OBJDIR)
fclean: clean
	rm -f $(NAME)
re: fclean all