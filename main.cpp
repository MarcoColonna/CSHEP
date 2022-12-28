#include <SFML/Graphics.hpp>
#include <cassert>
#include <complex>
#include <iostream>
#include <string>
#include <thread>

#include <tbb/tbb.h>
#include <chrono>
#include <fstream>

using Complex = std::complex<double>;

int mandelbrot(Complex const& c)
{
  int i = 0;
  auto z = c;
  for (; i != 256 && norm(z) < 4.; ++i) {
    z = z * z + c;
  }
  return i;
}

auto to_color(int k)
{
  return k < 256 ? sf::Color{static_cast<sf::Uint8>(10 * k), 0, 0}
                 : sf::Color::Black;
}

int main()
{
  int const display_width{600};
  int const display_height{600};

  Complex const top_left{-2.2, 1.5};
  Complex const lower_right{0.8, -1.5};
  auto const diff = lower_right - top_left;

  auto const delta_x = diff.real() / display_width;
  auto const delta_y = diff.imag() / display_height;

  sf::RenderWindow window(sf::VideoMode(display_width, display_height),
                          "Mandelbrot Set");
  window.setFramerateLimit(60);

  sf::Image image;
  image.create(window.getSize().x, window.getSize().y);

    // Set up an array of granularity values to test
  int granularities[] = {1, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 150, 160, 170, 180, 190, 200};

  // Create a vector to store the elapsed times for each granularity
  std::vector<std::pair<int, long long>> elapsed_times;

  for (int granularity : granularities) {
    auto start = std::chrono::high_resolution_clock::now();
    tbb::parallel_for(0, display_height - granularity + 1, granularity, [&](int row) {
      // Process rows [row, row + granularity)
      for (int r = row; r < row + granularity; ++r) {
        for (int column = 0; column != display_width; ++column) {
          auto k = mandelbrot(top_left + Complex{delta_x * column, delta_y * r});
          image.setPixel(column, r, to_color(k));
        }
      }
    });
  
    auto stop = std::chrono::high_resolution_clock::now();
    auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
    elapsed_times.push_back({granularity, elapsed_time});
  }

  // Save the elapsed times to a file
  std::ofstream out("elapsed_times.txt");
  if (!out) {
    std::cout << "Failed to open file for writing" << std::endl;
    return 1;
  }

  for (auto const& [granularity, time] : elapsed_times) {
    out << granularity << " " << time << std::endl;
  }

  out.close();

  
  // Save the image to a file
  if (!image.saveToFile("image.png")) {
    std::cout << "Failed to save image to file" << std::endl;
  }
  
  sf::Texture texture;
  texture.loadFromImage(image);
  sf::Sprite sprite;
  sprite.setTexture(texture);
  while (window.isOpen()) {
    sf::Event event;
    while (window.pollEvent(event)) {
      if (event.type == sf::Event::Closed) window.close();
    }

    window.clear();

    window.draw(sprite);

    window.display();

    using namespace std::chrono_literals;
    std::this_thread::sleep_for(15ms);
  }
}
