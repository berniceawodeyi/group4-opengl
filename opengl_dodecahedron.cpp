#include <GLFW/glfw3.h>

#include <array>
#include <cstdlib>
#include <iostream>
#include <cmath>

bool isPaused = false;
float animationTime = 0.0f;
const float PI = 3.1415926535f;
const float PHI = 1.6180339887f;
const float INV_PHI = 1.0f / PHI;
const float FLOOR_Y = -1.6f;
const float START_SCALE = 0.85f;
const float GROWN_SCALE = 1.35f;
const float REST_CENTER_Y = FLOOR_Y + PHI * START_SCALE;
const float PEAK_CENTER_Y = REST_CENTER_Y + 2.4f;

const float GRAVITY = 9.8f;

const float REST_SECONDS = 1.0f;
const float LEVITATE_SECONDS = 3.0f;
const float ORBIT_SECONDS = 4.0f;
const float GROW_SECONDS = 2.0f;
const float FALL_SECONDS = 0.57f;
const float BOUNCE_SECONDS = 0.32f;

const float LOOP_SECONDS =
REST_SECONDS +
LEVITATE_SECONDS +
ORBIT_SECONDS +
GROW_SECONDS +
FALL_SECONDS +
BOUNCE_SECONDS;

struct Vec3 {
    float x;
    float y;
    float z;
};

struct AnimationPose {
    float y;
    float scale;
    float rotationX;
    float rotationY;
    float rotationZ;
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

float clamp01(float value) {
    if (value < 0.0f) {
        return 0.0f;
    }

    if (value > 1.0f) {
        return 1.0f;
    }

    return value;
}

float easeInOutCubic(float t) {
    t = clamp01(t);

    if (t < 0.5f) {
        return 4.0f * t * t * t;
    }

    float f = -2.0f * t + 2.0f;
    return 1.0f - (f * f * f) / 2.0f;
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

        glColor4f(0.50f, 0.16f, 0.90f, 0.55f);

        for (int index : face) {
            Vec3 v = DODECAHEDRON_VERTICES[index];
            glVertex3f(v.x, v.y, v.z);
        }

        glEnd();
    }
}

void drawDodecahedronOutline() {
    glDisable(GL_LIGHTING);

    glLineWidth(3.0f);
    glColor3f(0.72f, 0.22f, 1.0f);

    for (const auto& face : DODECAHEDRON_FACES) {
        glBegin(GL_LINE_LOOP);

        for (int index : face) {
            Vec3 v = DODECAHEDRON_VERTICES[index];
            glVertex3f(v.x, v.y, v.z);
        }

        glEnd();
    }

    glLineWidth(1.0f);
    glEnable(GL_LIGHTING);
}

void setupLighting() {
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDepthMask(GL_TRUE);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    glShadeModel(GL_SMOOTH);

    GLfloat lightPosition[] = { -3.0f, 4.0f, 5.0f, 1.0f };
    GLfloat lightAmbient[] = { 0.18f, 0.16f, 0.22f, 1.0f };
    GLfloat lightDiffuse[] = { 0.85f, 0.80f, 1.00f, 1.0f };
    GLfloat lightSpecular[] = { 0.35f, 0.25f, 0.45f, 1.0f };

    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);

    GLfloat materialSpecular[] = { 0.25f, 0.18f, 0.35f, 1.0f };
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);
    glMaterialf(GL_FRONT, GL_SHININESS, 24.0f);
}

void drawFloor(float objectScale) {
    // Temporarily disable lighting so the floor color stays simple and readable.
    glDisable(GL_LIGHTING);

    glColor3f(0.14f, 0.14f, 0.17f);

    glBegin(GL_QUADS);
    glVertex3f(-6.0f, FLOOR_Y, -4.0f);
    glVertex3f(6.0f, FLOOR_Y, -4.0f);
    glVertex3f(6.0f, FLOOR_Y, 4.0f);
    glVertex3f(-6.0f, FLOOR_Y, 4.0f);
    glEnd();

    // A soft oval under the object, like a contact shadow.
    glColor3f(0.04f, 0.035f, 0.05f);

    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0.0f, FLOOR_Y + 0.01f, 0.0f);

    for (int i = 0; i <= 48; i++) {
        float angle = static_cast<float>(i) / 48.0f * 2.0f * PI;
        float shadowScale = objectScale / START_SCALE;

        float x = cosf(angle) * 1.35f * shadowScale;
        float z = sinf(angle) * 0.55f * shadowScale;

        glVertex3f(x, FLOOR_Y + 0.01f, z);
    }

    glEnd();

    glEnable(GL_LIGHTING);
}

