// Guy Shechter
// June 2004
// 
// Uncomment one of these includes depending on your architecture.
// Your installation location may vary.
//
//
// For Linux use this line:
//
//#include "/usr/local/matlab/extern/include/mex.h"
//
//
// For Windows systems use this line:
//
//#include "c:\matlab701\extern\include\mex.h"
//#include "mex.h"
//
//
// For Mac Os X systems use this line :
//
//#include "/Applications/MATLAB6p5p1/extern/include/mex.h"
//
//

#pragma once




//
//   The definition of a node in the k-d tree.
//
typedef struct node {
  double *pt;
  int orientation;
  unsigned int index;
  struct node *left, *right;
} Node;

//
//   The definition of the Tree root node
//
typedef struct tree {
  struct node *rootptr;
  int dims;
} Tree;



//
// Timing structures used to for debuging purposes
//
typedef struct timevalg {
  long    tv_sec;        /* seconds since Jan. 1, 1970 */
  long tv_usec;       /* and microseconds */
} TV;
typedef struct timezone {
  int  tz_minuteswest;     /* of Greenwich */
  int  tz_dsttime;    /* type of dst correction to apply */
} TZ;


/* PROTOTYPE DECLARATIONS */
double calcdistance  (double *pt1, double *pt2, int Dim);
Node*  rangeQuery    (Node *v, double distance, double *pt, int D);
Node*  pointLocation (Node *v, double *pt, int D);
void   display_tree  (Node *nodeptr, int D);
void   run_queries   (Node *pVertex, double *model, int M, int D, 
		      double *closest_pt, double *distance, short ReturnType);
void   free_tree     (Node *pVertex);
Tree*  build_kdtree  (double *reference, int N, int D, int *index, 
		      int L, int offset);
Node *build_kdtree_core (double *reference, int N, int D, int *index, 
			 int L, int offset);
void   quicksort     (int *ra, int p, int r, double *reference, 
		      int offset, int D);
int    partition     (int *a,  int p, int r, double *reference, 
		      int offset, int D);
void   run_range_search (Node *pVertex, double *model, int M, int D, 
			 double distlim, double **pts_in_range, unsigned int *L, 
			 unsigned int **indices);


