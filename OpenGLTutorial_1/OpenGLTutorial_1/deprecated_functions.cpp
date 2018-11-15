/*
readFile for shaders txt
*/
char* readFile(const char* fileName) {
int filesize;
FILE *plik;
char* result;

plik = fopen(fileName, "rb");
fseek(plik, 0, SEEK_END);
filesize = ftell(plik);
fseek(plik, 0, SEEK_SET);
result = new char[filesize + 1];
fread(result, 1, filesize, plik);
result[filesize] = 0;
fclose(plik);

return result;
}


/*
VERTEX SHADER SOURCE CODE
FRAGMENT SHADER SOURCE CODE
*/
const char *vertexShaderSource = readFile("vertexshader.vs");
const char *fragmentShaderSource = readFile("fragmentshader.fs");


/*
VERTEX SHADER
vertex_creating:	initialize / link to *vertexShaderSource code / build and compile
compiling:			check if processed correctly / if not - print error
*/
int vertexShader = glCreateShader(GL_VERTEX_SHADER);
glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
glCompileShader(vertexShader);

int success;
char infoLog[512];
glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
if (!success) 	{
glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
}

/*
FRAGMENT SHADER
fragment_creating:	initialize / link to *fragmentShaderSource code / build and compile
compiling:			check if processed correctly / if not - print error
*/
unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
glCompileShader(fragmentShader);
glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
if (!success)
{
glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
}

/*
SHADER PROGRAM
linking:	create shaderProgram / link verex / link fragment / link all together
errors:		check for errors
deleting:	delete vertex / delete fragment
*/
int shaderProgram = glCreateProgram();
glAttachShader(shaderProgram, vertexShader);
glAttachShader(shaderProgram, fragmentShader);
glLinkProgram(shaderProgram);
glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
if (!success) {
glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
}
glDeleteShader(vertexShader);
glDeleteShader(fragmentShader);


/* 
update the uniform color
*/
float timeValue = glfwGetTime();
float greenValue = sin(timeValue) / 2.0f + 0.5f;
int vertexColorLocation = glGetUniformLocation(shaderProgram, "ourColor");
glUniform4f(vertexColorLocation, 0.5f, greenValue, greenValue/2, 1.0f);
