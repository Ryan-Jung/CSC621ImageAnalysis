
#include <iostream>

// Software Guide : BeginCodeSnippet
#include "itkVectorGradientAnisotropicDiffusionImageFilter.h"
#include "itkVectorGradientMagnitudeImageFilter.h"
#include "itkWatershedImageFilter.h"
#include "itkUnaryFunctorImageFilter.h"
#include "itkGradientMagnitudeRecursiveGaussianImageFilter.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkVectorCastImageFilter.h"
#include "itkScalarToRGBPixelFunctor.h"
// Software Guide : EndCodeSnippet

int main(int argc, char *argv[])
{

	const unsigned int InputDimension = 3;
	typedef float InternalPixelType;
	typedef itk::Image< InternalPixelType, InputDimension > InternalImageType;
	typedef itk::ImageFileReader< InternalImageType > ReaderType;
	ReaderType::Pointer reader = ReaderType::New();
	try
	{
		reader->SetFileName(argv[1]);
	}
	catch (...)
	{
		return EXIT_FAILURE;
	}
	reader->Update();

	const unsigned int OutputDimension = 3;
	typedef itk::RGBPixel<unsigned char> RGBPixelType;
	typedef itk::Image< RGBPixelType, OutputDimension > RGBImageType;
	typedef  itk::ImageFileWriter< RGBImageType > WriterType;
	WriterType::Pointer writer = WriterType::New();
	writer->SetFileName(argv[2]);

	typedef   itk::GradientMagnitudeRecursiveGaussianImageFilter<
		InternalImageType,
		InternalImageType
	> GradientMagnitudeFilterType;

	GradientMagnitudeFilterType::Pointer gradienMagnitudeFilter = GradientMagnitudeFilterType::New();
	gradienMagnitudeFilter->SetInput(reader->GetOutput());
	gradienMagnitudeFilter->SetSigma(1.0);

	typedef  itk::WatershedImageFilter< InternalImageType > WatershedFilterType;
	WatershedFilterType::Pointer watershedFilter = WatershedFilterType::New(); 
	watershedFilter->SetInput(gradienMagnitudeFilter->GetOutput());
	watershedFilter->SetThreshold(atof(argv[3]));
	watershedFilter->SetLevel(atof(argv[4]));

	typedef itk::Functor::ScalarToRGBPixelFunctor< unsigned long > ColorMapFunctorType;
	typedef WatershedFilterType::OutputImageType  LabeledImageType;
	typedef itk::UnaryFunctorImageFilter<
		LabeledImageType,
		RGBImageType,
		ColorMapFunctorType
	> ColorMapFilterType;

	ColorMapFilterType::Pointer colorMapFilter = ColorMapFilterType::New();
	colorMapFilter->SetInput(watershedFilter->GetOutput());

	writer->SetInput(colorMapFilter->GetOutput());

	try
	{
		writer->Update();
	}
	catch (itk::ExceptionObject & excep)
	{
		//cerr << "Exception !" << endl;
		//cerr << excep.GetDescription() << endl;
		return EXIT_FAILURE;
	}


	return EXIT_SUCCESS;
}