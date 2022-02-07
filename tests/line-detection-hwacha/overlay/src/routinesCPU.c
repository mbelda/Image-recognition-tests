#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "routinesCPU.h"
#include "vec_func.h"

/* Time */
#include <sys/time.h>
#include <sys/resource.h>

unsigned long read_cycles(void)
{
  unsigned long cycles;
  asm volatile ("rdcycle %0" : "=r" (cycles));
  return cycles;
}

unsigned long read_time(void)
{
  unsigned long time;
  asm volatile ("rdtime %0" : "=r" (time));
  return time;
}

unsigned long read_instret(void)
{
  unsigned long instret;
  asm volatile ("rdinstret %0" : "=r" (instret));
  return instret;
}
#define DEG2RAD 0.017453f
#define DATA_SIZE_NR_row 5
#define DATA_SIZE_NR_col 5
#define DATA_SIZE_Gx_row 5
#define DATA_SIZE_Gx_col 4
#define DATA_SIZE_Gy_row 4
#define DATA_SIZE_Gy_col 5

void canny(uint8_t *im, uint8_t *image_out,
	int *NR, float *G, int *phi, int *Gx, int *Gy, uint8_t *pedge,
	float level,
	int height, int width)
{
	int i, j;
	int ii, jj;
	float PI = 3.141593;
	float * phi_aux = (float *)malloc(sizeof(float) * width * height);

	float lowthres, hithres;

	int mask_NR[DATA_SIZE_NR_row][DATA_SIZE_NR_col] = {
		{2,4,5,4,2},
		{4,9,12,9,4},
		{5,12,15,12,5},
		{4,9,12,9,4},
		{2,4,5,4,2}
	};

	int value = 0;


	for(i=2; i<height-2; i++)
		for(j=2; j<width-2; j++)
		{
			int data_NR[DATA_SIZE_NR_row][DATA_SIZE_NR_col] = {
				{im[(i-2)*width+(j-2)], im[(i-2)*width+(j-1)], im[(i-2)*width+(j)], im[(i-2)*width+(j+1)], im[(i-2)*width+(j+2)]},
				{im[(i-1)*width+(j-2)], im[(i-1)*width+(j-1)], im[(i-1)*width+(j)], im[(i-1)*width+(j+1)], im[(i-1)*width+(j+2)]},
				{im[(i  )*width+(j-2)], im[(i  )*width+(j-1)], im[(i  )*width+(j)], im[(i  )*width+(j+1)], im[(i  )*width+(j+2)]},
				{im[(i+1)*width+(j-2)], im[(i+1)*width+(j-1)], im[(i+1)*width+(j)], im[(i+1)*width+(j+1)], im[(i+1)*width+(j+2)]},
				{im[(i+2)*width+(j-2)], im[(i+2)*width+(j-1)], im[(i+2)*width+(j)], im[(i+2)*width+(j+1)], im[(i+2)*width+(j+2)]}
			};

			//Multiplica todos
			for(int i=0; i < DATA_SIZE_NR_row; i++){
				vec_mul_asm(DATA_SIZE_NR_col, data_NR[i], mask_NR[i]);
				
			}

			//Suma los resultados
			for (int i = 1; i < DATA_SIZE_NR_row; i++){
				vec_add_asm(DATA_SIZE_NR_col, data_NR[0], data_NR[i]);
			}
			
			//Sumo el vector de las sumas
			value = 0;
			for(int i= 0; i < DATA_SIZE_NR_col; i++){
				value += data_NR[0][i];
			}

			NR[i*width+j] = value/159;
			/*
			// Noise reduction
			NR[i*width+j] =
				 (2*im[(i-2)*width+(j-2)] +  4*im[(i-2)*width+(j-1)] +  5*im[(i-2)*width+(j)] +  4*im[(i-2)*width+(j+1)] + 2*im[(i-2)*width+(j+2)]
				+ 4*im[(i-1)*width+(j-2)] +  9*im[(i-1)*width+(j-1)] + 12*im[(i-1)*width+(j)] +  9*im[(i-1)*width+(j+1)] + 4*im[(i-1)*width+(j+2)]
				+ 5*im[(i  )*width+(j-2)] + 12*im[(i  )*width+(j-1)] + 15*im[(i  )*width+(j)] + 12*im[(i  )*width+(j+1)] + 5*im[(i  )*width+(j+2)]
				+ 4*im[(i+1)*width+(j-2)] +  9*im[(i+1)*width+(j-1)] + 12*im[(i+1)*width+(j)] +  9*im[(i+1)*width+(j+1)] + 4*im[(i+1)*width+(j+2)]
				+ 2*im[(i+2)*width+(j-2)] +  4*im[(i+2)*width+(j-1)] +  5*im[(i+2)*width+(j)] +  4*im[(i+2)*width+(j+1)] + 2*im[(i+2)*width+(j+2)])
				/159;*/
		}

	int mask_Gx[DATA_SIZE_Gx_row][DATA_SIZE_Gx_col] = {
		{1,2,-2,-1},
		{4,8,-8,-4},
		{6,12,-12,-6},
		{4,8,-8,-4},
		{1,2,-2,-1}
	};

	int mask_Gy[DATA_SIZE_Gy_row][DATA_SIZE_Gy_col] = {
		{-1,-4,-6,-4, -1},
		{-2,-8,-12,-8,-2},
		{2,8,12,8,2},
		{1,4,6,4,1}
	};

	for(i=2; i<height-2; i++)
		for(j=2; j<width-2; j++)
		{	

			int data_Gx[DATA_SIZE_Gx_row][DATA_SIZE_Gx_col] = {
				{im[(i-2)*width+(j-2)], im[(i-2)*width+(j-1)], im[(i-2)*width+(j+1)], im[(i-2)*width+(j+2)]},
				{im[(i-1)*width+(j-2)], im[(i-1)*width+(j-1)], im[(i-1)*width+(j+1)], im[(i-1)*width+(j+2)]},
				{im[(i  )*width+(j-2)], im[(i  )*width+(j-1)], im[(i  )*width+(j+1)], im[(i  )*width+(j+2)]},
				{im[(i+1)*width+(j-2)], im[(i+1)*width+(j-1)], im[(i+1)*width+(j+1)], im[(i+1)*width+(j+2)]},
				{im[(i+2)*width+(j-2)], im[(i+2)*width+(j-1)], im[(i+2)*width+(j+1)], im[(i+2)*width+(j+2)]}
			};


			//Multiplica todos
			for(int i=0; i < DATA_SIZE_Gx_row; i++){
				vec_mul_asm(DATA_SIZE_Gx_col, data_Gx[i], mask_Gx[i]);
			}

			//Suma los resultados
			for (int i = 1; i < DATA_SIZE_Gx_row; i++){
				vec_add_asm(DATA_SIZE_Gx_col, data_Gx[0], data_Gx[i]);
			}
			
			//Sumo el vector de las sumas
			value = 0;
			for(int i= 0; i < DATA_SIZE_Gx_col; i++){
				value += data_Gx[0][i];
			}

			Gx[i*width+j] = value;

			// Intensity gradient of the image
			/*
			Gx[i*width+j] = 
				 (1*NR[(i-2)*width+(j-2)] +  2*NR[(i-2)*width+(j-1)] +  (-2)*NR[(i-2)*width+(j+1)] + (-1)*NR[(i-2)*width+(j+2)]
				+ 4*NR[(i-1)*width+(j-2)] +  8*NR[(i-1)*width+(j-1)] +  (-8)*NR[(i-1)*width+(j+1)] + (-4)*NR[(i-1)*width+(j+2)]
				+ 6*NR[(i  )*width+(j-2)] + 12*NR[(i  )*width+(j-1)] + (-12)*NR[(i  )*width+(j+1)] + (-6)*NR[(i  )*width+(j+2)]
				+ 4*NR[(i+1)*width+(j-2)] +  8*NR[(i+1)*width+(j-1)] +  (-8)*NR[(i+1)*width+(j+1)] + (-4)*NR[(i+1)*width+(j+2)]
				+ 1*NR[(i+2)*width+(j-2)] +  2*NR[(i+2)*width+(j-1)] +  (-2)*NR[(i+2)*width+(j+1)] + (-1)*NR[(i+2)*width+(j+2)]);
			*/

			/*
			Gy[i*width+j] = 
				 ((-1)*NR[(i-2)*width+(j-2)] + (-4)*NR[(i-2)*width+(j-1)] +  (-6)*NR[(i-2)*width+(j)] + (-4)*NR[(i-2)*width+(j+1)] + (-1)*NR[(i-2)*width+(j+2)]
				+ (-2)*NR[(i-1)*width+(j-2)] + (-8)*NR[(i-1)*width+(j-1)] + (-12)*NR[(i-1)*width+(j)] + (-8)*NR[(i-1)*width+(j+1)] + (-2)*NR[(i-1)*width+(j+2)]
				+    2*NR[(i+1)*width+(j-2)] +    8*NR[(i+1)*width+(j-1)] +    12*NR[(i+1)*width+(j)] +    8*NR[(i+1)*width+(j+1)] +    2*NR[(i+1)*width+(j+2)]
				+    1*NR[(i+2)*width+(j-2)] +    4*NR[(i+2)*width+(j-1)] +     6*NR[(i+2)*width+(j)] +    4*NR[(i+2)*width+(j+1)] +    1*NR[(i+2)*width+(j+2)]);
			*/
			
			int data_Gy[DATA_SIZE_Gy_row][DATA_SIZE_Gy_col] = {
                                {im[(i-2)*width+(j-2)], im[(i-2)*width+(j-1)], im[(i-2)*width+(j)], im[(i-2)*width+(j+1)], im[(i-2)*width+(j+2)]},
                                {im[(i-1)*width+(j-2)], im[(i-1)*width+(j-1)], im[(i-1)*width+(j)], im[(i-1)*width+(j+1)], im[(i-1)*width+(j+2)]},
                                {im[(i+1)*width+(j-2)], im[(i+1)*width+(j-1)], im[(i+1)*width+(j)], im[(i+1)*width+(j+1)], im[(i+1)*width+(j+2)]},
                                {im[(i+2)*width+(j-2)], im[(i+2)*width+(j-1)], im[(i+2)*width+(j)], im[(i+2)*width+(j+1)], im[(i+2)*width+(j+2)]}
                        };


			//Multiplica todos
			for(int i=0; i < DATA_SIZE_Gy_row; i++){
				vec_mul_asm(DATA_SIZE_Gy_col, data_Gy[i], mask_Gy[i]);
			}

			//Suma los resultados
			for (int i = 1; i < DATA_SIZE_Gy_row; i++){
				vec_add_asm(DATA_SIZE_Gy_col, data_Gy[0], data_Gy[i]);
			}
			
			//Sumo el vector de las sumas
			value = 0;
			for(int i= 0; i < DATA_SIZE_Gy_col; i++){
				value += data_Gy[0][i];
			}

			Gy[i*width+j] = value;



			G[i*width+j]   = sqrtf((Gx[i*width+j]*Gx[i*width+j])+(Gy[i*width+j]*Gy[i*width+j]));	//G = √Gx²+Gy²
			phi_aux[i*width+j] = atan2f(fabs(Gy[i*width+j]),fabs(Gx[i*width+j]));

			if(fabs(phi_aux[i*width+j])<=PI/8 )
				phi[i*width+j] = 0;
			else if (fabs(phi_aux[i*width+j])<= 3*(PI/8))
				phi[i*width+j] = 45;
			else if (fabs(phi_aux[i*width+j]) <= 5*(PI/8))
				phi[i*width+j] = 90;
			else if (fabs(phi_aux[i*width+j]) <= 7*(PI/8))
				phi[i*width+j] = 135;
			else phi[i*width+j] = 0;
	}

	free(phi_aux);

	// Edge
	for(i=3; i<height-3; i++)
		for(j=3; j<width-3; j++)
		{
			pedge[i*width+j] = 0;
			if(phi[i*width+j] == 0){
				if(G[i*width+j]>G[i*width+j+1] && G[i*width+j]>G[i*width+j-1]) //edge is in N-S
					pedge[i*width+j] = 1;

			} else if(phi[i*width+j] == 45) {
				if(G[i*width+j]>G[(i+1)*width+j+1] && G[i*width+j]>G[(i-1)*width+j-1]) // edge is in NW-SE
					pedge[i*width+j] = 1;

			} else if(phi[i*width+j] == 90) {
				if(G[i*width+j]>G[(i+1)*width+j] && G[i*width+j]>G[(i-1)*width+j]) //edge is in E-W
					pedge[i*width+j] = 1;

			} else if(phi[i*width+j] == 135) {
				if(G[i*width+j]>G[(i+1)*width+j-1] && G[i*width+j]>G[(i-1)*width+j+1]) // edge is in NE-SW
					pedge[i*width+j] = 1;
			}
		}

	// Hysteresis Thresholding
	lowthres = level/2;
	hithres  = 2*(level);

	for(i=3; i<height-3; i++)
		for(j=3; j<width-3; j++)
		{
			image_out[i*width+j] = 0;
			if(G[i*width+j]>hithres && pedge[i*width+j])
				image_out[i*width+j] = 255;
			else if(pedge[i*width+j] && G[i*width+j]>=lowthres && G[i*width+j]<hithres)
				// check neighbours 3x3
				for (ii=-1;ii<=1; ii++)
					for (jj=-1;jj<=1; jj++)
						if (G[(i+ii)*width+j+jj]>hithres)
							image_out[i*width+j] = 255;
		}
}


