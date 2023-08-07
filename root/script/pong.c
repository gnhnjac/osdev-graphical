#include <gfx.h>
#include <syscalls.h>
#include <stdlib.h>
#include <stdio.h>

int main_stub (int argc, char *args[]);

#define SCREEN_WIDTH 600	//window height
#define SCREEN_HEIGHT 440	//window width

uint8_t font_buff[256*16];

//function prototypes
//initilise SDL
int init(int w, int h, int argc, char *args[]);

typedef struct ball_s {

	int x, y; /* position on the screen */
	int w,h; // ball width and height
	int dx, dy; /* movement vector */
} ball_t;

typedef struct paddle {

	int x,y;
	int w,h;
} paddle_t;

// Program globals
static ball_t ball;
static paddle_t paddle[2];
int score[] = {0,0};
int width, height;		//used if fullscreen

WINDOW window;

//inisilise starting position and sizes of game elemements
static void init_game() {
	
	ball.x = SCREEN_WIDTH / 2;
	ball.y = SCREEN_HEIGHT / 2;
	ball.w = 10;
	ball.h = 10;
	ball.dy = 1;
	ball.dx = 1;
	
	paddle[0].x = 20;
	paddle[0].y = SCREEN_HEIGHT / 2 - 50 ;
	paddle[0].w = 10;
	paddle[0].h = 50;

	paddle[1].x = SCREEN_WIDTH - 20 - 10;
	paddle[1].y = SCREEN_HEIGHT / 2 - 50;
	paddle[1].w = 10;
	paddle[1].h = 50;
}

int check_score() {
	
	int i;

	//loop through player scores
	for(i = 0; i < 2; i++) {
	
		//check if score is @ the score win limit
		if (score[i] == 10 ) {
		
			//reset scores
			score[0] = 0;
			score[1] = 0;
			
			//return 1 if player 1 score @ limit
			if (i == 0) {

				return 1;	

			//return 2 if player 2 score is @ limit
			} else {
				
				return 2;
			}
		}
	}
	
	//return 0 if no one has reached a score of 10 yet
	return 0;
}

//if return value is 1 collision occured. if return is 0, no collision.
int check_collision(ball_t a, paddle_t b) {

	int left_a, left_b;
	int right_a, right_b;
	int top_a, top_b;
	int bottom_a, bottom_b;

	left_a = a.x;
	right_a = a.x + a.w;
	top_a = a.y;
	bottom_a = a.y + a.h;

	left_b = b.x;
	right_b = b.x + b.w;
	top_b = b.y;
	bottom_b = b.y + b.h;
	

	if (left_a > right_b) {
		return 0;
	}

	if (right_a < left_b) {
		return 0;
	}

	if (top_a > bottom_b) {
		return 0;
	}

	if (bottom_a < top_b) {
		return 0;
	}

	return 1;
}

/* This routine moves each ball by its motion vector. */
static void move_ball() {
	
	/* Move the ball by its motion vector. */
	ball.x += ball.dx;
	ball.y += ball.dy;
	
	/* Turn the ball around if it hits the edge of the screen. */
	if (ball.x < 0) {
		
		score[1] += 1;
		init_game();
	}

	if (ball.x > SCREEN_WIDTH - 10) { 
		
		score[0] += 1;
		init_game();
	}

	if (ball.y < 0 || ball.y > SCREEN_HEIGHT - 10) {
		
		ball.dy = -ball.dy;
	}

	//check for collision with the paddle
	int i;

	for (i = 0; i < 2; i++) {
		
		int c = check_collision(ball, paddle[i]); 

		//collision detected	
		if (c == 1) {
			
			//ball moving left
			if (ball.dx < 0) {
					
				ball.dx -= 1;

			//ball moving right
			} else {
					
				ball.dx += 1;
			}
			
			//change ball direction
			ball.dx = -ball.dx;
			
			//change ball angle based on where on the paddle it hit
			int hit_pos = (paddle[i].y + paddle[i].h) - ball.y;

			if (hit_pos >= 0 && hit_pos < 7) {
				ball.dy = 4;
			}

			if (hit_pos >= 7 && hit_pos < 14) {
				ball.dy = 3;
			}
			
			if (hit_pos >= 14 && hit_pos < 21) {
				ball.dy = 2;
			}

			if (hit_pos >= 21 && hit_pos < 28) {
				ball.dy = 1;
			}

			if (hit_pos >= 28 && hit_pos < 32) {
				ball.dy = 0;
			}

			if (hit_pos >= 32 && hit_pos < 39) {
				ball.dy = -1;
			}

			if (hit_pos >= 39 && hit_pos < 46) {
				ball.dy = -2;
			}

			if (hit_pos >= 46 && hit_pos < 53) {
				ball.dy = -3;
			}

			if (hit_pos >= 53 && hit_pos <= 60) {
				ball.dy = -4;
			}

			//ball moving right
			if (ball.dx > 0) {

				//teleport ball to avoid mutli collision glitch
				if (ball.x < 30) {
				
					ball.x = 30;
				}
				
			//ball moving left
			} else {
				
				//teleport ball to avoid mutli collision glitch
				if (ball.x > 600) {
				
					ball.x = 600;
				}
			}
		}
	}
}

