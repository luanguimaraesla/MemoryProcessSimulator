#ifndef MMS_THREAD_ARGS
#define MMS_THREAD_ARGS
#include "mms_structures.h"
#include "mms_gui_generation.h"

struct ExecProcessArg{
	struct Memory *memory;
	struct MemoryCase *selfCase;
};

struct RandomCreateProcessesArg{
	struct MemoryCase * (*addProcessFunction)(numberOfSpaces, priority, struct Memory *);
	unsigned int numberOfProcesses;
	numberOfSpaces maxProcessSize;
	priority maxPriorityIndex;
	_program_time maxProcessGenerateSleep;
	struct Memory *memory;
};

struct FrameUpdateArg{
	bool * frame_update;
	struct Memory *memory;
	struct UIParameters *ui_params;
	struct RandomCreateProcessesArg *aux;
	int insertionID;
};

typedef struct ExecProcessArg execution_arg;
typedef struct RandomCreateProcessesArg rcp_arg;
typedef struct FrameUpdateArg fu_arg;

#endif
