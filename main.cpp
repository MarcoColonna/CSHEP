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
  
  // Open a file for writing
  std::ofstream out("mandelbrot.txt");
  if (!out) {
    std::cout << "Failed to open file for writing" << std::endl;
    return 1;
  }
  
  auto start = std::chrono::high_resolution_clock::now();
  tbb::parallel_for(0, display_height, [&](int row) {
    for (int column = 0; column != display_width; ++column) {
      auto k = mandelbrot(top_left + Complex{delta_x * column, delta_y * row});
      image.setPixel(column, row, to_color(k));
      // Write the result to the file
      out << k << " ";
    }
    out << std::endl; // Add a newline after each row
  });
  
  auto stop = std::chrono::high_resolution_clock::now();
  auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
  std::cout << "Elapsed time: " << elapsed_time << " milliseconds" << std::endl;
  
  // Close the file
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
