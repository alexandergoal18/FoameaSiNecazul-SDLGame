#include <iostream>
#include <vector>
#include <SDL3/SDL.h>
#include "rectangle.h"
#include "circle.h"

//define window dimensions
constexpr int WINDOW_WIDTH{700};
constexpr int WINDOW_HEIGHT{800};

// Vertical scrolling grid related variables
constexpr float VERTICAL_SCROLLING_SPEED{ 100 };
constexpr int VERTICAL_SCROLLING_GRID_SQUARE_SIZE{ 20 };
std::vector<Rectangle> scrollingGrid;

// Animation related variables
float DT{ 0.01f };
Uint64 startTime{ 0 };
Uint64 endTime{ 0 };

Rectangle player, e1, e2, e3;
std::vector<Circle> playerCircles;
std::vector<Circle> enemyCircles;
Uint64 lastShotTime = 0;
Uint64 shotInterval = 300;
Uint64 lastSpawnTime1 = 0;
Uint64 spawnInterval1 = 3000;
Uint64 lastSpawnTime2 = 0;
Uint64 spawnInterval2 = 3000;
Uint64 lastSpawnTime3 = 0;
Uint64 spawnInterval3 = 3000;
int port = WINDOW_WIDTH / 3;
void initRectangle() {
    player.pos = glm::vec2(335.0f, 775.0f);
    player.width = 50.0f;
    player.height = 50.0f;
    player.color = SDL_Color{ 0, 255, 0, 255 };
    player.dir = glm::vec2(0.0f, 0.0f);
    player.speed = 500.0f;
    player.alive = true;
}

void initRect1() {
    e1.pos = glm::vec2(0 + rand() % port, -50.0f);
    e1.width = 50.0f;
    e1.height = 50.0f;
    e1.color = SDL_Color{ 250,150,0,255 };
    e1.dir = glm::vec2(0.0f, 1.0f);
    e1.speed = 50 + rand() % 100;
    e1.alive = true;
    lastSpawnTime1 = SDL_GetTicks();
    spawnInterval1 = 1000;
}

void initRect2() {
    e2.pos = glm::vec2(port + rand() % port, -50.0f);
    e2.width = 50.0f;
    e2.height = 50.0f;
    e2.color = SDL_Color{ 250,150,0,255 };
    e2.dir = glm::vec2(0.0f, 1.0f);
    e2.speed = 50 + rand() % 100;
    e2.alive = true;
    lastSpawnTime2 = SDL_GetTicks();
    spawnInterval2 = 1000;
}

void initRect3() {
    e3.pos = glm::vec2(2 * port + rand() % port, -50.0f);
    e3.width = 50.0f;
    e3.height = 50.0f;
    e3.color = SDL_Color{ 250,150,0,255 };
    e3.dir = glm::vec2(0.0f, 1.0f);
    e3.speed = 50 + rand() % 100;
    e3.alive = true;
    lastSpawnTime3 = SDL_GetTicks();
    spawnInterval3 = 1000;
}

void initPlayerCircle() {
    Circle c = Circle();
    c.radius = 10.0f;
    c.pos = player.pos;
    c.color = SDL_Color{ 255, 0, 0, 255 };
    c.dir = glm::vec2(0.0f, -1.0f);
    c.speed = 500.0f;

    playerCircles.push_back(c);
    
}

glm::mat2 rotationMatrix(float degrees) {
    float rad = glm::radians(degrees);

    return glm::mat2(
        cos(rad), sin(rad),
        -sin(rad), cos(rad)
    );
}

void initEnemyCircle(Rectangle e) {
    Circle c = Circle();
    c.radius = 10.0f;
    c.pos = e.pos;
    c.color = SDL_Color{ 255, 0, 255, 255 };
    c.dir = glm::normalize(player.pos - e.pos);
    c.speed = 100.0f;

    enemyCircles.push_back(c);

    Circle c1 = Circle();
    c1.radius = 10.0f;
    c1.pos = e.pos;
    c1.color = SDL_Color{ 255, 0, 255, 255 };
    c1.dir = glm::normalize(player.pos - e.pos) * rotationMatrix(15);
    c1.speed = 100.0f;

    enemyCircles.push_back(c1);

    Circle c2 = Circle();
    c2.radius = 10.0f;
    c2.pos = e.pos;
    c2.color = SDL_Color{ 255, 0, 255, 255 };
    c2.dir = glm::normalize(player.pos - e.pos) * rotationMatrix(-15);
    c2.speed = 100.0f;

    enemyCircles.push_back(c2);

}

