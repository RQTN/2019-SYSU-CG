#pragma once
#include <iostream>
#include <vector>
#include <algorithm>


struct Point
{
	int x, y;

	Point(int x_, int y_) : x(x_), y(y_) {}
};

bool operator == (const Point& lhs, const Point& rhs)
{
	return (lhs.x == rhs.x) && (lhs.y == rhs.y);
}

struct bBox
{
	int xmin;
	int xmax;
	int ymin;
	int ymax;
};


namespace Utils
{
	// Normalize: convert int point value to float NDC
	std::vector<float> points2NDC(const std::vector<Point>& data_, const unsigned int scr_width, const unsigned int scr_height)
	{
		std::vector<float> data;
		data.resize(2 * data_.size());
		for (auto i = 0; i < data_.size(); ++i)
		{
			data[2 * i] = 2 * data_[i].x / float(scr_width);
			data[2 * i + 1] = 2 * data_[i].y / float(scr_height);
		}
		return data;
	}
}

// use Edge-equation
namespace TriangleRasterization
{
	int min3(const int a, const int b, const int c)
	{
		int min_ab = std::min(a, b);
		return std::min(min_ab, c);
	}

	int max3(const int a, const int b, const int c)
	{
		int max_ab = std::max(a, b);
		return std::max(max_ab, c);
	}

	// since the point's coor are int type, so do not need to ceil
	void bound3(const Point& p0, const Point& p1, const Point& p2, bBox& bbox)
	{
		bbox.xmin = min3(p0.x, p1.x, p2.x);
		bbox.xmax = max3(p0.x, p1.x, p2.x);
		bbox.ymin = min3(p0.y, p1.y, p2.y);
		bbox.ymax = max3(p0.y, p1.y, p2.y);
	}

	std::vector<int> getABC(const Point& p0, const Point& p1)
	{
		// line equation: Ax + By + C = 0
		std::vector<int> ABC(3);

		ABC[0] = p0.y - p1.y;
		ABC[1] = p1.x - p0.x;
		ABC[2] = p0.x * p1.y - p1.x * p0.y;

		return ABC;
	}

	int checkInsideSymbol(std::vector<int>& ABC, const int x, const int y)
	{
		int f = ABC[0] * x + ABC[1] * y + ABC[2];
		return (f > 0) ? 1 : -1;
	}

	std::vector<Point> edgeEquations(const Point& p0, const Point& p1, const Point& p2)
	{
		std::vector<Point> pvec;
		bBox bbox;
		bound3(p0, p1, p2, bbox);

		// get 3 lines params
		std::vector<int> linesABC[3] = {};
		linesABC[0] = getABC(p0, p1);
		linesABC[1] = getABC(p0, p2);
		linesABC[2] = getABC(p1, p2);

		int insideSymbol[3] = {};
		insideSymbol[0] = checkInsideSymbol(linesABC[0], p2.x, p2.y);
		insideSymbol[1] = checkInsideSymbol(linesABC[1], p1.x, p1.y);
		insideSymbol[2] = checkInsideSymbol(linesABC[2], p0.x, p0.y);

		for (int i = bbox.xmin; i <= bbox.xmax; ++i)
		{
			for (int j = bbox.ymin; j <= bbox.ymax; ++j)
			{
				bool inside = true;
				for (int k = 0; k < 3; ++k)
				{
					// check if the points has same symbol
					if (checkInsideSymbol(linesABC[k], i, j) * insideSymbol[k] <= 0)	// if not(points are on the other side or on the line)
					{
						inside = false;
						break;
					}
				}
				if (inside)
				{
					pvec.push_back(Point(i, j));
				}

			}
		}

		return pvec;
	}

}


namespace Bresenham
{
	void swap(Point& p0, Point& p1)
	{
		Point tmp = p0;
		p0 = p1;
		p1 = tmp;
	}

	void negX(Point& p)
	{
		p.x = -p.x;
	}