static void move_paddle_ai() {

	int center = paddle[0].y + 25;
	int screen_center = SCREEN_HEIGHT / 2 - 25;
	int ball_speed = ball.dy;

	if (ball_speed < 0) {
	
		ball_speed = -ball_speed;
	}

	//ball moving right
	if (ball.dx > 0) {
		
		//return to center position
		if (center < screen_center) {
			
			paddle[0].y += ball_speed;
		
		} else {
		
			paddle[0].y -= ball_speed;	
		}

	//ball moving left
	} else {
	
		//ball moving down
		if (ball.dy > 0) { 
			
			if (ball.y > center) { 
				
				paddle[0].y += ball_speed;

			} else { 
			
				paddle[0].y -= ball_speed;
			}
		}
		
		//ball moving up
		if (ball.dy < 0) {
		
			if (ball.y < center) { 
				
				paddle[0].y -= ball_speed;
			
			} else {
			
				paddle[0].y += ball_speed;
			}
		}

		//ball moving stright across
		if (ball.dy == 0) {
			
			if (ball.y < center) { 
			
				paddle[0].y -= 5;

			} else {
			
				paddle[0].y += 5;
			}
		}	 		
	}
}

static void move_paddle(int d) {

	// if the down arrow is pressed move paddle down
	if (d == 0) {
		
		if(paddle[1].y >= SCREEN_HEIGHT - paddle[1].h) {
		
			paddle[1].y = SCREEN_HEIGHT - paddle[1].h;
		
		} else { 
		
			paddle[1].y += 5;
		}
	}
	
	// if the up arrow is pressed move paddle up
	if (d == 1) {

		if(paddle[1].y <= 0) {
		
			paddle[1].y = 0;

		} else {
		
			paddle[1].y -= 5;
		}
	}
}

static void draw_game_over(int p) { 
	
	gfx_fill_rect(&window,0,0,window.width,window.height,0);
	gfx_paint_bmp16(&window,"a:\\pong\\gameover.bmp",0,0);
	//draw to the display
	display_window_section(&window,0,0,window.width,window.height);
	
}

static void draw_menu() {
	gfx_fill_rect(&window,0,0,window.width,window.height,0);
	gfx_paint_bmp16(&window,"a:\\pong\\title.bmp",0,0);
	//draw to the display
	display_window_section(&window,0,0,window.width,window.height);

}

static void draw_background() {
 
	
}

static void draw_net() {

	//draw the net
	int i,r;

	for(i = 0; i < 15; i++) {
		
		gfx_fill_rect(&window, SCREEN_WIDTH / 2,20+30*i,5,15, 0xf);


	}
	//draw to the display
	display_window_section(&window,SCREEN_WIDTH / 2,20,5,window.height);
}

static void draw_ball(uint8_t color) {

	if (color != 0)
	{

		gfx_fill_rect(&window, ball.x,ball.y,ball.w,ball.h, color);
		//draw to the display
		display_window_section(&window,ball.x,ball.y,ball.w,ball.h);

	}
	else
	{
		gfx_fill_rect(&window, ball.x-5,ball.y-5,ball.w+10,ball.h+10, color);
		//draw to the display
		display_window_section(&window,ball.x-5,ball.y-5,ball.w+10,ball.h+10);
	}
}

static void draw_paddle(uint8_t color) {

	int i;

	for (i = 0; i < 2; i++) {

		gfx_fill_rect(&window, paddle[i].x,paddle[i].y,paddle[i].w,paddle[i].h, color);
		//draw to the display
		display_window_section(&window,paddle[i].x,paddle[i].y,paddle[i].w,paddle[i].h);
	
	}
}