bool detectcollisionrect(Rectangle a, Rectangle b) {
    return (
        abs(a.pos.x - b.pos.x) * 2 < (a.width + b.width) &&
        abs(a.pos.y - b.pos.y) * 2 < (a.height + b.height)
        );
}

float clamp(float v, float minim, float maxim) {
    return std::max(minim, std::min(v, maxim));
}

bool detectcollisioncircle(Circle c, Rectangle r) {

    float width = r.width;
    float height = r.height;
    float xs = r.pos.x - width / 2;
    float xf = r.pos.x + width / 2;
    float ys = r.pos.y - height / 2;
    float yf = r.pos.y + height / 2;
    float cx = c.pos.x;
    float cy = c.pos.y;
    float closestx = clamp(c.pos.x, xs, xf);
    float closesty = clamp(c.pos.y, ys, yf);

    if (closestx > cx)
        std::swap(closestx, cx);

    if (closesty > cy)
        std::swap(closesty, cy);

    //cx cy always bigger

    float dx = cx - closestx;
    float dy = cy - closesty;
    float distance = dx * dx + dy * dy;
    if (sqrt(distance) <= c.radius)
        return 1;
    return 0;
}


//define SDL Window related variables
SDL_Window *window{nullptr};
SDL_Renderer *renderer{nullptr};
SDL_Event currentEvent;
SDL_Color backgroundColor{255, 255, 255, 255};

bool quit{false};

float mouseX{-1.0f}, mouseY{-1.0f};

float displayScale{1.0f};


void initScollingRectangles() {
    int numGridRectangles = (WINDOW_HEIGHT / VERTICAL_SCROLLING_GRID_SQUARE_SIZE) / 2 + 2;

    for (int i = 0; i < numGridRectangles; i++) {
        Rectangle newGridRectangleLeft, newGridRectangleRight;

        newGridRectangleLeft.width = newGridRectangleLeft.height = VERTICAL_SCROLLING_GRID_SQUARE_SIZE;
        newGridRectangleLeft.pos.x = newGridRectangleLeft.width / 2.0f;
        newGridRectangleLeft.pos.y = WINDOW_HEIGHT - newGridRectangleLeft.height / 2.0f - 2.0f * i * newGridRectangleLeft.height;
        newGridRectangleLeft.vel = glm::vec2(0.0f, VERTICAL_SCROLLING_SPEED);
        newGridRectangleLeft.color = SDL_Color{ 235, 235, 235, 255 };

        newGridRectangleRight.width = newGridRectangleRight.height = VERTICAL_SCROLLING_GRID_SQUARE_SIZE;
        newGridRectangleRight.pos.x = WINDOW_WIDTH - newGridRectangleRight.width / 2.0f;
        newGridRectangleRight.pos.y = WINDOW_HEIGHT - newGridRectangleRight.height / 2.0f - 2.0f * i * newGridRectangleRight.height;
        newGridRectangleRight.vel = glm::vec2(0.0f, VERTICAL_SCROLLING_SPEED);
        newGridRectangleRight.color = SDL_Color{ 235, 235, 235, 255 };

        scrollingGrid.push_back(newGridRectangleLeft);
        scrollingGrid.push_back(newGridRectangleRight);
    }
}

