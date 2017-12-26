/* --------------------------------------------------------------------------- *
 * TMOYourOperatorName.cpp: implementation of the TMOYourOperatorName class.   *
 * --------------------------------------------------------------------------- */

#include "TMOAlsam06.h"

#include "../tmolib/matrix.h"
/* --------------------------------------------------------------------------- *
 * Constructor serves for describing a technique and input parameters          *
 * --------------------------------------------------------------------------- */
TMOAlsam06::TMOAlsam06()
{
	SetName(L"Alsam06");						// TODO - Insert operator name
	SetDescription(L"Grey Colour Sharpening Video");	// TODO - Insert description

	dParameter.SetName(L"ParameterName");				
	dParameter.SetDescription(L"ParameterDescription");	
	dParameter.SetDefault(1);							
	dParameter=1.;
	dParameter.SetRange(-1000.0,1000.0);				
	this->Register(dParameter);
	
}

TMOAlsam06::~TMOAlsam06()
{
}



/* --------------------------------------------------------------------------- *
 * This overloaded function is an implementation of your tone mapping operator *
 * --------------------------------------------------------------------------- */
int TMOAlsam06::Transform()
{

	// Source image is stored in local parameter pSrc
	// Destination image is in pDst

	// Initialy images are in RGB format, but you can 
	// convert it into other format


	int height = pSrc->GetHeight();
	int width = pSrc->GetWidth();							// x, y as color information
	int m = height * width;
	int pixel = height * width;



	std::ofstream outfile("bc.txt");

	double* pSourceData = pSrc->GetData();				// You can work at low level data
	double* pDestinationData = pDst->GetData();			// Data are stored in form of array 
															// of three doubles representing
														// three colour components
	//outfile << height <<  "  " << width << "\n" << std::endl;


    int ko=0;
    int j = 0;

    double sum;
    mtx::Matrix field(m, 3);
	mtx::Matrix mat(3, 3);
	mtx::Matrix mat3(3, 3);
	mtx::Matrix all(pSrc->GetHeight(), pSrc->GetWidth());
	mtx::Matrix blurr(pSrc->GetHeight(), pSrc->GetWidth());
	mtx::Vector V(3);
	mtx::Vector Ve(3);


	for (j = 0; j < pSrc->GetHeight(); j++)
	{
		pSrc->ProgressBar(j, pSrc->GetHeight());	// You can provide progress bar

		for (int i = 0; i < pSrc->GetWidth(); i++)
		{
				field[ko][0] = *pSourceData++;
				field[ko][1] = *pSourceData++;
				field[ko][2] = *pSourceData++;
				
				ko++;
		}
	}


	mtx::Matrix mat23(3, 3);

	for(int q=0; q<3; q++)
	{
		for(int j=0; j<3; j++)
		{
			sum=0.0;
			for(int k=0; k< m-1; k++)
			{
				sum = sum + field[k][q] * field[k][j];
			}
			mat[q][j] = sum;
		}
	}

	sym_eig(mat, mat23, V);

	for(int q=0; q<3; q++)
	{
		for(int j=0; j<3; j++)
		{
			mat3[q][j] = mat23[q][0] * mat23[j][0];
		}
	}

	double suma = 0.0;
	int widths = 0;
	int heights = 0;


	for (int q = 0; q < m-1; q++)
	{
		suma = 0.0;
		for (int j = 0; j < 3; j++)
		{
			sum = 0.0;

			for(int k=0; k< 3; k++)
			{
				sum += field[q][k] * mat3[k][j];							
			}
			suma += sum*sum;
		}
		double Y  =  0.299*field[q][0] + 0.587*field[q][1] + 0.114*field[q][2];
		all[heights][widths] = Y;
		widths++;

		if(widths >= pSrc->GetWidth()){
			heights++;
			widths = 0;
		}
	}

	mtx::Matrix Gu(pSrc->GetHeight(), pSrc->GetWidth());

	double q = 1.0;
	double eu = 2.718;

	int size_of_gu = 50;
	int size_of_half = size_of_gu/2;

	double sigma = 1.1;
    double r, s = 5.0 * sigma * sigma;
 
    double sume = 0.0;

 ///////////////////////////////////////////////////
    ///////////////////////////////////////////////////
    for (int x = -size_of_half; x <= size_of_half; x++)
    {
        for(int y = -size_of_half; y <= size_of_half; y++)
        {
            r = sqrt(x*x + y*y);
            Gu[x + size_of_half][y + size_of_half] = (exp(-(r*r)/s))/(M_PI * s);
            sume += Gu[x + size_of_half][y + size_of_half];
        }
    }
 
    for(int i = 0; i < size_of_gu; ++i)
        for(int j = 0; j < size_of_gu; ++j)
            Gu[i][j] /= sume;
///////////////////////////////////////////////////
///////////////////////////////////////////////////

	for (int y = 0; y < pSrc->GetHeight(); y++){
		for (int x = 0; x < pSrc->GetWidth(); x++){
			suma = 0.0;
			int newX, newY;

			for (int j = 0; j < size_of_gu; j++)
			{	
				for (int i = 0; i < size_of_gu; i++)
				{
					if((x + (i - size_of_gu/2 )) >= 0 && (x+(i - size_of_gu/2 )) < pSrc->GetWidth() &&  (y + (j - size_of_gu/2 )) >= 0 && (y+(j - size_of_gu/2 )) < pSrc->GetHeight() )
					{
						newX = x + (i - size_of_gu/2 );
						newY = y + (j - size_of_gu/2 );
						suma += Gu[j][i] * all[newY][newX]	;
					}
				}
			}
			blurr[y][x] = suma;
		}
	}

	int k = 0;

	for (int i = 0; i < pSrc->GetHeight(); ++i)
	{
		for (int j = 0; j < pSrc->GetWidth(); ++j)
		{
			blurr[i][j] = all[i][j] + field[k][0] - blurr[i][j] + field[k][1] - blurr[i][j] + field[k][2] - blurr[i][j];
			k++;
/*
			if(blurr[i][j] >= 1){
				blurr[i][j]= 1;
			}*/
		}
	}

	for (j = 0; j < pSrc->GetHeight(); j++)
	{

		for (int i = 0; i < pSrc->GetWidth(); i++)
		{
			*pDestinationData++ = blurr[j][i];
			*pDestinationData++ = blurr[j][i];
			*pDestinationData++ = blurr[j][i];
		}
	}

	pSrc->ProgressBar(j, pSrc->GetHeight());
	outfile.close();
	return 0;
}

