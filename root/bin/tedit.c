#include "stdio.h"
#include "syscalls.h"
#include "stdlib.h"
#include "gfx.h"
#include "string.h"
#include "tedit.h"

#define WIDTH 800
#define HEIGHT 500
#define STATUS_HEIGHT CHAR_HEIGHT

PTextRegion alloc_text_region()
{

	return (PTextRegion)calloc(sizeof(TextRegion));

}

PLineTextRegion alloc_line_text_region()
{

	return (PLineTextRegion)calloc(sizeof(LineTextRegion));

}


PLineTextRegion load_file(int fd)
{

	PLineTextRegion base_text_region = alloc_line_text_region();
	PLineTextRegion cur_line_text_region = base_text_region;
	PTextRegion cur_text_region = &base_text_region->r;

	while(true)
	{

		char c;

		if (!fread(fd, &c, 1))
			break;

		cur_text_region->buffer[cur_text_region->pos++] = c;

		if (cur_text_region->pos == TEXT_REGION_BUFFER_SIZE)
		{

			cur_text_region->next = alloc_text_region();
			cur_text_region->next->prev = cur_text_region;
			cur_text_region = cur_text_region->next;

		}

		if (c == '\n')
		{

			cur_line_text_region->next_line = alloc_line_text_region();
			cur_line_text_region->next_line->prev_line = cur_line_text_region;
			cur_line_text_region->next_line->line = cur_line_text_region->line + 1;
			cur_line_text_region = cur_line_text_region->next_line;
			cur_text_region = &cur_line_text_region->r;

		}

	}

	return base_text_region;

}

void write_file(int fd, PLineTextRegion ltr)
{

	while(ltr)
	{

		PTextRegion tr = &ltr->r;

		while(tr)
		{
			fwrite(fd,tr->buffer,tr->pos);
			tr = tr->next;
		}

		ltr = ltr->next_line;

	}

}

void display_status(PWINDOW win, char* font, size_t top_line, size_t left_col, size_t cursor_line, size_t cursor_col, bool is_insert)
{

	gfx_fill_rect(win, 0, HEIGHT, WIDTH, STATUS_HEIGHT, 0);

	char cursor_line_buf[14] = "Ln xxxxxxxxxx";
	char cursor_col_buf[17] = ", Col xxxxxxxxxx";

	itoa(cursor_line,cursor_line_buf+3,10);
	itoa(cursor_col,cursor_col_buf+6,10);

	int i = 0;
	while(cursor_line_buf[i])
	{

		gfx_paint_char_bg(win,cursor_line_buf[i],i*CHAR_WIDTH,HEIGHT,0,0xFFFFFF,font);

		i++;
		//cursor_line_buf++;

	}

	int j = 0;
	while(cursor_col_buf[j])
	{

		gfx_paint_char_bg(win,cursor_col_buf[j],i*CHAR_WIDTH,HEIGHT,0,0xFFFFFF,font);

		i++;
		j++;

	}

	if (is_insert)
		gfx_paint_char_bg(win,'i',(i + 1)*CHAR_WIDTH,HEIGHT,0xFFFFFF,0,font);

	display_window_section(win,0,HEIGHT,WIDTH,STATUS_HEIGHT);

}

