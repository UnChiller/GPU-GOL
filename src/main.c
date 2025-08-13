#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


#define ALLOW_INTERACTION // comment this to disabled mouse

const GLuint width = 16384;
const GLuint height = 16384;
GLuint scale = 1;
GLdouble posX = 0;
GLdouble posY = 0;
#define TARGET_FPS 10.0;

// get rid of error squiggles
#define CLOCK_MONOTONIC 1

double get_time() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

#ifdef ALLOW_INTERACTION
double lastX, lastY;
int dragging = 0;
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    (void)mods;
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            dragging = 1;
            glfwGetCursorPos(window, &lastX, &lastY);
        } else if (action == GLFW_RELEASE) {
            dragging = 0;
        }
    }
}
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    (void)window;
    if (dragging) {
        double dx = xpos - lastX;
        double dy = ypos - lastY;

        posX -= dx;
        posY += dy;

        lastX = xpos;
        lastY = ypos;
    }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    (void)scancode;
    (void)mods;
    if (action == GLFW_REPEAT) {
        switch (key) {
            case GLFW_KEY_KP_6:
            case GLFW_KEY_RIGHT:
                posX++;
                break;
            case GLFW_KEY_KP_4:
            case GLFW_KEY_LEFT:
                posX--;
                break;
            case GLFW_KEY_KP_2:
            case GLFW_KEY_DOWN:
                posY--;
                break;
            case GLFW_KEY_KP_8:
            case GLFW_KEY_UP:
                posY++;
                break;
        }
    }
    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose(window, GLFW_TRUE);
                break;
            case GLFW_KEY_KP_ADD:
            case GLFW_KEY_EQUAL:
                scale++;
                break;
            case GLFW_KEY_KP_SUBTRACT:
            case GLFW_KEY_MINUS:
                if (scale > 1) {
                    scale--;
                }
                break;
            case GLFW_KEY_KP_6:
            case GLFW_KEY_RIGHT:
                posX++;
                break;
            case GLFW_KEY_KP_4:
            case GLFW_KEY_LEFT:
                posX--;
                break;
            case GLFW_KEY_KP_2:
            case GLFW_KEY_DOWN:
                posY--;
                break;
            case GLFW_KEY_KP_8:
            case GLFW_KEY_UP:
                posY++;
                break;
            /*default:
                printf("pressed %i\n", key);
                break;*/
        }
    }
}

#endif
#ifdef TARGET_FPS
const double FRAME_TIME = 1.0 / TARGET_FPS;
#endif

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    (void)window;
    glViewport(0, 0, width, height);
}

char* load_file(const char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) {
        fprintf(stderr, "Failed to open shader file: %s\n", path);
        exit(1);
    }
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    char* buf = (char*)malloc(len + 1);
    if (!buf) {
        fprintf(stderr, "Out of memory reading shader: %s\n", path);
        fclose(f);
        exit(1);
    }
    len = fread(buf, 1, len, f);
    buf[len] = 0;
    fclose(f);
    return buf;
}

static GLuint compile_shader_from_file(const char* path, GLenum type) {
    char* source = load_file(path);
    if (!source) return 0;

    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, (const GLchar* const*)&source, NULL);
    free(source);
    glCompileShader(shader);

    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (!status) {
        char log[1024];
        glGetShaderInfoLog(shader, sizeof(log), NULL, log);
        fprintf(stderr, "Error compiling %s:\n%s\n", path, log);
        glDeleteShader(shader);
        exit(1);
    }
    return shader;
}

GLuint create_compute_shader(const char* path) {
    GLuint shader = compile_shader_from_file(path, GL_COMPUTE_SHADER);

    GLuint prog = glCreateProgram();
    glAttachShader(prog, shader);

    glLinkProgram(prog);

    GLint status;
    glGetProgramiv(prog, GL_LINK_STATUS, &status);
    if (!status) {
        char log[1024];
        glGetProgramInfoLog(prog, sizeof(log), NULL, log);
        fprintf(stderr, "Program link error:\n%s\n", log);
        glDeleteProgram(prog);
        exit(1);
    }

    glDeleteShader(shader);
    return prog;
}

