SOURCES = laser.cpp camera.cpp gui.cpp
PROGRAM = laser

CXX = ccache g++
CCFLAGS = -fdiagnostics-color=always -pipe -Ofast -mfpu=neon-vfpv4 -mfloat-abi=hard -mcpu=cortex-a7 -Wall -Wstrict-aliasing=0
INCLUDES = -pthread -lm -lopencv_core -lopencv_imgproc -lopencv_calib3d -lopencv_highgui

OBJ_DIR = obj/

BUILD_PRINT = \033[1;34mBuilding $<\033[0m
OBJS = ${SOURCES:%.cpp=%.o}

.SUFFIXES: .cpp .o

.cpp.o:
	@echo "$(BUILD_PRINT)"
	@${CXX} ${CCFLAGS} -c $< -o $(OBJ_DIR)$@ ${INCLUDES}

$(PROGRAM): ${OBJS}
	@echo "$(BUILD_PRINT)"
	@${CXX} ${CCFLAGS} ${OBJS:%=$(OBJ_DIR)%} -o $(PROGRAM) ${INCLUDES}
	
clean:
	rm -rf $(OBJ_DIR)*.o $(PROGRAM)