void display_file(PWINDOW win, char* font, PLineTextRegion ltr, size_t top_line, size_t left_col, size_t cursor_line, size_t cursor_col)
{

	gfx_clear_win(win);

	while (ltr->line != top_line)
	{

		if (ltr->line > top_line)
			ltr = ltr->prev_line;
		else
			ltr = ltr->next_line;

	}

	PTextRegion cur_text_region = &ltr->r;
	size_t cur_pos = 0;
	size_t cur_line_pos = 0;

	while (ltr)
	{
		while(cur_text_region)
		{

			char c;

			c = cur_text_region->buffer[cur_pos];

			if (c != '\r' && c != '\n')
			{
				if (cur_line_pos >= left_col && ltr->line >= top_line && c != '\x00') 
					gfx_paint_char_bg(win,c,(cur_line_pos-left_col)*CHAR_WIDTH,(ltr->line-top_line)*CHAR_HEIGHT,0,0xFFFFFF,font);
			}
			else
			{
				break;
			}

			cur_pos++;
			cur_line_pos++;

			if (cur_pos >= cur_text_region->pos)
			{

				cur_text_region = cur_text_region->next;
				cur_pos = 0;

			}

			if (cur_line_pos >= left_col && (cur_line_pos-left_col)*CHAR_WIDTH >= WIDTH)
				break;

		}

		ltr = ltr->next_line;
		if (ltr)
		{
			cur_text_region = &ltr->r;
			cur_pos = 0;
			cur_line_pos = 0;

			if ((ltr->line-top_line)*CHAR_HEIGHT >= HEIGHT)
				break;
		}

	}

	int screen_cursor_row = (cursor_line-top_line)*CHAR_HEIGHT;
	int screen_cursor_col = (cursor_col-left_col)*CHAR_WIDTH;
	gfx_fill_rect(win,screen_cursor_col,screen_cursor_row,2,CHAR_HEIGHT,0xFFFFFF);

	display_window_section(win,0,0,WIDTH,HEIGHT);

}

bool advance_cursor_left(PLineTextRegion *p_ltr, PTextRegion *p_tr, size_t *p_cursor_line, size_t *p_cursor_col)
{

	PLineTextRegion ltr = *p_ltr;
	PTextRegion tr = *p_tr;

	size_t cursor_line = *p_cursor_line;
	size_t cursor_col = *p_cursor_col;

	if (cursor_col == 0)
	{

		if (ltr->prev_line)
		{

			*p_ltr = ltr->prev_line;
			tr = &ltr->prev_line->r;
			while(tr->next)
			{
				cursor_col += tr->pos;
				tr = tr->next;
			}
			cursor_col += tr->pos;
			*p_cursor_col = cursor_col - 2;
			*p_cursor_line = cursor_line - 1;
			*p_tr = tr;

		}
		else
		{
			return false;
		}

	}
	else
	{

		if (!(cursor_col % TEXT_REGION_BUFFER_SIZE))
		{

			*p_tr = tr->prev;

		}

		*p_cursor_col = cursor_col - 1;

	}

	return true;

}

bool advance_cursor_right(PLineTextRegion *p_ltr, PTextRegion *p_tr, size_t *p_cursor_line, size_t *p_cursor_col)
{

	PLineTextRegion ltr = *p_ltr;
	PTextRegion tr = *p_tr;

	size_t cursor_line = *p_cursor_line;
	size_t cursor_col = *p_cursor_col;


	if ((cursor_col%TEXT_REGION_BUFFER_SIZE == tr->pos-1 && tr->buffer[tr->pos-1] == '\n') || (cursor_col%TEXT_REGION_BUFFER_SIZE == tr->pos && tr->buffer[tr->pos-1] != '\n'))
	{

		if (ltr->next_line)
		{

			*p_ltr = ltr->next_line;
			cursor_col = 0;
			*p_cursor_col = cursor_col;
			*p_cursor_line = cursor_line + 1;
			*p_tr = &ltr->next_line->r;

		}
		else
		{
			return false;
		}

	}
	else
	{

		*p_cursor_col = cursor_col + 1;

		if (!((cursor_col+1) % TEXT_REGION_BUFFER_SIZE))
		{
			*p_tr = tr->next;

		}

	}

	return true;

}

bool advance_cursor_up(PLineTextRegion *p_ltr, PTextRegion *p_tr, size_t *p_cursor_line, size_t *p_cursor_col)
{

	PLineTextRegion ltr = *p_ltr;
	PTextRegion tr = *p_tr;

	size_t cursor_line = *p_cursor_line;
	size_t cursor_col = *p_cursor_col;

	if (ltr->prev_line)
	{

		*p_ltr = ltr->prev_line;
		tr = &ltr->prev_line->r;
		size_t pos_read = 0;
		while(tr->next && pos_read < cursor_col)
		{
			pos_read += tr->pos;
			tr = tr->next;
		}
		pos_read += tr->pos;
		if (pos_read < cursor_col)
		{
			if (tr->buffer[tr->pos-1] == '\n')
				cursor_col = pos_read-1;
			else
				cursor_col = pos_read;
		}
		else if (cursor_col <= pos_read && cursor_col > pos_read - 2)
			cursor_col = pos_read-1;
		*p_tr = tr;
		*p_cursor_col = cursor_col;
		*p_cursor_line = cursor_line - 1;

	}
	else
	{

		return false;

	}

	return true;

}