static GLuint create_render_program() {
    GLuint vertexShader   = compile_shader_from_file("fullscreen.vert", GL_VERTEX_SHADER);
    GLuint fragmentShader = compile_shader_from_file("display.frag", GL_FRAGMENT_SHADER);
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vertexShader);
    glAttachShader(prog, fragmentShader);
    glLinkProgram(prog);

    GLint status;
    glGetProgramiv(prog, GL_LINK_STATUS, &status);
    if (!status) {
        char log[1024];
        glGetProgramInfoLog(prog, sizeof(log), NULL, log);
        fprintf(stderr, "Program link error:\n%s\n", log);
        glDeleteProgram(prog);
        exit(1);
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return prog;
}

int main() {
    printf("starting\n");
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
    #ifdef ALLOW_INTERACTION
    GLFWwindow* window = glfwCreateWindow(1920, 1080, "Life View", NULL, NULL);
    #else
    GLFWwindow* window = glfwCreateWindow(width*scale, width*scale, "Life View", NULL, NULL);
    glfwSetWindowAttrib(window, GLFW_RESIZABLE, GLFW_FALSE);
    #endif
    glfwMakeContextCurrent(window);
    glewInit();
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    #ifdef ALLOW_INTERACTION
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetKeyCallback(window, key_callback);
    #endif


    printf("compiling shaders\n");
    GLuint lifestep = create_compute_shader("life.comp");
    GLuint swap = create_compute_shader("copy.comp");
    GLuint rand_fill = create_compute_shader("rand.comp");
    GLuint render_prog = create_render_program();



    printf("allocating memory\n");
    const GLulong numValues = width*height/8; //bytes
    GLuint ssbo[2]; // Shader Storage Buffer Object, holds the buffer object ID
    glGenBuffers(2, ssbo); //create IDs
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo[0]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, numValues, NULL, GL_DYNAMIC_COPY); // Allocate memory for the buffer
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, /*layout(binding = 0)*/0, ssbo[0]); // Bind the buffer
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo[1]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, numValues, NULL, GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, /*layout(binding = 1)*/1, ssbo[1]);

    printf("running rand fill\n");
    glUseProgram(rand_fill);
    glDispatchCompute((numValues + 255) / 256, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    glDeleteProgram(rand_fill);

    printf("running\n");
    GLuint vao;
    glGenVertexArrays(1, &vao);
    while (!glfwWindowShouldClose(window)) {
        #ifdef TARGET_FPS
        double start = get_time();
        #endif
        //posX+=1;
        //posY+=1;
        glUseProgram(lifestep);
        glUniform2ui(glGetUniformLocation(lifestep, "gridSize"), width, height);
        glDispatchCompute(width/8, height/8, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        glUseProgram(swap);
        glDispatchCompute(numValues/4/64, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        // Render quad
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(render_prog);
        glUniform1ui(glGetUniformLocation(render_prog, "scale"), scale);
        glUniform2ui(glGetUniformLocation(render_prog, "position"), posX, posY);
        glUniform2ui(glGetUniformLocation(render_prog, "bufSize"), width, height);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glfwSwapBuffers(window);
        glfwPollEvents();
        // --- Framerate limiting ---
        #ifdef TARGET_FPS
        double elapsed = get_time() - start;
        while (elapsed < FRAME_TIME) {
            elapsed = get_time() - start;
            glClear(GL_COLOR_BUFFER_BIT);
            glUseProgram(render_prog);
            glUniform1ui(glGetUniformLocation(render_prog, "scale"), scale);
            glUniform2ui(glGetUniformLocation(render_prog, "position"), posX, posY);
            glBindVertexArray(vao);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            /*struct timespec ts;
            ts.tv_sec = 0;
            ts.tv_nsec = (long)((FRAME_TIME - elapsed) * 1e9);
            nanosleep(&ts, NULL);*/
            glfwSwapBuffers(window);
            glfwPollEvents();
            if (glfwWindowShouldClose(window)) break;
        }
        #endif
    }
    printf("exiting\n");

    glDeleteBuffers(2, ssbo);
    glDeleteProgram(lifestep);
    glDeleteProgram(swap);
    glDeleteProgram(render_prog);
    glfwDestroyWindow(window);
    glfwTerminate();
}
