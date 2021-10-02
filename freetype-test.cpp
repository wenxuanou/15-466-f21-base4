
#include <ft2build.h>
#include FT_FREETYPE_H

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <hb.h>
#include <hb-ft.h>

// OpenGL Headers
/*
#include <windows.h>                                      // (The GL Headers Need It)
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
 */


#define FONT_SIZE 36
#define MARGIN (FONT_SIZE * .5)

//This file exists to check that programs that use freetype / harfbuzz link properly in this base code.
//You probably shouldn't be looking here to learn to use either library.

int main(int argc, char **argv) {

	// command line input
    const char *fontfile;
    const char *text;

    if (argc < 3)
    {
        fprintf (stderr, "usage: hello-harfbuzz font-file.ttf text\n");
        exit (1);
    }
    
    fontfile = argv[1];
    text = argv[2];

    //Initialize FreeType and create FreeType font face.
    FT_Library ft_library;
    FT_Face ft_face;
    FT_Error ft_error;

    if ((ft_error = FT_Init_FreeType (&ft_library)))	// load library
        abort();
    if ((ft_error = FT_New_Face (ft_library, fontfile, 0, &ft_face)))	// load font, ttf file
        abort();
    if ((ft_error = FT_Set_Char_Size (ft_face, FONT_SIZE*64, FONT_SIZE*64, 0, 0)))	// set font size
        abort();

    //Create hb-ft font
    hb_font_t *hb_font;
    hb_font = hb_ft_font_create (ft_face, NULL);

    //Create hb-buffer and populate
    hb_buffer_t *hb_buffer;
    hb_buffer = hb_buffer_create ();
    
	hb_buffer_add_utf8 (hb_buffer, text, -1, 0, -1);	// fill up buffer with text, utf8 code point
    hb_buffer_guess_segment_properties (hb_buffer);
    
    //shape text to glyph
    hb_shape (hb_font, hb_buffer, NULL, 0);

    //Get glyph information and positions out of the buffer
	//glyph is still in face
    unsigned int len = hb_buffer_get_length (hb_buffer);	// indicate number of characters in buffer
    hb_glyph_info_t *info = hb_buffer_get_glyph_infos (hb_buffer, NULL); // store codepoint and glyph cluster
    hb_glyph_position_t *pos = hb_buffer_get_glyph_positions (hb_buffer, NULL);	// store character position and offset

	
	// drawing with opengl
	
	// size of glyph for entire string
	double width = 2 * MARGIN;
	double height = 2 * MARGIN;
	for (unsigned int i = 0; i < len; i++)
	{
		width  += pos[i].x_advance / 64.;
		height -= pos[i].y_advance / 64.;
	}
	if (HB_DIRECTION_IS_HORIZONTAL (hb_buffer_get_direction(hb_buffer)))
		height += FONT_SIZE;
	else
		width  += FONT_SIZE;

	
	//TODO: set up opengl
	/*
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowSize(800,600);
	glutCreateWindow("Hello World");
	setup();
	glutDisplayFunc(display);
	glutMainLoop();
	 */
	
	
	//set up free type rasterization
	if ((ft_error = FT_Set_Pixel_Sizes (ft_face, 0, 16)))	// set pixel size, need integer width and height
		abort();
	
	// draw each character
	double current_x = 0;	// current character position
	double current_y = 0;
	for(unsigned int i = 0; i < len; i++){
		//get glyph info
		hb_codepoint_t gid   = info[i].codepoint;	// get glyph id, uint32_t -> unsigned int
		unsigned int cluster = info[i].cluster;
		double x_position = current_x + pos[i].x_offset / 64.;
		double y_position = current_y + pos[i].y_offset / 64.;
		
		if((ft_error = FT_Load_Glyph(ft_face, gid, FT_LOAD_DEFAULT))){abort();}
		if((ft_error = FT_Render_Glyph(ft_face->glyph, FT_RENDER_MODE_NORMAL))){abort();}
		
		FT_GlyphSlot slot = ft_face->glyph;	// access bitmap, format changed to bitmap
	    FT_Bitmap bitmap = slot->bitmap;
		
		// bitmap for current character
		/*
		glBitmap(width, height,
				 x_position, y_position,
				 
				 GL_UNSIGNED_BYTE,
				 bitmap);
		*/
		   
		
		char glyphname[32];
			  hb_font_get_glyph_name (hb_font, gid, glyphname, sizeof (glyphname));

		printf ("glyph='%s'	cluster=%d	position=(%g,%g)\n",
		  glyphname, cluster, x_position, y_position);
		   
		printf ("bitmap shape: width=%d, height=%d \n",
				bitmap.width, bitmap.rows);
		
		printf ("current pos: x= %g, y=%g \n",
				current_x, current_y);
		
		// update position
		current_x += pos[i].x_advance / 64.;
		current_y += pos[i].y_advance / 64.;
	}
	
	

    //free buffer
    hb_buffer_destroy (hb_buffer);
    hb_font_destroy (hb_font);

    FT_Done_Face (ft_face);
    FT_Done_FreeType (ft_library);

}



/*
text render api:

1. render before game, like load, look up
Load< T > textstream

for(i){
    if(textstream[i].name == "paragraph 1"){
        draw(textstream[i], position)
    }

}

2. render in game, read string and render at location
text.renderer("string", position);

*think of glypth, ligatures / non-ligatures


*/