bool advance_cursor_down(PLineTextRegion *p_ltr, PTextRegion *p_tr, size_t *p_cursor_line, size_t *p_cursor_col)
{

	PLineTextRegion ltr = *p_ltr;
	PTextRegion tr = *p_tr;

	size_t cursor_line = *p_cursor_line;
	size_t cursor_col = *p_cursor_col;


	if (ltr->next_line)
	{

		*p_ltr = ltr->next_line;
		tr = &ltr->next_line->r;
		size_t pos_read = 0;
		while(tr->next && pos_read < cursor_col)
		{
			pos_read += tr->pos;
			tr = tr->next;
		}
		pos_read += tr->pos;
		if (pos_read < cursor_col)
		{
			if (tr->buffer[tr->pos-1] == '\n')
				cursor_col = pos_read-1;
			else
				cursor_col = pos_read;
		}
		else if(cursor_col <= pos_read && cursor_col > pos_read - 2)
			cursor_col = pos_read-1;

		*p_tr = tr;
		*p_cursor_col = cursor_col;
		*p_cursor_line = cursor_line + 1;

	}
	else
	{

		return false;

	}

	return true;
}

void fit_screen_to_cursor(size_t cursor_line, size_t cursor_col, size_t *p_top_line, size_t *p_left_col)
{
	
	int screen_cursor_row = (cursor_line-*p_top_line)*CHAR_HEIGHT;
	int screen_cursor_col = (cursor_col-*p_left_col)*CHAR_WIDTH;
	if (screen_cursor_row < 0)
		*p_top_line = cursor_line;
	else if (screen_cursor_row > HEIGHT)
		*p_top_line = (cursor_line-(int)(HEIGHT/CHAR_HEIGHT));
	else if (screen_cursor_col < 0)
		*p_left_col = cursor_col;
	else if (screen_cursor_col > WIDTH)
		*p_left_col = (cursor_col-(int)(WIDTH/CHAR_WIDTH)+1);

}

bool append_at_position(PTextRegion tr, size_t cursor_col, char c)
{

	if (c == '\b')
		return remove_at_position(tr, cursor_col);
	else if (c != '\n')
	{

		PTextRegion tmp_tr = tr;

		while (tmp_tr->next)
			tmp_tr = tmp_tr->next;

		if (tmp_tr->pos == TEXT_REGION_BUFFER_SIZE && !tmp_tr->next)
		{
			tmp_tr->next = alloc_text_region();
			tmp_tr->next->prev = tmp_tr;
			tmp_tr = tmp_tr->next;
		}

		PTextRegion last_tr = tmp_tr;

		while (tmp_tr != tr)
		{
			for (int i = tmp_tr->pos - 1; i >= 0; i--)
			{

				if (i + 1 != TEXT_REGION_BUFFER_SIZE)
					tmp_tr->buffer[i+1] = tmp_tr->buffer[i];
				else
					tmp_tr->next->buffer[0] = tmp_tr->buffer[i];

			}

			tmp_tr = tmp_tr->prev;

		}

		for (int i = tr->pos - 1; i >= (int)(cursor_col%TEXT_REGION_BUFFER_SIZE); i--)
		{

			if ((i + 1) != TEXT_REGION_BUFFER_SIZE)
				tr->buffer[i+1] = tr->buffer[i];
			else
				tr->next->buffer[0] = tr->buffer[i];

		}

		tr->buffer[cursor_col%TEXT_REGION_BUFFER_SIZE] = c;

		last_tr->pos++;

	}
	else
	{

		PLineTextRegion new_ltr = alloc_line_text_region();

		PTextRegion new_tr = &new_ltr->r;

		for (int i = (int)(cursor_col%TEXT_REGION_BUFFER_SIZE), j = 0; i < tr->pos; i++, j++)
			new_tr->buffer[j] = tr->buffer[i];

		new_tr->pos = tr->pos - (int)(cursor_col%TEXT_REGION_BUFFER_SIZE);

		PTextRegion tmp_tr = tr->next;

		while(tmp_tr)
		{

			for (int i = 0; i < tmp_tr->pos; i++)
			{

				new_tr->buffer[new_tr->pos++] = tmp_tr->buffer[i];

				if (new_tr->pos == TEXT_REGION_BUFFER_SIZE)
				{

					new_tr->next = alloc_text_region();
					new_tr->next->prev = new_tr;
					new_tr = new_tr->next;

				}

			}

			PTextRegion tmp_tmp_tr = tmp_tr;

			tmp_tr = tmp_tr->next;

			free(tmp_tmp_tr);

		}

		tr->next = 0;

		tr->pos = cursor_col%TEXT_REGION_BUFFER_SIZE + 2;
		tr->buffer[cursor_col%TEXT_REGION_BUFFER_SIZE] = '\r';
		tr->buffer[cursor_col%TEXT_REGION_BUFFER_SIZE+1] = '\n';

		while(tr->prev)
		 	tr = tr->prev;

		PLineTextRegion ltr = (PLineTextRegion)tr;
		new_ltr->next_line = ltr->next_line;
		if (ltr->next_line)
			new_ltr->next_line->prev_line = new_ltr;
		ltr->next_line = new_ltr;
		new_ltr->prev_line = ltr;
		new_ltr->line = ltr->line;

		while(new_ltr)
		{
			new_ltr->line = new_ltr->line + 1;
			new_ltr = new_ltr->next_line;
		}

	}

	return true;

}

