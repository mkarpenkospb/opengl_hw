#include "utils.hpp"
#include <iostream>
#include <fstream>
#include <ios>
#include <string>
#include <sstream>
#include <cstdio>


void get_vertex_array(
        std::vector<unsigned int>& vertex_indexes,
        std::vector<float>& vertex_data,
        const tinyobj::shape_t& shape,
        const tinyobj::attrib_t& attrib
        ) {
    size_t size = shape.mesh.indices.size();
    for (int i = 0; i < size; ++i) {
        vertex_indexes.push_back(i);
        tinyobj::index_t idx = shape.mesh.indices[i];

        vertex_data.push_back(attrib.vertices[3 * idx.vertex_index]);
        vertex_data.push_back(attrib.vertices[3 * idx.vertex_index + 1]);
        vertex_data.push_back(attrib.vertices[3 * idx.vertex_index + 2]);

        vertex_data.push_back(attrib.normals[3 * idx.normal_index]);
        vertex_data.push_back(attrib.normals[3 * idx.normal_index + 1]);
        vertex_data.push_back(attrib.normals[3 * idx.normal_index + 2]);

        vertex_data.push_back(attrib.texcoords[2 * idx.texcoord_index]);
        vertex_data.push_back(attrib.texcoords[2 * idx.texcoord_index + 1]);
    }
}


//https://opengameart.org/content/mountain-skyboxes
// https://habr.com/ru/post/347750/
unsigned int loadCubemap(const std::vector<std::string>& faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
    stbi_set_flip_vertically_on_load(false);
    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                         0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}