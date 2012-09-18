
/* GLOBAL VARIABLES */
CvHaarClassifierCascade *cascade;
CvMemStorage            *storage;

CvPoint3D32f detectFaces( IplImage *img , CvPoint3D32f prev, CvRect *face)
{
	int i;
	float rx,ry,rs;

	CvSeq *faces = cvHaarDetectObjects(
			img,
			cascade,
			storage,
			1.1,
			3,
			0, /*CV_HAAR_DO_CANNY_PRUNNING*/
			cvSize( 40, 40 ) );

	/* LOAD PREVIOUS VALUE IN CASE OF NOT DETECTION */
	rx = prev.x;
	ry = prev.y;
	rs = prev.z;


	for( i = 0 ; i < ( faces ? faces->total : 0 ) ; i++ ) {
		CvRect *r = ( CvRect* )cvGetSeqElem( faces, i );
		cvRectangle( img,
					 cvPoint( r->x, r->y ),
					 cvPoint( r->x + r->width, r->y + r->height ),
					 CV_RGB( 255, 0, 0 ), 1, 8, 0 );
		rx=r->x+r->width/2  - img->width/2;
		ry=r->y+r->height/2 - img->height/2;
		rs=r->height;

		*face = cvRect(r->x,r->y,r->width,r->height);
		// Dist_new = Dist_old * height_old / height_new
//		printf("%d %d %d\n", rx, ry, r->width);
	}


	return(cvPoint3D32f(rx,ry,rs));
}


//    TODO add stability: http://opencv.willowgarage.com/documentation/motion_analysis_and_object_tracking.html
//    CvKalman* kalman = cvCreateKalman( 3, 3, 0 );
CvPoint3D32f filter ( CvPoint3D32f p ,CvPoint3D32f prev) 
{
	CvPoint3D32f q = p; 
	q.x = (q.x + prev.x)/2;
	q.y = (q.y + prev.y)/2;
	q.z = (q.z + prev.z)/2;

	return q;
}

void cvgenerateview(CvPoint3D32f p, IplImage *canvas, const char *win )
{
	cvRectangle(canvas, cvPoint(0,0), cvPoint(canvas->width, canvas->height), CV_RGB(0,0,0),CV_FILLED,8,0);
	CvPoint b;
	b.y = canvas->height/2 +p.y*0.4;
	b.x = canvas->width/2 +p.x*0.4;
	cvCircle(canvas, b, 50, CV_RGB(0,0,255),CV_FILLED,8,0);

	b.y = canvas->height/2 +p.y*0.6;
	b.x = canvas->width/2 +p.x*0.6;
	cvCircle(canvas, b, 55, CV_RGB(0,255,0),CV_FILLED,8,0);


	b.y = canvas->height/2 +p.y;
	b.x = canvas->width/2 +p.x;
	cvCircle(canvas, b, 60, CV_RGB(255,0,0),CV_FILLED,8,0);

	cvShowImage(win, canvas);
}