void getlines(int threshold, uint32_t *accumulators, int accu_width, int accu_height, int width, int height, 
	float *sin_table, float *cos_table,
	int *x1_lines, int *y1_lines, int *x2_lines, int *y2_lines, int *lines)
{
	int rho, theta, ii, jj;
	uint32_t max;

	for(rho=0;rho<accu_height;rho++)
	{
		for(theta=0;theta<accu_width;theta++)  
		{  

			if(accumulators[(rho*accu_width) + theta] >= threshold)  
			{  
				//Is this point a local maxima (9x9)  
				max = accumulators[(rho*accu_width) + theta]; 
				for(int ii=-4;ii<=4;ii++)  
				{  
					for(int jj=-4;jj<=4;jj++)  
					{  
						if( (ii+rho>=0 && ii+rho<accu_height) && (jj+theta>=0 && jj+theta<accu_width) )  
						{  
							if( accumulators[((rho+ii)*accu_width) + (theta+jj)] > max )  
							{
								max = accumulators[((rho+ii)*accu_width) + (theta+jj)];
							}  
						}  
					}  
				}  

				if(max == accumulators[(rho*accu_width) + theta]) //local maxima
				{
					int x1, y1, x2, y2;  
					x1 = y1 = x2 = y2 = 0;  

					if(theta >= 45 && theta <= 135)  
					{
						if (theta>90) {
							//y = (r - x cos(t)) / sin(t)  
							x1 = width/2;  
							y1 = ((float)(rho-(accu_height/2)) - ((x1 - (width/2) ) * cos_table[theta])) / sin_table[theta] + (height / 2);
							x2 = width;  
							y2 = ((float)(rho-(accu_height/2)) - ((x2 - (width/2) ) * cos_table[theta])) / sin_table[theta] + (height / 2);  
						} else {
							//y = (r - x cos(t)) / sin(t)  
							x1 = 0;  
							y1 = ((float)(rho-(accu_height/2)) - ((x1 - (width/2) ) * cos_table[theta])) / sin_table[theta] + (height / 2);
							x2 = width*2/5;  
							y2 = ((float)(rho-(accu_height/2)) - ((x2 - (width/2) ) * cos_table[theta])) / sin_table[theta] + (height / 2); 
						}
					} else {
						//x = (r - y sin(t)) / cos(t);  
						y1 = 0;  
						x1 = ((float)(rho-(accu_height/2)) - ((y1 - (height/2) ) * sin_table[theta])) / cos_table[theta] + (width / 2);  
						y2 = height;  
						x2 = ((float)(rho-(accu_height/2)) - ((y2 - (height/2) ) * sin_table[theta])) / cos_table[theta] + (width / 2);  
					}
					x1_lines[*lines] = x1;
					y1_lines[*lines] = y1;
					x2_lines[*lines] = x2;
					y2_lines[*lines] = y2;
					(*lines)++;
				}
			}
		}
	}
}

