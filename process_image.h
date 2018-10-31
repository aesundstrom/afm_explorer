#ifndef PROCESS
#define PROCESS


#include <iostream>
#include <ostream>
#include <iterator>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <set>
#include <map>
#include <algorithm>
#include "highgui.h"
#include "cv.h"
#include "cxcore.h"
#include "cv_thin.h"


using namespace std;


class FilterImages {

    typedef struct {
        int x;
        int y;
        int offset;
    } Pixel;

    typedef struct {
        double        length;
        int           num_pixels;
        vector<Pixel> pixels;
    } Branch;

    typedef struct {
        int           num_pixels;
        int           num_extremities;
        int           num_contours;
        vector<Pixel> pixels;
        vector<Pixel> extremities;
        vector<Pixel> contours;
    } Component;

    typedef struct {
        double         length;
        int            num_pixels;
        int            num_branches;
        vector<Pixel>  pixels;
        vector<int>    directions;
        vector<Branch> branches;
    } Path;

    typedef struct {
        int dx;
        int dy;
    } Neighbor;

    CvSize size;
        
public:

    IplImage         *inputImage, *filteredImage, *backboneImage;
    short            *component_id_map;
    short            *backbone_id_map;
    vector<Component> components;
    vector<Path>      backbones;
    vector<Neighbor>  canonical_neighbor_order;

    FilterImages();
    ~FilterImages();

    void LoadImage( char *filename );
    void ApplyFilters( int thr_method, int fixed_thr_too_bright_elim, int fixed_thr_too_dim_elim, int adaptive_thr_too_dim_elim, int adaptive_thr_method, int adaptive_thr_box_dim );
    void FindBackbones();

    int  NearBackboneId( int x, int y );
    void HighlightBackboneId( unsigned char *data, int backbone_id, bool color );


private:

    void FindPathsThroughConnectedComponentFrom( set<int> &comp, vector<Path> &bb, set<int> &seen, int &pid, int root_pid, int pix, int dir );
    void FindConnectedComponents( IplImage *image );
    void FindConnectedComponentsIn( IplImage *image, short cid, set<int> &seen, int x, int y );
    FilterImages::Path CopyPath( Path &p );
    int  Neighbors( IplImage *image, int px, int py, int po );
    int  Dir( int dx, int dy );

    void OutputTab( int indent );
    void OutputPixel( Pixel &p, int indent );
    void OutputPixelVector( vector<Pixel> &p_vec, int indent );
    void OutputBranch( Branch &b, int indent );
    void OutputBranchVector( vector<Branch> &b_vec, int indent );
    void OutputComponent( Component &c, int indent );
    void OutputComponentVector( vector<Component> &c_vec, int indent );
    void OutputPath( Path &p, int indent );
    void OutputPathVector( vector<Path> &p_vec, int indent );
    
    void FreeBranch( Branch &b );
    void FreeBranchVector( vector<Branch> &b_vec );
    void FreeComponent( Component &c );
    void FreeComponentVector( vector<Component> &c_vec );
    void FreePath( Path &p );
    void FreePathVector( vector<Path> &p_vec );

};


#endif
