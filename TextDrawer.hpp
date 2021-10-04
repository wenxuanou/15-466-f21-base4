//
//  TextDrawer.hpp
//  
//
//  Created by owen ou on 2021/10/1.
//

#include <ft2build.h>
#include FT_FREETYPE_H

#include <hb.h>
#include <hb-ft.h>

#include <glm/glm.hpp>

#include <string>
#include <vector>

struct TextDrawer {
	
	//Start drawing; will remember world_to_clip matrix:
	TextDrawer(std::string const &fontfile, glm::mat4 const &world_to_clip);
	
	//Finish drawing (push attribs to GPU):
	~TextDrawer();
		
	void drawText(std::string const &text,
				  glm::vec3 const &position = glm::vec3(0.0f, 0.0f, 0.0f),
				  glm::vec3 const &radius = glm::vec3(5.0f, 1.0f, 0.0f),
				  glm::u8vec4 const &color = glm::u8vec4(0xff));
	
	//text stuff
	FT_Library ft_library;
	FT_Face ft_face;
	FT_Error ft_error;
	hb_font_t *hb_font = nullptr;
	hb_buffer_t *hb_buffer = nullptr;
	
	unsigned int len = 0;	// lengh of hb_buffer == number of characters
	hb_glyph_info_t *info = nullptr;		// codepoint and glyph cluster
	hb_glyph_position_t *pos = nullptr;		// position to draw glyph in line
	
	const FT_F26Dot6 fontsize = 36;
	const double margin = fontsize * 0.5;
	
	//projection matrix
	glm::mat4 const world_to_clip;
	
	//charater buffer
//	std::vector<unsigned int> CharTexBuff;	//character texture id buffer
	
};
