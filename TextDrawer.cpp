//
//  TextDrawer.cpp
//  
//
//  Created by owen ou on 2021/10/1.
//  Reference code: https://github.com/harfbuzz/harfbuzz-tutorial/blob/master/hello-harfbuzz-freetype.c
//					https://www.cs.uregina.ca/Links/class-info/315/WWW/Lab5/index_gl3.html

#include "TextDrawer.hpp"

// for drawing
#include "ColorProgram.hpp"
#include "gl_errors.hpp"
#include <glm/gtc/type_ptr.hpp>

// for loading font
#include <ft2build.h>
#include FT_FREETYPE_H

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <hb.h>
#include <hb-ft.h>

//n.b. declared static so they don't conflict with similarly named global variables elsewhere:
static GLuint vertex_buffer = 0;
static GLuint vertex_buffer_for_color_program = 0;

//initialize opengl vertex buffer, binding
//grab from DrawLines.cpp
static Load< void > setup_buffers(LoadTagDefault, [](){
	
	{
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
	}
	
	{
		glGenBuffers(1, &vertex_buffer);

		glGenVertexArrays(1, &vertex_buffer_for_color_program);
		glBindVertexArray(vertex_buffer_for_color_program);
	 
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
		//reserve space for 2D squad, 6 vertice, 4 float each
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	 
		glEnableVertexAttribArray(color_program->Color_vec4);
	 
		//vertex array object
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	 
		glBindBuffer(GL_ARRAY_BUFFER, 0);	//unbind
		glBindVertexArray(0);	//unbind
	}
	
	
	GL_ERRORS(); //PARANOIA: make sure nothing strange happened during setup

});


TextDrawer::TextDrawer(std::string const &fontfile, glm::mat4 const &world_to_clip_) : world_to_clip(world_to_clip_){
	
	// ---------Initialize GL buffer ----------------
	
	
	// ---------Initialize FT and harfbuzz------------
	
	// load library
	if ((ft_error = FT_Init_FreeType (&ft_library))){
		std::cout << "abort at library" << std::endl;
		abort();
	}
	// load font, ttf file
	const char * fontname = fontfile.c_str();
	if ((ft_error = FT_New_Face (ft_library, fontname, 0, &ft_face))){
		std::cout << "abort at fontname" << std::endl;
		abort();
	}
	// set font size
	if ((ft_error = FT_Set_Char_Size (ft_face, fontsize*64, fontsize*64, 0, 0))){
		std::cout << "abort at charsize" << std::endl;
		abort();
	}
	
	// set pixel size, need integer width and height
	if ((ft_error = FT_Set_Pixel_Sizes (ft_face, 0, 16))){
		std::cout << "abort at pixel size" << std::endl;
		abort();
	}
	
	//Create hb-ft font
	hb_font = hb_ft_font_create (ft_face, NULL);

	//Create hb-buffer and populate
	hb_buffer = hb_buffer_create();
}

// grab from DrawLines.cpp
TextDrawer::~TextDrawer(){
	//TODO: draw square with text texture on
	//upload vertices to vertex_buffer:
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer); //set vertex_buffer as current
	//TODO:
	glBufferData(GL_ARRAY_BUFFER, attribs.size() * sizeof(attribs[0]), attribs.data(), GL_STREAM_DRAW); //upload attribs array
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//set color_program as current program:
	glUseProgram(color_program->program);

	//upload OBJECT_TO_CLIP to the proper uniform location:
	glUniformMatrix4fv(color_program->OBJECT_TO_CLIP_mat4, 1, GL_FALSE, glm::value_ptr(world_to_clip));

	//use the mapping vertex_buffer_for_color_program to fetch vertex data:
	glBindVertexArray(vertex_buffer_for_color_program);

	//run the OpenGL pipeline:
	glDrawArrays(GL_LINES, 0, GLsizei(attribs.size()));

	//reset vertex array to none:
	glBindVertexArray(0);

	//reset current program to none:
	glUseProgram(0);
	
	//free buffer
	hb_buffer_destroy (hb_buffer);
	hb_font_destroy (hb_font);

	FT_Done_Face (ft_face);
	FT_Done_FreeType (ft_library);
}