void init_cos_sin_table(float *sin_table, float *cos_table, int n)
{
	int i;
	for (i=0; i<n; i++)
	{
		sin_table[i] = sinf(i*DEG2RAD);
		cos_table[i] = cosf(i*DEG2RAD);
	}
}

void houghtransform(uint8_t *im, int width, int height, uint32_t *accumulators, int accu_width, int accu_height, 
	float *sin_table, float *cos_table)
{
	int i, j, theta;

	float hough_h = ((sqrt(2.0) * (float)(height>width?height:width)) / 2.0);

	for(i=0; i<accu_width*accu_height; i++)
		accumulators[i]=0;	

	float center_x = width/2.0; 
	float center_y = height/2.0;
	for(i=0;i<height;i++)  
	{  
		for(j=0;j<width;j++)  
		{  
			if( im[ (i*width) + j] > 250 ) // Pixel is edge  
			{  
				for(theta=0;theta<180;theta++)  
				{  
					float rho = ( ((float)j - center_x) * cos_table[theta]) + (((float)i - center_y) * sin_table[theta]);
					accumulators[ (int)((round(rho + hough_h) * 180.0)) + theta]++;

				} 
			} 
		} 
	}
}

uint8_t *image_RGB2BW(uint8_t *image_in, int height, int width)
{
	int i, j;
	uint8_t *imageBW = (uint8_t *)malloc(sizeof(uint8_t) * width * height);
	float R, B, G;

	for (i = 0; i < height; i++)
		for (j = 0; j < width; j++)
		{
			R = (float)(image_in[3 * (i * width + j)]);
			G = (float)(image_in[3 * (i * width + j) + 1]);
			B = (float)(image_in[3 * (i * width + j) + 2]);

			imageBW[i * width + j] = (uint8_t)(0.2989 * R + 0.5870 * G + 0.1140 * B);
		}

	return imageBW;
}

