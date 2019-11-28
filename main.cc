#include <SFML/Graphics.hpp>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#define VGA_X 640
#define VGA_Y 480
#define BLUE 0x001F
#define GREEN 0x07E0
#define RED 0xF800
#define BLACK 0x0000

#define MAX_MMAP_SIZE (VGA_X * VGA_Y * sizeof(unsigned int))

int parse_line(const std::string line, int *buffer) {
  std::regex val_reg("(\\d+),(\\d+),(0[xX]){0,1}([0-9a-fA-F]+)");
  std::stringstream dec_str;
  std::smatch m;

  if (!std::regex_search(line, m, val_reg)) {
    return -1;
  }

  int x = std::stoi(m[1]);
  int y = std::stoi(m[2]);
  int val;
  if (m[3] == "0x" || m[3] == "0X") {
    int n;
    std::istringstream(m[4]) >> std::hex >> n;
    val = n;
  } else
    val = std::stoi(m[4]);

  buffer[(y * VGA_X) + x] = val;
  return 0;
}

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

  int pid = fork();

  if (pid < 0) {
    perror("ERROR on fork");
    exit(1);
  }

  /******************************************************
   *********** Child for reading fifo *******************
   ******************************************************/
  if (pid == 0) {
    std::string fifo_name = "/tmp/vga_buffer";
    mkfifo(fifo_name.c_str(), 0666);

    std::ifstream f;
    while (1) {
      f.open(fifo_name, std::ifstream::in);
      if (f.fail()) {
        std::cout << "Failed to read file: " << '"' << fifo_name << '"'
                  << std::endl;
        return -1;
      }
      std::string line;
      while (std::getline(f, line)) {
        parse_line(line, buffer);
      }
      f.close();
    }
  }
  /******************************************************
   *********** Parrent for displaying from buffer *******
   ******************************************************/
  else {
    sf::RenderWindow window(sf::VideoMode(VGA_X, VGA_Y), "floating_window");
    sf::Uint8 *pixels = new sf::Uint8[VGA_X * VGA_Y * sizeof(int)];

    sf::Texture texture;
    if (!texture.create(VGA_X, VGA_Y))
      return -1;

    sf::Event event;
    sf::Sprite sprite(texture);

    while (window.isOpen()) {
      int byte_cnt = 0;
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
  }
  return 0;
}
