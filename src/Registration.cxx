#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkGDCMImageIO.h"
#include "itkDemonsRegistrationFilter.h"
#include "itkHistogramMatchingImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkWarpImageFilter.h"
#include "itkLinearInterpolateImageFunction.h"


/*
	argv[1] fixed image path
	argv[2] moving image path
	argv[3] output image path
	argv[4] number of iterations
*/
int main( int argc, char *argv  [] )
{	
	if (argc < 4) {
		return EXIT_FAILURE;
	}
	std::cout << "Running demon's registration" << std::endl;
	
	typedef signed short InputPixelType;
	const unsigned int Dimension = 2;
	typedef itk::Image <InputPixelType, Dimension> ImageType;

	typedef itk::ImageFileReader<ImageType> ReaderType;
	ReaderType::Pointer fixedImage = ReaderType::New();
	ReaderType::Pointer movingImage = ReaderType::New();

	//char * ctScanOrig = "C:\\Users\\Owner\\Desktop\\ct_scan\\ct_1\\000000.dcm";
	char * ctScanOrig = argv[1];
	fixedImage->SetFileName(ctScanOrig);

	//char * ctScanNew = "C:\\Users\\Owner\\Desktop\\ct_scan\\ct_2\\000024.dcm";
	char * ctScanNew = argv[2];
	movingImage->SetFileName(ctScanNew);

	typedef itk::GDCMImageIO ImageIOType;
	ImageIOType::Pointer gdcmImageIO = ImageIOType::New();


	fixedImage->SetImageIO(gdcmImageIO);
	movingImage->SetImageIO(gdcmImageIO);
	try {
		fixedImage->Update();
		movingImage->Update();
	}
	catch (itk::ExceptionObject & e) {
		std::cerr << "Exception in file reader";
		return EXIT_FAILURE;
	}
	typedef float InternalPixelType;
	typedef itk::Image<InternalPixelType, Dimension> InternalImageType;
	typedef itk::CastImageFilter<ImageType, InternalImageType> ImageCast;

	ImageCast::Pointer fixedImageCast = ImageCast::New();
	ImageCast::Pointer movingImageCast = ImageCast::New();
	fixedImageCast->SetInput(fixedImage->GetOutput());
	movingImageCast->SetInput(movingImage->GetOutput());

	typedef itk::HistogramMatchingImageFilter<InternalImageType, InternalImageType> HistMatcher;
	HistMatcher::Pointer matcher = HistMatcher::New();

	matcher->SetInput(movingImageCast->GetOutput());
	matcher->SetReferenceImage(fixedImageCast->GetOutput());
	matcher->SetNumberOfHistogramLevels(1024);
	matcher->SetNumberOfMatchPoints(7);
	matcher->ThresholdAtMeanIntensityOn();

	typedef itk::Vector<float, Dimension> VectorPixelType;
	typedef itk::Image<VectorPixelType, Dimension> DisplacementFieldType;
	typedef itk::DemonsRegistrationFilter<InternalImageType, InternalImageType, DisplacementFieldType> DemonFilter;

	DemonFilter::Pointer demonFilter = DemonFilter::New();

	demonFilter->SetFixedImage(fixedImageCast->GetOutput());
	demonFilter->SetMovingImage(matcher->GetOutput());

	//demonFilter->SetNumberOfIterations(100);
	demonFilter->SetNumberOfIterations(atoi(argv[4]));
	demonFilter->SetStandardDeviations(1);

	demonFilter->Update();

	typedef itk::WarpImageFilter < ImageType, ImageType, DisplacementFieldType> WarpType;
	typedef itk::LinearInterpolateImageFunction<ImageType, double> LinearInterpolator;

	WarpType::Pointer warper = WarpType::New();
	LinearInterpolator::Pointer interpolator = LinearInterpolator::New();

	warper->SetInput(movingImage->GetOutput());
	warper->SetInterpolator(interpolator);
	warper->SetOutputSpacing(fixedImage->GetOutput()->GetSpacing());
	warper->SetOutputOrigin(fixedImage->GetOutput()->GetOrigin());
	warper->SetOutputDirection(fixedImage->GetOutput()->GetDirection());

	warper->SetDisplacementField(demonFilter->GetOutput());

	
	typedef itk::ImageFileWriter<ImageType> FileWriter;
	FileWriter::Pointer fileWriter = FileWriter::New();
	//fileWriter->SetFileName("C:\\Users\\Owner\\Desktop\\OutPut.dcm");
	fileWriter->SetFileName(argv[3]);
	fileWriter->SetInput(warper->GetOutput());
	try {
		fileWriter->Update();
	}
	catch (itk::ExceptionObject & e) {
		std::cerr << "Exception writing file";
	}

	std::cout << "Finished! Press any key to terminate" << std::endl;
	std::getchar();
	return EXIT_SUCCESS;
}

