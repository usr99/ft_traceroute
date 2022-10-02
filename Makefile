##################################################
#			GLOBAL VARIABLES DEFINITION			 #
##################################################

TARGET	= ft_traceroute

CFLAGS	= -Wall -Wextra -g #-Werror
CC		= gcc

INC 	= -I ./include -I ./libft

SRCDIR	= ./src/
SRC		= main.c tracerouting.c options.c name_resolution.c packets.c utils.c debug.c
HEADERS = ft_traceroute.h options.h packets.h
DEPS = ${addprefix include/, ${HEADERS}}

OBJDIR	= ./objs/
OBJS	= ${addprefix ${OBJDIR}, ${SRC:.c=.o}}

LIBFT	= libft/libft.a
LIBS	= -lpthread

##################################################
#			OUTPUT VARIABLES DEFINITION			 #
##################################################

RED = \e[1;31m
GREEN = \e[1;32m
BLUE = \e[1;34m
PURPLE = \e[0;35m
RESET = \e[0;0m

COMPILE = ${GREEN}Compiling${RESET}
BUILD = ${BLUE}Building${RESET}
CLEAN = ${RED}Cleaning${RESET}

##################################################
#				COMPILATION RULES				 #
##################################################

${OBJDIR}%.o: ${SRCDIR}%.c ${DEPS}
	@echo "${COMPILE} $<"
	@${CC} ${CFLAGS} -c $< ${INC} -o $@

${TARGET}: ${OBJDIR} ${OBJS} ${LIBFT}
	@echo "${BUILD} $@"
	@${CC} ${OBJS} ${LIBFT} -o $@ ${LIBS}
	@echo "${PURPLE}Program was built successfully, have fun playing with ${GREEN}$@"

${OBJDIR}:
	@mkdir -p ${OBJDIR}

${LIBFT}:
	@echo "${BUILD} libft"
	@${MAKE} --no-print-directory -C libft

##################################################
#  				   USUAL RULES					 #
##################################################

all: ${TARGET}

clean:
	@echo "${CLEAN} objects"
	@rm -rf ${OBJDIR}
	@echo "${CLEAN} libft"
	@${MAKE} --no-print-directory clean -C libft

fclean: clean
	@echo "${CLEAN} ${TARGET}"
	@rm -rf ${TARGET}
	@${MAKE} --no-print-directory fclean -C libft

re: fclean all

.PHONY:	all clean fclean re
