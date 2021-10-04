
#include <ft2build.h>
#include FT_FREETYPE_H

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <hb.h>
#include <hb-ft.h>

#include <iostream>

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

	printf ("entire string: width=%g, height=%g\n",
			width, height);

	
	//set up free type rasterization
	if ((ft_error = FT_Set_Pixel_Sizes (ft_face, 0, 16)))	// set pixel size, need integer width and height
		abort();
	
	// draw each character
	// ft_face has the entiro glyph, hb_buffer tells where to find wanted character
	double current_x = 0;	// current character position
	double current_y = 0;
	
	for(unsigned int i = 0; i < len; i++){
		//get glyph info
		hb_codepoint_t gid   = info[i].codepoint;	// get glyph id, uint32_t -> unsigned int
		unsigned int cluster = info[i].cluster;
		double x_position = current_x + pos[i].x_offset / 64.;
		double y_position = current_y + pos[i].y_offset / 64.;
		
		FT_GlyphSlot slot = ft_face->glyph;	// access bitmap, format changed to bitmap
	    FT_Bitmap bitmap = slot->bitmap;
		
		if((ft_error = FT_Load_Glyph(ft_face, gid, FT_LOAD_DEFAULT))){abort();}
		if((ft_error = FT_Render_Glyph(ft_face->glyph, FT_RENDER_MODE_NORMAL))){abort();}
		   
		
		
		
		
		
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
	
	
	/*
	{
		FT_Bitmap const &bitmap = ft_face->glyph->bitmap;

		std::cout << "Bitmap (" << bitmap.width << "x" << bitmap.rows << "):\n";
		std::cout << "  pitch is " << bitmap.pitch << "\n";
		std::cout << "  pixel_mode is " << int32_t(bitmap.pixel_mode) << "; num_grays is " << bitmap.num_grays << "\n";
		if (bitmap.pixel_mode == FT_PIXEL_MODE_GRAY && bitmap.num_grays == 256 && bitmap.pitch >= 0) {
			for (uint32_t row = 0; row < bitmap.rows; ++row) {
				std::cout << "   ";
				for (uint32_t col = 0; col < bitmap.width; ++col) {
					uint8_t val = bitmap.buffer[row * std::abs(bitmap.pitch) + col];
					if (val < 128) std::cout << '.';
					else std::cout << '#';
				}
				std::cout << '\n';
			}
		} else {
			std::cout << "  (bitmap is not FT_PIXEL_MODE_GRAY with 256 levels and upper-left origin, not dumping)" << "\n";
		}
		std::cout.flush();
	}
	 */
	
	
    //free buffer
    hb_buffer_destroy (hb_buffer);
    hb_font_destroy (hb_font);

    FT_Done_Face (ft_face);
    FT_Done_FreeType (ft_library);

}



