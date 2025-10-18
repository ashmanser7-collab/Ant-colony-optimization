#include <GLFW/glfw3.h>
#include <cmath>
#include <vector>
#include <iostream>
#include <algorithm>

const double PI = 3.14159265358979323846;
long frames = 0;
int width = 1900;
int height = 1050;
float size = 10;
bool esc_was_pressed = false;

float distance_power = 5;
float pheromone_power = 1;
float pheromone_degedation = 0.02;
float intial_pheromone_strength = 1;
int pheromone_intensity = 5;
int num_ants = 100;
float elitism_strength = 0.1;

struct Node {
    int x;
    int y;
    Node(int nx, int ny) : x(nx), y(ny) {}
};

std::vector<Node> nodes;

std::vector<std::vector<float>> distances;
std::vector<std::vector<float>> pheromone_trails;

class Ant {
public:
    int starting_index;
    int location_index;
    float total_length = 0;
    std::vector<int> travelled;
    std::vector<int> destinations;
    bool journey_complete = false;

    Ant(int start, const std::vector<int>& node_indices)
        : starting_index(start), location_index(start), destinations(node_indices)
    {
        travelled.push_back(location_index);
        destinations.erase(std::remove(destinations.begin(), destinations.end(), location_index), destinations.end());
    }

    void step() {
        if (journey_complete || destinations.empty()) return;

        std::vector<float> nearby_distances = distances[location_index];
        std::vector<float> destination_weights;
        float total_weights = 0.0f;

        for (int dest : destinations) {
            float d = nearby_distances[dest];
            float p = pheromone_trails[location_index][dest];

            if (d <= 0.0f || p <= 0.0f) {
                destination_weights.push_back(0.0f);
                continue;
            }

            float weight = pow(1.0f / d, distance_power) * pow(p, pheromone_power);
            destination_weights.push_back(weight);
            total_weights += weight;
        }

        int index = 0;

        if (total_weights <= 0.0f) {
            index = rand() % destinations.size();
        } else {
            int rand_value = 1 + (rand() % 10000);
            int tally = 0;
            for (size_t i = 0; i < destination_weights.size(); i++) {
                int prob = static_cast<int>(destination_weights[i] / total_weights * 10000);
                tally += prob;
                if (rand_value <= tally) {
                    index = i;
                    break;
                }
            }
            if (index >= destinations.size()) index = destinations.size() - 1;
        }

        int new_location_index = destinations[index];
        total_length += nearby_distances[new_location_index];
        travelled.push_back(new_location_index);

        pheromone_trails[location_index][new_location_index] += 1.0f;
        pheromone_trails[new_location_index][location_index] += 1.0f;

        location_index = new_location_index;
        destinations.erase(destinations.begin() + index);

        if (destinations.empty()) {
            total_length += distances[location_index][starting_index];
            travelled.push_back(starting_index);
            pheromone_trails[location_index][starting_index] += 1.0f;
            pheromone_trails[starting_index][location_index] += 1.0f;
            journey_complete = true;
        }
    }
};


std::vector<Ant> ants;

void mouse_button_callback_preparing(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double xpos;
        double ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        ypos = height - ypos;
        nodes.push_back(Node(int(xpos), int(ypos)));
        std::cout << "Node added at: (" << xpos << ", " << ypos << ")" << std::endl;
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        double xpos;
        double ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        ypos = height - ypos;
        for (int i = 0; i < nodes.size(); i++) {
            float dist = hypot(nodes[i].x-xpos, nodes[i].y-ypos);
            if (dist < size) {
                nodes.erase(nodes.begin() + i);
            }
        }
    }
}

int clicked_node;
bool node1 = true;

void mouse_button_callback_running(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double xpos;
        double ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        ypos = height - ypos;
        for (int i = 0; i < nodes.size(); i++) {
            float dist = hypot(nodes[i].x-xpos, nodes[i].y-ypos);
            if (dist < size) {
                if (node1 || clicked_node == i) {
                    clicked_node = i;
                    node1 = false;
                } else {
                    node1 = true;
                    std::cout << pheromone_trails[i][clicked_node] << " pheromone strength" << std::endl;
                    std::cout << distances[i][clicked_node] << " distance" << std::endl;
                }
            }
        }
    }
}