bool initWindow() {
    bool success{true};

    //Try to initialize SDL
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL initialization failed: %s\n", SDL_GetError());
        success = false;
    } else {
        //Try to create the window and renderer
        displayScale = SDL_GetDisplayContentScale(1);

        if (!SDL_CreateWindowAndRenderer(
            "SDL Hello World Example",
            static_cast<int>(displayScale * WINDOW_WIDTH),
            static_cast<int>(displayScale * WINDOW_HEIGHT),
            0,
            &window, &renderer)) {
            SDL_Log("Failed to create window and renderer: %s\n", SDL_GetError());
            success = false;
        } else {
            //Apply global display scaling to renderer
            SDL_SetRenderScale(renderer, displayScale, displayScale);

            //Set background color
            SDL_SetRenderDrawColor(renderer, backgroundColor.r, backgroundColor.g, backgroundColor.b,
                                   backgroundColor.a);

            //Apply background color
            SDL_RenderClear(renderer);
        }
    }

    return success;
}

void initSetup() {
    playerCircles.clear();
    enemyCircles.clear();
    initRectangle();
    initRect1();
    initRect2();
    initRect3();
}

void processEvents() {
    // Check for events in queue
    while (SDL_PollEvent(&currentEvent)) {

        // User requests quit
        if (currentEvent.type == SDL_EVENT_QUIT) {
            quit = true;
        }

        // Keyboard event
        if (currentEvent.type == SDL_EVENT_KEY_DOWN) {
            switch (currentEvent.key.key) {
            case SDLK_ESCAPE:
                quit = true;
                break;
            case SDLK_UP:
                break;
            case SDLK_DOWN:
                break;
            case SDLK_LEFT:
                break;
            case SDLK_RIGHT:
                break;
            case SDLK_R:
                initSetup();
                break;
            case SDLK_S:
                std::cout << "S key was pressed" << std::endl;
                break;
            default:
                break;
            }
        }
    }

    // Get mouse position and button state once per frame
    SDL_MouseButtonFlags mouseButtons =
        SDL_GetMouseState(&mouseX, &mouseY);

    mouseX /= displayScale;
    mouseY /= displayScale;

    // Automatic shooting every 0.3 seconds
    Uint64 currentTime = SDL_GetTicks();

    if (player.alive &&
        (mouseButtons & SDL_BUTTON_MASK(SDL_BUTTON_LEFT)) &&
        currentTime - lastShotTime >= shotInterval) {

        initPlayerCircle();
        lastShotTime = currentTime;
    }
}


