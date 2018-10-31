#include "process_image.h"
#include "image_processing_control.h"




// TODO:
// 
// + comment every procedure
// + make variable names consistent and clear
// + implement better Euclidean length estimation of paths (see Rivetti, et al. and my notes with Bud)
// + implement simple geometric hashing on longest paths and their branches
// + implement adaptive thresholding like I did in filter.pl and ImageProcessing.pm
// + implement my middle pixel finding algorithm (depends on finding outer contour pixels)
//
// Observations:
//
// + pixel cross patterns yield diamond longest path [e.g. 01141710.001.afme_backbones.bmp: (624,506)]
//
// Questions:
//
// + Do we need skip patterns 1 and 2?  What do they mean?
// + Is the canonical neighbor order always a good thing?




FilterImages::FilterImages()
{
    // initialize the class image pointers, yet to be assigned...
    inputImage       = NULL;  // ...in LoadImage
    filteredImage    = NULL;  // ...in ApplyFilters
    backboneImage    = NULL;  // ...in FindBackbones
    // ...which may be done repetetively

    // initialize the class id map pointers, yet to be allocated...
    component_id_map = NULL;  // ...in FindBackbones
    backbone_id_map  = NULL;  // ...in FindBackbones
    // ...which may be done repetetively

    // initialize the class canonical neighbor order
    Neighbor n1, n2, n3, n4, n5, n6, n7, n8;
    n1.dx = +1; n1.dy = -1;  // DIAG 1
    n2.dx = -1; n2.dy = -1;  // DIAG 2
    n3.dx = -1; n3.dy = +1;  // DIAG 3
    n4.dx = +1; n4.dy = +1;  // DIAG 4
    n5.dx = +1; n5.dy =  0;  // HOVE 1
    n6.dx =  0; n6.dy = -1;  // HOVE 2
    n7.dx = -1; n7.dy =  0;  // HOVE 3
    n8.dx =  0; n8.dy = +1;  // HOVE 4
    canonical_neighbor_order.push_back( n1 );
    canonical_neighbor_order.push_back( n2 );
    canonical_neighbor_order.push_back( n3 );
    canonical_neighbor_order.push_back( n4 );
    canonical_neighbor_order.push_back( n5 );
    canonical_neighbor_order.push_back( n6 );
    canonical_neighbor_order.push_back( n7 );
    canonical_neighbor_order.push_back( n8 );
}




FilterImages::~FilterImages()
{
    // deallocate class images
    if (inputImage != NULL) {
        cvReleaseData( inputImage );
    }
    if (filteredImage != NULL) {
        cvReleaseData( filteredImage );
    }
    if (backboneImage != NULL) {
        cvReleaseData( backboneImage );
    }

    // deallocate class id maps
    if (component_id_map != NULL) {
        free( component_id_map );
    }
    if (backbone_id_map != NULL) {
        free( backbone_id_map );
    }

    // deallocate class vectors
    FreeComponentVector( components );
    FreePathVector( backbones );
}




// Input: a filename
// Output: a new IplImage, initialized from the image loaded from the given filename
void FilterImages::LoadImage( char *filename ) {

    // deallocate the input image if necessary
    if (inputImage != NULL) {
        cvReleaseData( inputImage );
    }

    // load the input image
    inputImage = cvLoadImage( filename, CV_LOAD_IMAGE_COLOR );

    // validate the input image
    if (! inputImage) {
        cerr << "Failed to load image '" << filename << "'." << endl;
        cerr << "Perhaps [Mac => PC]: the endian byte ordering is wrong; try regenerating the image on this system." << endl;
        exit( 1 );
    }

    return;
}




