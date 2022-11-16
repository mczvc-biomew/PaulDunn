/* http://paulbourke.net/fractals/clifford/paul_richards/main.cpp */
#ifndef PAULBOURKE_COLOR
#define PAULBOURKE_COLOR

class Color
{
  public:
	double r, g, b;

	Color(const double &red = 0, 
		const double &green = 0, 
		const double &blue = 0) 
		: r(red), g(green), b(blue)
	{
	}

	Color &operator+=(const Color &rhs)
	{
		r += rhs.r;
		g += rhs.g;
		b += rhs.b;
		return *this;
	}

	static Color createHue(double h)
	{
		h *= 6.0;
		

		int hi = static_cast<int>(h);
		double hf = h - hi;

		switch (hi % 6)
		{
		case 0:
			return Color(1.0, hf, 0.0);
		case 1:
			return Color(1.0 - hf, 1.0, 0.0);
		case 2:
			return Color(0.0, 1.0, hf);
		case 3:
			return Color(0.0, 1.0 - hf, 1.0);
		case 4:
			return Color(hf, 0.0, 1.0);
		case 5:
			return Color(1.0, 0.0, 1.0 - hf);
		}

		return Color();
	}

	Color operator+(const Color &rhs) const
	{
		return Color(*this) += rhs;
	}
};

#endif