GLFWwindow* initiate_window() {
    GLFWwindow* window = glfwCreateWindow(width, height, "Ant Colony Optimization", NULL, NULL);
    return window;
}

void drawCircle(float cx, float cy, float r, int num_segments) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= num_segments; i++) {
        float theta = 2.0f * PI * float(i) / float(num_segments);
        float x = r * std::cos(theta);
        float y = r * std::sin(theta);
        glVertex2f(x + cx, y + cy);
    }
    glEnd();
}

void calculateDistances() {
    float dx;
    float dy;
    std::vector<std::vector<float>> new_distances;
    std::vector<float> new_new_distances;
    float dist;
    for (int i = 0; i < nodes.size(); i++) {
        new_new_distances.clear();
        for (int j = 0; j < nodes.size(); j++) {
            dx = nodes[i].x - nodes[j].x;
            dy = nodes[i].y - nodes[j].y;
            dist = hypot(dx, dy);
            new_new_distances.push_back(dist);
        }
        new_distances.push_back(new_new_distances);
    }
    distances = new_distances;
}

float pheromone_total;

void initializePheromones() {
    pheromone_trails.clear();
    std::vector<float> layer;
    for (int i = 0; i < nodes.size(); i++) {
        layer.push_back(intial_pheromone_strength);
    }
    for (int i = 0; i < nodes.size(); i++) {
        pheromone_trails.push_back(layer);
    }
}

void prepare(GLFWwindow* window) {
    nodes.clear();
    pheromone_trails.clear();
    ants.clear();
    bool preparing = true;
    glfwSetMouseButtonCallback(window, mouse_button_callback_preparing);
    while (preparing && !glfwWindowShouldClose(window)) {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {if (!esc_was_pressed) {preparing = false; esc_was_pressed = true;}} else {esc_was_pressed = false;}

        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            int num;
            std::cout << "Number of nodes: ";
            std::cin >> num;
            int x;
            int y;
            for (int i = 0; i < num; i++) {
                bool good = false;
                while (!good) {
                    x = (rand() % int(width-size*4)) + size*2;
                    y = (rand() % int(height-size*4)) + size*2;
                    good = true;
                    for (int j = 0; j < nodes.size(); j++) {
                        float dist = hypot(nodes[j].x-x, nodes[j].y-y);
                        if (dist < size*3) {
                            good = false;
                            break;
                        }
                    }
                }
                nodes.push_back(Node(x, y));
            }
        }

        for (int i = 0; i < nodes.size(); i++) {
            drawCircle(nodes[i].x, nodes[i].y, size, 10);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwSetMouseButtonCallback(window, mouse_button_callback_running);
    calculateDistances();
    initializePheromones();
}

float a = (1.0f - pheromone_degedation);
void drawPheremones() {
    float maximum;
    
    glBegin(GL_LINES);

    pheromone_total = 0;

    for (int i = 0; i < pheromone_trails.size()-1; i++) {
        maximum = *std::max_element(pheromone_trails[i].begin(), pheromone_trails[i].end());
        if (maximum == 0) {maximum = 1;}
        for (int j = i+1; j < pheromone_trails.size(); j++) {
            pheromone_trails[i][j] *= a;
            pheromone_trails[j][i] *= a;
            float alpha = pheromone_trails[i][j]/maximum;
            if (alpha < 0.1) {continue;}
            glColor4f(0.8f, 0.8f, 0.8f, alpha);
            glVertex2f(nodes[i].x, nodes[i].y);
            glVertex2f(nodes[j].x, nodes[j].y);
            pheromone_total += pheromone_trails[i][j];
        }
    }
    glEnd();
}

void drawPath(std::vector<int> path) {
    glColor4f(0.8f, 0.8f, 0.8f, 1.0f);
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i < path.size(); i++) {
        glVertex2d(nodes[path[i]].x, nodes[path[i]].y);
    }
    glEnd();
}

