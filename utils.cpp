#include "utils.hpp"
#include <iostream>
#include <fstream>
#include <ios>
#include <string>
#include <sstream>
#include <cstdio>


void get_vertex_array(
        unsigned int* vertex_indexes,
        unsigned int* normals_indexes,
        unsigned int* tex_indexes
        , tinyobj::shape_t shape) {
    size_t size = shape.mesh.indices.size();
    for (int i = 0; i <size; ++i) {
        vertex_indexes[i] = shape.mesh.indices[i].vertex_index;
        normals_indexes[i] = shape.mesh.indices[i].normal_index;
        tex_indexes[i] = shape.mesh.indices[i].texcoord_index;
    }

}


