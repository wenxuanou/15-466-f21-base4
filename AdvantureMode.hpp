#include "Mode.hpp"

#include "Scene.hpp"
#include "Sound.hpp"
#include "TextDrawer.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>

struct AdvantureMode : Mode {
	AdvantureMode();
	virtual ~AdvantureMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//----- game state -----
	glm::vec2 scene_radius = glm::vec2(7.0f, 5.0f);             //size of the scene
	
	//text drawer
	//TextDrawer textdrawer;
	
	//Shader program that draws transformed, vertices tinted with vertex colors:
	ColorTextureProgram color_texture_program;

	//Buffer used to hold vertex data during drawing:
	GLuint vertex_buffer = 0;

	//Vertex Array Object that maps buffer locations to color_texture_program attribute locations:
	GLuint vertex_buffer_for_color_texture_program = 0;

	//Solid white texture:
	GLuint white_tex = 0;

	//matrix that maps from clip coordinates to court-space coordinates:
	glm::mat3x2 clip_to_court = glm::mat3x2(1.0f);
	// computed in draw() as the inverse of OBJECT_TO_CLIP
	// (stored here so that the mouse handling code can use it to position the paddle)
	
};