// Input: an image and set of image processing parameters
// Output: a new IplImage resulting from the image processing steps herein: color conversion, histogram equalization, smoothing, thresholding
// Note: this may be called repeatedly, so perform the proper deallocations and allocations
void FilterImages::ApplyFilters(
    int thr_method,
    int fixed_thr_too_bright_elim,
    int fixed_thr_too_dim_elim,
    int adaptive_thr_too_dim_elim,
    int adaptive_thr_method,
    int adaptive_thr_box_dim
) {

    // there must be an input image
    if (! inputImage) {
        cerr << "ApplyFilters error: an input image must be assigned prior to this." << endl;
        exit( 1 );
    }

    // initialize image size
    size = cvGetSize( inputImage );    

    // deallocate the filtered image if necessary
    if (filteredImage != NULL) {
        cvReleaseData( filteredImage );
    }

    // allocate filtered image
    filteredImage = cvCreateImage( size, IPL_DEPTH_8U, 1 );

    // allocate and initialize temporary images
    IplImage *tempImage1 = cvCreateImage( size, IPL_DEPTH_8U, 1 );
    IplImage *tempImage2 = cvCreateImage( size, IPL_DEPTH_8U, 1 );
    IplImage *tempImage3 = cvCreateImage( size, IPL_DEPTH_8U, 1 );
    IplImage *tempImage4 = cvCreateImage( size, IPL_DEPTH_8U, 1 );
    IplImage *tempImage5 = cvCreateImage( size, IPL_DEPTH_8U, 1 );

    // convert the image from RGB to grayscale
    cvCvtColor( inputImage, tempImage1, CV_RGB2GRAY );

    // equalize the image histogram
    cvEqualizeHist( tempImage1, tempImage2 );

    // median smooth the image
    cvSmooth( tempImage2, tempImage3, CV_MEDIAN, 5, 0, 0, 0 );

    // apply thresholds, according to input parameters...
    if (thr_method == 0) {  // fixed
        // simple fixed thresholding: eliminate pixels that are too bright
        cvThreshold( tempImage3, tempImage4, fixed_thr_too_bright_elim, 255, CV_THRESH_TOZERO_INV );
        // then eliminate those that are too dim
        cvThreshold( tempImage4, tempImage5, fixed_thr_too_dim_elim, 255, CV_THRESH_BINARY );
        // those that remain have maximum intensity
    }
    else if (thr_method == 1) {  // adaptive
        // simple adaptive thresholding: eliminate pixels that are too dim
        cvThreshold( tempImage3, tempImage4, adaptive_thr_too_dim_elim, 255, CV_THRESH_TOZERO );
        // locally threshold, using a neighborhood
        if (adaptive_thr_method == 0) {  // mean
            cvAdaptiveThreshold( tempImage4, tempImage5, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, adaptive_thr_box_dim, 0 );
        }
        else if (adaptive_thr_method == 1) {  // gaussian
            cvAdaptiveThreshold( tempImage4, tempImage5, 255, CV_ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY, adaptive_thr_box_dim, 0 );
        }
    }

    // erode the image connected components into 1-D trees
    cvThin( tempImage5, filteredImage );

    // deallocate the temporary images
    cvReleaseData( tempImage1 );
    cvReleaseData( tempImage2 );
    cvReleaseData( tempImage3 );
    cvReleaseData( tempImage4 );
    cvReleaseData( tempImage5 );

    return;
}