bool insert_at_position(PTextRegion tr, size_t cursor_col, char c)
{

	if (c == '\n' || c == '\b' || (cursor_col%TEXT_REGION_BUFFER_SIZE == tr->pos-2 && tr->buffer[tr->pos-1] == '\n') || (cursor_col%TEXT_REGION_BUFFER_SIZE == tr->pos && tr->buffer[tr->pos-1] != '\n'))
		return append_at_position(tr,cursor_col,c);

	tr->buffer[cursor_col%TEXT_REGION_BUFFER_SIZE] = c;

	return true;

}

bool remove_at_position(PTextRegion tr, size_t cursor_col)
{

	if (cursor_col == 0)
	{

		while(tr->prev)
			tr = tr->prev;

		PLineTextRegion ltr = (PLineTextRegion)tr;

		PLineTextRegion prev_ltr = ltr->prev_line;

		if (prev_ltr)
		{

			prev_ltr->next_line = ltr->next_line;
			if (ltr->next_line)
				ltr->next_line->prev_line = prev_ltr;

			PTextRegion prev_tr = &prev_ltr->r;

			prev_ltr = prev_ltr->next_line;
			while (prev_ltr)
			{
				prev_ltr->line--;
				prev_ltr = prev_ltr->next_line;
			}

			while (prev_tr->next)
				prev_tr = prev_tr->next;

			prev_tr->pos -= 2;

			while (tr)
			{

				for (int i = 0; i < tr->pos; i++)
				{

					prev_tr->buffer[prev_tr->pos++] = tr->buffer[i];

					if (prev_tr->pos == TEXT_REGION_BUFFER_SIZE)
					{

						prev_tr->next = alloc_text_region();
						prev_tr->next->prev = prev_tr;
						prev_tr = prev_tr->next;

					}

				}

				PTextRegion tmp_tr = tr;

				tr = tr->next;

				free(tmp_tr);

			}

		}
		else
			return false;

	}
	else
	{

		cursor_col--;

		for (int i = cursor_col%TEXT_REGION_BUFFER_SIZE; i < tr->pos; i++)
		{

			if (i + 1 < TEXT_REGION_BUFFER_SIZE)
				tr->buffer[i] = tr->buffer[i+1];
			else
			{
				if (tr->next)
					tr->buffer[i] = tr->next->buffer[0];
			}
			
		}

		PTextRegion last_tr;

		if (!tr->next)
			last_tr = tr;

		tr = tr->next;

		while(tr)
		{

			for (int i = 0; i < tr->pos; i++)
			{

				if (i + 1 < TEXT_REGION_BUFFER_SIZE)
					tr->buffer[i] = tr->buffer[i+1];
				else
				{
					if (tr->next)
						tr->buffer[i] = tr->next->buffer[0];
				}

			}

			if (!tr->next)
				last_tr = tr;

			tr = tr->next;

		}

		if (last_tr->pos == 0)
			if (last_tr->prev)
			{
				last_tr->prev->next = 0;
				free(last_tr);
			}
			else
				return false;
		else
			last_tr->pos--;

	}

	return true;

}

