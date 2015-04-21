#include <math.h>
#include <stdlib.h>
#include <glib.h>
#include "stability_region.h"
/* npoints > 1 */

double **stability_region(gboolean (*functionPtr)(double[2]),double xmin, double xmax, double ymin, double ymax, long int npoints, double err ){
	double x;
	double step;
	double **points;
	double point[2];
	double iter_min,iter_max,iter_curr,iter_prev;
	long int i;

	points = (double **)malloc(npoints*sizeof(double*));
	for (i=0;i<npoints;i++){
		points[i] = (double*)malloc(2*sizeof(double));
	}
	step = (xmax-xmin)/((double)(npoints)-1);

	for (i=0, x=xmin; i<npoints; i++, x += step){
		iter_max = ymax;
		iter_min = ymin;
		iter_curr = (ymax+ymin)/((double)2);
		point[0] = x; point[1] = iter_curr;
		if (functionPtr(point)){
			iter_min = iter_curr;
		}
		else{
			iter_max = iter_curr;
		}
		do{
			iter_prev = iter_curr;
			iter_curr = (iter_max+iter_min)/((double)2);
			point[0] = x; point[1] = iter_curr;
			if (functionPtr(point)){
				iter_min = iter_curr;
			}
			else{
				iter_max = iter_curr;
			}
		} while (fabs(iter_prev-iter_curr)>=err );
		points[i][0] = x; points[i][1] = iter_curr;
	}
	return points;
}
