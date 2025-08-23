NAME =  WebServ
OBJDIR = obj
TMP = tmp
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98  -fsanitize=address -g3
RM = rm -rf
#################
VPATH = inc:src
SRC = main.cpp Parser.cpp Request.cpp server.cpp client.cpp Response.cpp CgiHandler.cpp
#SRC +=  CgiHandler.cpp  CgiHandler.cpp#AutoIndex.cpp StatusCode.cpp  Library.cpp main.cpp

OBJS =  $(addprefix $(OBJDIR)/,  $(notdir $(SRC:.cpp=.o)))

$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: all clean fclean re

all: $(NAME)

$(NAME): $(OBJS)
	@mkdir -p $(TMP)
	$(CXX) $(CXXFLAGS) $(OBJDIR)/*.o -o $(NAME)
clean:
	rm -rf $(OBJDIR)
fclean: clean
	rm -rf $(NAME) $(TMP)
re: fclean all