AnimationPose getPoseForTime(float totalTime) {
    float t = fmodf(totalTime, LOOP_SECONDS);

    AnimationPose pose;
    pose.y = REST_CENTER_Y;
    pose.scale = START_SCALE;
    pose.rotationX = -16.0f;
    pose.rotationY = 22.0f;
    pose.rotationZ = 0.0f;

    if (t < REST_SECONDS) {
        pose.y = REST_CENTER_Y;
        pose.scale = START_SCALE;
    }
    else if (t < REST_SECONDS + LEVITATE_SECONDS) {
        float localTime = t - REST_SECONDS;
        float progress = localTime / LEVITATE_SECONDS;
        float eased = easeInOutCubic(progress);

        pose.y = REST_CENTER_Y + (PEAK_CENTER_Y - REST_CENTER_Y) * eased;
        pose.scale = START_SCALE;
    }
    else if (t < REST_SECONDS + LEVITATE_SECONDS + ORBIT_SECONDS) {
        float localTime = t - REST_SECONDS - LEVITATE_SECONDS;

        pose.y = PEAK_CENTER_Y;
        pose.scale = START_SCALE;

        pose.rotationX += localTime * 24.0f;
        pose.rotationY += localTime * 32.0f;
        pose.rotationZ += sinf(localTime * 0.75f) * 5.0f;
    }
    else if (t < REST_SECONDS + LEVITATE_SECONDS + ORBIT_SECONDS + GROW_SECONDS) {
        float localTime = t - REST_SECONDS - LEVITATE_SECONDS - ORBIT_SECONDS;
        float progress = localTime / GROW_SECONDS;
        float eased = easeInOutCubic(progress);

        pose.y = PEAK_CENTER_Y;
        pose.scale = START_SCALE + (GROWN_SCALE - START_SCALE) * eased;

        pose.rotationX += ORBIT_SECONDS * 24.0f + localTime * 18.0f;
        pose.rotationY += ORBIT_SECONDS * 32.0f + localTime * 26.0f;
    }
    else {
        float localTime = t - REST_SECONDS - LEVITATE_SECONDS - ORBIT_SECONDS - GROW_SECONDS;
        float grownRestY = FLOOR_Y + PHI * GROWN_SCALE;

        pose.scale = GROWN_SCALE;

        if (localTime < FALL_SECONDS) {
            pose.y = PEAK_CENTER_Y - 0.5f * GRAVITY * localTime * localTime;

            if (pose.y < grownRestY) {
                pose.y = grownRestY;
            }
        }
        else {
            float bounceT = (localTime - FALL_SECONDS) / BOUNCE_SECONDS;
            bounceT = clamp01(bounceT);

            float bounce = sinf(bounceT * PI) * (1.0f - bounceT) * 0.14f;
            pose.y = grownRestY + bounce;
        }

        pose.rotationX += ORBIT_SECONDS * 24.0f + GROW_SECONDS * 18.0f + localTime * 10.0f;
        pose.rotationY += ORBIT_SECONDS * 32.0f + GROW_SECONDS * 26.0f + localTime * 14.0f;
    }

    return pose;
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action != GLFW_PRESS) {
        return;
    }

    if (key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    if (key == GLFW_KEY_SPACE) {
        isPaused = !isPaused;
    }
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

    glfwSetKeyCallback(window, keyCallback);

    setupLighting();

    double previousTime = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        double currentTime = glfwGetTime();
        float deltaTime = static_cast<float>(currentTime - previousTime);
        previousTime = currentTime;

        if (!isPaused) {
            animationTime += deltaTime;
        }
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

        glTranslatef(0.0f, -1.0f, -10.5f);

        AnimationPose pose = getPoseForTime(animationTime);

        drawFloor(pose.scale);

        glPushMatrix();

        glTranslatef(0.0f, pose.y, 0.0f);

        glRotatef(pose.rotationX, 1.0f, 0.0f, 0.0f);
        glRotatef(pose.rotationY, 0.0f, 1.0f, 0.0f);
        glRotatef(pose.rotationZ, 0.0f, 0.0f, 1.0f);

        glScalef(pose.scale, pose.scale, pose.scale);

        drawDodecahedron();
        drawDodecahedronOutline();

        glPopMatrix();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS;
}