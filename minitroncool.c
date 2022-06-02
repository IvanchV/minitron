#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>
#include <ncurses.h>

#define red 0x00FF0000
#define blue 0x000000FF

int b = 0;

int work_flag = 1;
void sig_handler(int sig)
{
	work_flag = 0;
}

void* thread1f(void *arg)
{
	
	while(work_flag == 1)
	{
		b = getc(stdin);
	}
	
	return NULL;
}

int main(int argc, char *argv[])
{
	unsigned long int my_ip, op_ip, my_color, op_color;
	struct sockaddr_in addr;
	int size = sizeof(addr), fb, sock, x, y, my_x, my_y, op_x, op_y, my_carx[40], my_cary[40], op_carx[40], op_cary[40];
	struct fb_var_screeninfo info;
	size_t fb_size, map_size, page_size;
	uint32_t *ptr;
	pthread_t thread1;
	char my_ch = 0, op_ch = 0, my_start, op_start, my_ch_prev, op_ch_prev;
	int a, my_end = 0, op_end = 0;
	signal(SIGINT, sig_handler);
	x = atoi(argv[1]);
	y = atoi(argv[2]);
	
	page_size = sysconf(_SC_PAGESIZE);
	fb = open("/dev/fb0", O_RDWR);
	ioctl(fb, FBIOGET_VSCREENINFO, &info);
	fb_size = sizeof(uint32_t) * info.xres_virtual * info.yres_virtual;
	map_size = (fb_size + (page_size - 1)) & (~(page_size - 1));
	ptr = mmap(NULL, map_size, PROT_READ | PROT_WRITE, MAP_SHARED, fb, 0);
	initscr();
	noecho();
	move(LINES - 2, 0);
	refresh();
	
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(12345);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	bind(sock, (struct sockaddr *)&addr, sizeof(addr));
	addr.sin_addr.s_addr = inet_addr(argv[3]);
	connect(sock, (struct sockaddr *)&addr, sizeof(addr));
	op_ip = ntohl(addr.sin_addr.s_addr);
	getsockname(sock, (struct sockaddr *)&addr, &size);
	my_ip = ntohl(addr.sin_addr.s_addr);
	
	pthread_create(&thread1, NULL, thread1f, NULL);
	
	memset(ptr, 0, map_size);
	if(my_ip > op_ip)
	{
		my_start = 'd';
		my_color = red;
		my_x = 0;
		my_y = 3;
		op_start = 'a';
		op_color = blue;
		op_x = x - 1;
		op_y = y - 4;
		
		a = 0;
		for(int i = 1; i <= 5; i++)
		{
			for(int j = 1; j <= 8; j++)
			{
				ptr[i * info.xres_virtual + j] = red;
				my_carx[a] = j;
				my_cary[a] = i;
				a++;
			}
		}	
		a = 0;
		for(int i = y - 6; i < y - 1; i++)
		{
			for(int j = x - 9; j < x - 1; j++)
			{
				ptr[i * info.xres_virtual + j] = blue;
				op_carx[a] = j;
				op_cary[a] = i;
				a++;
			}
		}	
	}
	else
	{
		op_start = 'd';
		op_color = red;
		op_x = 0;
		op_y = 3;
		my_start = 'a';
		my_color = blue;
		my_x = x - 1;
		my_y = y - 4;
		
		a = 0;
		for(int i = 1; i <= 5; i++)
		{
			for(int j = 1; j <= 8; j++)
			{
				ptr[i * info.xres_virtual + j] = red;
				op_carx[a] = j;
				op_cary[a] = i;
				a++;
			}
		}	
		a = 0;
		for(int i = y - 6; i < y - 1; i++)
		{
			for(int j = x - 9; j < x - 1; j++)
			{
				ptr[i * info.xres_virtual + j] = blue;
				my_carx[a] = j;
				my_cary[a] = i;
				a++;
			}
		}	
	}

	for(int i = 0; i < y; i++)
	{
		ptr[i * info.xres_virtual + 0] = my_color + 0x1000000;
	}
	for(int i = 0; i < y; i++)
	{
		ptr[i * info.xres_virtual + x - 1] = my_color + 0x1000000;
	}
	for(int i = 0; i < x; i++)
	{
		ptr[0 * info.xres_virtual + i] = my_color + 0x1000000;
	}
	for(int i = 0; i < x; i++)
	{
		ptr[(y - 1) * info.xres_virtual + i] = my_color + 0x1000000;
	}
	
	while(work_flag == 1 && b == 0)
	{
	}
	while(work_flag == 1)
	{
		my_ch_prev = my_ch;
		my_ch = b;
		if(my_ch_prev == 0)
		{
			my_ch = my_start;
			b = my_start;
		}
		else if(my_ch != 'w' && my_ch != 's' && my_ch != 'a' && my_ch != 'd'
		|| my_ch_prev == 'w' && my_ch == 's' || my_ch_prev == 's' && my_ch == 'w'
		|| my_ch_prev == 'a' && my_ch == 'd' || my_ch_prev == 'd' && my_ch == 'a'
		|| my_ch_prev == 'w' && my_ch == 'w' || my_ch_prev == 's' && my_ch == 's'
		|| my_ch_prev == 'a' && my_ch == 'a' || my_ch_prev == 'd' && my_ch == 'd')
		{
			my_ch = my_ch_prev;
		}
		if(send(sock, &my_ch, sizeof(char), 0) == -1)
		{
			work_flag = 0;
			break;
		}
		
		op_ch_prev = op_ch;
		if(recv(sock, &op_ch, sizeof(char), 0) == -1)
		{
			work_flag = 0;
			break;
		}
		if(op_ch_prev == 0)
		{
			op_ch = op_start;
		}
		else if(op_ch != 'w' && op_ch != 's' && op_ch != 'a' && op_ch != 'd'
		|| op_ch_prev == 'w' && op_ch == 's' || op_ch_prev == 's' && op_ch == 'w'
		|| op_ch_prev == 'a' && op_ch == 'd' || op_ch_prev == 'd' && op_ch == 'a'
		|| op_ch_prev == 'w' && op_ch == 'w' || op_ch_prev == 's' && op_ch == 's'
		|| op_ch_prev == 'a' && op_ch == 'a' || op_ch_prev == 'd' && op_ch == 'd')
		{
			op_ch = op_ch_prev;
		}
		
		curs_set(1);
		for(int i = 0; i < 40; i++)
		{
			ptr[my_cary[i] * info.xres_virtual + my_carx[i]] = 0;
			ptr[op_cary[i] * info.xres_virtual + op_carx[i]] = 0;
		}
		
		if(my_ch == 'w')
		{
			my_y--;
		}
		if(my_ch == 's')
		{
			my_y++;
		}
		if(my_ch == 'a')
		{
			my_x--;
		}
		if(my_ch == 'd')
		{
			my_x++;
		}
		
		if(op_ch == 'w')
		{
			op_y--;
		}
		if(op_ch == 's')
		{
			op_y++;
		}
		if(op_ch == 'a')
		{
			op_x--;
		}
		if(op_ch == 'd')
		{
			op_x++;
		}
		
		ptr[my_y * info.xres_virtual + my_x] = my_color;
		ptr[op_y * info.xres_virtual + op_x] = op_color;
		a = 0;
		if(my_ch == 'w')
		{
			for(int i = my_y - 8; i <= my_y - 1; i++)
			{
				for(int j = my_x - 2; j <= my_x + 2; j++)
				{
					if(i < 1 || j < 1 || i >= y - 1 || j >= x - 1 || ptr[i * info.xres_virtual + j] == op_color || ptr[i * info.xres_virtual + j] == my_color)
					{
						my_end = 1;
						work_flag = 0;
					}
					else
					{
						ptr[i * info.xres_virtual + j] = my_color;
					}
					my_carx[a] = j;
					my_cary[a] = i;
					a++;
				}
			}
		}
		if(my_ch == 's')
		{
			for(int i = my_y + 1; i <= my_y + 8; i++)
			{
				for(int j = my_x - 2; j <= my_x + 2; j++)
				{
					if(i < 1 || j < 1 || i >= y - 1 || j >= x - 1 || ptr[i * info.xres_virtual + j] == op_color || ptr[i * info.xres_virtual + j] == my_color)
					{
						my_end = 1;
						work_flag = 0;
					}
					else
					{
						ptr[i * info.xres_virtual + j] = my_color;
					}
					my_carx[a] = j;
					my_cary[a] = i;
					a++;
				}
			}
		}
		if(my_ch == 'a')
		{
			for(int i = my_y - 2; i <= my_y + 2; i++)
			{
				for(int j = my_x - 8; j <= my_x - 1; j++)
				{
					if(i < 1 || j < 1 || i >= y - 1 || j >= x - 1 || ptr[i * info.xres_virtual + j] == op_color || ptr[i * info.xres_virtual + j] == my_color)
					{
						my_end = 1;
						work_flag = 0;
					}
					else
					{
						ptr[i * info.xres_virtual + j] = my_color;
					}
					my_carx[a] = j;
					my_cary[a] = i;
					a++;
				}
			}
		}
		if(my_ch == 'd')
		{
			for(int i = my_y - 2; i <= my_y + 2; i++)
			{
				for(int j = my_x + 1; j <= my_x + 8; j++)
				{
					if(i < 1 || j < 1 || i >= y - 1 || j >= x - 1 || ptr[i * info.xres_virtual + j] == op_color || ptr[i * info.xres_virtual + j] == my_color)
					{
						my_end = 1;
						work_flag = 0;
					}
					else
					{
						ptr[i * info.xres_virtual + j] = my_color;
					}
					my_carx[a] = j;
					my_cary[a] = i;
					a++;
				}
			}
		}
		a = 0;
		if(op_ch == 'w')
		{
			for(int i = op_y - 8; i <= op_y - 1; i++)
			{
				for(int j = op_x - 2; j <= op_x + 2; j++)
				{
					if(i < 1 || j < 1 || i >= y - 1 || j >= x - 1 || ptr[i * info.xres_virtual + j] == op_color || ptr[i * info.xres_virtual + j] == my_color)
					{
						op_end = 1;
						work_flag = 0;
					}
					else
					{
						ptr[i * info.xres_virtual + j] = op_color;
					}
					op_carx[a] = j;
					op_cary[a] = i;
					a++;
				}
			}
		}
		if(op_ch == 's')
		{
			for(int i = op_y + 1; i <= op_y + 8; i++)
			{
				for(int j = op_x - 2; j <= op_x + 2; j++)
				{
					if(i < 1 || j < 1 || i >= y - 1 || j >= x - 1 || ptr[i * info.xres_virtual + j] == op_color || ptr[i * info.xres_virtual + j] == my_color)
					{
						op_end = 1;
						work_flag = 0;
					}
					else
					{
						ptr[i * info.xres_virtual + j] = op_color;
					}
					op_carx[a] = j;
					op_cary[a] = i;
					a++;
				}
			}
		}
		if(op_ch == 'a')
		{
			for(int i = op_y - 2; i <= op_y + 2; i++)
			{
				for(int j = op_x - 8; j <= op_x - 1; j++)
				{
					if(i < 1 || j < 1 || i >= y - 1 || j >= x - 1 || ptr[i * info.xres_virtual + j] == op_color || ptr[i * info.xres_virtual + j] == my_color)
					{
						op_end = 1;
						work_flag = 0;
					}
					else
					{
						ptr[i * info.xres_virtual + j] = op_color;
					}
					op_carx[a] = j;
					op_cary[a] = i;
					a++;
				}
			}
		}
		if(op_ch == 'd')
		{
			for(int i = op_y - 2; i <= op_y + 2; i++)
			{
				for(int j = op_x + 1; j <= op_x + 8; j++)
				{
					if(i < 1 || j < 1 || i >= y - 1 || j >= x - 1 || ptr[i * info.xres_virtual + j] == op_color || ptr[i * info.xres_virtual + j] == my_color)
					{
						op_end = 1;
						work_flag = 0;
					}
					else
					{
						ptr[i * info.xres_virtual + j] = op_color;
					}
					op_carx[a] = j;
					op_cary[a] = i;
					a++;
				}
			}
		}
		curs_set(0);
		
		usleep(62500);
	}
	
	for(int i = 0; i < 40; i++)
	{
		for(int j = 0; j < 40; j++)
		{
			if(my_carx[i] == op_carx[j] && my_cary[i] == op_cary[j])
			{
				my_end = 1;
				op_end = 1;
			}
		}
	}
	if(my_end == 1 && op_end == 1)
	{
		printw("DRAW!\n");
	}
	else if(my_end == 0 && op_end == 1)
	{
		printw("YOU WIN!\n");
	}
	else if(my_end == 1 && op_end == 0)
	{
		printw("YOU LOSE!\n");
	}
	refresh();
	
	close(fb);
	munmap(ptr, map_size);
	close(sock);
	endwin();
	return 0;
}
