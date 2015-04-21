#include <math.h>
#include <stdlib.h>
#include <float.h>
#include <glib.h>
#include "delay.h"
/* npoints > 1 */

double **delay(double (*functionPtr)(double),double xmin, double xmax, double ymax, long int npoints){
	double x,y;
	double step;
	double **points;
	long int i,j;
	double lambda;
	double b;

	points = (double **)malloc(npoints*sizeof(double*)); 
	for (i=0;i<npoints;i++){ 
		points[i] = (double*)malloc(2*sizeof(double));
	} 
	step = (xmax-xmin)/((double)(npoints)-1);

	for (i=0, x=xmin; i<npoints; i++, x += step){
		y = functionPtr(x);
		points[i][0] = x;
		points[i][1] = y;
		if (y>ymax) {break;}
	}

	/* if you get over ymax, just pick points in the straight line between last point and (xmax,DLB_MAX) */
	lambda = (y - DBL_MAX)/(x-xmax);
	for (j=i+1;j<npoints; j++, x += step){
		points[j][0] = x;
		points[j][1] = lambda*x+b;
	}
	return points;
}
