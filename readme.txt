Requirement: GLFW,GLEW,GLM stb_image(https://github.com/nothings/stb)
Files included:Main.cpp, Camera.h, ShaderProgram.h, Water.cpp/h, ImageData.h,assets

Camera.h: used for the spherical coordinate based click/drag camera.
Move forward/backward using arrow key and limit the phi between 0.1-179.
The same camera used in assingment 5.

ImageData.h: used to load the water/displacement bmp since the provided function in assignment 4 didn't. I chose
to use the stb_image to get it's width,height data for texture.

Water.cpp/h: define the structure of rendering the waves.
	
	planeMeshQuads: Generate the vertices and indices for the plane needed 

	Water:Get the vertices and indices and save them into VAO,VBO,EBO and define out vertex attribute pointer(pos,normal,uv)
	Also load the water and displacement texture and bind it.

	Draw: draw the water and the displacement as textrue and draw it.

ShaderProgramBox.h: Class for a program that contains shaders for the box arounding the function. 
Also contain helper function to modify uniform variable within the shader's code. 
Create the shader, compile it and linking a program to both shader and the class itself holds an instance
of ID that can be reference in texture mesh and main. 

	Vertex Shader: Pretty standard vertex shader. The time is used
	to move the texture by defining the output uv based as a function based on time.

	TCS shader: specify the number of vertrices to output as a patch of quads. define our output
	pos,uv,normal array based on the gl invocationID as index. set the outer and inner tess level 
	for quads(4 Outer and 2 Inner) from parameter ol il from main.

	TES shader: Define our abstract quads as in and get Tesselation coordinate of x and y and lerp them
	based on glsl's mix function based on the tesscoord. output the uv,pos,normal

	Geo shader: in as triangle and triangle_strip as out. Have our gerstner wave function the value follow
	the formula given in the assignemnt. loop through each triangle, apply the water displacement map
	texture and create 4 wave functions to combine.calculate the normal and apply the mvp to position generated
	and their 

	frag shader:handle the Phong lighting and use the texture rgb values as ambient. apply diffuse with
	normal and light direction. specular with normal and eyedir.

Main.cpp: runs everything including ShaderProgram and load the texture. provide the key/curosr
feedback for camra and update the time is the glfw loop. Define our outer tess and inner tess level here.


