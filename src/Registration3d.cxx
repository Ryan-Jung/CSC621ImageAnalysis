#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImageSeriesReader.h"
#include "itkGDCMSeriesFileNames.h"
#include "itkGDCMImageIO.h"
#include "itkDemonsRegistrationFilter.h"
#include "itkHistogramMatchingImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkWarpImageFilter.h"
#include "itkLinearInterpolateImageFunction.h"


/*
	argv[1] fixed images path
	argv[2] moving images path
	argv[3] output image path should have .nii file extension
	argv[4] number of iterations
*/
int main( int argc, char *argv  [] )
{
	
/*
  argc = 4;
	argv[1] = "C:\\Users\\Owner\\Desktop\\ct_scan\\ct_12";
	argv[2] = "C:\\Users\\Owner\\Desktop\\ct_scan\\ct_21";
	argv[3] = "C:\\Users\\Owner\\Desktop\\output3.nii";
	argv[4] = "1";
*/

	if (argc < 4) {
		return EXIT_FAILURE;
	}
	std::cout << "Running demon's registration" << std::endl;
	
	typedef signed short InputPixelType;
	const unsigned int Dimension = 3;
	typedef itk::Image <InputPixelType, Dimension> ImageType;

	typedef itk::ImageSeriesReader< ImageType > ReaderType;
	ReaderType::Pointer fixedReader = ReaderType::New();
	ReaderType::Pointer movingReader = ReaderType::New();

	typedef itk::GDCMImageIO ImageIOType;
	ImageIOType::Pointer gdcmImageIO = ImageIOType::New();

	fixedReader->SetImageIO(gdcmImageIO);
	movingReader->SetImageIO(gdcmImageIO);

	typedef itk::GDCMSeriesFileNames NameGeneratorType;
	NameGeneratorType::Pointer fixedNameGenerator = NameGeneratorType::New();
	NameGeneratorType::Pointer movingNameGenerator = NameGeneratorType::New();

	fixedNameGenerator->SetUseSeriesDetails(true);
	fixedNameGenerator->SetDirectory(argv[1]);

	movingNameGenerator->SetUseSeriesDetails(true);
	movingNameGenerator->SetDirectory(argv[2]);
	
	const std::vector<std::string> & fixedSeriesUID = fixedNameGenerator->GetSeriesUIDs();
	std::string fixedImageSeriesIdentifier = fixedSeriesUID.begin()->c_str();
	std::vector<std::string> fixedImageFiles = fixedNameGenerator->GetFileNames(fixedImageSeriesIdentifier);

	const std::vector<std::string> & movingSeriesUID = movingNameGenerator->GetSeriesUIDs();
	std::string movingImageSeriesIdentifier = movingSeriesUID.begin()->c_str();
	std::vector<std::string> movingImageFiles = movingNameGenerator->GetFileNames(movingImageSeriesIdentifier);

	fixedReader->SetFileNames(fixedImageFiles);
	movingReader->SetFileNames(movingImageFiles);
	
	try {
		fixedReader->Update();
		movingReader->Update();
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
	fixedImageCast->SetInput(fixedReader->GetOutput());
	movingImageCast->SetInput(movingReader->GetOutput());

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

	warper->SetInput(movingReader->GetOutput());
	warper->SetInterpolator(interpolator);
	warper->SetOutputSpacing(fixedReader->GetOutput()->GetSpacing());
	warper->SetOutputOrigin(fixedReader->GetOutput()->GetOrigin());
	warper->SetOutputDirection(fixedReader->GetOutput()->GetDirection());

	warper->SetDisplacementField(demonFilter->GetOutput());

	typedef itk::Image<unsigned short, Dimension > OutputImageType;
	typedef itk::CastImageFilter<ImageType, OutputImageType> OutputCastType;

	OutputCastType::Pointer caster = OutputCastType::New();
	caster->SetInput(warper->GetOutput());
	typedef itk::ImageFileWriter<OutputImageType> FileWriter;
	FileWriter::Pointer fileWriter = FileWriter::New();

	fileWriter->SetFileName(argv[3]);
	fileWriter->SetInput(caster->GetOutput());
	//fileWriter->SetInput(movingReader->GetOutput());
	
	try {
		fileWriter->Update();
	}
	catch (itk::ExceptionObject & e) {
		std::cerr << "Exception writing file" << std::endl;
		std::cout << e.GetDescription() << std::endl;
		
	}

	std::cout << "Finished! Press any key to terminate" << std::endl;
	std::getchar();
	return EXIT_SUCCESS;
}


