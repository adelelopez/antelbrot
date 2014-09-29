// ANTelbrot - a Mandelbrot set viewer using perturbation theory 
// Author: Adele Dewey-Lopez
// Uses the SuperFractalThing perturbation theory algorithm by K.I. Martin
// http://www.superfractalthing.co.nf/sft_maths.pdf

#include <SFML/Graphics.hpp>
#include <complex>
#include <vector>
#include <cmath>
#include <iostream>
#include <cmath>
#include <string>
#include <map>
#include <regex>
#include <gmpxx.h>

using std::cout;
using std::endl;
using std::vector;
using std::cin;

// Color algorithms

sf::Color interpolate(const sf::Color& color1, const sf::Color& color2, 
	const double& mid)
{	
	int r = color1.r + (color2.r - color1.r) * mid;
	int g = color1.g + (color2.g - color1.g) * mid;
	int b = color1.b + (color2.b - color1.b) * mid;
	return sf::Color(r,g,b);
}

// This function takes a vector of colors, and returns a gradient based on
// the original vector
vector<sf::Color> color_table(const vector<sf::Color>& gradient)
{
	vector<sf::Color> v;
	double d = 0;
	for (auto i = gradient.begin(); i != gradient.end(); ++i)
	{
		// interpolate between this and the next color of the input gradient 
		auto next = (i + 1 == gradient.end()) ? gradient.begin() : i + 1;
		for (double d = 0; d < 1; d+=0.01)
			v.push_back(interpolate(*i, *next, d));
	}
	return v;
}

sf::Color palette(const vector<sf::Color>& gradient, double zn_size, int iter)
{
	// use smooth coloring 
	double nu = iter - std::log2(std::log2(zn_size));

	nu *= 10;

	int i = (int) nu % gradient.size();

	return gradient[i];		
}

// Math algorithms

// high precision point used for perturbation theory method
// produces a list of iteration values used to compute the surrounding points
vector<std::complex<double>> deep_zoom_point(const mpf_class& center_r, const mpf_class& center_i,
	int depth)
{
	vector<std::complex<double>> v;
	mpf_class xn_r = center_r;
	mpf_class xn_i = center_i;

	for (int i = 0; i != depth; ++i)
	{	
		// pre multiply by two
		mpf_class re = xn_r + xn_r;
		mpf_class im = xn_i + xn_i;

		std::complex<double> c(re.get_d(),im.get_d());

		v.push_back(c);
	
		// make sure our numbers don't get too big
		if (re > 1024 || im > 1024 || re < -1024 || im < -1024)
			return v;

		// calculate next iteration, remember re = 2 * xn_r 
		xn_r = xn_r * xn_r - xn_i * xn_i + center_r;
		xn_i = re * xn_i + center_i;
	}
	return v;
}


// Color the pixel (i,j) (in screen coordinates)
sf::Color pt(const int& i, const int& j, const vector<std::complex<double>>& x, 
	const sf::Vector2u& size, const double& radius, 
	const vector<sf::Color>& gradient)
{
	int window_radius = (size.x < size.y) ? size.x : size.y;
	// find the complex number at the center of this pixel
	std::complex<double> d0 (radius * (2 * i - (int) size.x) / window_radius, 
							-radius * (2 * j - (int) size.y) / window_radius);

	int iter = 0;

	int max_iter = x.size();

	double zn_size;
	// run the iteration loop
	std::complex<double> dn = d0;
	do
	{
		dn*= x[iter] + dn;
		dn+= d0;
		++iter;
		zn_size = std::norm(x[iter] * 0.5 + dn);

	} while (zn_size < 256 && iter < max_iter);

	// color according to iteration using logarithmic smoothing
	sf::Color c0 = palette(gradient, zn_size, iter);
	if (iter == max_iter)
		return sf::Color::Black;	// if it's in the set, color black
	
	return c0;
}

void update(sf::VertexArray* set, const sf::Vector2u& size, 
	const vector<std::complex<double>>& x, const double& radius, 
	const vector<sf::Color>& gradient)
{
	for (int i = 0; i != size.x; ++i)
	{
		for (int j = 0; j != size.y; ++j)
		{
			(*set)[i + size.x * j].position = sf::Vector2f(i,j);
			(*set)[i + size.x * j].color = pt(i, j, x, size, radius, gradient);
		}
	}
}

bool invalid_digit_char(const char& c)
{
	switch (c)
	{
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case '-':
		case '.':
		case 'e':
		case 'E':
			return false;
		default:
			return true;
	}
}