// Input: a filtered image
// Output: a backbone image, a mutated components vector, and a mutated backbones vector
void FilterImages::FindBackbones() {

    // there must be a filtered image
    if (! filteredImage) {
        cerr << "FindBackbones error: a filtered image must be assigned prior to this." << endl;
        exit( 1 );
    }

    // deallocate the backbone image if necessary
    if (backboneImage != NULL) {
        cvReleaseData( backboneImage );
    }

    // deallocate class id maps if necessary
    if (component_id_map != NULL) {
        free( component_id_map );
    }
    if (backbone_id_map != NULL) {
        free( backbone_id_map );
    }

    // initialize class vectors if necessary
    if (components.size() != 0) {
        FreeComponentVector( components );
    }
    if (backbones.size() != 0) {
        FreePathVector( backbones );
    }

    // allocate and initialize the backbone image
    backboneImage = cvCreateImage( size, IPL_DEPTH_8U, 1 );
    memset( backboneImage->imageData, 0, backboneImage->height * backboneImage->width * backboneImage->nChannels * sizeof( unsigned char ) );

    // allocate and initialize the component id map
    component_id_map = (short *)malloc( size.width * size.height * sizeof( short ) );
    memset( component_id_map, -1, size.width * size.height * sizeof( short ) );

    // allocate and initialize the backbone id map
    backbone_id_map = (short *)malloc( size.width * size.height * sizeof( short ) );
    memset( backbone_id_map, -1, size.width * size.height * sizeof( short ) );

    // find connected components in the input image, and add them to the map
    FindConnectedComponents( filteredImage );

    // initialize the connected component id counter
    int component_id = 0;

    // for each connected component...
    for (vector<Component>::iterator
         cit  = components.begin();
         cit != components.end();
         cit++) {

        // if the connected component is a singleton (one pixel), then skip it
        if (cit->num_pixels == 1) {
            cerr << "Warning: the following connected component is a singleton (one pixel =>  zero-length path):";
            OutputComponent( *cit, 1 );
            component_id++;
            continue;
        }

        // initialize path id and backbone candidates
        int path_id = 0;
        vector<Path> backbone_candidates;

        // create a hash of the connected component pixels
        set<int> component_pixels;
        for (vector<Pixel>::iterator
             pi  = cit->pixels.begin();
             pi != cit->pixels.end();
             pi++) { 
            component_pixels.insert( pi->offset );
        }

        // for each extremity pixel in the connected component...
        for (vector<Pixel>::iterator
             eit  = cit->extremities.begin();
             eit != cit->extremities.end();
             eit++) {

            // initialize seen hash
            set<int> seen;

            // initialize an initial path
            Path init_path;
            init_path.length       = 0;
            init_path.num_pixels   = 0;
            init_path.num_branches = 1;
            init_path.pixels.clear();
            init_path.directions.clear();

            // initialize an initial branch
            Branch init_branch;
            init_branch.length     = 0;
            init_branch.num_pixels = 0;
            init_branch.pixels.clear();

            // add the initial branch to the initial path
            init_path.branches.push_back( init_branch );

            // add the initial path to the backbone candidates
            backbone_candidates.push_back( init_path );

            // generate all possible paths starting at the extremity pixel
            FindPathsThroughConnectedComponentFrom( component_pixels, backbone_candidates, seen, path_id, path_id, eit->offset, 0 );

            // increment the path id
            path_id++;
        }

        // if the backbone candidate vector is empty for this connected component, then warn and skip it
        if (backbone_candidates.size() == 0) {
            cerr << "Warning: the backbone candidate vector is empty for the following connected component:" << endl << endl;
            OutputComponent( *cit, 1 );
            component_id++;
            continue;
        }

        // select the path having maximum length
        double max_path_length = 0.0;
        vector<Path>::iterator max_path;
        for (vector<Path>::iterator
             path_it  = backbone_candidates.begin();
             path_it != backbone_candidates.end();
             path_it++) {
            if (path_it->length > max_path_length) {
                max_path_length = path_it->length;
                max_path        = path_it;
            }
        }

        // if no maximum was found for this connected component, then warn and skip it
        if (max_path_length == 0.0) {
            cerr << "Warning: the backbone candidate vector, while not empty, had no paths of nonzero length for the following component:" << endl << endl;
            OutputComponent( *cit, 1 );
            component_id++;
            FreePathVector( backbone_candidates );
            continue;
        }

        // record the maximum length path as a backbone
        backbones.push_back( CopyPath( *max_path ) );

        // write the longest path pixels into the backbone id map and backbone image
        unsigned char *data = (unsigned char *)backboneImage->imageData;
        for (vector<Pixel>::iterator
             backbone_pixel_it  = max_path->pixels.begin();
             backbone_pixel_it != max_path->pixels.end();
             backbone_pixel_it++) {
            for (int
                 chan = 0;
                 chan < backboneImage->nChannels;
                 chan++) {
                short *bp = backbone_id_map + backbone_pixel_it->offset;
                *bp = component_id;
                int loc = (backbone_pixel_it->offset * backboneImage->nChannels) + chan;
                data[loc] = 255;
            }
        }
        
        // deallocate the backbone candidates
        FreePathVector( backbone_candidates );

        // increment the connected component id counter
        component_id++;
    }

    return;
}




