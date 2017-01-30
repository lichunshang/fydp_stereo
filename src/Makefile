################################################################################
# stereoCamera Makefile.
################################################################################


PROJECT_ROOT=..

BASE_NAME = stereoCamera
NAME      = ${BASE_NAME}${D}
#LIBS      = -L${PROJECT_ROOT}/lib -lflycapture${D} ${FC2_DEPS}
OUTDIR    = ${PROJECT_ROOT}/bin
CXX_SRCS  = ${wildcard *.cpp}
C_SRCS    = ${wildcard *.c}
CXX_OBJS  = ${CXX_SRCS:.cpp=.o}
C_OBJS    = ${C_SRCS:.c=.o}
OBJS      = $(C_OBJS) $(CXX_OBJS)
INC_DIRS  = ${PROJECT_ROOT}/include /usr/include/flycapture
LIB_DIRS  = ${PROJECT_ROOT}/lib
LIBRARIES = flycapture
MV        = mv
RM        = rm
OBJ_FILES = *.o

CXXFLAGS  += -Wall -Wextra -ggdb
CPPFLAGS += $(foreach includedir,$(INC_DIRS),-I$(includedir))
LDFLAGS += $(foreach librarydir,$(LIB_DIRS),-L$(librarydir))
LDFLAGS += $(foreach library,$(LIBRARIES),-l$(library))

print-%  : ; @echo $* = $($*)

.PHONY: all clean distclean
all: $(NAME)

$(NAME): $(OBJS)
	${CXX} ${CXXFLAGS} ${CPPFLAGS} ${OBJS} -o ${NAME} ${LDFLAGS}
	-@${MV} ${NAME} ${PROJECT_ROOT}/${NAME}
	-@${MV} ${OBJ_FILES} ${OUTDIR}/

clean:
	@- $(RM) ${PROJECT_ROOT}/$(NAME)
	@- $(RM) ${OUTDIR}/$(OBJ_FILES)
	@- $(RM) $(OBJ_FILES)

distclean: clean

define OBJECT_DEPENDS_ON_CORRESPONDING_HEADER
    $(1) : ${1:.o=.h}
endef

$(foreach object_file,$(program_OBJS),$(eval $(call OBJECT_DEPENDS_ON_CORRESPONDING_HEADER,$(object_file))))
