#include <iostream>
#include "PoissonImageEdit.h"

PoissonImageEdit::PoissonImageEdit()
{
}

int PoissonImageEdit::getIndex(int offset_x, int offset_y) 
{
	int index =  offset_y * width + offset_x;
	return index;
}

/*
A_l * p = b + A_r * q
4p(i,j) = p(i-1,j) + p(i+1,j) + p(i,j-1) + p(i,j+1)
4p(i,j) - p(i-1,j) - p(i+1,j) - p(i,j-1) - p(i,j+1) = 0
*/
void PoissonImageEdit::makeMatrix_A(int w, int h)
{
	// Width and Height of rectangle region
	width = w;
	height = h;
	std::vector<T> coefficients;
	int index_r = 0;
	for (int offset_y = 0; offset_y < h; offset_y++)
	{
		for (int offset_x = 0; offset_x < w; offset_x++)
		{
			coefficients.push_back(T(index_r, index_r, 4));
			
			if (offset_y - 1 > -1)
				coefficients.push_back(T(index_r, getIndex(offset_x, offset_y - 1), -1));
			if (offset_y + 1 < h)
				coefficients.push_back(T(index_r, getIndex(offset_x, offset_y + 1), -1));
			if (offset_x - 1 > -1)
				coefficients.push_back(T(index_r, getIndex(offset_x - 1, offset_y), -1));
			if (offset_x + 1 < w)
				coefficients.push_back(T(index_r, getIndex(offset_x + 1, offset_y), -1));
			index_r++;
		}
	}
	count_point = index_r;

	matrix_A.resize(count_point, count_point);
	matrix_A.setFromTriplets(coefficients.begin(), coefficients.end());
	//¾ØÕó×öLU·Ö½â
	LU.compute(matrix_A);
}


void PoissonImageEdit::makeMatrix_b(QPoint p_start, const QImage *image_)
{
	matrix_q = Eigen::MatrixXd::Zero(count_point, 3);
	for (int offset_y = 0; offset_y < height; offset_y++)
	{
		for (int offset_x = 0; offset_x < width; offset_x++)
		{
			QPoint p = p_start + QPoint(offset_x, offset_y);
			set_elem(p, getIndex(offset_x, offset_y), image_);
		}
	}
}

void PoissonImageEdit::set_elem(QPoint p, int index, const QImage* image_)
{
	QRgb color0, color1, color2, color3, color4;
	QPoint delta[4] = { QPoint(-1,0),QPoint(1,0),QPoint(0,-1),QPoint(0,1) };

	for (int j = 0; j < 4; j++)
	{
		QPoint q = p + delta[j];
		matrix_q(index, 0) += get_grad1(p, q, image_, 0);
		matrix_q(index, 1) += get_grad1(p, q, image_, 1);
		matrix_q(index, 2) += get_grad1(p, q, image_, 2);
	}
}

int PoissonImageEdit::get_grad1(QPoint p, QPoint q, const QImage* image_,int rgb)
{
	switch (rgb)
	{
		case 0:
			return  qRed(image_->pixel(p)) - qRed(image_->pixel(q));
		case 1:
			return  qGreen(image_->pixel(p)) - qGreen(image_->pixel(q));
		case 2:
			return  qBlue(image_->pixel(p)) - qBlue(image_->pixel(q));
		default:
			break;
	}
	return 0;
}

void PoissonImageEdit::clone(QImage* image_, QPoint p_start_)
{
	addBoundaryToMatrix_b(p_start_, image_);
	matrix_b += matrix_q;
	calculate_newP();
	write_newP(image_, p_start_);
}

/*
b = r g b
*/
void PoissonImageEdit::addBoundaryToMatrix_b(QPoint p_start, const QImage* image_)
{
	matrix_b = Eigen::MatrixXd::Zero(count_point, 3);
	QRgb color, color2;
	QPoint p;
	int offset_x = 0, offset_y = 0;
	for (int offset_x = 0; offset_x < width; offset_x++)
	{
		p = p_start + QPoint(offset_x, -1);
		set_elem(image_->pixel(p), getIndex(offset_x, 0));
	
		p = p_start + QPoint(offset_x, height);
		set_elem(image_->pixel(p), getIndex(offset_x, height-1));
	}

	for (int offset_y = 0; offset_y < height; offset_y++)
	{
		p = p_start + QPoint(-1, offset_y);
		set_elem(image_->pixel(p), getIndex(0, offset_y));

		p = p_start + QPoint(width, offset_y);
		set_elem(image_->pixel(p), getIndex(width -1, offset_y));
	}
}

void PoissonImageEdit::set_elem(QRgb color, int index)
{
	matrix_b(index, 0) += qRed(color);
	matrix_b(index, 1) += qGreen(color);
	matrix_b(index, 2) += qBlue(color);
}


void PoissonImageEdit::calculate_newP()
{
	matrix_p = LU.solve(matrix_b);
}

void PoissonImageEdit::write_newP(QImage* image_, QPoint p_start_)
{
	int index = 0;
	for (int offset_y = 0; offset_y < height; offset_y++)
	{
		for (int offset_x = 0; offset_x < width; offset_x++)
		{
			int r, g, b;
			r = (int)matrix_p(index, 0);
			r = (r<0) ? 0 : r;
			r = (r>255) ? 255 : r;
			g = (int)matrix_p(index, 1);
			g = (g<0) ? 0 : g;
			g = (g>255) ? 255 : g;
			b = (int)matrix_p(index, 2);
			b = (b<0) ? 0 : b;
			b = (b>255) ? 255 : b;
			
			QRgb color = qRgb(r, g, b);
			image_->setPixel(p_start_+QPoint(offset_x, offset_y), color);
			index++;
		}
	}
}

PoissonImageEdit::~PoissonImageEdit()
{
}
