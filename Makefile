all:
	g++ prog4.cpp GLSL_helper.cpp mesh.cpp MStackHelp.cpp GeometryCreator.cpp -o prog4 -DGL_GLEXT_PROTOTYPES -framework OpenGL -framework GLUT