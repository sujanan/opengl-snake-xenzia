#include <stdio.h>
#include <GL/glut.h>

unsigned int redisplay_interval = 20;

int window_width = 480;
int window_height = 640;

#define L GLUT_KEY_LEFT
#define U GLUT_KEY_UP
#define R GLUT_KEY_RIGHT
#define D GLUT_KEY_DOWN

#define IS_V(dir) (dir == U || dir == D)
#define IS_H(dir) (dir == L || dir == R)

#define CONCAT0(a, b) a ## b
#define CONCAT1(a, b) CONCAT0(a, b)

#define DIR(dir) CONCAT1(dir_, dir)

const int DIR(L)[2] = { -1,  0 };
const int DIR(U)[2] = {  0, -1 };
const int DIR(R)[2] = {  1,  0 };
const int DIR(D)[2] = {  0,  1 };

#define X 0
#define Y 1

struct SnakeNode {
    int dir;
    int p[2];
    int q[2];
    struct SnakeNode *next;
    struct SnakeNode *prev;
};

struct Snake {
    struct SnakeNode *head;
    struct SnakeNode *tail;
};

#define SNAKE_NODE(dir, x1, y1, x2, y2) { dir, { x1, y1 }, { x2, y2 }, NULL, NULL }

struct Snake viper;

unsigned int snake_node_buffer_index = -1;
struct SnakeNode snake_node_buffer[1024];

static inline struct SnakeNode *add_snake_node_buffer(struct SnakeNode *node) {
    snake_node_buffer[++snake_node_buffer_index] = *node;
    return &snake_node_buffer[snake_node_buffer_index];
}

static inline void remove_snake_node_buffer() {
    if (snake_node_buffer_index > -1) {
        --snake_node_buffer_index;
    }
}

void snake_add(struct Snake *snake, struct SnakeNode *node) {
    node = add_snake_node_buffer(node);
    snake->head->prev = node;
    node->next = snake->head;
    snake->head = node;
}

void snake_remove(struct Snake *snake) {
    snake->tail = snake->tail->prev;
    if (snake->tail) {
        snake->tail->next = NULL;
    }
    remove_snake_node_buffer();
}

static void move(struct SnakeNode *head, struct SnakeNode *tail, 
        const int *head_dir, const int *tail_dir) {
    head->p[X] += head_dir[X];
    head->p[Y] += head_dir[Y];
    tail->q[X] += tail_dir[X];
    tail->q[Y] += tail_dir[Y];
}

void snake_move(struct Snake *snake) {
    struct SnakeNode *head = snake->head;
    struct SnakeNode *tail = snake->tail;
    if (tail->p[X] == tail->q[X] && tail->p[Y] == tail->q[Y]) {
        snake_remove(snake);
    }
    const int *head_dir;
    const int *tail_dir;
    struct SnakeNode node;
    node.prev = NULL;
    node.next = NULL;
    int add = 0;
    switch (head->dir) {
    case L:
        head_dir = DIR(L);
        if (head->p[X] == 0) {
            node.dir = L;
            node.p[X] = window_width;
            node.p[Y] = head->p[Y];
            node.q[X] = window_width;
            node.q[Y] = head->p[Y];
            add = 1;
        }
        break;
    case U:
        head_dir = DIR(U);
        if (head->p[Y] == 0) {
            node.dir = U;
            node.p[X] = head->p[X];
            node.p[Y] = window_height;
            node.q[X] = head->p[X];
            node.q[Y] = window_height;
            add = 1;
        }
        break;
    case R:
        head_dir = DIR(R);
        if (head->p[X] == window_width) {
            node.dir = R;
            node.p[X] = 0;
            node.p[Y] = head->p[Y];
            node.q[X] = 0;
            node.q[Y] = head->p[Y];
            add = 1;
        }
        break;
    case D:
        head_dir = DIR(D);
        if (head->p[Y] == window_height) {
            node.dir = D;
            node.p[X] = head->p[X];
            node.p[Y] = 0;
            node.q[X] = head->p[X];
            node.q[Y] = 0;
            add = 1;
        }
        break;
    }
    int remove = 0;
    switch (tail->dir) {
    case L:
        tail_dir = DIR(L);
        remove = tail->q[X] == 0;
        break;
    case U:
        tail_dir = DIR(U);
        remove = tail->q[Y] == 0;
        break;
    case R:
        tail_dir = DIR(R);
        remove = tail->q[X] == window_width;
        break;
    case D:
        tail_dir = DIR(D);
        remove = tail->q[Y] == window_height;
        break;
    }
    move(head, tail, head_dir, tail_dir);
    if (add) {
        snake_add(snake, &node);
    }
    if (remove) {
        snake_remove(snake);
    }
}

void reshape(int width, int height) {
    glOrtho(0.0, width, height, 0.0, -1, 1);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glBegin(GL_LINES);
    snake_move(&viper);
    struct SnakeNode *node = viper.head;
    while (node) {
        glVertex2i(node->p[X], node->p[Y]);
        glVertex2i(node->q[X], node->q[Y]);
        node = node->next;
    }
    glEnd();
    glutSwapBuffers();
}

void timer(int extra) {
    glutPostRedisplay();
    glutTimerFunc(redisplay_interval, timer, 0);
}

void special(int key, int x, int y) {
    struct SnakeNode *head = viper.head;
    int b = 0;
    switch (key) {
    case L:
    case R:
        b = IS_V(head->dir);
        break;
    case U:
    case D:
        b = IS_H(head->dir);
        break;
    }
    if (b) {
        struct SnakeNode node = SNAKE_NODE(key, head->p[X], head->p[Y], head->p[X], head->p[Y]);
        snake_add(&viper, &node);
    }
}

int main(int argc, char **argv) {
    struct SnakeNode node = SNAKE_NODE(D, 10, 100, 10, 10);
    viper.head = &node;
    viper.tail = &node;
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(window_width, window_height);
    glutCreateWindow("Snake");
    glutReshapeFunc(reshape);
    glutDisplayFunc(display);
    glutTimerFunc(redisplay_interval, timer, 0);
    glutSpecialFunc(special);
    glutMainLoop();
    return 0;
}