// Input: a connected component pixel hash, backbone candidate vector, seen hash, global path id, local path id, pixel, and direction
// Output: a mutated backbone candidate vector, seen hash, and global path id
void FilterImages::FindPathsThroughConnectedComponentFrom( set<int> &comp, vector<Path> &bb, set<int> &seen, int &pid, int root_pid, int pix, int dir )
{
    // initialize a home pixel from the input pixel
    Pixel home_pix;
    home_pix.x      = pix % size.width;
    home_pix.y      = pix / size.width;
    home_pix.offset = pix;

    // if the home pixel has been seen, then return
    if (seen.find( home_pix.offset ) != seen.end()) {
        return;
    }

    // record the home pixel's been seen
    seen.insert( home_pix.offset );

    // update the path's data accordingly
    if      (dir == 1 || dir == 3 || dir == 5 || dir == 7) {
        bb[root_pid].length += 1;
        bb[root_pid].directions.push_back( dir );
        bb[root_pid].branches.back().length += 1;
    }
    else if (dir == 2 || dir == 4 || dir == 6 || dir == 8) {
        bb[root_pid].length += sqrt(2);
        bb[root_pid].directions.push_back( dir );
        bb[root_pid].branches.back().length += sqrt(2);
    }
    bb[root_pid].num_pixels++;
    bb[root_pid].pixels.push_back( home_pix );
    bb[root_pid].branches.back().num_pixels++;
    bb[root_pid].branches.back().pixels.push_back( home_pix );

    // look for viable neighbor pixels of the home pixel in canonical order (DIAGs first, then HOVEs)...
    vector<int> neighbor_pix, neighbor_dir;
    for (vector<Neighbor>::iterator
         neighbor_it  = canonical_neighbor_order.begin();
         neighbor_it != canonical_neighbor_order.end();
         neighbor_it++) {

        // for convenience
        int dx = neighbor_it->dx;
        int dy = neighbor_it->dy;

        // if the neighbor pixel is the home pixel, then skip it
        if (dx == 0 &&
            dy == 0) {
            continue;
        }

        // compute the neighbor pixel's offset
        int n_pix = ((home_pix.y + dy) * size.width) + (home_pix.x + dx);

        // if the neighbor pixel has been seen, then skip it
        if (seen.find( n_pix ) != seen.end()) {
            continue;
        }

        // if the neighbor pixel is not in the connected component being explored, then skip it
        if (comp.find( n_pix ) == comp.end()) {
            continue;
        }

        // compute the neighbor pixel's direction from the home pixel
        int n_dir = Dir( dx, dy );

        // skip pattern #1: DIAG neighbor where a HOVE neighbor has been visited
        if (((n_dir == 2 || n_dir == 8) && seen.find( ((home_pix.y + 0) * size.width) + (home_pix.x + 1) ) != seen.end()) ||
            ((n_dir == 2 || n_dir == 4) && seen.find( ((home_pix.y - 1) * size.width) + (home_pix.x + 0) ) != seen.end()) ||
            ((n_dir == 4 || n_dir == 6) && seen.find( ((home_pix.y + 0) * size.width) + (home_pix.x - 1) ) != seen.end()) ||
            ((n_dir == 6 || n_dir == 8) && seen.find( ((home_pix.y + 1) * size.width) + (home_pix.x + 0) ) != seen.end())) {
            continue;
        }

        // skip pattern #2: HOVE neighbor where a DIAG neighbor has neem visited
        if ((n_dir == 1 && (seen.find( ((home_pix.y - 1) * size.width) + (home_pix.x + 1) ) != seen.end()   ||
                            seen.find( ((home_pix.y + 1) * size.width) + (home_pix.x + 1) ) != seen.end())) ||
            (n_dir == 3 && (seen.find( ((home_pix.y - 1) * size.width) + (home_pix.x + 1) ) != seen.end()   ||
                            seen.find( ((home_pix.y - 1) * size.width) + (home_pix.x - 1) ) != seen.end())) ||
            (n_dir == 5 && (seen.find( ((home_pix.y - 1) * size.width) + (home_pix.x - 1) ) != seen.end()   ||
                            seen.find( ((home_pix.y + 1) * size.width) + (home_pix.x - 1) ) != seen.end())) ||
            (n_dir == 7 && (seen.find( ((home_pix.y + 1) * size.width) + (home_pix.x - 1) ) != seen.end()   ||
                            seen.find( ((home_pix.y + 1) * size.width) + (home_pix.x + 1) ) != seen.end()))) {
            continue;
        }            

        // schedule the viable neighbor pixel to be explored
        neighbor_pix.push_back( n_pix );
        neighbor_dir.push_back( n_dir );
    }

    // traverse the viable neighbor pixels of the home pixel...
    for (int
         i = 0;
         i < neighbor_pix.size();
         i++) {

        // if the home pixel has more than one viable neighbor (i.e. it's not an extremity or intra-branch pixel)...
        if (neighbor_pix.size() > 1) {
          
            // increment the path id
            pid++;

            // copy the root path data to the new path
            Path new_path = CopyPath( bb[root_pid] );

            // increment the number of branches
            new_path.num_branches++;

            // initialize a new branch
            Branch new_branch;
            new_branch.length     = 0;
            new_branch.num_pixels = 0;
            new_branch.pixels.clear();

            // add the new branch to the new path
            new_path.branches.push_back( new_branch );

            // add the new path to the backbone candidates
            bb.push_back( new_path );
        }

        // recursively explore the neighbor pixel
        FindPathsThroughConnectedComponentFrom( comp, bb, seen, pid, pid, neighbor_pix[i], neighbor_dir[i] );
    }

    // if the root path data has been copied to its children, then clean it up without mutating the index values of other elements
    if (neighbor_pix.size() > 1) {
        bb[root_pid].length       = 0;
        bb[root_pid].num_pixels   = 0;
        bb[root_pid].num_branches = 0;
        bb[root_pid].pixels.clear();
        bb[root_pid].directions.clear();
        bb[root_pid].branches.clear();
    }

    return;
}