void draw_lines(uint8_t *imgtmp, int width, int height, int *x1, int *y1, int *x2, int *y2, int nlines)
{
	int x, y, wl, l;
	int width_line=9;

	for(l=0; l<nlines; l++)
		for(wl=-(width_line>>1); wl<=(width_line>>1); wl++)
			for (x=x1[l]; x<x2[l]; x++)
			{
				y = (float)(y2[l]-y1[l])/(x2[l]-x1[l])*(x-x1[l])+y1[l]; //Line eq. known two points
				if (x+wl>0 && x+wl<width && y>0 && y<height)
				{
					imgtmp[3*((y)*width+x+wl)  ] = 255;
					imgtmp[3*((y)*width+x+wl)+1] = 0;
					imgtmp[3*((y)*width+x+wl)+2] = 0;
				}
			}
}

void line_asist_CPU(uint8_t *im, int height, int width,
	uint8_t *imEdge, int *NR, float *G, int *phi, int *Gx, int *Gy, uint8_t *pedge,
	float *sin_table, float *cos_table, 
	uint32_t *accum, int accu_height, int accu_width,
	int *x1, int *y1, int *x2, int *y2, int *nlines)
{
	int threshold;
	long cycles, time, instret;

	/* Canny */

    cycles = - read_cycles();
    time = - read_time();
    instret = - read_instret();
	canny(im, imEdge,
		NR, G, phi, Gx, Gy, pedge,
		1000.0f, //level
		height, width);
	cycles += read_cycles();
    time += read_time();
    instret += read_instret();
	printf("Canny\nCycles: %ld \nTime: %ld \nInstret: %ld \n", cycles, time, instret);

	/* hough transform */
	cycles = - read_cycles();
    time = - read_time();
    instret = - read_instret();
	houghtransform(imEdge, width, height, accum, accu_width, accu_height, sin_table, cos_table);
	cycles += read_cycles();
    time += read_time();
    instret += read_instret();
	printf("Hough transform\nCycles: %ld \nTime: %ld \nInstret: %ld \n", cycles, time, instret);

	if (width>height) threshold = width/6;
	else threshold = height/6;

	cycles = - read_cycles();
    time = - read_time();
    instret = - read_instret();
	getlines(threshold, accum, accu_width, accu_height, width, height, 
		sin_table, cos_table,
		x1, y1, x2, y2, nlines);
	cycles += read_cycles();
    time += read_time();
    instret += read_instret();
	printf("Getlines\nCycles: %ld \nTime: %ld \nInstret: %ld \n", cycles, time, instret);
}
