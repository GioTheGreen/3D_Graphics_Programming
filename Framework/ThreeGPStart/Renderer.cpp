#include "Renderer.h"
#include "Camera.h"
#include "ImageLoader.h"

Renderer::Renderer()
{

}

// On exit must clean up any OpenGL resources e.g. the program, the buffers
Renderer::~Renderer()
{
	// TODO: clean up any memory used including OpenGL objects via glDelete* calls
	glDeleteProgram(m_program);
	glDeleteProgram(m_program2);
	glDeleteBuffers(1, &m_VAO);
	for (size_t i = 0; i < models.size(); i++)
	{
		models[i].~model();
	}
}

// Use IMGUI for a simple on screen GUI
void Renderer::DefineGUI()
{
	// Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	ImGui::Begin("3GP");						// Create a window called "3GP" and append into it.

	ImGui::Text("Visibility.");					// Display some text (you can use a format strings too)	

	ImGui::Checkbox("Wireframe", &m_wireframe);	// A checkbox linked to a member variable

	ImGui::Checkbox("Cull Face", &m_cullFace);

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	ImGui::End();
}

// Load, compile and link the shaders and create a program object to host them
bool Renderer::CreateProgram(std::string vertexName, std::string fragmentName, GLuint& program)
{
	if (program)
		glDeleteProgram(program);

	// Create a new program (returns a unqiue id)
	program = glCreateProgram();

	// Load and create vertex and fragment shaders
	GLuint vertex_shader{ Helpers::LoadAndCompileShader(GL_VERTEX_SHADER, "Data/Shaders/" + vertexName + ".vert") };
	GLuint fragment_shader{ Helpers::LoadAndCompileShader(GL_FRAGMENT_SHADER, "Data/Shaders/" + fragmentName+ ".frag") };
	if (vertex_shader == 0 || fragment_shader == 0)
		return false;

	// Attach the vertex shader to this program (copies it)
	glAttachShader(program, vertex_shader);

	// The attibute location 0 maps to the input stream "vertex_position" in the vertex shader
	// Not needed if you use (location=0) in the vertex shader itself
	//glBindAttribLocation(m_program, 0, "vertex_position");

	// Attach the fragment shader (copies it)
	glAttachShader(program, fragment_shader);

	// Done with the originals of these as we have made copies
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	// Link the shaders, checking for errors
	if (!Helpers::LinkProgramShaders(program))
		return false;

	return true;
}