// Input: an image and an allocated and initialized connected component id image
// Output: a mutated connected component id image representing the pixels' connected component ids
void FilterImages::FindConnectedComponents( IplImage *image )
{
    // initialize
    short component_id = 0;
    unsigned char *ip;
    short *cp;

    // iterate over the image pixels
    for (int
         y = 0;
         y < image->height;
         y++) {

        for (int
             x = 0;
             x < image->width;
             x++) {

            // place the image and component id map cursors
            ip = (unsigned char *)image->imageData + (y * image->widthStep) + x;
            cp = component_id_map + (y * size.width) + x;

            // if the image pixel intensity value is non-zero and the corresponding component id map has its initial value...
            if (*ip >  0 &&
                *cp == -1) {

                // create a set of pixels representing the connected component
                set<int> seen;

                // find the set of 8-connected (seen) pixels, starting with (x,y), comprising the connected component
                FindConnectedComponentsIn( image, component_id, seen, x, y );

                // if the connected component has more than one pixel...
                if (seen.size() > 1) {

                    // determine how many of its pixels are extremities
                    vector<Pixel> candidate_component;
                    int num_extremities = 0;
                    for (set<int>::iterator
                         pixel_it  = seen.begin();
                         pixel_it != seen.end();
                         pixel_it++) {
                        int x = *pixel_it % size.width;
                        int y = *pixel_it / size.width;
                        Pixel pix;
                        pix.x = x;
                        pix.y = y;
                        pix.offset = *pixel_it;
                        candidate_component.push_back( pix );
                        int n = Neighbors( image, x, y, -1 );
                        if (n == 1) {
                            num_extremities++;
                        }
                    }

                    // if the connected component has extremity pixels, then it's admissible (otherwise, downstream, no path through the connected component can be computed)
                    if (num_extremities > 0) {

                        // create and initialize a certified component
                        Component certified_component;
                        certified_component.num_pixels = 0;
                        certified_component.num_extremities = 0;
                        certified_component.num_contours = 0;

                        // iterate over the candidate connected component pixels, initializing the certified connected component accordingly
                        short *scp;
                        for (vector<Pixel>::iterator
                             pixel_it  = candidate_component.begin();
                             pixel_it != candidate_component.end();
                             pixel_it++) {

                            certified_component.pixels.push_back( *pixel_it );
                            certified_component.num_pixels++;
                            int n = Neighbors( image, pixel_it->x, pixel_it->y, -1 );
                            if (n == 1) {
                                certified_component.extremities.push_back( *pixel_it );
                                certified_component.num_extremities++;
                            }
                            if (n < 8) {
                                certified_component.contours.push_back( *pixel_it );
                                certified_component.num_contours++;
                            }

                            // draw the pixel onto the component id map
                            scp = component_id_map + pixel_it->offset;
                            *scp = component_id;
                        }

                        // add the certified connected component onto the list of connected components
                        components.push_back( certified_component );
                        component_id++;
                    }
                }
            }
        }
    }

    return;
}




