#include <GLFW/glfw3.h>

#include <array>
#include <cstdlib>
#include <iostream>
#include <cmath>

const float PI = 3.1415926535f;
const float PHI = 1.6180339887f;
const float INV_PHI = 1.0f / PHI;

struct Vec3 {
    float x;
    float y;
    float z;
};

const std::array<Vec3, 20> DODECAHEDRON_VERTICES = { {
    {-1.0f, -1.0f, -1.0f},
    {-1.0f, -1.0f,  1.0f},
    {-1.0f,  1.0f, -1.0f},
    {-1.0f,  1.0f,  1.0f},
    { 1.0f, -1.0f, -1.0f},
    { 1.0f, -1.0f,  1.0f},
    { 1.0f,  1.0f, -1.0f},
    { 1.0f,  1.0f,  1.0f},

    {0.0f, -INV_PHI, -PHI},
    {0.0f, -INV_PHI,  PHI},
    {0.0f,  INV_PHI, -PHI},
    {0.0f,  INV_PHI,  PHI},

    {-INV_PHI, -PHI, 0.0f},
    {-INV_PHI,  PHI, 0.0f},
    { INV_PHI, -PHI, 0.0f},
    { INV_PHI,  PHI, 0.0f},

    {-PHI, 0.0f, -INV_PHI},
    {-PHI, 0.0f,  INV_PHI},
    { PHI, 0.0f, -INV_PHI},
    { PHI, 0.0f,  INV_PHI},
} };

const std::array<std::array<int, 5>, 12> DODECAHEDRON_FACES = { {
    {{17, 16, 0, 12, 1}},
    {{10, 8, 0, 16, 2}},
    {{14, 12, 0, 8, 4}},
    {{3, 17, 1, 9, 11}},
    {{5, 9, 1, 12, 14}},
    {{3, 13, 2, 16, 17}},
    {{6, 10, 2, 13, 15}},
    {{15, 13, 3, 11, 7}},
    {{5, 14, 4, 18, 19}},
    {{6, 18, 4, 8, 10}},
    {{11, 9, 5, 19, 7}},
    {{19, 18, 6, 15, 7}},
} };

void setPerspective(float fovDegrees, float aspect, float nearPlane, float farPlane) {
    float top = tanf(fovDegrees * PI / 360.0f) * nearPlane;
    float right = top * aspect;

    glFrustum(-right, right, -top, top, nearPlane, farPlane);
}

Vec3 subtract(Vec3 a, Vec3 b) {
    return {
        a.x - b.x,
        a.y - b.y,
        a.z - b.z
    };
}

Vec3 cross(Vec3 a, Vec3 b) {
    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

Vec3 normalize(Vec3 v) {
    float length = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);

    if (length == 0.0f) {
        return { 0.0f, 1.0f, 0.0f };
    }

    return {
        v.x / length,
        v.y / length,
        v.z / length
    };
}

void drawDodecahedron() {
    for (const auto& face : DODECAHEDRON_FACES) {
        Vec3 a = DODECAHEDRON_VERTICES[face[0]];
        Vec3 b = DODECAHEDRON_VERTICES[face[1]];
        Vec3 c = DODECAHEDRON_VERTICES[face[2]];

        Vec3 edge1 = subtract(b, a);
        Vec3 edge2 = subtract(c, a);

        Vec3 normal = normalize(cross(edge1, edge2));

        glBegin(GL_POLYGON);

        glNormal3f(normal.x, normal.y, normal.z);

        glColor3f(0.50f, 0.18f, 0.85f);

        for (int index : face) {
            Vec3 v = DODECAHEDRON_VERTICES[index];
            glVertex3f(v.x, v.y, v.z);
        }

        glEnd();
    }
}

void setupLighting() {
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    glShadeModel(GL_SMOOTH);

    GLfloat lightPosition[] = { -3.0f, 4.0f, 5.0f, 1.0f };
    GLfloat lightAmbient[] = { 0.18f, 0.16f, 0.22f, 1.0f };
    GLfloat lightDiffuse[] = { 0.85f, 0.80f, 1.00f, 1.0f };
    GLfloat lightSpecular[] = { 1.00f, 0.95f, 1.00f, 1.0f };

    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);

    GLfloat materialSpecular[] = { 0.85f, 0.65f, 1.00f, 1.0f };
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);
    glMaterialf(GL_FRONT, GL_SHININESS, 48.0f);
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW.\n";
        return EXIT_FAILURE;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    GLFWwindow* window = glfwCreateWindow(
        1280,
        720,
        "OpenGL Dodecahedron",
        nullptr,
        nullptr
    );

    if (!window) {
        std::cerr << "Failed to create GLFW window.\n";
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    setupLighting();

    while (!glfwWindowShouldClose(window)) {
        int width;
        int height;
        glfwGetFramebufferSize(window, &width, &height);

        if (height == 0) {
            height = 1;
        }

        float aspect = static_cast<float>(width) / static_cast<float>(height);

        glViewport(0, 0, width, height);

        glClearColor(0.05f, 0.06f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        setPerspective(45.0f, aspect, 0.1f, 100.0f);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glTranslatef(0.0f, 0.0f, -5.0f);

        float time = static_cast<float>(glfwGetTime());
        glRotatef(time * 40.0f, 0.0f, 1.0f, 0.0f);

        drawDodecahedron();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS;
}