// Load / create geometry into OpenGL buffers	
bool Renderer::InitialiseGeometry()
{
	// Load and compile shaders into m_program
	if (!CreateProgram("vertex_shader", "fragment_shader", m_program)) 
		return false;
	if (!CreateProgram("vertex_shader_normal", "fragment_shader_normal", m_program2))
		return false;

	// Helpers has an object for loading 3D geometry, supports most types

	// E.g. Load in the jeep
	Helpers::ModelLoader loader;
	//if (!loader.LoadFromFile("Data\\Models\\Sphere\\sphere.obj"))
	//		return false;
	if (!loader.LoadFromFile("Data\\Models\\AquaPig\\hull.obj"))
		return false;
	model hull;
	if (!loader.LoadFromFile("Data\\Models\\AquaPig\\wing_right.obj"))
		return false;
	model wing_right;
	if (!loader.LoadFromFile("Data\\Models\\AquaPig\\wing_left.obj"))
		return false;
	model wing_left;
	if (!loader.LoadFromFile("Data\\Models\\AquaPig\\propeller.obj"))
		return false;
	model propeller;
	if (!loader.LoadFromFile("Data\\Models\\AquaPig\\gun_base.obj"))
		return false;
	model gun_base;
	if (!loader.LoadFromFile("Data\\Models\\AquaPig\\gun.obj"))
		return false;
	model gun;

	wing_right.applyTranslation(glm::vec3(-2.231,0.272,-2.663));
	wing_left.applyTranslation(glm::vec3(2.231, 0.272, -2.663));
	propeller.applyTranslation(glm::vec3(0, 1.395, -3.616));
	propeller.applyRotation(glm::vec3(3.1415 / 2, 0, 0));
	gun_base.applyTranslation(glm::vec3(0, 0.569, -1.866));
	gun.applyTranslation(glm::vec3(0,1.506,0.644));

	gun_base.addChild(&gun);
	hull.addChild(&wing_right);
	hull.addChild(&wing_left);
	hull.addChild(&propeller);
	hull.addChild(&gun_base);

	model tarrain;

	models.push_back(tarrain);
	models.push_back(hull);
	models.push_back(wing_right);
	models.push_back(wing_left);
	models.push_back(propeller);
	models.push_back(gun_base);
	models.push_back(gun);

	Helpers::ImageLoader TerrainTex;
	if (!TerrainTex.Load("Data\\Textures\\grass11.bmp"))
		return false;

	//GLuint texture;
	glGenTextures(1, &TerrianTexture);
	glBindTexture(GL_TEXTURE_2D, TerrianTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TerrainTex.Width(), TerrainTex.Height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, TerrainTex.GetData());
	glGenerateMipmap(GL_TEXTURE_2D);

	

	// -1 to take into account extra vertex at end
	int numCellsX{ /*20 };*/(int)TerrainTex.Width() - 1 }; //Or any number!
	int numCellsZ{ /*20 };*/(int)TerrainTex.Height() - 1 }; //Or any number!
	float cellSize{ 1};

	// Number of vertices will be one more in each dimension
	int numVertsX{ numCellsX + 1 };
	int numVertsZ{ numCellsZ + 1 };
	int numVerts{ numVertsX * numVertsZ };

	std::vector<glm::vec3> TexPositions;
	std::vector<glm::vec3> TexNormals;
	std::vector<glm::vec2> Texcoords;
	std::vector<GLuint> Texelements;


	Helpers::ImageLoader hightmap;
	if (!hightmap.Load("Data\\Heightmaps\\master_blurred_sm.jpg"))
		return false;

	float vertexXtoImage = (float)hightmap.Width() / numVertsX;
	float vertexZtoImage = (float)hightmap.Height() / numVertsZ;
	BYTE* imageData = hightmap.GetData();



	for (int i = 0; i < numVertsX; i++)
	{
		for (int j = 0; j < numVertsZ; j++)
		{
			int imageX = vertexXtoImage * i;
			int imageZ = vertexZtoImage * j;
			size_t offset = ((size_t)imageX + (size_t)imageZ * hightmap.Width()) * 4;
			BYTE BTheight = imageData[offset];

			TexPositions.push_back(glm::vec3(i*cellSize, BTheight * 0.15, j * cellSize));
			TexNormals.push_back(glm::vec3(0,0,0));
			Texcoords.push_back(glm::vec2(i/ numVertsX,j/ numVertsZ));
		}
	}

	bool diamon{ false };
	for (size_t i = 0; i < numCellsX; i++)
	{
		for (size_t j = 0; j < numCellsZ; j++)
		{
			int startVertexIndex = (i * numVertsZ) + j;

			if (diamon)
			{
				Texelements.push_back(GLuint(startVertexIndex));
				Texelements.push_back(GLuint(startVertexIndex + numVertsZ + 1));
				Texelements.push_back(GLuint(startVertexIndex + numVertsZ));

				Texelements.push_back(GLuint(startVertexIndex));
				Texelements.push_back(GLuint(startVertexIndex + 1));
				Texelements.push_back(GLuint(startVertexIndex + 1 + numVertsZ));

				diamon = false;
			}
			else
			{
				Texelements.push_back(GLuint(startVertexIndex));
				Texelements.push_back(GLuint(startVertexIndex + 1));
				Texelements.push_back(GLuint(startVertexIndex + numVertsZ));

				Texelements.push_back(GLuint(startVertexIndex + 1));
				Texelements.push_back(GLuint(startVertexIndex + 1 + numVertsZ));
				Texelements.push_back(GLuint(startVertexIndex + numVertsZ));

				diamon = true;
			}
		}
		if (numCellsZ%2 == 0)
		{
			if (diamon)
			{
				diamon = false;
			}
			else
			{
				diamon = true;
			}
		}
	}


	//normals

	for (int i = 0; i < Texelements.size(); i+= 3)
	{
		glm::vec3 a = TexPositions[Texelements[i]];
		glm::vec3 b = TexPositions[Texelements[i+1]];
		glm::vec3 c = TexPositions[Texelements[i + 2]];

		glm::vec3 TriNormal = glm::normalize(glm::cross(b-a,c-a));
		TexNormals[Texelements[i]] += TriNormal;
		TexNormals[Texelements[i + 1]] += TriNormal;
		TexNormals[Texelements[i + 2]] += TriNormal;
	}



	//Helpers::ModelLoader Shere;
	//if (!Shere.LoadFromFile("Data\\Models\\Sphere\\sphere.obj"))
	//	return false;
	//GLuint positionsVBO;
	//glGenBuffers(1, &positionsVBO);
	//glBindBuffer(GL_ARRAY_BUFFER, positionsVBO);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * Shere.GetMeshVector().size(), Shere.GetMeshVector().data(), GL_STATIC_DRAW);
	//glBindBuffer(GL_ARRAY_BUFFER, 0);


	//GLuint elementEBO;
	//m_numElements = Shere.GetMeshVector().size();
	//glGenBuffers(1, &elementEBO);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementEBO);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * Shere.GetMeshVector().size(), Shere.GetMeshVector().data(), GL_STATIC_DRAW);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	//newElementsAndVAOs(m_VAO, m_numElements);

	//glGenVertexArrays(1, &m_ELVAO[m_VAO].m_VAO);
	//glBindVertexArray(m_ELVAO[m_VAO].m_VAO);
	//glBindBuffer(GL_ARRAY_BUFFER, positionsVBO);
	//glEnableVertexAttribArray(0);
	//glVertexAttribPointer(
	//	0,                  // attribute 0
	//	3,                  // size in components of each item in the stream e.g. a position has 3 components x,y and z
	//	GL_FLOAT,           // type of the item
	//	GL_FALSE,           // normalized or not (advanced)
	//	0,                  // stride (advanced)
	//	(void*)0            // array buffer offset (advanced)
	//);

	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementEBO);
	//glBindVertexArray(0);

	//m_VAO++;


	//std::vector<glm::vec3> CubePositions;
	//std::vector<glm::vec3> CubeNormals;
	//std::vector<GLuint> CubeElements;


	//CubePositions.push_back(glm::vec3(-1,-1,-1));
	//CubePositions.push_back(glm::vec3(1,-1,-1));
	//CubePositions.push_back(glm::vec3(1,1,-1));
	//CubePositions.push_back(glm::vec3(-1,1,-1));

	//CubePositions.push_back(glm::vec3(1,-1,-1));
	//CubePositions.push_back(glm::vec3(1,-1,1));
	//CubePositions.push_back(glm::vec3(1,1,1));
	//CubePositions.push_back(glm::vec3(1,1,-1));

	//CubePositions.push_back(glm::vec3(1,-1,1));
	//CubePositions.push_back(glm::vec3(-1,-1,1));
	//CubePositions.push_back(glm::vec3(-1,1,1));
	//CubePositions.push_back(glm::vec3(1,1,1));

	//CubePositions.push_back(glm::vec3(-1,-1,1));
	//CubePositions.push_back(glm::vec3(-1,-1,-1));
	//CubePositions.push_back(glm::vec3(-1,1,-1));
	//CubePositions.push_back(glm::vec3(-1,1,1));

	//CubePositions.push_back(glm::vec3(-1,1,-1));
	//CubePositions.push_back(glm::vec3(1,1,-1));
	//CubePositions.push_back(glm::vec3(1,1,1));
	//CubePositions.push_back(glm::vec3(-1,1,1));

	//CubePositions.push_back(glm::vec3(1,-1,1));
	//CubePositions.push_back(glm::vec3(-1,-1,1));
	//CubePositions.push_back(glm::vec3(-1,-1,-1));
	//CubePositions.push_back(glm::vec3(1,-1,-1));

	//CubeElements.push_back()

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



	GLuint positionsVBO;
	glGenBuffers(1, &positionsVBO);
	glBindBuffer(GL_ARRAY_BUFFER, positionsVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * TexPositions.size(), TexPositions.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLuint normalsVBO;
	glGenBuffers(1, &normalsVBO);
	glBindBuffer(GL_ARRAY_BUFFER, normalsVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * TexNormals.size(), TexNormals.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLuint texcoordsVBO;
	glGenBuffers(1, &texcoordsVBO);
	glBindBuffer(GL_ARRAY_BUFFER, texcoordsVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * Texcoords.size(), Texcoords.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLuint elementEBO;
	glGenBuffers(1, &elementEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * Texelements.size(), Texelements.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// We can extract from the mesh via:
	//mesh.vertices  - a vector of glm::vec3 (3 floats) giving the position of each vertex
	//mesh.elements - a vector of unsigned ints defining which vertices make up each triangle
	// 
	// TODO: create VBO for the vertices and a EBO for the elements
	// TODO: create a VBA to wrap everything and specify locations in the shaders

	models[0].initialize(newElementsAndVAOs(m_VAO, Texelements.size()), &m_program2, &TerrianTexture);
	//newElementsAndVAOs(m_VAO, Texelements.size());

	glGenVertexArrays(1, &models[0].getElementsAndVAOs()->m_VAO);
	glBindVertexArray(models[0].getElementsAndVAOs()->m_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, positionsVBO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0,                  // attribute 0
		3,                  // size in components of each item in the stream e.g. a position has 3 components x,y and z
		GL_FLOAT,           // type of the item
		GL_FALSE,           // normalized or not (advanced)
		0,                  // stride (advanced)
		(void*)0            // array buffer offset (advanced)
	);


	glBindBuffer(GL_ARRAY_BUFFER, normalsVBO);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(
		1,                  // attribute 0
		3,                  // size in components of each item in the stream e.g. a position has 3 components x,y and z
		GL_FLOAT,           // type of the item
		GL_FALSE,           // normalized or not (advanced)
		0,                  // stride (advanced)
		(void*)0            // array buffer offset (advanced)
	);

	glBindBuffer(GL_ARRAY_BUFFER, texcoordsVBO);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(
		2,                  // attribute 0
		2,                  // size in components of each item in the stream e.g. a position has 3 components x,y and z
		GL_FLOAT,           // type of the item
		GL_FALSE,           // normalized or not (advanced)
		0,                  // stride (advanced)
		(void*)0            // array buffer offset (advanced)
	);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementEBO);
	glBindVertexArray(0);

	m_VAO++;








	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	// Now we can loop through all the mesh in the loaded model:
	for (const Helpers::Mesh& mesh : loader.GetMeshVector())
	{
		GLuint positionsVBO;
		glGenBuffers(1, &positionsVBO);
		glBindBuffer(GL_ARRAY_BUFFER, positionsVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * mesh.vertices.size(), mesh.vertices.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		GLuint normalsVBO;
		glGenBuffers(1, &normalsVBO);
		glBindBuffer(GL_ARRAY_BUFFER, normalsVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * mesh.normals.size(), mesh.normals.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		GLuint texcoordsVBO;
		glGenBuffers(1, &texcoordsVBO);
		glBindBuffer(GL_ARRAY_BUFFER, texcoordsVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * mesh.uvCoords.size(), mesh.uvCoords.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		GLuint elementEBO;
		//m_numElements = mesh.elements.size();
		glGenBuffers(1, &elementEBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * mesh.elements.size(), mesh.elements.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		// We can extract from the mesh via:
		//mesh.vertices  - a vector of glm::vec3 (3 floats) giving the position of each vertex
		//mesh.elements - a vector of unsigned ints defining which vertices make up each triangle
		// 
		// TODO: create VBO for the vertices and a EBO for the elements
		// TODO: create a VBA to wrap everything and specify locations in the shaders

		models[m_VAO].initialize(newElementsAndVAOs(m_VAO, mesh.elements.size()), &m_program, &texture);

		glGenVertexArrays(1, &models[m_VAO].getElementsAndVAOs()->m_VAO);
		glBindVertexArray(models[m_VAO].getElementsAndVAOs()->m_VAO);
		glBindBuffer(GL_ARRAY_BUFFER, positionsVBO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(
			0,                  // attribute 0
			3,                  // size in components of each item in the stream e.g. a position has 3 components x,y and z
			GL_FLOAT,           // type of the item
			GL_FALSE,           // normalized or not (advanced)
			0,                  // stride (advanced)
			(void*)0            // array buffer offset (advanced)
		);


		glBindBuffer(GL_ARRAY_BUFFER, normalsVBO);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(
			1,                  // attribute 0
			3,                  // size in components of each item in the stream e.g. a position has 3 components x,y and z
			GL_FLOAT,           // type of the item
			GL_FALSE,           // normalized or not (advanced)
			0,                  // stride (advanced)
			(void*)0            // array buffer offset (advanced)
		);

		glBindBuffer(GL_ARRAY_BUFFER, texcoordsVBO);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(
			2,                  // attribute 0
			2,                  // size in components of each item in the stream e.g. a position has 3 components x,y and z
			GL_FLOAT,           // type of the item
			GL_FALSE,           // normalized or not (advanced)
			0,                  // stride (advanced)
			(void*)0            // array buffer offset (advanced)
		);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementEBO);
		glBindVertexArray(0);

		m_VAO++;
	}

	Helpers::ImageLoader tex;
	if (!tex.Load("Data\\Models\\AquaPig\\aqua_pig_2K.png"))
		return false;
	//GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex.Width(), tex.Height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, tex.GetData());
	glGenerateMipmap(GL_TEXTURE_2D);


	/*glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);
	glUniform1i(glGetUniformLocation(m_program, "sampler_tex"), 0);*/



	return true;
}

// Render the scene. Passed the delta time since last called.
void Renderer::Render(const Helpers::Camera& camera, float deltaTime, glm::vec3 camarapos)
{
	CamaraPos = camarapos;
	// Configure pipeline settings
	glEnable(GL_DEPTH_TEST);

	if (m_cullFace)
		glEnable(GL_CULL_FACE);
	else
		glDisable(GL_CULL_FACE);

	// Wireframe mode controlled by ImGui
	if (m_wireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// Clear buffers from previous frame
	glClearColor(0.0f, 0.0f, 0.0f, 0.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// TODO: Compute viewport and projection matrix

	// TODO: Compute camera view matrix and combine with projection matrix for passing to shader

	// TODO: Send the combined matrix to the shader in a uniform

	// TODO: render each mesh. Send the correct model matrix to the shader in a uniform



	//std::vector<glm::vec3> translateMesh;
	////translateMesh.push_back(glm::vec3(100, 20, 100));
	//translateMesh.push_back(glm::vec3(0, 0, 0));
	//translateMesh.push_back(glm::vec3(0, 0, 0));
	//translateMesh.push_back(glm::vec3(-2.231, 0.272, -2.663));
	//translateMesh.push_back(glm::vec3(2.231, 0.272, -2.663));
	//translateMesh.push_back(glm::vec3(0, 1.395, -3.616));
	//translateMesh.push_back(glm::vec3(0, 0.569, -1.866));
	//translateMesh.push_back(glm::vec3(0, 0.569, -1.866) + glm::vec3(0, 1.506, 0.644));

	/*std::vector<glm::vec3> RotateMesh;*/




	GLint viewportSize[4];
	glGetIntegerv(GL_VIEWPORT, viewportSize);
	const float aspect_ratio = viewportSize[2] / (float)viewportSize[3];
	// Compute camera view matrix and combine with projection matrix for passing to shader
	glm::mat4 view_xform = glm::lookAt(camera.GetPosition(), camera.GetPosition() + camera.GetLookVector(), camera.GetUpVector());
	glm::mat4 projection_xform = glm::perspective(glm::radians(45.0f), aspect_ratio, 1.0f, 4000.0f);
	glm::mat4 combined_xform = projection_xform * view_xform;

	/*static float angle{ 0.5f * 3.1415920f };
	rotAngle += (2 * 3.1415920f) / 2000;
	if (rotAngle >= (2 * 3.1415920f))
	{
		rotAngle = 0;
	}*/
	models[4].applyRotation(glm::vec3(0, (2 * 3.1415920f) / 2000, 0));
	for (size_t i = 0; i < models.size(); i++)
	{
		glm::mat4 model_xform;
		//model_xform = glm::rotate(glm::mat4(1), model, glm::vec3(1, 0, 0));
		model_xform = glm::translate(glm::mat4(1), models[i].getPos());
		model_xform = glm::rotate(model_xform, models[i].getRot().r, glm::vec3(1, 0, 0));
		model_xform = glm::rotate(model_xform, models[i].getRot().g, glm::vec3(0, 1, 0));
		model_xform = glm::rotate(model_xform, models[i].getRot().b, glm::vec3(0, 0, 1));
		model_xform = glm::scale(model_xform, models[i].getScl());
		

		

		//glm::mat4 model_xform = glm::mat4(1);
		// Use our program. Doing this enables the shaders we attached previously.
		glUseProgram(models[i].getProgram());

		// Send the combined matrix to the shader in a uniform
		GLuint combined_xform_id = glGetUniformLocation(models[i].getProgram(), "combined_xform");
		glUniformMatrix4fv(combined_xform_id, 1, GL_FALSE, glm::value_ptr(combined_xform));

		// Uncomment all the lines below to rotate cube first round y then round x
		/*static float angle = 0;
		static bool rotateY = true;*/

		//if (rotateY) // Rotate around y axis		
		//	model_xform = glm::rotate(model_xform, angle, glm::vec3{ 0 ,1,0 });
		//else // Rotate around x axis		
		//	model_xform = glm::rotate(model_xform, angle, glm::vec3{ 1 ,0,0 });

		//angle += 0.001f;
		//if (angle > glm::two_pi<float>())
		//{
		//	angle = 0;
		//	rotateY = !rotateY;
		//}

		// Send the model matrix to the shader in a uniform
		GLuint model_xform_id = glGetUniformLocation(models[i].getProgram(), "model_xform");
		glUniformMatrix4fv(model_xform_id, 1, GL_FALSE, glm::value_ptr(model_xform));




		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, models[i].getTexture());
		glUniform1i(glGetUniformLocation(models[i].getProgram(), "sampler_tex"), 0);

		glBindVertexArray(models[i].getElementsAndVAOs()->m_VAO);
		glDrawElements(GL_TRIANGLES, models[i].getElementsAndVAOs()->m_numElements, GL_UNSIGNED_INT, (void*)0);
	}

}

ElementsAndVAOs Renderer::newElementsAndVAOs(GLuint VAO, GLuint Elements)
{
	ElementsAndVAOs n;
	n.m_numElements = Elements;
	n.m_VAO = VAO;
	return n;
}



