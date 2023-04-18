#include "Water.h"
#include "ImageData.h"
#include <sstream>
#include <fstream>
//Make a plane for the water to be rendered in
void planeMeshQuads(std::vector<Water::Vertex>& verts, std::vector<unsigned int>& indices,
	float min, float max, float stepsize) {
	float x = min;
	float y = 0;
	//map x and z into (0,1) range for uv.
	for (float x = min; x <= max; x += stepsize) {
		for (float z = min; z <= max; z += stepsize) {
			float u = (x - min) / (max - min);
			float v = (z - min) / (max - min);

			Water::Vertex vert;
			vert.position = glm::vec3(x, y, z);
			vert.uv = glm::vec2(u, v);
			vert.normal = glm::vec3(0, 1, 0);
			verts.emplace_back(vert);
		}
	}

	// Index data for the mesh quad
	int nCols = (max - min) / stepsize + 1;
	int i = 0, j = 0;
	for (float x = min; x <= max - 1; x += stepsize) {
		j = 0;
		for (float z = min; z <= max - 1; z += stepsize) {
			//CCW starting from the top left corner(0,1,2,3)
			indices.emplace_back(i * nCols + j);			
			indices.emplace_back(i * nCols + j + 1);		
			indices.emplace_back((i + 1) * nCols + j + 1);
			indices.emplace_back((i + 1) * nCols + j);	
			j++;
		}
		i++;
	}
}

Water::Water(float min, float max, float stepsize) {
	//generate the plane mesh and store it inside the vertices and indices
	planeMeshQuads(verts, indices, min, max, stepsize);
	VAO = 0;
	VBO = 0;
	EBO = 0;
	//Generate VAO and VBO and start filling the buffer
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(Vertex), &verts[0], GL_STATIC_DRAW);

	// position vertex pointer
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(0);

	// uv pointer
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Vertex::uv));
	glEnableVertexAttribArray(1);

	// Normal pointer
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Vertex::normal));
	glEnableVertexAttribArray(2);

	//EBO for the indices data
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	//water texture
	ImageData waterData("assets/water.bmp");
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, waterData.width, waterData.height, 0, GL_RGB, GL_UNSIGNED_BYTE, waterData.data);
	glGenerateMipmap(GL_TEXTURE_2D); 

	// Displacement map texture
	ImageData displaceData("assets/displacement-map1.bmp");
	glGenTextures(1, &dispMapTexture);
	glBindTexture(GL_TEXTURE_2D, dispMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, displaceData.width, displaceData.height, 0, GL_RGB, GL_UNSIGNED_BYTE, displaceData.data);
	glGenerateMipmap(GL_TEXTURE_2D);
}
//render the water
void Water::draw(ShaderProgram& shader, glm::mat4 mvp) {
	glBindVertexArray(VAO);
	glUseProgram(shader.ID);

	shader.setUniformMatrix4fv("mvp",mvp);
	shader.setUniform1i("tex", 0);
	shader.setUniform1i("dispMap", 1);

	//two differnt texture to bind to two differnt texture 
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, dispMapTexture);

	//specify patch parameter since we are using quads.
	glPatchParameteri(GL_PATCH_VERTICES, 4);  
	glDrawElements(GL_PATCHES, indices.size(), GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
	glUseProgram(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}