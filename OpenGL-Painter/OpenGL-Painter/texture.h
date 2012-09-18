
GLuint LoadTextureRAW( IplImage  * image, int wrap )
{
    GLuint texture;
//    int width, height;

    // allocate a texture name
    glGenTextures( 1, &texture );

    // select our current texture
    glBindTexture( GL_TEXTURE_2D, texture );

    // select modulate to mix texture with color for shading
    glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

    // when texture area is small, bilinear filter the closest mipmap
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                     GL_LINEAR_MIPMAP_NEAREST );
    // when texture area is large, bilinear filter the first mipmap
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    // if wrap is true, the texture wraps over at the edges (repeat)
    //       ... false, the texture ends at the edges (clamp)
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                     wrap ? GL_REPEAT : GL_CLAMP );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                     wrap ? GL_REPEAT : GL_CLAMP );

// DATA IS SORED BACKWARDS IN BMP FILES (USE GL_BGR_EXT
    // build our texture mipmaps
	cvFlip( image, image, -1 );
    gluBuild2DMipmaps( GL_TEXTURE_2D, 3, image->width, image->height,
                       GL_BGR_EXT, GL_UNSIGNED_BYTE, image->imageData );

// IF DATA IS STORED AS RGB
//    // build our texture mipmaps
//    gluBuild2DMipmaps( GL_TEXTURE_2D, 3, width, height,
//                       GL_RGB, GL_UNSIGNED_BYTE, data );

    return texture;
}






// load a 256x256 RGB .RAW file as a texture
GLuint LoadTextureRAW_RGBA( const char * filename, int width, int height, int wrap )
{
    GLuint texture;
	//    int width, height;
    char* data;
    FILE * file;
	
    // open texture data
    file = fopen( filename, "rb" );
    if ( file == NULL ) return 0;
	
    // allocate buffer
	//    width = 256;
	//    height = 256;
    data = (char*) malloc( width * height * 4 );
	
    // read texture data
    fread( data, 54, 1, file );
    fread( data, width * height * 4, 1, file );
    fclose( file );
	
    // allocate a texture name
    glGenTextures( 1, &texture );
	
    // select our current texture
    glBindTexture( GL_TEXTURE_2D, texture );
	
    // select modulate to mix texture with color for shading
    glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	
    // when texture area is small, bilinear filter the closest mipmap
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
					GL_LINEAR_MIPMAP_NEAREST );
    // when texture area is large, bilinear filter the first mipmap
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	
    // if wrap is true, the texture wraps over at the edges (repeat)
    //       ... false, the texture ends at the edges (clamp)
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
					wrap ? GL_REPEAT : GL_CLAMP );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
					wrap ? GL_REPEAT : GL_CLAMP );
	
	// DATA IS SORED BACKWARDS IN BMP FILES (USE GL_BGR_EXT
    // build our texture mipmaps
    gluBuild2DMipmaps( GL_TEXTURE_2D, 4, width, height,
					  GL_BGRA_EXT, GL_UNSIGNED_BYTE, data );
	
	// IF DATA IS STORED AS RGB
	//    // build our texture mipmaps
	//    gluBuild2DMipmaps( GL_TEXTURE_2D, 3, width, height,
	//                       GL_RGB, GL_UNSIGNED_BYTE, data );
	
	
    // free buffer
    free( data );
	
    return texture;
}