// Input: an image, connected component id image, connected component id, x, and y
// Output: a mutated connected component id image
void FilterImages::FindConnectedComponentsIn( IplImage *image, short cid, set<int> &seen, int x, int y )
{
    seen.insert( (y * size.width) + x );
    unsigned char *ip;
    for (int
         dy  = -1;
         dy <= +1;
         dy++) {
        for (int
             dx  = -1;
             dx <= +1;
             dx++) {
            if (x + dx <  0          ||
                x + dx >= size.width ||
                y + dy < 0           ||
                y + dy >= size.height) {
                continue;
            }
            if (dx == 0 &&
                dy == 0) {
                continue;
            }
            ip = (unsigned char *)image->imageData + ((y + dy) * image->widthStep) + (x + dx);
            if (*ip > 0 &&
                seen.find( ((y + dy) * size.width) + (x + dx) ) == seen.end()) {
                FindConnectedComponentsIn( image, cid, seen, x + dx, y + dy );
            }
        }
    }
    return;
}




// Input: a path
// Output: a copy of the input path
FilterImages::Path FilterImages::CopyPath( Path &p )
{
    Path new_path;
    new_path.length       = p.length;
    new_path.num_pixels   = p.num_pixels;
    new_path.num_branches = p.num_branches;
    for (vector<Pixel>::iterator
         pi  = p.pixels.begin();
         pi != p.pixels.end();
         pi++) {
        Pixel new_pix;
        new_pix.x      = pi->x;
        new_pix.y      = pi->y;
        new_pix.offset = pi->offset;
        new_path.pixels.push_back( new_pix );
    }
    for (int
         i = 0;
         i < p.directions.size();
         i++) {
        new_path.directions.push_back( p.directions[i] );
    }
    for (vector<Branch>::iterator
         bi  = p.branches.begin();
         bi != p.branches.end();
         bi++) {
        Branch new_branch;
        new_branch.length     = bi->length;
        new_branch.num_pixels = bi->num_pixels;
        for (vector<Pixel>::iterator
             pi  = bi->pixels.begin();
             pi != bi->pixels.end();
             pi++) {
            Pixel new_pix;
            new_pix.x      = pi->x;
            new_pix.y      = pi->y;
            new_pix.offset = pi->offset;
            new_branch.pixels.push_back( new_pix );
        }
        new_path.branches.push_back( new_branch );
    }
    return new_path;
}




// Input: an image, mode, x, y, and offset
// Output: the number of ((x,y) || offset)'s neighbors in the image
int FilterImages::Neighbors( IplImage *image, int px, int py, int po )
{
    int x = -1;
    int y = -1;
    int o = -1;
    if (px == -1 &&
        py == -1 &&
        po == -1) {
        return -1;
    }
    if (px == -1 &&
        py == -1) {
        x = po % size.width;
        y = po / size.width;
        o = po;
    }
    if (po == -1) {
        x = px;
        y = py;
        o = (y * size.width) + x;
    }

    unsigned char *ip;
    int n = 0;
    for (int
         dy  = -1;
         dy <= +1;
         dy++) {
        for (int
             dx  = -1;
             dx <= +1;
             dx++) {
            if (x + dx <  0          ||
                x + dx >= size.width ||
                y + dy < 0           ||
                y + dy >= size.height) {
                continue;
            }
            if (dx == 0 &&
                dy == 0) {
                continue;
            }
            ip = (unsigned char *)image->imageData + ((y + dy) * image->widthStep) + (x + dx);
            if (*ip > 0) {
                n++;
            }
        }
    }

    return n;
}




// Input: a dx and dy, each between -1 and 1
// Output: a number between 1 and 8 representing the direction
// Note: implements canonical counter-clockwise 8 directions, assuming (0,0) in NW position
int FilterImages::Dir( int dx, int dy )
{
    if (dx ==  1 && dy ==  0) { return 1; }
    if (dx ==  1 && dy == -1) { return 2; }
    if (dx ==  0 && dy == -1) { return 3; }
    if (dx == -1 && dy == -1) { return 4; }
    if (dx == -1 && dy ==  0) { return 5; }
    if (dx == -1 && dy ==  1) { return 6; }
    if (dx ==  0 && dy ==  1) { return 7; }
    if (dx ==  1 && dy ==  1) { return 8; }
    return -1;
}




//
//
void FilterImages::OutputTab( int indent )
{
    for (int
         i = 0;
         i < indent;
         i++) {
        cout << "\t";
    }
    return;
}




//
//
void FilterImages::OutputPixel( Pixel &p, int indent )
{
    OutputTab( indent );
    cout << "("
         << p.x
         << ","
         << p.y
         << ","
         << p.offset
         << ")"
         << endl;
    return;
}




