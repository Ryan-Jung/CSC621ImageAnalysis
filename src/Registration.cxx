#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkGDCMImageIO.h"
#include "itkDemonsRegistrationFilter.h"
#include "itkHistogramMatchingImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkWarpImageFilter.h"
#include "itkLinearInterpolateImageFunction.h"



int main()
{
	typedef signed short InputPixelType;
	const unsigned int Dimension = 2;
	typedef itk::Image <InputPixelType, Dimension> ImageType;

	typedef itk::ImageFileReader<ImageType> ReaderType;
	ReaderType::Pointer origImage = ReaderType::New();
	ReaderType::Pointer newImage = ReaderType::New();

	char * ctScanOrig = "C:\\Users\\Owner\\Desktop\\ct_scan\\ct_1\\000000.dcm";
	origImage->SetFileName(ctScanOrig);

	char * ctScanNew = "C:\\Users\\Owner\\Desktop\\ct_scan\\ct_2\\000000.dcm";
	newImage->SetFileName(ctScanNew);

	typedef itk::GDCMImageIO ImageIOType;
	ImageIOType::Pointer gdcmImageIO = ImageIOType::New();

	origImage->SetImageIO(gdcmImageIO);
	newImage->SetImageIO(gdcmImageIO);
	try {
		origImage->Update();
		newImage->Update();
	}
	catch (itk::ExceptionObject & e) {
		std::cerr << "Exception in file reader";
		return EXIT_FAILURE;
	}
	typedef float InternalPixelType;
	typedef itk::Image<InternalPixelType, Dimension> InternalImageType;
	typedef itk::CastImageFilter<ImageType, InternalImageType> ImageCast;

	ImageCast::Pointer origImageCast = ImageCast::New();
	ImageCast::Pointer newImageCast = ImageCast::New();
	origImageCast->SetInput(origImage->GetOutput());
	newImageCast->SetInput(newImage->GetOutput());

	

	std::cout << "ITK Hello World test!" << std::endl;
	std::getchar();
	return EXIT_SUCCESS;
}

