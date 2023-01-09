#include <SFML/Graphics.hpp>
#include <tbb/parallel_for.h>
#include <tbb/blocked_range2d.h>
#include <chrono>
#include <cassert>
#include <complex>
#include <iostream>
#include <string>
#include <thread>
#include <fstream>

using Complex = std::complex<double>;

int mandelbrot(Complex const& c) // Function that calculates the Mandelbrot set
// value for a given complex number 'c'
{
    int i = 0; // Initialize integer 'i' to 0
    auto z = c; // Create a complex number 'z' that is initialized to 'c'
    for (; i != 256 && norm(z) < 4.; ++i) // Loop until either 'i' is equal to
    // 256 or the norm of 'z' is greater
    // than 4
    {
        z = z * z + c; // Update 'z' by multiplying it by itself and adding 'c'
    }
    return i; // Return the value of 'i' after the loop finishes
}

auto to_color(int k, double opt = 0.0) // Function that converts an integer
// value of the Mandelbrot set to a
// color
{
    if (!opt)
    {
        return k < 256 ? sf::Color{ static_cast<sf::Uint8>(10 * k), 0, 0 } : sf::Color::Black;
    }
    else
    {
        if (opt == 1.0)
            return k < 256 ? sf::Color{ 0, static_cast<sf::Uint8>(10 * k), 0 } : sf::Color::Black;
        else
            return k < 256 ? sf::Color{ 0, 0, static_cast<sf::Uint8>(10 * k) } : sf::Color::Black;
        // Return a color based on the value of 'k'
    }
}

int main()
{
    int const display_width{ 800 }; // Constant integer for the width of the image
    int const display_height = display_width; // Constant integer for the height
    // of the image, initialized to the
    // same value as the width

    Complex const top_left{ -2.2,
        1.5 }; // Constant complex number for the top left corner of the image
    Complex const lower_right{ 0.8,
        -1.5 }; // Constant complex number for the lower right corner of the image
    auto const diff
        = lower_right - top_left; // Calculate the difference between the two complex numbers

    auto const delta_x = diff.real() / display_width; // Calculate the difference
    // in the real component
    // between the two complex
    // numbers, divided by the
    // width of the image
    auto const delta_y = diff.imag() / display_height; // Calculate the difference in the
    // imaginary component between the two
    // complex numbers, divided by the height
    // of the image

    std::vector<std::pair<int, double> > elapsed_times; // Vector to store the
    // elapsed time and grain
    // size for each iteration
    // of the loop

    sf::Image image; // Create an image object
    image.create(display_width, display_height); // Initialize the image with the
    // specified width and height

    // Vary the grain size of the parallel_for loop
    for (int grain_size = 1; grain_size <= display_height;
         grain_size < 20 ? grain_size += 1 : grain_size += 20)
    {
        // Measure the time taken to process the image
        auto start = std::chrono::high_resolution_clock::now();

        tbb::parallel_for(
            tbb::blocked_range2d<int>(0, display_height, grain_size, 0, display_width, grain_size),
            [&](const tbb::blocked_range2d<int>& fragment)
            {
                for (int row = fragment.rows().begin(); row != fragment.rows().end(); ++row)
                {
                    for (int column = fragment.cols().begin(); column != fragment.cols().end();
                         ++column)
                    {
                        auto k = mandelbrot(
                            top_left + Complex{ delta_x * column, delta_y * row }); // Calculate the
                        // Mandelbrot set value
                        // for the current pixel
                        image.setPixel(column, row, to_color(k)); // Set the color of the
                        // current pixel based
                        // on the calculated
                        // Mandelbrot set value
                    }
                }
            }); // By default a simple_partitioner is used

        auto end = std::chrono::high_resolution_clock::now(); // Record the time
        // after the image
        // has been processed
        auto elapsed_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start)
                                .count(); // Calculate the elapsed time in microseconds

        elapsed_times.emplace_back(grain_size,
            elapsed_time); // Store the grain size and elapsed time in the vector

        // std::cout << "Grain size: " << grain_size << ", elapsed time: " <<
        // elapsed_time << " microseconds" << std::endl;

        // If the current grain size is not the maximum and is divisible by 200
        // Run the parallel_for loop using the current grain size to process the
        // image
        if (grain_size != display_height && !(grain_size % 200))
        {
            std::string namefile = std::string("Mandelbrot_gs_") + std::to_string(grain_size)
                + std::string(".png"); // Create a file name using the current grain size
            auto color = grain_size / 200.0;
            tbb::parallel_for(tbb::blocked_range2d<int>(
                                  0, display_height, grain_size, 0, display_width, grain_size),
                [&](const tbb::blocked_range2d<int>& fragment)
                {
                    for (int row = fragment.rows().begin(); row != fragment.rows().end(); ++row)
                    {
                        for (int column = fragment.cols().begin(); column != fragment.cols().end();
                             ++column)
                        {
                            auto k
                                = mandelbrot(top_left + Complex{ delta_x * column, delta_y * row });
                            image.setPixel(column, row, to_color(k, color));
                        }
                    }
                }); // Run the parallel_for loop using the current grain size to
            // process the image
            image.saveToFile(namefile); // Save the image to the file with the created file name
        }
    }
    std::vector<int> grains;
    std::vector<double> times;

    // This code block creates an output file and writes the grain size and
    // elapsed time for each iteration to it
    std::ofstream file("Time_vs_grain_size.txt",
        std::ios::file); // Create an output file named "Time_vs_grain_size.txt"
    file << "Grain size and execution time\n\nGrain size\tExecution time "
           "[ms]\n\n"; // Write a header to the file

    // Iterate through the elapsed_times vector, extracting the grain size and
    // elapsed time for each iteration
    for (auto const & [ grain_size, elapsed_time ] : elapsed_times)
    {
        grains.push_back(grain_size); // Add the grain size to the grains vector
        times.push_back(
            elapsed_time / 1000.); // Add the elapsed time in milliseconds to the times vector
        file << grain_size << "\t\t" << elapsed_time / 1000.
            << '\n'; // Write the grain size and elapsed time to the file
    }

    // Find the minimum elapsed time in the times vector
    auto minimum_time = std::min_element(times.begin(), times.end());

    // Print the minimum elapsed time and corresponding grain size to the console
    // and to the output file
    std::cout << "\nThe minimum time  " << times[std::distance(times.begin(), minimum_time)]
              << " ms correspons to the grain size "
              << grains[std::distance(times.begin(), minimum_time)] << ".\n\n";
    file << "\nThe minimum execution time " << times[std::distance(times.begin(), minimum_time)]
        << " ms) corresponds to the grain size "
        << grains[std::distance(times.begin(), minimum_time)] << '.';

    file.close(); // Close the output file
}