int main() {

    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    GLFWwindow* window = initiate_window();

    glfwMakeContextCurrent(window);

    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, 0, height, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    prepare(window);

    float shortest_path_length = 0;
    std::vector<int> shortest_path;
    bool draw_shortest_path = false;
    bool p_was_pressed = false;

    std::vector<int> dummy_indicies;

    for (int i = 0; i < nodes.size(); i++) {
        dummy_indicies.push_back(i);
        if (nodes.size() == i+1) {
            shortest_path_length += distances[i][0];
        } else {
            shortest_path_length += distances[i][i+1];
        }
    }
    shortest_path = dummy_indicies;

    for (int i = 0; i < num_ants; i++) {ants.push_back(Ant(int(rand()%nodes.size()), dummy_indicies));}

    int ants_processesed = 0;

    bool q_was_pressed = false;

    while (!glfwWindowShouldClose(window))
    {
        frames += 1;
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        for (size_t i = 0; i < ants.size();) {
            ants[i].step();
            if (ants[i].journey_complete) {
                if (ants[i].total_length < shortest_path_length) {
                    shortest_path_length = ants[i].total_length;
                    shortest_path = ants[i].travelled;
                }
                ants.erase(ants.begin() + i);
                ants.push_back(Ant(int(rand() % nodes.size()), dummy_indicies));
                ants_processesed += 1;
            } else {
                ++i;
            }
        }
        if (draw_shortest_path) {
            drawPath(shortest_path);
        } else {
            drawPheremones();
        }
        glColor4f(0.8f, 0.8f, 0.8f, 1.0f);

        for (int i = 0; i < nodes.size(); i++) {drawCircle(nodes[i].x, nodes[i].y, size, 10);}

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            if (!esc_was_pressed) {
                esc_was_pressed = true;
                prepare(window);
                shortest_path_length = 0;
                shortest_path.clear();
                draw_shortest_path = false;
                p_was_pressed = false;

                dummy_indicies.clear();

                for (int i = 0; i < nodes.size(); i++) {
                    dummy_indicies.push_back(i);
                    if (nodes.size() == i+1) {
                        shortest_path_length += distances[i][0];
                    } else {
                        shortest_path_length += distances[i][i+1];
                    }
                }
                shortest_path = dummy_indicies;
                ants_processesed = 0;

                for (int i = 0; i < num_ants; i++) {ants.push_back(Ant(int(rand()%nodes.size()), dummy_indicies));}
            }
        } else {esc_was_pressed = false;}
        
        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
            if (!p_was_pressed) {
                draw_shortest_path = !draw_shortest_path;
                p_was_pressed = true;
            }
        } else {
            p_was_pressed = false;
        }
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
            if (!q_was_pressed) {
                std::cout << nodes.size() << " nodes" << std::endl;
                std::cout << shortest_path_length << " Shortest path" << std::endl;
                std::cout << ants_processesed << " Ants processed" << std::endl;
                std::cout << frames << " frames" << std::endl;
                std::cout << pheromone_total << " pheromone count" << std::endl << std::endl;
                std::cout << pheromone_degedation << " pheromone degredation" << std::endl;
                std::cout << pheromone_intensity << " pheromone intensity" << std::endl;
                std::cout << pheromone_power << " pheromone power" << std::endl;
                std::cout << distance_power << " distance power" << std::endl;
                std::cout << num_ants << " number of ants" << std::endl;
                std::cout << elitism_strength << " elitism strength" << std::endl;
                std::string action;
                std::cin >> action;
                if (action == "1") {
                    std::cin >> pheromone_degedation;
                }
                if (action == "2") {
                    std::cin >> pheromone_intensity;
                }
                if (action == "3") {
                    std::cin >> pheromone_power;
                }
                if (action == "4") {
                    std::cin >> distance_power;
                }
                if (action == "5") {
                    std::cin >> num_ants;
                }
                if (action == "6") {
                    std::cin >> elitism_strength;
                }
                q_was_pressed = true;
            }
        } else {
            q_was_pressed = false;
        }

        for (int i = 0; i < shortest_path.size()-1; i++) {
            pheromone_trails[shortest_path[i]][shortest_path[i+1]] += elitism_strength;
            pheromone_trails[shortest_path[i+1]][shortest_path[i]] += elitism_strength;
        }
        pheromone_trails[shortest_path[0]][shortest_path[shortest_path.size()-1]] += elitism_strength;
        pheromone_trails[shortest_path[shortest_path.size()-1]][shortest_path[0]] += elitism_strength;

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}