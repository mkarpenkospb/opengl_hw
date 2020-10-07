#pragma once

#include <GL/glew.h>
#include <vector>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "tiny_obj_loader.h"
#include <stb_image.h>

void get_vertex_array(
        std::vector<unsigned int>& vertex_indexes,
        std::vector<float>& vertex_data,
        const tinyobj::shape_t& shape,
        const tinyobj::attrib_t& attrib
);

unsigned int loadCubemap(const std::vector<std::string>& faces);


