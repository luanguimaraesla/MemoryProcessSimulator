#ifndef GUI_GENERATION
#define GUI_GENERATION
#include <X11/Xlib.h>
#include "mms_structures.h"
#include "mms_thread_args.h"

#define MEMORY_UI_X 600
#define MEMORY_UI_Y 180
#define GREEN "#00FF00"
#define BLACK "#00000f"
#define RED "#FFFFFF"

struct UIParameters{
	Display *dis;
	Window win;
	double byte_size;
};

typedef struct UIParameters ui_param;

ui_param * createUI();
void printMemory(Memory *memory, ui_param *ui_params);

#endif
