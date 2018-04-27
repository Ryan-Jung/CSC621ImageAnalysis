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
	ReaderType::Pointer fixedImage = ReaderType::New();
	ReaderType::Pointer movingImage = ReaderType::New();

	char * ctScanOrig = "C:\\Users\\Owner\\Desktop\\ct_scan\\ct_2\\000002.dcm";
	fixedImage->SetFileName(ctScanOrig);

	char * ctScanNew = "C:\\Users\\Owner\\Desktop\\ct_scan\\ct_2\\000000.dcm";
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

	demonFilter->SetNumberOfIterations(50);
	demonFilter->SetStandardDeviations(15.00);

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
	fileWriter->SetFileName("C:\\Users\\Owner\\Desktop\\OutPut2.dcm");
	fileWriter->SetInput(warper->GetOutput());
	try {
		fileWriter->Update();
	}
	catch (itk::ExceptionObject & e) {
		std::cerr << "Exception writing file";
	}

	std::cout << "ITK Hello World test!" << std::endl;
	std::getchar();
	return EXIT_SUCCESS;
}

