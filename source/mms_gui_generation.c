#include "mms_gui_generation.h"
#include "mms_terminal_log_functions.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void printMemory(Memory *memory, ui_param *ui_params){
	MemoryCase *firstCase = memory->begin;
	static int green_rect_count;
	static int black_rect_count;
	static char available[15], used[15], running[15], total[15], status[15];
	static XColor black_col, green_col, red_col;
	static Colormap colormap_black, colormap, colormap_red;
	static GC black_gc, green_gc, red_gc;
	static bool setColor = true;
	static double begin, size;
	static double end, end_2;
	static XRectangle r, r2;
	Font font;
	XRectangle *green_rects = (XRectangle *) malloc (sizeof(XRectangle) * memory->running);
	XRectangle *black_rects = (XRectangle *) malloc (sizeof(XRectangle) * memory->running);
	if(setColor){
		/*green gc*/
		colormap = DefaultColormap(ui_params->dis, 0);
		green_gc = XCreateGC(ui_params->dis, ui_params->win, 0, 0);
		XParseColor(ui_params->dis, colormap, GREEN, &green_col);
		XAllocColor(ui_params->dis, colormap, &green_col);
		XSetForeground(ui_params->dis, green_gc, green_col.pixel);

		/*black gc*/
		colormap_black = DefaultColormap(ui_params->dis, 0);
		black_gc = XCreateGC(ui_params->dis, ui_params->win, 0, 0);
		XParseColor(ui_params->dis, colormap_black, BLACK, &black_col);
		XAllocColor(ui_params->dis, colormap_black, &black_col);
		XSetForeground(ui_params->dis, black_gc, black_col.pixel);

		/*red gc*/
		colormap_red = DefaultColormap(ui_params->dis, 0);
		red_gc = XCreateGC(ui_params->dis, ui_params->win, 0, 0);
		XParseColor(ui_params->dis, colormap_red, RED, &red_col);
		XAllocColor(ui_params->dis, colormap_red, &red_col);
		XSetForeground(ui_params->dis, red_gc, red_col.pixel);
	
		setColor = false;
	}

	XDrawRectangle(ui_params->dis, ui_params->win, red_gc, 150, 10, 604, 180);
	XFlush(ui_params->dis);

	/*array de retangulos para serem inseridos*/
	green_rect_count = 0;
	black_rect_count = 0;

	do{
		if(firstCase->type == hole){
			begin = ((firstCase->begin)*(ui_params->byte_size));
			size = ((firstCase->size)*(ui_params->byte_size));
			if((MEMORY_UI_X - begin) < size){

				end = (double)MEMORY_UI_X - begin;
				
				r.x = begin + 152;
				r.y = 12;
				r.width = end;
				r.height = 176;

				green_rects[green_rect_count] = r;
				green_rect_count++;

				end_2 = (double)((size + begin)- (double)MEMORY_UI_X);

				r2.x = 152;
				r2.y = 12;
				r2.width = end_2;
				r2.height = 176;

				green_rects[green_rect_count] = r2;
				green_rect_count++;

			} else {

				r.x = begin + 152;
				r.y = 12;
				r.width = size;
				r.height = 176;

				green_rects[green_rect_count] = r;
				green_rect_count++;
			}
		}
		else{
			begin = ((firstCase->begin)*(ui_params->byte_size));
			size = ((firstCase->size)*(ui_params->byte_size));
			if((MEMORY_UI_X - begin) < size){

				end = (double)MEMORY_UI_X - begin;
		
				r.x = begin + 152;
				r.y = 12;
				r.width = end;
				r.height = 176;

				black_rects[black_rect_count] = r;
				black_rect_count++;
				
				end_2 = (double)((size + begin)- (double)MEMORY_UI_X);

				r2.x = 152;
				r2.y = 12;
				r2.width = end_2;
				r2.height = 176;

				black_rects[black_rect_count] = r2;
				black_rect_count++;

			} else {
				r.x = begin + 152;
				r.y = 12;
				r.width = size;
				r.height = 176;

				black_rects[black_rect_count] = r;
				black_rect_count++;

			}
		}

		/*insere os retangulos*/
		
		firstCase = firstCase->next;
	}while(firstCase != memory->begin);

	XFillRectangles(ui_params->dis, ui_params->win, black_gc, green_rects, green_rect_count);
	XFillRectangles(ui_params->dis, ui_params->win, green_gc, black_rects, black_rect_count);
	/*cria as strings*/
	sprintf(available, "%-14lu", memory->available);
	sprintf(used, "%-14lu", memory->inUse);
	sprintf(running, "%-14lu",memory->running);
	sprintf(total, "%-14lu",memory->total);

	/*limpa a area de string*/
	XClearArea(ui_params->dis, ui_params->win, 0,0, 140, 90, 0);

	/*desenha strings na tela.*/
	XDrawString(ui_params->dis, ui_params->win, green_gc, 10, 20, "Avaible: ", 8);
	XDrawString(ui_params->dis, ui_params->win, green_gc, 70, 20, available, 8);
	XDrawString(ui_params->dis, ui_params->win, green_gc, 10, 35, "Using:  ", 8);
	XDrawString(ui_params->dis, ui_params->win, green_gc, 70, 35, used, 8);
	XDrawString(ui_params->dis, ui_params->win, green_gc, 10, 50 , "Running:  ", 8);
	XDrawString(ui_params->dis, ui_params->win, green_gc, 70, 50, running, 8);
	XDrawString(ui_params->dis, ui_params->win, green_gc, 10, 75, "Total:  ", 8);
	XDrawString(ui_params->dis, ui_params->win, green_gc, 70, 75, total, 8);

	XFillRectangle(ui_params->dis, ui_params->win, green_gc, 10, 90, 100, 40);
	if(ui_params->status == 0)
		strcpy(status, "Running");
	else
		strcpy(status, "Paused ");
	font = XLoadFont(ui_params->dis, "*x16");
	XSetFont(ui_params->dis,black_gc,font);
	XDrawString(ui_params->dis, ui_params->win, black_gc, 30, 120, status, 7);
	XFlush(ui_params->dis);

	/*printMemoryTerminal(memory);*/

}

ui_param * createUI(fu_arg *frame_update_args){
	Display *dis;
	Window win;
	ui_param *ui;
	dis = XOpenDisplay(NULL);
	win = XCreateSimpleWindow(dis, RootWindow(dis, 0), 1, 1, 800, 200, 0, BlackPixel (dis, 0), BlackPixel(dis, 0));
	XMapWindow(dis, win);
	XFlush(dis);
	ui = (ui_param*)malloc(sizeof(ui_param));
	ui->win = win;
	ui->dis = dis;
	return ui;
}
