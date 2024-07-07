#include "Sprite.h"
#include <glm/gtx/string_cast.hpp>


Sprite::~Sprite()
{
	// Pede pra OpenGL desalocar os buffers
	glDeleteVertexArrays(1, &VAO);
}

void Sprite::inicializar(GLuint texID, int nAnimations, int nFrames, glm::vec3 pos, glm::vec3 escala, float angulo)
{
	this->texID = texID;
	this->pos = pos;
	this->escala = escala;
	this->angulo = angulo;
	this->nFrames = nFrames;
	this->nAnimations = nAnimations;
	this->iFrame = 0;
	this->iAnimation = 0;
	this->pastTime;
	this->horizontalScale.x = 1.0 / (float)nFrames;
	this->verticalScale.y = 1.0 / (float)nAnimations;
	this->frameDuration = 1.0f / 12.0f; // 12 FPS
	horizontalOffsetTex.x = 1.0 / (float)nFrames;
	verticalOffsetTex.y = 1.0 / (float)nAnimations;

	// Aqui setamos as coordenadas x, y e z do tri�ngulo e as armazenamos de forma
	// sequencial, j� visando mandar para o VBO (Vertex Buffer Objects)
	// Cada atributo do v�rtice (coordenada, cores, coordenadas de textura, normal, etc)
	// Pode ser arazenado em um VBO �nico ou em VBOs separados
	GLfloat vertices[] = {
		//x     y    z    r    g    b    s    t
		//Triangulo 0
		-0.5, 0.5,  0.0, 1.0, 0.0, 0.0, 0.0, verticalOffsetTex.y,			// v0
		-0.5, -0.5, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0,					// v1
		0.5,  0.5,  0.0, 1.0, 0.0, 0.0, horizontalOffsetTex.x, verticalOffsetTex.y,   // v2
		 //Triangulo 1	
		 -0.5, -0.5, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0,					// v1
		0.5,  -0.5, 0.0, 1.0, 0.0, 0.0, horizontalOffsetTex.x, 0.0,			// v3
		0.5,  0.5,  0.0, 1.0, 0.0, 0.0, horizontalOffsetTex.x, verticalOffsetTex.y	// v2
		
	};

	GLuint VBO;
	//Gera��o do identificador do VBO
	glGenBuffers(1, &VBO);
	//Faz a conex�o (vincula) do buffer como um buffer de array
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	//Envia os dados do array de floats para o buffer da OpenGl
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	//                                              vertices.data()  
	//Gera��o do identificador do VAO (Vertex Array Object)
	glGenVertexArrays(1, &VAO);
	// Vincula (bind) o VAO primeiro, e em seguida  conecta e seta o(s) buffer(s) de v�rtices
	// e os ponteiros para os atributos 
	glBindVertexArray(VAO);
	//Para cada atributo do vertice, criamos um "AttribPointer" (ponteiro para o atributo), indicando: 
	// Localiza��o no shader * (a localiza��o dos atributos devem ser correspondentes no layout especificado no vertex shader)
	// Numero de valores que o atributo tem (por ex, 3 coordenadas xyz) 
	// Tipo do dado
	// Se est� normalizado (entre zero e um)
	// Tamanho em bytes 
	// Deslocamento a partir do byte zero 

	//Atributo 0 - posi��o
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	//Atributo 1 - cor
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	//Atributo 2 - coordenadas de textura
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	// Observe que isso � permitido, a chamada para glVertexAttribPointer registrou o VBO como o objeto de buffer de v�rtice 
	// atualmente vinculado - para que depois possamos desvincular com seguran�a
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Desvincula o VAO (� uma boa pr�tica desvincular qualquer buffer ou array para evitar bugs medonhos)
	glBindVertexArray(0);

}

void Sprite::desenhar()
{
	atualizar();

	glBindTexture(GL_TEXTURE_2D, texID);
	glBindVertexArray(VAO); //Conectando ao buffer de geometria
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindTexture(GL_TEXTURE_2D, 0); //unbind
	glBindVertexArray(0); //unbind
}


void Sprite::moverParaDireita()
{
	if (escala.x < 0.0)
		escala.x = -escala.x;

}

void Sprite::moverParaEsquerda()
{
	if (escala.x > 0.0)
		escala.x = -escala.x;
}

void Sprite::atualizar()
{
	float now = (float) glfwGetTime();

	float dt = now - pastTime;

	if (dt >= 1 / FPS) {
		int prevFrame = iFrame;
		iFrame = (iFrame + 1) % nFrames;

		// Alterna entre linhas do spritesheet (se houver, como na Flame)
		if (prevFrame > iFrame) {
			iAnimation = (iAnimation + 1) % nAnimations;
		}

		pastTime = now;
	}

	// Calcula deslocamentos nas coordenadas da textura
	float offsetTexFrameS = iFrame * horizontalOffsetTex.x;
	float offsetTexFrameT = iAnimation * verticalOffsetTex.y;
	shader->setVec2("offsetTex", offsetTexFrameS, offsetTexFrameT);

	glm::mat4 model = glm::mat4(1);
	model = glm::translate(model, pos);

	if (this->angulo == 180.0f) {
		model = glm::rotate(model, glm::radians(angulo), glm::vec3(0.0, 1.0, 0.0));
	}
	else {
		model = glm::rotate(model, glm::radians(angulo), glm::vec3(0.0, 0.0, 1.0));
	}

	model = glm::scale(model, escala);
	shader->setMat4("model", glm::value_ptr(model));

}