static void draw_player_0_score() {
	
	uint32_t dest_x = (SCREEN_WIDTH / 2) - 12 - CHAR_WIDTH;

	gfx_paint_char_bg(&window,score[0]+'0',dest_x,0,0,0x4,font_buff);
	display_window_section(&window,dest_x,0,CHAR_WIDTH,CHAR_HEIGHT);

}

static void draw_player_1_score() {

	uint32_t dest_x = (SCREEN_WIDTH / 2) + 12;
	
	gfx_paint_char_bg(&window,score[1]+'0',dest_x,0,0,0x4,font_buff);
	display_window_section(&window,dest_x,0,CHAR_WIDTH,CHAR_HEIGHT);

}

void _main (int argc, char *args[]) {
		
	//SDL Window setup
	if (init(SCREEN_WIDTH, SCREEN_HEIGHT, argc, args) == 1) {
		
		terminate();
	}
	
	width = window.width;
	height = window.height;
	
	int quit = 0;
	int state = 0;
	int r = 0;
	bool drawn = false;
	
	// Initialize the ball position data. 
	init_game();
	
	//render loop
	while(quit == 0) {
	

		EVENT e;

		get_window_event(&window,&e);
		
		//display main menu
		if (state == 0 ) {
			
			if (e.event_type == EVENT_KBD_PRESS)
			{

				char scancode = e.event_data&0xFF;
				if (scancode == K_SPACE) {
				
					state = 1;
					gfx_fill_rect(&window,0,0,window.width,window.height,0);
					display_window_section(&window,0,0,window.width,window.height);
				}

			}
		
			//draw menu 
			if (!drawn)
			{
				draw_menu();
				drawn = true;
			}
		
		//display gameover
		} else if (state == 2) {
			
			if (e.event_type == EVENT_KBD_PRESS)
			{

				char scancode = e.event_data&0xFF;

				if (scancode == K_SPACE) {
				
					state = 0;
					drawn = false;
					gfx_fill_rect(&window,0,0,window.width,window.height,0);
					display_window_section(&window,0,0,window.width,window.height);
					sleep(500);
				}
				if (scancode == K_ESCAPE) {
			
					quit = 1;
				}

			}

			if (r == 1) {

				//if player 1 is AI if player 1 was human display the return value of r not 3
				if (!drawn)
				{
					draw_game_over(3);
					drawn = true;
				}

			} else {
			
				//display gameover
				if (!drawn)
				{
					draw_game_over(r);
					drawn = true;
				}
			}
				
		//display the game
		} else if (state == 1) {
			
			//check score
			r = check_score();
		
			//if either player wins, change to game over state
			if (r == 1) {
				
				state = 2;	
				gfx_fill_rect(&window,0,0,window.width,window.height,0);
				display_window_section(&window,0,0,window.width,window.height);
				drawn = false;

			} else if (r == 2) {
			
				state = 2;	
				gfx_fill_rect(&window,0,0,window.width,window.height,0);
				display_window_section(&window,0,0,window.width,window.height);
				drawn = false;
			}
			
			//* Put the ball on the screen.
			draw_ball(0);

			//draw paddles
			draw_paddle(0);

			if (e.event_type == EVENT_KBD_PRESS)
			{

				char scancode = e.event_data&0xFF;

				if (scancode == K_ESCAPE) {
				
					quit = 1;
				}
				
				else if (scancode == K_DOWN) {

					move_paddle(0);
				}

				else if (scancode == K_UP) {

					move_paddle(1);
				}
			}

			//paddle ai movement
			move_paddle_ai();

			//* Move the balls for the next frame. 
			move_ball();
			
			//draw net
			draw_net();

			//draw paddles
			draw_paddle(0xf);
			
			//* Put the ball on the screen.
			draw_ball(0xf);
	
			//draw the score
			draw_player_0_score();
	
			//draw the score
			draw_player_1_score();

			sleep(1000/60);

		}
	}
	 
	terminate();
	__builtin_unreachable();
	
}

int init(int width, int height, int argc, char *args[]) {

	window.w_name = "pong";
	window.event_handler.event_mask = GENERAL_EVENT_KBD;
	create_window(&window,100,100,width,height);
	load_font((void *)font_buff);
	
	return 0;
}