//
//
void FilterImages::OutputPixelVector( vector<Pixel> &p_vec, int indent )
{
    OutputTab( indent );
    cout << "pixels:" << endl;
    for (vector<Pixel>::iterator
         pixel_it  = p_vec.begin();
         pixel_it != p_vec.end();
         pixel_it++) {
        OutputPixel( *pixel_it, indent + 1 );
    }
    return;
}




//
//
void FilterImages::OutputBranch( Branch &b, int indent )
{
    OutputTab( indent );
    cout << "length = "<< b.length << endl;
    OutputTab( indent );
    cout << "num pixels = "<< b.num_pixels << endl;
    OutputPixelVector( b.pixels, indent );
    return;
}




//
//
void FilterImages::OutputBranchVector( vector<Branch> &b_vec, int indent )
{
    OutputTab( indent );
    cout << "branches:" << endl;
    for (vector<Branch>::iterator
         branch_it  = b_vec.begin();
         branch_it != b_vec.end();
         branch_it++) {
        OutputBranch( *branch_it, indent + 1 );
        OutputTab( indent + 1 );
        cout << "-----" << endl;
    }
    return;
}




//
//
void FilterImages::OutputComponent( Component &c, int indent )
{
    OutputTab( indent );
    cout << "num pixels = " << c.num_pixels << endl;
    OutputTab( indent );
    cout << "num extremities = " << c.num_extremities << endl;
    OutputTab( indent );
    cout << "num contours = " << c.num_contours << endl;
    OutputTab( indent );
    cout << "all" << endl;
    OutputPixelVector( c.pixels, indent );
    OutputTab( indent );
    cout << "extremity" << endl;
    OutputPixelVector( c.extremities, indent );
    OutputTab( indent );
    cout << "contour" << endl;
    OutputPixelVector( c.contours, indent );
    return;
}




//
//
void FilterImages::OutputComponentVector( vector<Component> &c_vec, int indent )
{
    OutputTab( indent );
    cout << "components:" << endl;
    for (vector<Component>::iterator
         component_it  = c_vec.begin();
         component_it != c_vec.end();
         component_it++) {
        OutputComponent( *component_it, indent + 1 );
        OutputTab( indent + 1 );
        cout << "-----" << endl;
    }
    return;
}




//
//
void FilterImages::OutputPath( Path &p, int indent )
{
    OutputTab( indent );
    cout << "length = " << p.length << endl;
    OutputTab( indent );
    cout << "num pixels = " << p.num_pixels << endl;
    OutputTab( indent );
    cout << "num branches = " << p.num_branches << endl;
    OutputPixelVector( p.pixels, indent );
    OutputTab( indent );
    cout << "directions:" << endl;
    for (int
         i = 0;
         i < p.directions.size();
         i++) {
        OutputTab( indent + 1 );
        cout << p.directions[i] << endl;
    }
    OutputBranchVector( p.branches, indent );
    return;
}




//
//
void FilterImages::OutputPathVector( vector<Path> &p_vec, int indent )
{
    OutputTab( indent );
    cout << "paths:" << endl;
    for (vector<Path>::iterator
         path_it  = p_vec.begin();
         path_it != p_vec.end();
         path_it++) {
        OutputPath( *path_it, indent + 1 );
        OutputTab( indent + 1 );
        cout << "-----" << endl;
    }
    return;
}




//
//
void FilterImages::FreeBranch( Branch &b )
{
    b.pixels.clear();
    return;
}




//
//
void FilterImages::FreeBranchVector( vector<Branch> &b_vec )
{
    for (vector<Branch>::iterator
         branch_it  = b_vec.begin();
         branch_it != b_vec.end();
         branch_it++) {
        FreeBranch( *branch_it );
    }
    b_vec.clear();
    return;
}




//
//
void FilterImages::FreeComponent( Component &c )
{
    c.pixels.clear();
    c.extremities.clear();
    c.contours.clear();
    return;
}




//
//
void FilterImages::FreeComponentVector( vector<Component> &c_vec )
{
    for (vector<Component>::iterator
         component_it  = c_vec.begin();
         component_it != c_vec.end();
         component_it++) {
        FreeComponent( *component_it );
    }
    c_vec.clear();
    return;
}




//
//
void FilterImages::FreePath( Path &p )
{
    p.pixels.clear();
    p.directions.clear();
    FreeBranchVector( p.branches );
    return;
}