void animate() {
    if (!player.alive)
        return; 
    glm::vec2 target = glm::vec2(mouseX, mouseY);

    glm::vec2 difference = target - player.pos;

    float distance = glm::length(difference);

    if (distance > 0.0f) {
        glm::vec2 direction = difference / distance;

        float movement = std::min(player.speed * DT, distance);

        player.pos += direction * movement;
    }

    // Scrolling grid
    for (int i = 0; i < scrollingGrid.size(); i++) {
        Rectangle& gridRectangle = scrollingGrid.at(i);

        gridRectangle.pos += gridRectangle.vel * DT;

        if (gridRectangle.pos.y > (WINDOW_HEIGHT + gridRectangle.height / 2.0f)) {
            gridRectangle.pos.y -= gridRectangle.height * scrollingGrid.size();
        }
    }
       
    for (Circle& c : playerCircles) {
        c.pos += c.dir * c.speed * DT;
    }

    for (Circle& c : enemyCircles) {
        c.pos += c.dir * c.speed * DT;
    }
    
    e1.pos += e1.dir*e1.speed * DT;
    e2.pos += e2.dir*e2.speed * DT;
    e3.pos += e3.dir*e3.speed * DT;

    for (Circle c : playerCircles) {
        if (detectcollisioncircle(c, e1)) {
            e1.alive = false;
            e1.pos = glm::vec2(-100, -100);
        }
    }
    for (Circle c : playerCircles) {
        if (detectcollisioncircle(c, e2)) {
            e2.alive = false;
            e2.pos = glm::vec2(-100, -100);
        }
    }
    for (Circle c : playerCircles) {
        if (detectcollisioncircle(c, e3)) {
            e3.alive = false;
            e3.pos = glm::vec2(-100, -100);
        }
    }

    for (Circle c : enemyCircles) {
        if (detectcollisioncircle(c, player)) {
            player.alive = false;
            player.color = { 255,0,0,255 };
            player.speed = 0;
        }
    }
    if (detectcollisionrect(player, e1) || detectcollisionrect(player, e2) || detectcollisionrect(player, e3)) {
        player.alive = false;
        player.color = { 255,0,0,255 };
        player.speed = 0;
    }

    if (!e1.alive && !e2.alive && !e3.alive) {
        player.alive = false;
        player.color = { 0, 0, 255, 255 };
        player.speed = 0;
    }

    Uint64 currentTime = SDL_GetTicks();

    if (currentTime - lastSpawnTime1 >= spawnInterval1) {
        lastSpawnTime1 = currentTime;
        spawnInterval1 = 3000;
        if (e1.alive)
            initEnemyCircle(e1);
        else
            initRect1();
    }
    if (currentTime - lastSpawnTime2 >= spawnInterval2) {
        lastSpawnTime2 = currentTime;
        spawnInterval2 = 3000;
        if (e2.alive)
            initEnemyCircle(e2);
        else
            initRect2();
    }
    if (currentTime - lastSpawnTime3 >= spawnInterval3) {
        lastSpawnTime3 = currentTime;
        spawnInterval3 = 3000;
        if (e3.alive)
            initEnemyCircle(e3);
        else
            initRect3();
    }

    if (e1.pos.y > 850) {
        e1.alive = false;
    }
    if (e2.pos.y > 850) {
        e2.alive = false;
    }
    if (e3.pos.y > 850) {
        e3.alive = false;
    }

    player.pos.x = clamp(player.pos.x,
        player.width / 2,
        WINDOW_WIDTH - player.width / 2);

    player.pos.y = clamp(player.pos.y,
        player.height / 2,
        WINDOW_HEIGHT - player.height / 2);

    for (int i = 0; i < playerCircles.size(); i++) {
        Circle& c = playerCircles[i];

        if (c.pos.x + c.radius < 0 ||
            c.pos.x - c.radius > WINDOW_WIDTH ||
            c.pos.y + c.radius < 0 ||
            c.pos.y - c.radius > WINDOW_HEIGHT) {

            playerCircles.erase(playerCircles.begin() + i);
            i--;
        }
    }

    for (int i = 0; i < enemyCircles.size(); i++) {
        Circle& c = enemyCircles[i];

        if (c.pos.x + c.radius < 0 ||
            c.pos.x - c.radius > WINDOW_WIDTH ||
            c.pos.y + c.radius < 0 ||
            c.pos.y - c.radius > WINDOW_HEIGHT) {

            enemyCircles.erase(enemyCircles.begin() + i);
            i--;
        }
    }
}

void drawFrame() {
    //Clear the background
    
    SDL_SetRenderDrawColor(renderer, backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
    SDL_RenderClear(renderer);

    //Draw scene

    //Draw scrolling grid
    
    for (int i = 0; i < scrollingGrid.size(); i++) {
        Rectangle& gridRectangle = scrollingGrid.at(i);
        gridRectangle.draw(renderer);
    }
    player.draw(renderer);
    if (player.alive) {
        if (e1.alive)
            e1.draw(renderer);
        if (e2.alive)
            e2.draw(renderer);
        if (e3.alive)
            e3.draw(renderer);
        for (Circle c : playerCircles) {
            c.draw(renderer);
        }

        for (Circle c : enemyCircles) {
            c.draw(renderer);
        }
        //Update window
        
    }
    SDL_RenderPresent(renderer);
    
}

void cleanup() {
    //Destroy renderer
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }

    //Destroy window
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }

    //Quit SDL
    SDL_Quit();
}



int main() {
    //Initialize window
    if (!initWindow()) {
        std::cout << "Failed to initialize" << std::endl;
        return -1;
    }

    SDL_zero(currentEvent);

	initScollingRectangles();
    initSetup();
    //Game loop
    while (!quit) {
        startTime = SDL_GetTicks();

        processEvents();
        animate();
        drawFrame();

        endTime = SDL_GetTicks();
        DT = (endTime - startTime) / 1000.0f;
    }

    cleanup();
    return 0;
}