WINDOW window;
char font_buff[256*16];
bool is_insert;

void _main(int argc, char **argv)
{

	if (argc != 2)
	{

		print("INCORRECT FORMAT ERROR: tedit FILEPATH");
		terminate();

	}

	char *filepath = argv[1];

	int file_desc = fopen(filepath);

	if (!file_desc)
	{

		printf("OPEN ERROR: file %s does not exist", filepath);
		terminate();

	}

	PLineTextRegion base_text_region = load_file(file_desc);
	PLineTextRegion cur_line_text_region = base_text_region;
	PTextRegion cur_text_region = &cur_line_text_region->r;
	size_t top_line = 0;
	size_t left_col = 0;

	size_t cursor_line = 0;
	size_t cursor_col = 0;

	fclose(file_desc);

	window.w_name = "text editor";
	window.event_handler.event_mask = GENERAL_EVENT_KBD;
	create_window(&window,100,100,WIDTH,HEIGHT+STATUS_HEIGHT);
	load_font((void *)font_buff);

	display_file(&window,font_buff,cur_line_text_region,top_line,left_col,cursor_line,cursor_col);
	display_status(&window,font_buff,top_line,left_col,cursor_line,cursor_col, is_insert);

	while(1)
	{

		EVENT e;

		get_window_event(&window,&e);

		if (e.event_type == EVENT_KBD_PRESS)
		{

			char scancode = e.event_data&0xFF;
			bool caps = e.event_data&0x100;
			bool shift = e.event_data&0x200;
			bool alt = e.event_data&0x400;

			bool action = false;

			char kbd_ascii;

			if (shift || caps)
				kbd_ascii = kbdus_shift[scancode];
			else
				kbd_ascii = kbdus[scancode];

			if (alt)
			{
				if (kbd_ascii == 'i')
				{
					is_insert = !is_insert;
					display_status(&window,font_buff,top_line,left_col,cursor_line,cursor_col, is_insert);
				}

				continue;
			}

			if (scancode == K_ESCAPE)
				break;
			else if (scancode == K_LEFT)
				action = advance_cursor_left(&cur_line_text_region, &cur_text_region, &cursor_line, &cursor_col);
			else if (scancode == K_RIGHT)
				action = advance_cursor_right(&cur_line_text_region, &cur_text_region, &cursor_line, &cursor_col);
			else if (scancode == K_UP)
				action = advance_cursor_up(&cur_line_text_region, &cur_text_region, &cursor_line, &cursor_col);
			else if (scancode == K_DOWN)
				action = advance_cursor_down(&cur_line_text_region, &cur_text_region, &cursor_line, &cursor_col);
			else if (kbd_ascii)
			{
				if (is_insert)
					action = insert_at_position(cur_text_region,cursor_col,kbd_ascii);
				else
					action = append_at_position(cur_text_region,cursor_col,kbd_ascii);
				if (action)
				{
					if (kbd_ascii == '\b')
						advance_cursor_left(&cur_line_text_region, &cur_text_region, &cursor_line, &cursor_col);
					else
						advance_cursor_right(&cur_line_text_region, &cur_text_region, &cursor_line, &cursor_col);
				}
			}

			if (action)
			{
				fit_screen_to_cursor(cursor_line,cursor_col, &top_line,&left_col);
				display_file(&window,font_buff,cur_line_text_region,top_line,left_col,cursor_line,cursor_col);
				display_status(&window,font_buff,top_line,left_col,cursor_line,cursor_col, is_insert);
			}

		}

	}

	file_desc = fopen(filepath);
	write_file(file_desc,base_text_region);
	fclose(file_desc);

	remove_window(&window);

	terminate();
	__builtin_unreachable();
}