	void flipXY(Point& p)
	{
		int tmp = p.x;
		p.x = p.y;
		p.y = tmp;
	}

	std::vector<Point> genLineData(const Point& p0_, const Point& p1_)
	{
		Point p0 = p0_;
		Point p1 = p1_;
		bool negFlag = false;
		bool flipFlag = false;

		int dx = std::abs(p0.x - p1.x);
		int dy = std::abs(p0.y - p1.y);

		if (p0.x < p1.x && p0.y > p1.y)
		{
			swap(p0, p1);
		}

		if (p0.x > p1.x && p0.y < p1.y)
		{
			negX(p0);
			negX(p1);
			negFlag = true;
		}

		if (p0.x > p1.x && p0.y > p1.y)
		{
			swap(p0, p1);
		}

		if (dx < dy)
		{
			flipXY(p0);
			flipXY(p1);
			flipFlag = true;
		}

		std::vector<Point> pvec;

		if (dx != 0) // line that is not parallel to x or y axis: draw basic situation line
		{
			// Bresenham
			int b_dx = p1.x - p0.x;
			int b_dy = p1.y - p0.y;
			int p = 2 * b_dy - b_dx;

			for (auto x = p0.x, y = p0.y; x <= p1.x; ++x) 
			{
				pvec.push_back(Point(x, y));
				if (p > 0)
				{
					p += (2 * b_dy - 2 * b_dx);
					++y;
				}
				else
				{
					p += 2 * b_dy;
				}
			}

			// reverse the transformation from basic line to the origin line
			if (flipFlag)
			{
				std::for_each(pvec.begin(), pvec.end(), flipXY);
				flipFlag = false;
			}

			if (negFlag)
			{
				std::for_each(pvec.begin(), pvec.end(), negX);
			}

		}
		else		// line that is parallel to x or y axis: also can use Bresenham! but not necessary
		{
			if (p0.x > p1.x)
			{
				swap(p0, p1);
			}

			for (auto x = p0.x; x <= p1.x; ++x)
			{
				pvec.push_back(Point(x, 0));
			}

			if (flipFlag)
			{
				std::for_each(pvec.begin(), pvec.end(), flipXY);
				flipFlag = false;
			}
		}

		return pvec;

	}

	std::vector<Point> genTriangleData(const Point& p0, const Point& p1, const Point& p2, bool isFilled = false)
	{
		auto datas = genLineData(p0, p1);
		auto line2Data = genLineData(p0, p2);
		auto line3Data = genLineData(p1, p2);

		datas.insert(datas.end(), line2Data.begin(), line2Data.end());
		datas.insert(datas.end(), line3Data.begin(), line3Data.end());

		if (isFilled)
		{
			auto insideData = TriangleRasterization::edgeEquations(p0, p1, p2);
			datas.insert(datas.end(), insideData.begin(), insideData.end());
		}

		return datas;
	}

	void addCirclePlot(std::vector<Point>& pvec, const Point& origin, const int x, const int y)
	{
		std::vector<Point> eightPoints = {
			Point(origin.x + x, origin.y + y), Point(origin.x + y, origin.y + x),
			Point(origin.x + x, origin.y - y), Point(origin.x - y, origin.y + x),
			Point(origin.x - x, origin.y + y), Point(origin.x + y, origin.y - x),
			Point(origin.x - x, origin.y - y), Point(origin.x - y, origin.y - x),
		};

		pvec.insert(pvec.end(), eightPoints.begin(), eightPoints.end());
	}

	std::vector<Point> genCircleData(const Point& origin, const int R)
	{
		std::vector<Point> pvec;

		int x = 0;
		int y = R;
		int d = 3 - 2 * R;

		while (x <= y)
		{
			addCirclePlot(pvec, origin, x, y);
			if (d < 0)
			{
				d += (4 * x + 6);
			}
			else
			{
				d += (4 * (x - y) + 10);
				--y;
			}
			++x;
		}

		return pvec;
	}

}