//
//
void FilterImages::FreePathVector( vector<Path> &p_vec )
{
    for (vector<Path>::iterator
         path_it  = p_vec.begin();
         path_it != p_vec.end();
         path_it++) {
        FreePath( *path_it );
    }
    p_vec.clear();
    return;
}




// Input: an (x,y) position
// Output: the backbone id close to (x,y), or -1 otherwise
int FilterImages::NearBackboneId( int x, int y ) {
    short *p;
    for (short
         dy  = -2;
         dy <= +2;
         dy++) {
        for (short
             dx  = -2;
             dx <= +2;
             dx++) {
            if (x + dx <  0          ||
                x + dx >= size.width ||
                y + dy <  0          ||
                y + dy >= size.height) {
                continue;
            }
            p = component_id_map + ((y + dy) * size.width) + (x + dx);
            if (*p != -1) {
                return *p;
            }
        }
    }
    return -1;
}




// Input: an image, backbone id, and color flag
// Output: a mutated image, whose backbones are either (color): colored black, except the given one, which is white; or (! color): colored white, except the given one, which is red
void FilterImages::HighlightBackboneId( unsigned char *data, int backbone_id, bool color ) {
    unsigned char Rb, Gb, Bb, Rf, Gf, Bf;
    if (color) { Rb = 0;   Gb = 0;   Bb = 0;    Rf = 255; Gf = 255; Bf = 255; }  // background is black, foreground is white
    else       { Rb = 255; Gb = 255; Bb = 255;  Rf = 255; Gf = 0;   Bf = 0;   }  // background is white, foreground is red
    
    short         *p = backbone_id_map;
    unsigned char *c = data;
    
    for (int
         k = 0;
         k < size.width * size.height;
         k++, p++) {
        if (*p != -1) { *c++ = Rb; *c++ = Gb; *c++ = Bb; }
        else          {  c += 3; }
    }
    
    p = backbone_id_map;
    c = data;
    for (int
         k = 0;
         k < size.width * size.height;
         k++, p++) {
        if (*p == backbone_id) { *c++ = Rf; *c++ = Gf; *c++ = Bf; }
        else                   { c += 3; }
    }

    return;
}




// int main( int argc, char* argv[] ) {

//     // validate the command line arguments
//     if (argc != 3) {
//         cout << "Usage: image </path/to/bmp_img_filename_stub> <nm per pixel>" << endl;
//         exit( 1 );
//     }
//     if (strlen( argv[1] ) > 80) {
//         cout << "Bad filename: too long." << endl;
//         exit( 2 );
//     }
//     if (atof( argv[2] ) == 0.0) {
//         cout << "Bad nm per pixel value: not a valid float." << endl;
//         exit( 3 );
//     }

//     // initialize essentials
//     char *img_fn_stub = argv[1];
//     double nmpp = atof( argv[2] );
//     FilterImages flt;

//     // load the image
//     char img_fn[100];
//     strcpy( img_fn, img_fn_stub );
//     strcat( img_fn, ".bmp" );
//     flt.LoadImage( img_fn );

//     // filter the image and save the result
//     flt.ApplyFilters(
//         __THRESHOLD_METHOD__,
//         __FIXED_THRESH_TOO_BRIGHT_ELIM__,
//         __FIXED_THRESH_TOO_DIM_ELIM__,
//         __ADAPTIVE_THRESH_TOO_DIM_ELIM__,
//         __ADAPTIVE_THRESH_METHOD__,
//         __ADAPTIVE_THRESH_BOX_DIM__
//     );
//     char filtered_img_fn[100];
//     strcpy( filtered_img_fn, img_fn_stub );
//     strcat( filtered_img_fn, ".afme_filtered.bmp" );
//     cvSaveImage( filtered_img_fn, flt.filteredImage );

//     // find the image backbones and save the result
//     flt.FindBackbones();
//     char backbones_img_fn[100];
//     strcpy( backbones_img_fn, img_fn_stub );
//     strcat( backbones_img_fn, ".afme_backbones.bmp" );
//     cvSaveImage( backbones_img_fn, flt.backboneImage );

//     // output the backbone lengths
//     for (int
//          i = 0;
//          i < flt.backbones.size();
//          i++) {
//         printf( "%d\t%.2f\n", i, flt.backbones[i].length * nmpp );
//     }

//     // exit
//     exit( 0 );

// }