void TextDrawer::drawText(std::string const &text,
						  glm::vec3 const &position,
						  glm::vec3 const &radius,
						  glm::u8vec4 const &color){
		
	// fill up buffer with text, utf8 code point
	const char * char_text = text.c_str();		// convert string to char
	hb_buffer_add_utf8 (hb_buffer, char_text, -1, 0, -1);
	hb_buffer_guess_segment_properties (hb_buffer);
	
	//shape text to glyph
	hb_shape (hb_font, hb_buffer, NULL, 0);

	//Get glyph information and positions out of the buffer
	//glyph is still in face
	len = hb_buffer_get_length (hb_buffer);	// indicate number of characters in buffer
	info = hb_buffer_get_glyph_infos (hb_buffer, NULL); // store codepoint and glyph cluster
	pos = hb_buffer_get_glyph_positions (hb_buffer, NULL);	// store character position and offset
	
	
	// draw  character
	
	// size of glyph for entire string
	float width = 2 * margin;
	float height = 2 * margin;
	for (unsigned int i = 0; i < len; i++)
	{
		width  += pos[i].x_advance / 64.;
		height -= pos[i].y_advance / 64.;
	}
	if (HB_DIRECTION_IS_HORIZONTAL (hb_buffer_get_direction(hb_buffer)))
		height += fontsize;
	else
		width  += fontsize;
	
	//get vertex for the square to draw text on
	//TODO: reserve size on vertex or texture buffer
	
	
	
	float current_x = position.x;	// current character position
	float current_y = position.y;
	
	//initialize texture and vertex buffer
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(vertex_buffer_for_color_program);	// set color program for current array object
	
	//for each character in string
	for(unsigned int i = 0; i < len; i++){
		//get glyph info
		hb_codepoint_t gid   = info[i].codepoint;	// get glyph id, uint32_t -> unsigned int
		unsigned int cluster = info[i].cluster;
		float x_position = current_x + pos[i].x_offset / 64.;
		float y_position = current_y + pos[i].y_offset / 64.;
		
		// render glypth as bitmap
		if((ft_error = FT_Load_Glyph(ft_face, gid, FT_LOAD_RENDER))){abort();}	// load and render glyph
		FT_GlyphSlot slot = ft_face->glyph;	// access bitmap, format changed to bitmap
		FT_Bitmap bitmap = slot->bitmap;	// slot->bitmap_top/bitmap_left tells bitmap location relative to
		
		// update VBO for each character, actual drawing position
		float w = pos[i].x_advance;
		float h = pos[i].y_advance;
		float vertices[6][4] = {
			{ x_position,     y_position + h,   0.0f, 0.0f },
			{ x_position,     y_position,       0.0f, 1.0f },
			{ x_position + w, y_position,       1.0f, 1.0f },

			{ x_position,     y_position + h,   0.0f, 0.0f },
			{ x_position + w, y_position,       1.0f, 1.0f },
			{ x_position + w, y_position + h,   1.0f, 0.0f }
		};
		
		
		// generate texture
		unsigned int texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RED,					//grayscale bitmap, single channel
			slot->bitmap.width,
			slot->bitmap.rows,
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			slot->bitmap.buffer
		);
		// set texture options
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		
//		CharTexBuff.push_back(texture);
		
		// render glyph texture over quad
//		glBindTexture(GL_TEXTURE_2D, texture);
		// update content of VBO memory
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		// render quad
		glDrawArrays(GL_TRIANGLES, 0, 6);
		
		// update position
		current_x += pos[i].x_advance / 64.;
		current_y += pos[i].y_advance / 64.;
	}
	
	//unbind vertex and texture buffer
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	
	
	
}
