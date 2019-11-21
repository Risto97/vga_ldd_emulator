#include <SFML/Graphics.hpp>
#include <chrono>
#include <iostream>
#include <ratio>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string>
#include <cstdlib>
#include <cstdio>

#define VGA_X 640
#define VGA_Y 480
#define BLUE 0x001F
#define GREEN 0x07E0
#define RED 0xF800
#define BLACK 0x0000
#define YELLOW (RED + GREEN)

#define MAX_MMAP_SIZE (VGA_X * VGA_Y * sizeof(unsigned int))

// using namespace std::chrono;

int main() {
  int *buffer;

  int fd = shm_open("vga_buffer", O_RDWR | O_CREAT, 0666);
  if (fd < 0) {
    std::cout << "ERROR creating shared memory vga_buffer\n";
    return EXIT_FAILURE;
  }
  ftruncate(fd, MAX_MMAP_SIZE);

  buffer = (int *)mmap(NULL, MAX_MMAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED,
                       fd, 0);
  printf("Mapped at %p\n", buffer);
  if (buffer == (void *)-1)
    return 0;

  sf::RenderWindow window(sf::VideoMode(VGA_X, VGA_Y), "floating_window");
  sf::Uint8 *pixels = new sf::Uint8[VGA_X * VGA_Y * sizeof(int)];

  sf::Texture texture;
  if (!texture.create(VGA_X, VGA_Y))
    return -1;

  sf::Event event;
  sf::Sprite sprite(texture);

  while (window.isOpen()) {
    int byte_cnt = 0;
    // high_resolution_clock::time_point t1 = high_resolution_clock::now();
    for (int i = 0; i < VGA_Y * VGA_X; i += 1) {
      pixels[byte_cnt] = (buffer[i] & RED) >> 8;
      pixels[byte_cnt + 1] = (buffer[i] & GREEN) >> 3;
      pixels[byte_cnt + 2] = (buffer[i] & BLUE) << 3;
      pixels[byte_cnt + 3] = 255;
      byte_cnt += 4;
    }
    texture.update(pixels);
    window.draw(sprite);
    window.display();

    // high_resolution_clock::time_point t2 = high_resolution_clock::now();
    // duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
    // std::cout << "It took me " << time_span.count() << " seconds.\n";

    window.pollEvent(event);
    if (event.type == sf::Event::Closed)
      window.close();
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) &&
            sf::Keyboard::isKeyPressed(sf::Keyboard::Q))
      window.close();
  }

  munmap(buffer, MAX_MMAP_SIZE);
  close(fd);
  shm_unlink("vga_buffer");
  return 0;
}
