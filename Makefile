# Nome de projecto
PROJECT = LCD

uname_m = $(shell uname -m)
DETECTED_ARCHITECTURE=$(uname_m)
ifeq ($(ARCH), x86_64)
	CFLAGS += -m64
	LDFLAGS += -m64
endif
ifeq ($(ARCH), x86)
	CFLAGS += -m32
	LDFLAGS += -m32
endif
ifndef RELEASE
	CFLAGS += -g
	DEFINES += -DDEBUG
endif
ifndef VERBOSE
	CMD_PREFIX = @
endif

# inclui ficheiro de dependencias, caso exista
DEPENDENCY_FILE=$(ARCHITECTURE_PREFIX_DIR)dependencies.mk
ifneq ($(filter $(DEPENDENCY_FILE), $(wildcard $(DEPENDENCY_FILE))), )
	include $(DEPENDENCY_FILE)
endif

# Caminhos
ARCHITECTURE_PREFIX_DIR = $(DETECTED_ARCHITECTURE)/
OBJ_PREFIX_DIR = $(ARCHITECTURE_PREFIX_DIR)objs/
ifndef OUTPUT_DIR
	OUTPUT_DIR = $(ARCHITECTURE_PREFIX_DIR)
endif

# Compilacao
INCLUDE_PATH +=  -I .. -I $(shell pwd)
DEFINES += -DLINUX
CFLAGS +=  -Wall -Wextra $(INCLUDE_PATH) $(DEFINES)

# Linkagem
LIB_PATH += 
LINKER_LIBS += 
LDFLAGS +=  $(LIB_PATH) $(LINKER_LIBS) 

# Quais os ficheiros de codigo. Esta parte devera ser copiada para APP ou LIB caso necessario
TARGET_C_SOURCES += I2C.c
TARGET_C_SOURCES += test.c
TARGET_C_SOURCES += LCD.c

# Quais os objectos a gerar.
TEMP_TARGET_ALL_OBJECTS = $(patsubst %.c,%.c.o,$(TARGET_C_SOURCES)) $(patsubst %.cpp,%.cpp.o,$(TARGET_CPP_SOURCES)) $(patsubst %.moc.cpp,%.moc.cpp.o,$(TARGET_MOCS))
TARGET_ALL_OBJECTS = $(addprefix $(OBJ_PREFIX_DIR),$(TEMP_TARGET_ALL_OBJECTS))

.DEFAULT_GOAL = app
OUTPUT_APP = $(OUTPUT_DIR)/$(PROJECT)
app : $(OUTPUT_APP)
export CFLAGS LDFLAGS

DEPENDENCY_PROJECTS += 
dependency_projects: | $(ARCHITECTURE_PREFIX_DIR)
	$(foreach dep_project,$(DEPENDENCY_PROJECTS),ARCH=$(DETECTED_ARCHITECTURE) make -C $(dep_project);)

clean:
	@echo [ $(PROJECT) $(DETECTED_ARCHITECTURE) ] Cleaning up $(PROJECT)
	$(CMD_PREFIX)$(RM) $(TARGET_ALL_OBJECTS) $(OUTPUT_APP) $(OUTPUT_LIB) $(TARGET_MOCS) $(ARCHITECTURE_PREFIX_DIR)dependencies.mk.*
	$(CMD_PREFIX)rm -Rf $(ARCHITECTURE_PREFIX_DIR)
	$(foreach dep_project,$(DEPENDENCY_PROJECTS),ARCH=$(DETECTED_ARCHITECTURE) make -C $(dep_project) clean;)
	$(CMD_PREFIX)echo [ $(PROJECT) $(DETECTED_ARCHITECTURE) ] Cleanup complete

$(OBJ_PREFIX_DIR):
	$(CMD_PREFIX)mkdir -p $(OBJ_PREFIX_DIR)
$(ARCHITECTURE_PREFIX_DIR):
	$(CMD_PREFIX)mkdir -p $(ARCHITECTURE_PREFIX_DIR)

$(ARCHITECTURE_PREFIX_DIR)dependencies.mk: $(TARGET_ALL_SOURCES) | $(OBJ_PREFIX_DIR)
	@echo [ $(PROJECT) $(DETECTED_ARCHITECTURE) ] Building dependency list
	$(CMD_PREFIX)$(CC) $(CFLAGS) -MM $(TARGET_CPP_SOURCES) $(TARGET_C_SOURCES) > $(ARCHITECTURE_PREFIX_DIR)dependencies.mk

$(OUTPUT_APP) : $(TARGET_ALL_OBJECTS) dependency_projects
	@echo -e "[ $(PROJECT) $(DETECTED_ARCHITECTURE) ] Linking application $(OUTPUT_APP)"
	$(CMD_PREFIX)$(CC) $(CFLAGS) $(TARGET_ALL_OBJECTS) -o $(OUTPUT_APP) $(LDFLAGS)

$(OBJ_PREFIX_DIR)%.c.o: %.c $(DEPENDENCY_FILE) | $(OBJ_PREFIX_DIR)
	@echo -e "[ $(PROJECT) $(DETECTED_ARCHITECTURE) ] Compiling $< "
	@mkdir -p `dirname $(OBJ_PREFIX_DIR)$*`
	$(CMD_PREFIX)$(CC) $(CFLAGS) -c $< -o $(OBJ_PREFIX_DIR)$*.c.o

$(OBJ_PREFIX_DIR)%.cpp.o: %.cpp $(DEPENDENCY_FILE) | $(OBJ_PREFIX_DIR)
	@echo -e "[ $(PROJECT) $(DETECTED_ARCHITECTURE) ] Compiling $< "
	@mkdir -p `dirname $(OBJ_PREFIX_DIR)$*`
	$(CMD_PREFIX)$(CXX) $(CFLAGS) -c $< -o $(OBJ_PREFIX_DIR)$*.cpp.o