int main()
{
	// prepare window and the pixel array
    sf::RenderWindow window(sf::VideoMode::getDesktopMode(), "ANTelbrot");
    sf::Vector2u size = window.getSize();
    int pixels = size.x * size.y;
    sf::VertexArray* mandelbrot = new sf::VertexArray(sf::Points, pixels);


    // prepare gradient
    vector<sf::Color> gradient;
    gradient.push_back(sf::Color::Black);
    gradient.push_back(sf::Color::Blue);
    gradient.push_back(sf::Color(128,0, 255));
    gradient.push_back(sf::Color::White);
    gradient.push_back(sf::Color::Yellow);
    gradient.push_back(sf::Color::Red);
    gradient = color_table(gradient);

   	double radius = 2;
	int depth = 1000;

	mpf_class center_r(0,100);
	mpf_class center_i(0,100);
    vector<std::complex<double>> x = deep_zoom_point(center_r, center_i, depth);

    update(mandelbrot, size, x, radius, gradient);

    // main loop
    while (window.isOpen())
    {
    	window.clear();
        window.draw(*mandelbrot);
        window.display();
        sf::Event event;
        while (window.pollEvent(event))
        {      
            switch (event.type)
			{
			case sf::Event::Closed:
			    window.close();
			    break;
			case sf::Event::Resized:
			{
				size = window.getSize();
				// make sure the view gets resized as well
 				window.setView(sf::View(sf::FloatRect(0,0,size.x,size.y)));

 				// resize the vertex array
				pixels = size.x * size.y;
				mandelbrot->resize(pixels);

			  	update(mandelbrot, size, x, radius, gradient);
			}
				break;
			case sf::Event::KeyPressed:
				switch(event.key.code)
				{
				    case sf::Keyboard::Escape:
				        window.close();
				        break;

				    case sf::Keyboard::R:
				    {
				    	cout << "Enter the new zoom radius: " << endl;
				    	cin >> radius;
				    	update(mandelbrot, size, x, radius, gradient);
				    }
				    	break;

				    case sf::Keyboard::D:
				    {
				      	cout << "Enter the new iteration depth: " << endl;    
				    	cin >> depth;    	
   					 	cout << "depth: " << depth << ". zoom: " << radius << endl;

   					 	x = deep_zoom_point(center_r, center_i, depth);
				    	update(mandelbrot, size, x, radius, gradient);
				    }
				    	break;

				    case sf::Keyboard::I:
				    {
				      	cout << "Enter the real coordinate value: " << endl;
				    	std::string r_str, i_str;
				    	getline(cin, r_str ,'\n');
				    	cout << "Enter the imaginary coordinate value: " << endl;
				    	getline(cin, i_str, '\n');
				    	cout << "Thank you for your cooperation." << endl;

				    	// regex for detecting valid float numbers the user may enter. 
				    	// a string of digits, optionally followed by a decimal point and another string of
				    	// digits, optionally followed by an exponent
				    	std::regex valid_float {"\\d+(.\\d*)?(e(\\+|-)?\\d+)?"};

				    	// remove commas and other non-number characters (e for exponents is allowed)
				    	r_str.erase(remove_if(r_str.begin(), r_str.end(), invalid_digit_char), r_str.end());
				    	i_str.erase(remove_if(i_str.begin(), i_str.end(), invalid_digit_char), i_str.end());
				  
				    	
				    	center_r = mpf_class(r_str.c_str(), 100);
				    	center_i = mpf_class(i_str.c_str(), 100);

				    	
   					 	cout << "center: " << center_r << " + i " << center_i << ". zoom: " << radius << endl;
			
						x = deep_zoom_point(center_r, center_i, depth);
				    	update(mandelbrot, size, x, radius, gradient);
				    }
				    	break;

				    case sf::Keyboard::Z:
				    {
				    	radius/= 2;
			     		cout << "center: " << center_r << " + i " << center_i << ". zoom: " << radius << endl;

				    	update(mandelbrot, size, x, radius, gradient);
				    }
				    	break;
				}
			    break;
			case sf::Event::MouseButtonPressed:
				if (event.mouseButton.button == sf::Mouse::Left)
				{	
					sf::Vector2f mouse(event.mouseButton.x, event.mouseButton.y);
					center_r+= radius * (2 * mouse.x - (int) size.x) / size.y;
				 	center_i+= -radius * (2 * mouse.y - (int) size.y) / size.y;
				 	radius/= 2;
   					cout << "center: " << center_r << " + i " << center_i << ". zoom: " << radius << endl;
  					x = deep_zoom_point(center_r, center_i, depth);

					update(mandelbrot, size, x, radius, gradient);
				}
				break;
			}
        }	  
	}
	delete mandelbrot;
    return 0;
}
